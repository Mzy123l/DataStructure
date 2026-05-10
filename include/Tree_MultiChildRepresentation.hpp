#pragma once

#include "treeBase.hpp"
#include "Node_MultiChildRepresentation.hpp"
#include <algorithm>
#include <queue>
#include <type_traits>
#include <type_traits>
#include <stack>


/*
*	@brief	树的多孩子
*			此方法需要维护迭代器
*/
namespace Tree
{
	template <typename T, typename Derived, typename Allocator = std::allocator<TreeNode::Node_MultiChildRepresentation<T>>>
	class Tree_MultiChildRepresentation :public TreeBase<T, Tree_MultiChildRepresentation<T, Derived, Allocator>, TreeNode::Node_MultiChildRepresentation<T>, Allocator>
	{
	public:

		using Base = TreeBase<T, Tree_MultiChildRepresentation, TreeNode::Node_MultiChildRepresentation<T>, Allocator>;
		using Node = TreeNode::Node_MultiChildRepresentation<T>;
		using NodePtr = Node*;
		using const_NodePtr = const Node*;
		using value_type = typename Base::value_type;
		using allocator_type = typename Base::allocator_type;
		using Tree = Tree_MultiChildRepresentation;
		using Base::m_allocator;
		using Base::m_root;
		using Base::m_size;

		// 默认构造函数
		Tree_MultiChildRepresentation(const allocator_type& alloc = allocator_type()) : Base(0, alloc) {}

		// 拷贝构造
		Tree_MultiChildRepresentation(const Tree_MultiChildRepresentation& other) :Base(other.m_size, other.m_allocator)
		{
			deep_copy(other);
		}

		Tree_MultiChildRepresentation& operator=(const Tree_MultiChildRepresentation& other)
		{
			if (this != &other)
			{
				// 清理当前树的资源
				this->clear();
				m_size = other.m_size;
				// 深拷贝
				deep_copy(other);
			}
			return *this;
		}
		// 移动构造
		Tree_MultiChildRepresentation(Tree_MultiChildRepresentation&& other) noexcept :
			Base(other.m_size, std::move(other.m_allocator))
		{	
			m_root = other.m_root;
			other.m_root = nullptr;
			other.m_size = 0;
		}

		// 移动赋值运算符
		Tree_MultiChildRepresentation& operator=(Tree_MultiChildRepresentation&& other) noexcept
		{
			if (this != &other)
			{
				// 清理当前树的资源
				this->clear();

				// 移动基类部分
				m_size = other.m_size;
				m_root = other.m_root;
				m_allocator = std::move(other.m_allocator);
				other.m_root = nullptr;
				other.m_size = 0;
				
			}
			return *this;
		}

		// 析构时清空
		~Tree_MultiChildRepresentation() { this->clear(); }

		// 清空整棵树，释放所有节点。
		void clear()
		{
			erase(m_root);
		}


		/*
		*	==== 添加新节点 ====
		*/
		// 插入 (父节点， 编号， 构造材料/成品)
		template <typename... Args>
		NodePtr insert_child(NodePtr parent, std::size_t pos, Args... args)
		{
			NodePtr new_node = this->create_node(std::forward<Args>(args)...);
			parent->insert_child(pos, new_node);
			++m_size;
			return new_node;
		}

		// 尾插--emplace
		template <typename... Args>
		NodePtr emplace_back_child(NodePtr parent, Args... args)
		{
			NodePtr new_node = this->create_node(std::forward<Args>(args)...);
			parent->add_child(new_node);
			++m_size;
			return new_node;
		}

		// 尾插--push
		template <typename U>
			requires std::is_convertible_v<U, T>
		NodePtr push_back_child(NodePtr parent, U&& u)
		{
			emplace_back_child(std::forward<U>(u));
		}

