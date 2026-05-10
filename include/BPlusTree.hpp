#pragma once

#include <concepts>
#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>
#include <functional>
#include <cstddef>

namespace Tree
{
	namespace Inner_BPlusTree
	{
		// Key 概念约束
		template<typename T>
		concept Comparable = requires(T a, T b)
		{
			{ a <=> b } -> std::convertible_to<std::partial_ordering>;
			{ a == b } -> std::convertible_to<bool>;
		};

		// 节点类型标签
		enum class NodeType : uint8_t
		{
			Inner,
			Leaf
		};

		// 统一节点类 - 通过标签区分类型
		template<Comparable Key, typename Value>
		class BPlusTreeNode
		{
		public:
			// 使用 pair<Key, Value> 而不是 pair<const Key, Value>
			using stored_value_type = std::pair<Key, Value>;

		private:
			NodeType type_;
			BPlusTreeNode* parent_ = nullptr;

			// 使用 union 存储不同类型的数据
			union Data
			{
				struct
				{
					std::vector<Key> keys_;
					std::vector<BPlusTreeNode*> children_;
				} inner_;

				struct
				{
					std::vector<stored_value_type> data_;
					BPlusTreeNode* next_ = nullptr;
					BPlusTreeNode* prev_ = nullptr;
				} leaf_;

				// 构造函数/析构函数
				Data() noexcept {}
				~Data() noexcept {}
			} data_;

		public:
			// 构造函数
			explicit BPlusTreeNode(NodeType type) noexcept : type_(type)
			{
				if (type_ == NodeType::Inner)
				{
					new (&data_.inner_) decltype(data_.inner_){};
				}
				else
				{
					new (&data_.leaf_) decltype(data_.leaf_){};
				}
			}

			// 析构函数
			~BPlusTreeNode() noexcept
			{
				if (type_ == NodeType::Inner)
				{
					data_.inner_.~decltype(data_.inner_)();
				}
				else
				{
					data_.leaf_.~decltype(data_.leaf_)();
				}
			}

			// 禁止复制
			BPlusTreeNode(const BPlusTreeNode&) = delete;
			BPlusTreeNode& operator=(const BPlusTreeNode&) = delete;

			// 允许移动
			BPlusTreeNode(BPlusTreeNode&& other) noexcept
				: type_(other.type_), parent_(other.parent_)
			{
				if (type_ == NodeType::Inner)
				{
					new (&data_.inner_) decltype(data_.inner_)(std::move(other.data_.inner_));
				}
				else
				{
					new (&data_.leaf_) decltype(data_.leaf_)(std::move(other.data_.leaf_));
				}
			}

			BPlusTreeNode& operator=(BPlusTreeNode&& other) noexcept
			{
				if (this != &other)
				{
					this->~BPlusTreeNode();
					new (this) BPlusTreeNode(std::move(other));
				}
				return *this;
			}

			// 类型判断
			bool is_leaf() const noexcept { return type_ == NodeType::Leaf; }
			NodeType get_type() const noexcept { return type_; }

			// 父指针访问
			BPlusTreeNode* parent() const noexcept { return parent_; }
			void set_parent(BPlusTreeNode* parent) noexcept { parent_ = parent; }

			// 内部节点数据访问
			std::vector<Key>& inner_keys() noexcept
			{
				assert(type_ == NodeType::Inner);
				return data_.inner_.keys_;
			}

			const std::vector<Key>& inner_keys() const noexcept
			{
				assert(type_ == NodeType::Inner);
				return data_.inner_.keys_;
			}

			std::vector<BPlusTreeNode*>& inner_children() noexcept
			{
				assert(type_ == NodeType::Inner);
				return data_.inner_.children_;
			}

			const std::vector<BPlusTreeNode*>& inner_children() const noexcept
			{
				assert(type_ == NodeType::Inner);
				return data_.inner_.children_;
			}

			// 叶节点数据访问
			std::vector<stored_value_type>& leaf_data() noexcept
			{
				assert(type_ == NodeType::Leaf);
				return data_.leaf_.data_;
			}

			const std::vector<stored_value_type>& leaf_data() const noexcept
			{
				assert(type_ == NodeType::Leaf);
				return data_.leaf_.data_;
			}

