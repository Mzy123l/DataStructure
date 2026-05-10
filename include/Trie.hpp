#pragma once
#include "Tree_MultiChildRepresentation.hpp"
#include <unordered_map>
#include <string>
#include <utility>
#include <stack>
#include <queue>

namespace Tree
{
	/*
	*  @brief	前缀树/字典树
	*  @brief	支持大小写字母+数字
	*/
	template <typename charT, typename Allocator = std::allocator<charT>>
	class Trie : public Tree_MultiChildRepresentation<
		charT,
		Trie<charT, Allocator>,
		// 从 charT 分配器 rebind 到节点分配器
		typename std::allocator_traits<Allocator>::template rebind_alloc<
		TreeNode::Node_MultiChildRepresentation<charT>
		>
	>
	{
	private:
		// 基类类型定义
		using Base = Tree_MultiChildRepresentation<
			charT,
			Trie<charT, Allocator>,
			typename std::allocator_traits<Allocator>::template rebind_alloc<
			TreeNode::Node_MultiChildRepresentation<charT>
			>
		>;
		using Node = typename Base::Node;
		using NodePtr = typename Base::NodePtr;
		using const_NodePtr = typename Base::const_NodePtr;

		// 从 charT 分配器 rebind 到映射表元素分配器
		using MapAlloc = typename std::allocator_traits<Allocator>::template rebind_alloc<
			std::pair<const NodePtr, bool>
		>;

		// 使用自定义分配器的 unordered_map
		using WordEndMap = std::unordered_map<
			NodePtr,
			bool,
			std::hash<NodePtr>,
			std::equal_to<NodePtr>,
			MapAlloc
		>;

		static constexpr size_t ALPHABET_SIZE = 62;  // 0-9, A-Z, a-z

		// 字符到索引的映射
		static constexpr size_t char_to_index(charT ch) noexcept
		{
			if (ch >= '0' && ch <= '9')
				return ch - '0';                     // 0-9: 0-9
			else if (ch >= 'A' && ch <= 'Z')
				return 10 + (ch - 'A');              // A-Z: 10-35
			else  // ch >= 'a' && ch <= 'z'
				return 36 + (ch - 'a');              // a-z: 36-61
		}

		// 是否是有效字符
		static constexpr bool is_valid_char(charT ch) noexcept
		{
			return (ch >= '0' && ch <= '9') ||
				(ch >= 'A' && ch <= 'Z') ||
				(ch >= 'a' && ch <= 'z');
		}

		// 内部辅助函数：根据字符获取子节点，如果不存在则返回nullptr
		NodePtr get_child_by_char(NodePtr node, charT ch) const
		{
			if (!node || !is_valid_char(ch)) return nullptr;
			size_t idx = char_to_index(ch);
			// 注意：child_count() 可能小于62，但child(i)在i越界时会返回nullptr
			return node->child(idx);
		}

		// 内部辅助函数：设置节点的子节点（根据字符）
		void set_child_by_char(NodePtr parent, charT ch, NodePtr child)
		{
			if (!parent || !is_valid_char(ch)) return;
			size_t idx = char_to_index(ch);

			parent->set_child(idx, child);
		}

		// 内部辅助函数：遍历到指定字符串的最后一个字符节点
		NodePtr traverse_to_node(const std::basic_string<charT>& str) const
		{
			if (!this->m_root) return nullptr;
			NodePtr current = this->m_root;
			for (charT ch : str)
			{
				if (!is_valid_char(ch)) return nullptr;
				current = get_child_by_char(current, ch);
				if (!current) return nullptr;
			}
			return current;
		}

		// 内部辅助函数：收集从某个节点开始的所有单词
		void collect_words(NodePtr node, std::basic_string<charT> current_prefix,
			std::vector<std::basic_string<charT>>& result) const
		{
			if (!node) return;
			// 如果当前节点是一个单词的结尾
			if (word_end_map.contains(node) && word_end_map.at(node))
			{
				result.push_back(current_prefix);
			}
			// 遍历所有可能的子节点
			for (size_t i = 0; i < node->child_count(); ++i)
			{
				NodePtr child = node->child(i);
				if (child)
				{
					auto opt_ref = child->value();
					if (opt_ref)
					{
						charT ch = opt_ref.value().get();
						collect_words(child, current_prefix + ch, result);
					}
				}
			}
		}

		// 深度拷贝整个Trie树
		void deep_copy(const Trie& other)
		{
			if (!other.m_root)
			{
				this->m_root = nullptr;
				this->m_size = 0;
				word_count = 0;
				word_end_map.clear();
				return;
			}

			// 用于层序遍历的两个队列
			std::queue<const_NodePtr> src_queue;
			std::queue<NodePtr> new_queue;

			// 创建根节点
			this->m_root = this->create_node(charT{});
			this->m_size = 1;
			word_count = other.word_count;

			// 建立原节点到新节点的映射
			std::unordered_map<const_NodePtr, NodePtr> node_map;
			node_map[other.m_root] = this->m_root;

			src_queue.push(other.m_root);
			new_queue.push(this->m_root);

			while (!src_queue.empty())
			{
				const_NodePtr old_node = src_queue.front();
				src_queue.pop();

				NodePtr new_node = new_queue.front();
				new_queue.pop();

				// 调整新节点的子节点向量大小为ALPHABET_SIZE
				this->resize(new_node, ALPHABET_SIZE);

				// 遍历所有可能的子节点
				for (size_t i = 0; i < ALPHABET_SIZE; ++i)
				{
					const_NodePtr old_child = nullptr;
					// 由于旧树的节点可能有62个子节点槽位，我们尝试访问
					// 注意：old_node->child(i)可能会越界，因为old_node的子节点向量可能没有62个槽位
					if (i < old_node->child_count())
					{
						old_child = old_node->child(i);
					}

					if (old_child)
					{
						// 创建新子节点
						charT ch = charT{};
						auto opt_ref = old_child->value();
						if (opt_ref)
						{
							ch = opt_ref.value().get();
						}

						NodePtr new_child = this->create_node(ch);
						new_node->set_child(i, new_child);
						++(this->m_size);

						// 记录映射
						node_map[old_child] = new_child;

						// 入队继续处理
						src_queue.push(old_child);
						new_queue.push(new_child);
					}
					else
					{
						// 确保这个位置是nullptr
						new_node->set_child(i, nullptr);
					}
				}
			}

			// 重建word_end_map
			word_end_map.clear();
			for (const auto& [old_node, is_word_end] : other.word_end_map)
			{
				if (is_word_end)
				{
					auto it = node_map.find(old_node);
					if (it != node_map.end())
					{
						word_end_map[it->second] = true;
					}
				}
			}
		}

		// 单词计数
		size_t word_count = 0;
		// 是否是末尾节点 - 使用自定义分配器的 unordered_map
		WordEndMap word_end_map;

	public:

		// 默认构造函数
		Trie(const Allocator& alloc = Allocator())
			: Base(typename Base::allocator_type(alloc))  // 传递给基类的节点分配器
			, word_end_map(0,
				typename WordEndMap::hasher(),     // 哈希函数对象
				typename WordEndMap::key_equal(),      // 相等比较函数对象
				MapAlloc(alloc))               // 从 charT 分配器构造 map 的分配器
		{}

		// 拷贝构造
		Trie(const Trie& other)
			: Base(other.get_allocator())  // 先创建空树
			, word_end_map(0,
				typename WordEndMap::hasher(),
				typename WordEndMap::key_equal(),
				MapAlloc(other.get_allocator()))
		{
			// 深度拷贝
			deep_copy(other);
		}

		// 拷贝赋值
		Trie& operator=(const Trie& other)
		{
			if (this != &other)
			{
				// 清理当前树
				this->clear();
				word_end_map.clear();
				word_count = 0;

				// 深度拷贝
				deep_copy(other);
			}
			return *this;
		}

		// 移动构造
		Trie(Trie&& other) noexcept
			: Base(std::move(other))
			, word_count(other.word_count)
			, word_end_map(std::move(other.word_end_map))
		{
			other.word_count = 0;
		}

		// 移动赋值
		Trie& operator=(Trie&& other) noexcept
		{
			if (this != &other)
			{
				Base::operator=(std::move(other));
				word_end_map = std::move(other.word_end_map);
				word_count = other.word_count;
				other.word_count = 0;
			}
			return *this;
		}

		// 析构函数交给基类
		~Trie() = default;

		/**
		 * @brief 插入单词
		 * 时间复杂度: O(L)，其中L是单词长度
		 */
		bool insert(const std::basic_string<charT>& word)
		{
			if (word.empty()) return false;
			// 确保根节点存在
			if (!this->m_root)
			{
				this->m_root = this->create_node(charT{});
				this->m_size = 1;
			}

			NodePtr current = this->m_root;

			for (charT ch : word)
			{
				if (!is_valid_char(ch))
				{
					throw std::invalid_argument("Invalid character. Only alphanumeric characters allowed.");
				}
				size_t index = char_to_index(ch);
				this->resize(current, 62);
				NodePtr child = get_child_by_char(current, ch);
				if (!child)
				{
					// 创建新节点
					child = this->create_node(ch);
					set_child_by_char(current, ch, child);
					++(this->m_size);
				}
				current = child;
			}

			if (word_end_map.contains(current) && word_end_map[current])
				return false;

			++word_count;
			word_end_map[current] = true;
			return true;
		}

		/*
		* @brief 搜索单词是否存在
		* 时间复杂度: O(L)
		* @param word 要搜索的单词
		* @return true 如果单词存在，false 否则
		*/
		bool search(const std::basic_string<charT>& word) const
		{
			NodePtr node = traverse_to_node(word);
			return node != nullptr && word_end_map.contains(node) && word_end_map.at(node);
		}

		/**
		* @brief 检查是否有以给定前缀开头的单词
		* 时间复杂度: O(P)，其中P是前缀长度
		* @param prefix 要检查的前缀
		* @return true 如果存在至少一个单词以该前缀开头，false 否则
		*/
		bool startsWith(const std::basic_string<charT>& prefix) const
		{
			return traverse_to_node(prefix) != nullptr;
		}

		/*
		* @brief 从Trie中删除一个单词
		* 时间复杂度: O(L)，但最坏情况可能需要回溯删除节点
		* @param word 要删除的单词
		* @return true 如果单词存在并被删除，false 如果单词不存在
		*/
		bool remove(const std::basic_string<charT>& word)
		{
			if (word.empty() || !this->m_root) return false;

			std::stack<std::pair<NodePtr, charT>> path; // 存储节点及其进入的字符
			NodePtr current = this->m_root;

			// 1. 查找单词路径
			for (charT ch : word)
			{
				if (!is_valid_char(ch)) return false;
				NodePtr child = get_child_by_char(current, ch);
				if (!child)
				{
					return false; // 单词不存在
				}
				path.push({ current, ch });
				current = child;
			}

			// 2. 检查是否是单词结尾
			if (!word_end_map.contains(current) || !word_end_map[current])
			{
				return false; // 不是完整单词
			}

			// 3. 移除单词结尾标记
			word_end_map[current] = false;
			// 如果这个节点还有其他"后代单词"或者有子节点，则不能删除节点
			bool can_delete_node = current->is_leaf(); // 首先检查是否是叶子节点

			// 4. 回溯，删除不必要的节点
			if (can_delete_node)
			{
				// 从最后一个字符节点开始回溯
				while (!path.empty())
				{
					auto [parent, ch] = path.top();
					path.pop();

					// 断开链接
					set_child_by_char(parent, ch, nullptr);
					this->destroy_node(current); // 销毁当前节点
					--(this->m_size);

					// 更新current为parent，继续检查parent是否可删
					current = parent;

					// 如果parent是其他单词的结尾，或者parent还有别的子节点，则停止删除
					if ((word_end_map.contains(current) && word_end_map[current]) || !current->is_leaf())
					{
						break;
					}
				}
			}
			--word_count;
			return true;
		}

		/**
		* @brief 获取Trie中存储的单词数量
		*/
		std::size_t words_count() const
		{
			return word_count;
		}

		/**
		* @brief 检查Trie是否为空（无单词）
		*/
		bool empty_words() const
		{
			return word_count == 0;
		}

		/**
		 * @brief 清空Trie中的所有单词和节点
		 */
		void clear()
		{
			Base::clear(); // 调用基类的clear，释放所有节点
			word_end_map.clear();
			word_count = 0;
		}

		/**
		* @brief 获取所有以给定前缀开头的单词
		* @param prefix 前缀
		* @return 包含所有匹配单词的向量
		*/
		std::vector<std::basic_string<charT>> get_words_with_prefix(const std::basic_string<charT>& prefix) const
		{
			std::vector<std::basic_string<charT>> result;
			NodePtr start_node = traverse_to_node(prefix);
			if (start_node)
			{
				collect_words(start_node, prefix, result);
			}
			return result;
		}
	};
}