		/*
		*	==== 销毁节点 ====
		*/
		// 销毁一个节点并销毁其子节点
		bool erase(NodePtr node)
		{
			if (!node)
				return false;  // 空指针检查

			// 如果删除的是根节点
			if (node == m_root)
				m_root = nullptr;

			std::queue<NodePtr> queue;
			queue.push(node);

			std::size_t deleted_count = 0;  // 记录删除的节点数

			while (!queue.empty())
			{
				NodePtr current = queue.front();
				queue.pop();

				// 在销毁节点前获取所有子节点
				std::size_t child_count = current->child_count();
				for (std::size_t i = 0; i < child_count; ++i)
				{
					NodePtr child = current->child(i);
					if (child)  // 检查子节点是否有效
					{
						queue.push(child);
					}
				}

				// 销毁当前节点
				this->destroy_node(current);
				++deleted_count;
			}

			// 更新树的大小
			m_size -= deleted_count;
			return true;
		}

		/*
		*	==== 查找节点 ====
		*/
		// 在树中查找   ---找不到返回nullptr---
		NodePtr find(const T& value) const
		{
			return find(m_root, value);
		}

		// 在指定子树中查找  ---找不到返回nullptr---
		NodePtr find(NodePtr subtree_root, const T& value) const
		{
			if (!subtree_root)
				return nullptr;
			std::queue<NodePtr> queue;
			queue.push(subtree_root);
			while (!queue.empty())
			{
				NodePtr current = queue.front();
				queue.pop();
				if (current->value() == value && current->value())
					return current;
				std::size_t child_count = current->child_count();
				for (std::size_t i = 0; i < child_count; ++i)
				{
					queue.push(current->child(i));
				}
			}
			return nullptr;
		}

		/*
		*	==== 孩子数量预留 ====
		*/
		void reserve(NodePtr node, std::size_t size)
		{
			node->reserve(size);
		}
		void resize(NodePtr node, std::size_t size)
		{
			node->resize(size);
		}
		/*
		*	==== 子树操作 ====
		*/
		// 分离子树，将某父节点的第index个孩子及其后代从当前树中分离出来，形成一棵新的子树
		Tree detach(NodePtr parent, std::size_t index)
		{

			if (parent->remove_child(parent->child(index)))
			{
				Tree_MultiChildRepresentation new_tree(m_allocator);
				new_tree.m_root = parent->child(index);
				std::queue<NodePtr> queue;
				queue.push(new_tree.m_root);
				while (!queue.empty())
				{
					NodePtr current = queue.front();
					queue.pop();
					++new_tree.m_size;
					std::size_t child_count = current->child_count();
					for (std::size_t i = 0; i < child_count; ++i)
					{
						queue.push(current->child(i));
					}
				}
				m_size -= new_tree.m_size;
				return new_tree;

			}
			return Tree_MultiChildRepresentation(m_allocator);
		}
		// 插入子树，将子树插入到某父节点的第index个位置   
		void insert_subTree(NodePtr parent, std::size_t index, NodePtr to_insert)
		{
			if (to_insert)
			{

				parent->insert_child(index, to_insert);
				std::queue<NodePtr> queue;
				queue.push(to_insert);
				while (!queue.empty())
				{
					NodePtr current = queue.front();
					queue.pop();
					++m_size;
					std::size_t child_count = current->child_count();
					for (std::size_t i = 0; i < child_count; ++i)
					{
						queue.push(current->child(i));
					}

				}
			}
		}
		// 插入子树，将子树插入到某父节点的第index个位置 @brief 警告: 该操作将使被插入的子树不可操作
		void insert_subTree(NodePtr parent, std::size_t pos, Tree&& subTree)
		{
			if (subTree.m_size)
			{
				parent->insert_child(pos, subTree.m_root);
				m_size += subTree.m_size;
			}
			m_size = 0;
			m_root = nullptr;
		}
		