			BPlusTreeNode* leaf_next() const noexcept
			{
				assert(type_ == NodeType::Leaf);
				return data_.leaf_.next_;
			}

			void set_leaf_next(BPlusTreeNode* next) noexcept
			{
				assert(type_ == NodeType::Leaf);
				data_.leaf_.next_ = next;
			}

			BPlusTreeNode* leaf_prev() const noexcept
			{
				assert(type_ == NodeType::Leaf);
				return data_.leaf_.prev_;
			}

			void set_leaf_prev(BPlusTreeNode* prev) noexcept
			{
				assert(type_ == NodeType::Leaf);
				data_.leaf_.prev_ = prev;
			}

			// 内存使用信息
			size_t memory_usage() const noexcept
			{
				size_t usage = sizeof(*this);
				if (type_ == NodeType::Inner)
				{
					usage += data_.inner_.keys_.capacity() * sizeof(Key);
					usage += data_.inner_.children_.capacity() * sizeof(BPlusTreeNode*);
				}
				else
				{
					usage += data_.leaf_.data_.capacity() * sizeof(stored_value_type);
				}
				return usage;
			}
		};
	}

	// B+树主类
	template<Inner_BPlusTree::Comparable Key,
		typename Value,
		typename Allocator = std::allocator<std::pair<const Key, Value>>>
	class BPlusTree
	{
	public:
		
		using value_type = std::pair<const Key, Value>;
		using stored_value_type = std::pair<Key, Value>;
		using node_type = Inner_BPlusTree::BPlusTreeNode<Key, Value>;
		using node_pointer = node_type*;
		using allocator_type = Allocator;
		using NodeType = Inner_BPlusTree::NodeType;

	private:
		node_pointer m_root = nullptr;           // 根节点
		size_t m_size = 0;                        // 元素数量
		size_t m_order;                           // B+树的阶数
		node_pointer m_leftmost_leaf = nullptr;  // 最左侧叶节点
		node_pointer m_rightmost_leaf = nullptr;  // 最右侧叶节点
		allocator_type m_allocator;               // 分配器

		// 节点分配器
		using node_allocator_type = typename std::allocator_traits<allocator_type>::
			template rebind_alloc<node_type>;
		node_allocator_type m_node_allocator;

	public:
		// 构造函数
		explicit BPlusTree(const allocator_type& alloc = allocator_type(),
			size_t order = 3) noexcept
			: m_order(std::max(order, static_cast<size_t>(3)))
			, m_allocator(alloc)
			, m_node_allocator(alloc)
		{}

		// 析构函数
		~BPlusTree() noexcept
		{
			clear();
		}

		// 禁止复制
		BPlusTree(const BPlusTree&) = delete;
		BPlusTree& operator=(const BPlusTree&) = delete;

		// 移动构造函数
		BPlusTree(BPlusTree&& other) noexcept
			: m_root(other.m_root)
			, m_size(other.m_size)
			, m_order(other.m_order)
			, m_leftmost_leaf(other.m_leftmost_leaf)
			, m_rightmost_leaf(other.m_rightmost_leaf)
			, m_allocator(std::move(other.m_allocator))
			, m_node_allocator(std::move(other.m_node_allocator))
		{
			other.m_root = nullptr;
			other.m_size = 0;
			other.m_order = 0;
			other.m_leftmost_leaf = nullptr;
			other.m_rightmost_leaf = nullptr;
		}

		// 移动赋值运算符
		BPlusTree& operator=(BPlusTree&& other) noexcept
		{
			if (this != &other)
			{
				clear();

				m_root = other.m_root;
				m_size = other.m_size;
				m_order = other.m_order;
				m_leftmost_leaf = other.m_leftmost_leaf;
				m_rightmost_leaf = other.m_rightmost_leaf;
				m_allocator = std::move(other.m_allocator);
				m_node_allocator = std::move(other.m_node_allocator);

				other.m_root = nullptr;
				other.m_size = 0;
				other.m_order = 0;
				other.m_leftmost_leaf = nullptr;
				other.m_rightmost_leaf = nullptr;
			}
			return *this;
		}

		// 获取根节点指针
		node_pointer root() noexcept { return m_root; }
		const node_type* root() const noexcept { return m_root; }