		/*
		*	==== 树与节点属性查询 ====
		*/
		// 树的高度
		std::size_t height() const
		{
			if (!m_root)
				return 0;
			return m_root->height();
		}
		// 判断祖先-后代关系
		bool is_ancestor_of(NodePtr ancestor, NodePtr descendant) const
		{
			if (!ancestor || !descendant)
				return false;
			std::queue<NodePtr> queue;
			queue.push(ancestor);
			while (!queue.empty())
			{
				NodePtr current = queue.front();
				std::size_t child_count = current->child_count();
				queue.pop();
				for (std::size_t i = 0; i < child_count; ++i)
				{
					NodePtr child = current->child(i);
					if (child == descendant)  // 检查子节点是否是后代
						return true;
					queue.push(child);
				}
			}
			return false;
		}
		// 迭代器前向声明
		class preorder_iterator;
		class postorder_iterator;
		class levelorder_iterator;

		// 迭代器获取方法
		preorder_iterator preorder_begin()
		{
			return preorder_iterator(m_root);
		}

		preorder_iterator preorder_end()
		{
			return preorder_iterator();
		}

		postorder_iterator postorder_begin() 
		{
			return postorder_iterator(m_root);
		}

		postorder_iterator postorder_end() 
		{
			return postorder_iterator();
		}

		levelorder_iterator levelorder_begin() 
		{
			return levelorder_iterator(m_root);
		}

		levelorder_iterator levelorder_end() 
		{
			return levelorder_iterator();
		}
	private:
		// 深拷贝函数
		void deep_copy(const Tree_MultiChildRepresentation& other)
		{
			// 如果原树为空，则直接返回
			if (!other.m_root || other.m_size == 0)
				return;


			// 用于层序遍历的两个队列
			// src_queue: 存放原树中待复制子节点的节点
			// new_queue: 存放新树中已创建、等待链接子节点的对应节点
			std::queue<const_NodePtr> src_queue;
			std::queue<NodePtr> new_queue;

			// 复制根节点
			NodePtr new_root_node = nullptr;
			new_root_node = this->create_node(other.m_root->value().value().get());
			m_root = new_root_node;


			// 将原树根节点和新树根节点分别入队
			src_queue.push(other.m_root);
			new_queue.push(m_root);

			// 层序遍历复制
			while (!src_queue.empty())
			{
				const_NodePtr current_src = src_queue.front();
				src_queue.pop();

				NodePtr current_new = new_queue.front();
				new_queue.pop();

				// 复制当前节点的所有子节点
				std::size_t child_num = current_src->child_count();
				for (std::size_t i = 0; i < child_num; ++i)
				{
					const_NodePtr src_child = current_src->child(i);
					if (!src_child)
					{
						continue;
					}

					// 创建子节点副本
					NodePtr new_child = nullptr;
					new_child = this->create_node(src_child->value().value().get());

					// 将子节点添加到父节点的children向量中
					current_new->add_child(new_child);

					// 将子节点分别入队
					src_queue.push(src_child);
					new_queue.push(new_child);
				}
			}
			m_size = other.m_size;
			m_allocator = other.m_allocator;

		}
		/*
*   ==== 迭代器部分 ====
*/
		public:
			// 前序遍历迭代器
			class preorder_iterator
			{
			public:
				using iterator_category = std::forward_iterator_tag;
				using value_type = T;
				using difference_type = std::ptrdiff_t;
				using pointer = T*;
				using reference = T&;
				using node_pointer = NodePtr;

				preorder_iterator() : current(nullptr) {}

				explicit preorder_iterator(node_pointer root)
				{
					if (root)
					{
						stack.push(root);
						current = root;
					}
				}

				reference operator*() const
				{
					return current->value().value().get();
				}

				pointer operator->() const
				{
					return &(current->value().value().get());
				}

				preorder_iterator& operator++()
				{
					if (stack.empty())
					{
						current = nullptr;
						return *this;
					}

					current = stack.top();
					stack.pop();

					// 将子节点逆序压栈，保证前序遍历顺序
					std::size_t child_count = current->child_count();
					for (std::size_t i = child_count; i > 0; --i)
					{
						stack.push(current->child(i - 1));
					}

					if (stack.empty())
					{
						current = nullptr;
					}
					else
					{
						current = stack.top();
					}

					return *this;
				}