		// 树是否为空
		bool empty() const noexcept { return m_size == 0; }

		// 获取树的节点总数
		size_t size() const noexcept { return m_size; }

		// 获取分配器
		allocator_type get_allocator() const noexcept { return m_allocator; }

		// 插入键值对
		void insert(const Key& key, const Value& value)
		{
			if (empty())
			{
				// 创建第一个叶节点
				node_pointer leaf = create_node(NodeType::Leaf);
				leaf->leaf_data().push_back({ key, value });
				m_root = leaf;
				m_size = 1;
				m_leftmost_leaf = leaf;
				m_rightmost_leaf = leaf;
				return;
			}

			// 查找插入位置
			node_pointer leaf = find_leaf(key);
			if (!leaf) return;

			// 插入到叶节点
			auto& data = leaf->leaf_data();
			auto it = std::lower_bound(data.begin(), data.end(), key,
				[](const stored_value_type& a, const Key& b) { return a.first < b; });

			// 如果键已存在，更新值
			if (it != data.end() && it->first == key)
			{
				it->second = value;
				return;
			}

			data.insert(it, { key, value });
			m_size++;

			// 如果叶节点溢出，需要分裂
			if (data.size() > m_order - 1)
			{
				split_leaf(leaf);
			}
		}

		// 查找键对应的值
		Value* find(const Key& key) noexcept
		{
			if (empty()) return nullptr;

			node_pointer leaf = find_leaf(key);
			if (!leaf) return nullptr;

			auto& data = leaf->leaf_data();
			auto it = std::lower_bound(data.begin(), data.end(), key,
				[](const stored_value_type& a, const Key& b) { return a.first < b; });

			if (it != data.end() && it->first == key)
			{
				return &it->second;
			}
			return nullptr;
		}

		const Value* find(const Key& key) const noexcept
		{
			return const_cast<BPlusTree*>(this)->find(key);
		}

		// 删除键
		bool erase(const Key& key) noexcept
		{
			if (empty()) return false;

			node_pointer leaf = find_leaf(key);
			if (!leaf) return false;

			auto& data = leaf->leaf_data();
			auto it = std::lower_bound(data.begin(), data.end(), key,
				[](const stored_value_type& a, const Key& b) { return a.first < b; });

			if (it == data.end() || it->first != key)
			{
				return false;
			}

			bool is_first = (it == data.begin());
			Key deleted_key = it->first;

			data.erase(it);
			m_size--;

			// 如果删除后叶节点为空
			if (data.empty())
			{
				remove_empty_leaf(leaf);
				return true;
			}

			// 如果删除的是第一个键，需要更新父节点的分隔键
			if (is_first && leaf != m_root)
			{
				update_all_parent_keys(leaf, deleted_key, data.front().first);
			}

			// 检查叶节点是否下溢
			if (leaf != m_root && data.size() < (m_order - 1) / 2)
			{
				handle_leaf_underflow(leaf);
			}

			return true;
		}

		// 清空树
		void clear() noexcept
		{
			if (m_root)
			{
				clear_recursive(m_root);
				m_root = nullptr;
				m_size = 0;
				m_leftmost_leaf = nullptr;
				m_rightmost_leaf = nullptr;
			}
		}

		// 获取叶节点链表
		node_pointer get_leftmost_leaf() const noexcept { return m_leftmost_leaf; }
		node_pointer get_rightmost_leaf() const noexcept { return m_rightmost_leaf; }

		// 内存使用统计
		size_t memory_usage() const noexcept
		{
			size_t total = sizeof(*this);
			if (m_root)
			{
				total += memory_usage_recursive(m_root);
			}
			return total;
		}

	private:
		// 创建节点
		node_pointer create_node(NodeType type)
		{
			node_pointer p = std::allocator_traits<node_allocator_type>::allocate(m_node_allocator, 1);
			try
			{
				std::allocator_traits<node_allocator_type>::construct(m_node_allocator, p, type);
				return p;
			}
			catch (...)
			{
				std::allocator_traits<node_allocator_type>::deallocate(m_node_allocator, p, 1);
				throw;
			}
		}

		// 销毁节点
		void destroy_node(node_pointer p) noexcept
		{
			if (p)
			{
				std::allocator_traits<node_allocator_type>::destroy(m_node_allocator, p);
				std::allocator_traits<node_allocator_type>::deallocate(m_node_allocator, p, 1);
			}
		}

		// 查找键应该所在的叶节点
		node_pointer find_leaf(const Key& key) const noexcept
		{
			if (!m_root) return nullptr;

			node_pointer current = m_root;
			while (!current->is_leaf())
			{
				auto& keys = current->inner_keys();
				auto& children = current->inner_children();

				auto it = std::lower_bound(keys.begin(), keys.end(), key);
				size_t index = it - keys.begin();
				// 关键修复：如果找到的键等于目标键，应进入右子树 (index + 1)
				if (it != keys.end() && *it == key)
				{
					++index;
				}
				current = children[index];
			}
			return current;
		}

		// 分裂叶节点
		void split_leaf(node_pointer leaf)
		{
			size_t split_point = leaf->leaf_data().size() / 2;
			Key split_key = leaf->leaf_data()[split_point].first;

			// 创建新叶节点
			node_pointer new_leaf = create_node(NodeType::Leaf);

			// 移动后半部分数据到新节点
			auto& leaf_data = leaf->leaf_data();
			auto& new_leaf_data = new_leaf->leaf_data();

			new_leaf_data.assign(std::make_move_iterator(leaf_data.begin() + split_point),
				std::make_move_iterator(leaf_data.end()));
			leaf_data.erase(leaf_data.begin() + split_point, leaf_data.end());

			// 更新叶节点链表
			new_leaf->set_leaf_next(leaf->leaf_next());
			new_leaf->set_leaf_prev(leaf);
			if (leaf->leaf_next())
			{
				leaf->leaf_next()->set_leaf_prev(new_leaf);
			}
			leaf->set_leaf_next(new_leaf);
			if (leaf == m_rightmost_leaf)
			{
				m_rightmost_leaf = new_leaf;
			}

			// 将分裂键插入父节点
			insert_into_parent(leaf, split_key, new_leaf);
		}

		// 将分裂键插入父节点
		void insert_into_parent(node_pointer left_child, const Key& key, node_pointer right_child)
		{
			if (left_child == m_root)
			{
				// 创建新的根节点
				node_pointer new_root = create_node(NodeType::Inner);
				auto& keys = new_root->inner_keys();
				auto& children = new_root->inner_children();

				keys.push_back(key);
				children.push_back(left_child);
				children.push_back(right_child);

				m_root = new_root;
				left_child->set_parent(new_root);
				right_child->set_parent(new_root);
				return;
			}

			node_pointer parent = left_child->parent();
			auto& keys = parent->inner_keys();
			auto& children = parent->inner_children();

			// 在父节点中查找插入位置
			auto it = std::lower_bound(keys.begin(), keys.end(), key);
			size_t index = it - keys.begin();

			keys.insert(it, key);
			children.insert(children.begin() + index + 1, right_child);
			right_child->set_parent(parent);

			// 如果父节点溢出，继续分裂
			if (keys.size() > m_order - 1)
			{
				split_inner_node(parent);
			}
		}

		// 分裂内部节点
		void split_inner_node(node_pointer inner)
		{
			auto& keys = inner->inner_keys();
			size_t split_point = keys.size() / 2;
			Key split_key = keys[split_point];

			// 创建新内部节点
			node_pointer new_inner = create_node(NodeType::Inner);
			auto& new_keys = new_inner->inner_keys();
			auto& new_children = new_inner->inner_children();
			auto& children = inner->inner_children();

			// 移动键和子节点
			new_keys.assign(std::make_move_iterator(keys.begin() + split_point + 1),
				std::make_move_iterator(keys.end()));
			keys.erase(keys.begin() + split_point, keys.end());

			new_children.assign(std::make_move_iterator(children.begin() + split_point + 1),
				std::make_move_iterator(children.end()));
			children.erase(children.begin() + split_point + 1, children.end());

			// 更新子节点的父指针
			for (auto child : new_children)
			{
				child->set_parent(new_inner);
			}

			// 将分裂键插入父节点
			insert_into_parent(inner, split_key, new_inner);
		}