				preorder_iterator operator++(int)
				{
					preorder_iterator tmp = *this;
					++(*this);
					return tmp;
				}

				bool operator==(const preorder_iterator& other) const
				{
					return current == other.current;
				}

				bool operator!=(const preorder_iterator& other) const
				{
					return !(*this == other);
				}



			private:
				node_pointer current;
				std::stack<node_pointer> stack;
			};

			// 后序遍历迭代器
			class postorder_iterator
			{
			public:
				using iterator_category = std::forward_iterator_tag;
				using value_type = T;
				using difference_type = std::ptrdiff_t;
				using pointer = T*;
				using reference = T&;
				using node_pointer = NodePtr;

				postorder_iterator() : current(nullptr) {}

				explicit postorder_iterator(node_pointer root)
				{

					if (root)
					{
						populate_stack(root);
						current = root;
					}
				}

				reference operator*() const
				{
					return current->value().value().get();
				}

				pointer operator->() const
				{
					return &(current->value().value().get());
				}

				postorder_iterator& operator++()
				{
					if (stack.empty())
					{
						current = nullptr;
						return *this;
					}

					current = stack.top();
					stack.pop();

					if (stack.empty())
					{
						current = nullptr;
					}
					else
					{
						current = stack.top();
					}

					return *this;
				}

				postorder_iterator operator++(int)
				{
					postorder_iterator tmp = *this;
					++(*this);
					return tmp;
				}

				bool operator==(const postorder_iterator& other) const
				{
					return current == other.current;
				}

				bool operator!=(const postorder_iterator& other) const
				{
					return !(*this == other);
				}



			private:
				void populate_stack(node_pointer node)
				{
					std::stack<node_pointer> temp_stack;
					temp_stack.push(node);

					while (!temp_stack.empty())
					{
						node_pointer current_node = temp_stack.top();
						temp_stack.pop();
						stack.push(current_node);

						// 将子节点按顺序压栈
						std::size_t child_count = current_node->child_count();
						for (std::size_t i = 0; i < child_count; ++i)
						{
							temp_stack.push(current_node->child(i));
						}
					}
				}

				node_pointer current;
				std::stack<node_pointer> stack;
			};

			// 层序遍历迭代器
			class levelorder_iterator
			{
			public:
				using iterator_category = std::forward_iterator_tag;
				using value_type = T;
				using difference_type = std::ptrdiff_t;
				using pointer = T*;
				using reference = T&;
				using node_pointer = NodePtr;

				levelorder_iterator() : current(nullptr) {}

				explicit levelorder_iterator(node_pointer root)
				{
					if (root)
					{
						queue.push(root);
						current = root;
					}
				}

				reference operator*() const
				{
					return current->value().value().get();
				}

				pointer operator->() const
				{
					return &(current->value().value().get());
				}

				levelorder_iterator& operator++()
				{
					if (queue.empty())
					{
						current = nullptr;
						return *this;
					}

					current = queue.front();
					queue.pop();

					// 将当前节点的所有子节点入队
					std::size_t child_count = current->child_count();
					for (std::size_t i = 0; i < child_count; ++i)
					{
						queue.push(current->child(i));
					}

					if (queue.empty())
					{
						current = nullptr;
					}
					else
					{
						current = queue.front();
					}

					return *this;
				}

				levelorder_iterator operator++(int)
				{
					levelorder_iterator tmp = *this;
					++(*this);
					return tmp;
				}

				bool operator==(const levelorder_iterator& other) const
				{
					return current == other.current;
				}

				bool operator!=(const levelorder_iterator& other) const
				{
					return !(*this == other);
				}


			private:
				node_pointer current;
				std::queue<node_pointer> queue;
			};
	};

}