		// 获取节点中的最小键
		Key get_min_key(node_pointer node) const noexcept
		{
			if (node->is_leaf())
			{
				return node->leaf_data().front().first;
			}
			else
			{
				return get_min_key(node->inner_children().front());
			}
		}

		// 完整更新父节点键
		void update_all_parent_keys(node_pointer node, const Key& old_key, const Key& new_key) noexcept
		{
			node_pointer current = node;
			node_pointer parent = node->parent();

			while (parent)
			{
				auto& keys = parent->inner_keys();
				auto& children = parent->inner_children();

				// 找到子节点在父节点中的索引
				size_t idx = 0;
				for (; idx < children.size(); ++idx)
				{
					if (children[idx] == current)
					{
						break;
					}
				}

				if (idx > 0 && idx - 1 < keys.size())
				{
					if (keys[idx - 1] == old_key)
					{
						keys[idx - 1] = new_key;
					}
					else
					{
						// 如果键不匹配，需要找到正确的键更新
						for (size_t i = 0; i < keys.size(); ++i)
						{
							if (keys[i] == old_key)
							{
								keys[i] = new_key;
								break;
							}
						}
					}
				}

				// 继续向上更新
				current = parent;
				parent = parent->parent();
			}
		}

		// 从父节点中删除子节点
		void remove_child_from_parent(node_pointer parent, node_pointer child) noexcept
		{
			auto& children = parent->inner_children();
			auto& keys = parent->inner_keys();

			// 找到子节点位置
			size_t idx = 0;
			for (; idx < children.size(); ++idx)
			{
				if (children[idx] == child)
				{
					break;
				}
			}

			if (idx >= children.size()) return; // 不应该发生

			// 删除子节点
			children.erase(children.begin() + idx);

			// 删除对应的键
			if (idx > 0)
			{
				keys.erase(keys.begin() + idx - 1);
			}
			else if (!keys.empty())
			{
				// 删除第一个子节点，需要删除第一个键
				keys.erase(keys.begin());
			}

			// 如果父节点是根节点且下溢
			if (parent == m_root)
			{
				if (children.size() == 1)
				{
					// 根节点只有一个子节点，降低树高
					m_root = children[0];
					m_root->set_parent(nullptr);
					destroy_node(parent);
				}
				else if (keys.size() < (m_order - 1) / 2)
				{
					// 根节点允许少于最小键数
					return;
				}
			}
			else if (keys.size() < (m_order - 1) / 2)
			{
				// 内部节点下溢
				handle_inner_node_underflow(parent);
			}
		}

		// 移除空叶节点
		void remove_empty_leaf(node_pointer leaf) noexcept
		{
			// 更新链表
			if (leaf->leaf_prev())
			{
				leaf->leaf_prev()->set_leaf_next(leaf->leaf_next());
			}
			if (leaf->leaf_next())
			{
				leaf->leaf_next()->set_leaf_prev(leaf->leaf_prev());
			}

			// 更新最左/最右叶节点指针
			if (leaf == m_leftmost_leaf)
			{
				m_leftmost_leaf = leaf->leaf_next();
			}
			if (leaf == m_rightmost_leaf)
			{
				m_rightmost_leaf = leaf->leaf_prev();
			}

			// 从父节点中删除
			node_pointer parent = leaf->parent();
			if (parent)
			{
				remove_child_from_parent(parent, leaf);
			}
			else
			{
				// 叶节点是根节点
				m_root = nullptr;
				m_leftmost_leaf = nullptr;
				m_rightmost_leaf = nullptr;
			}

			destroy_node(leaf);
		}

		// 处理叶节点下溢
		bool handle_leaf_underflow(node_pointer leaf) noexcept
		{
			node_pointer parent = leaf->parent();
			if (!parent) return false;

			auto& parent_children = parent->inner_children();
			auto& parent_keys = parent->inner_keys();

			// 找到叶节点在父节点中的位置
			size_t leaf_idx = 0;
			for (; leaf_idx < parent_children.size(); ++leaf_idx)
			{
				if (parent_children[leaf_idx] == leaf)
				{
					break;
				}
			}

			if (leaf_idx >= parent_children.size()) return false;

			auto& leaf_data = leaf->leaf_data();
			size_t min_keys = (m_order - 1) / 2;

			// 尝试从左兄弟借用
			if (leaf_idx > 0)
			{
				node_pointer left_sibling = parent_children[leaf_idx - 1];
				auto& left_data = left_sibling->leaf_data();

				if (left_data.size() > min_keys)
				{
					// 从左兄弟借用最后一个元素
					leaf_data.insert(leaf_data.begin(), left_data.back());
					left_data.pop_back();

					// 更新父节点键
					if (leaf_idx - 1 < parent_keys.size())
					{
						parent_keys[leaf_idx - 1] = leaf_data.front().first;
					}
					return true;
				}
			}

			// 尝试从右兄弟借用
			if (leaf_idx < parent_children.size() - 1)
			{
				node_pointer right_sibling = parent_children[leaf_idx + 1];
				auto& right_data = right_sibling->leaf_data();

				if (right_data.size() > min_keys)
				{
					// 从右兄弟借用第一个元素
					leaf_data.push_back(right_data.front());
					right_data.erase(right_data.begin());

					// 更新父节点键
					if (leaf_idx < parent_keys.size())
					{
						parent_keys[leaf_idx] = leaf_data.back().first;
					}
					if (leaf_idx + 1 < parent_keys.size())
					{
						parent_keys[leaf_idx + 1] = right_data.front().first;
					}
					return true;
				}
			}

			// 无法借用，需要合并
			if (leaf_idx > 0)
			{
				// 与左兄弟合并
				return merge_with_left_sibling(leaf, leaf_idx);
			}
			else
			{
				// 与右兄弟合并
				return merge_with_right_sibling(leaf, leaf_idx);
			}
		}

		// 与左兄弟合并
		bool merge_with_left_sibling(node_pointer leaf, size_t leaf_idx) noexcept
		{
			node_pointer parent = leaf->parent();
			if (!parent) return false;

			node_pointer left_sibling = parent->inner_children()[leaf_idx - 1];
			auto& left_data = left_sibling->leaf_data();
			auto& leaf_data = leaf->leaf_data();

			// 移动所有元素到左兄弟
			left_data.insert(left_data.end(),
				std::make_move_iterator(leaf_data.begin()),
				std::make_move_iterator(leaf_data.end()));

			// 更新链表
			left_sibling->set_leaf_next(leaf->leaf_next());
			if (leaf->leaf_next())
			{
				leaf->leaf_next()->set_leaf_prev(left_sibling);
			}
			if (leaf == m_rightmost_leaf)
			{
				m_rightmost_leaf = left_sibling;
			}

			// 从父节点中删除叶节点
			remove_child_from_parent(parent, leaf);

			// 销毁叶节点
			destroy_node(leaf);

			return true;
		}

		// 与右兄弟合并
		bool merge_with_right_sibling(node_pointer leaf, size_t leaf_idx) noexcept
		{
			node_pointer parent = leaf->parent();
			if (!parent) return false;

			node_pointer right_sibling = parent->inner_children()[leaf_idx + 1];
			auto& right_data = right_sibling->leaf_data();
			auto& leaf_data = leaf->leaf_data();

			// 移动所有元素到当前叶节点
			leaf_data.insert(leaf_data.end(),
				std::make_move_iterator(right_data.begin()),
				std::make_move_iterator(right_data.end()));

			// 更新链表
			leaf->set_leaf_next(right_sibling->leaf_next());
			if (right_sibling->leaf_next())
			{
				right_sibling->leaf_next()->set_leaf_prev(leaf);
			}
			if (right_sibling == m_rightmost_leaf)
			{
				m_rightmost_leaf = leaf;
			}

			// 从父节点中删除右兄弟
			remove_child_from_parent(parent, right_sibling);

			// 销毁右兄弟
			destroy_node(right_sibling);

			return true;
		}

		// 处理内部节点下溢
		void handle_inner_node_underflow(node_pointer inner)
		{
			node_pointer parent = inner->parent();
			if (!parent) return;

			auto& parent_children = parent->inner_children();
			auto& parent_keys = parent->inner_keys();
			auto& inner_keys = inner->inner_keys();
			auto& inner_children = inner->inner_children();

			// 找到内部节点在父节点中的位置
			size_t index = 0;
			for (; index < parent_children.size(); ++index)
			{
				if (parent_children[index] == inner)
				{
					break;
				}
			}

			// 尝试从左兄弟节点借用
			if (index > 0)
			{
				node_pointer left_sibling = parent_children[index - 1];
				auto& left_keys = left_sibling->inner_keys();
				auto& left_children = left_sibling->inner_children();

				if (left_keys.size() > (m_order - 1) / 2)
				{
					// 从左兄弟节点借用一个键
					Key parent_key = parent_keys[index - 1];

					// 从父节点借一个键，父节点从左兄弟拿最大键
					inner_keys.insert(inner_keys.begin(), parent_key);
					parent_keys[index - 1] = left_keys.back();
					left_keys.pop_back();

					// 移动左兄弟的最后一个子节点
					if (!left_children.empty())
					{
						node_pointer child = left_children.back();
						inner_children.insert(inner_children.begin(), child);
						child->set_parent(inner);
						left_children.pop_back();
					}
					return;
				}
			}

			// 尝试从右兄弟节点借用
			if (index < parent_children.size() - 1)
			{
				node_pointer right_sibling = parent_children[index + 1];
				auto& right_keys = right_sibling->inner_keys();
				auto& right_children = right_sibling->inner_children();

				if (right_keys.size() > (m_order - 1) / 2)
				{
					// 从右兄弟节点借用一个键
					Key parent_key = parent_keys[index];

					// 从父节点借一个键，父节点从右兄弟拿最小键
					inner_keys.push_back(parent_key);
					parent_keys[index] = right_keys.front();
					right_keys.erase(right_keys.begin());

					// 移动右兄弟的第一个子节点
					if (!right_children.empty())
					{
						node_pointer child = right_children.front();
						inner_children.push_back(child);
						child->set_parent(inner);
						right_children.erase(right_children.begin());
					}
					return;
				}
			}

			// 无法借用，需要合并
			if (index > 0)
			{
				// 与左兄弟合并
				node_pointer left_sibling = parent_children[index - 1];
				auto& left_keys = left_sibling->inner_keys();
				auto& left_children = left_sibling->inner_children();

				// 添加父节点的分隔键
				left_keys.push_back(parent_keys[index - 1]);

				// 添加当前节点的键
				left_keys.insert(left_keys.end(),
					std::make_move_iterator(inner_keys.begin()),
					std::make_move_iterator(inner_keys.end()));

				// 添加当前节点的子节点
				for (auto child : inner_children)
				{
					child->set_parent(left_sibling);
					left_children.push_back(child);
				}

				// 从父节点中删除当前节点
				remove_child_from_parent(parent, inner);
				destroy_node(inner);
			}
			else
			{
				// 与右兄弟合并
				node_pointer right_sibling = parent_children[index + 1];
				auto& right_keys = right_sibling->inner_keys();
				auto& right_children = right_sibling->inner_children();

				// 添加父节点的分隔键
				inner_keys.push_back(parent_keys[index]);

				// 添加右兄弟的键
				inner_keys.insert(inner_keys.end(),
					std::make_move_iterator(right_keys.begin()),
					std::make_move_iterator(right_keys.end()));

				// 添加右兄弟的子节点
				for (auto child : right_children)
				{
					child->set_parent(inner);
					inner_children.push_back(child);
				}

				// 从父节点中删除右兄弟节点
				remove_child_from_parent(parent, right_sibling);
				destroy_node(right_sibling);
			}
		}

		

		// 递归清除所有节点
		void clear_recursive(node_pointer node) noexcept
		{
			if (!node->is_leaf())
			{
				auto& children = node->inner_children();
				for (auto child : children)
				{
					clear_recursive(child);
				}
			}
			destroy_node(node);
		}

		// 递归计算内存使用
		size_t memory_usage_recursive(node_pointer node) const noexcept
		{
			size_t usage = node->memory_usage();

			if (!node->is_leaf())
			{
				auto& children = node->inner_children();
				for (auto child : children)
				{
					usage += memory_usage_recursive(child);
				}
			}

			return usage;
		}
	};
}