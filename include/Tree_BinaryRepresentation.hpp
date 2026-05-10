#pragma once 
#include "Node_BinaryRepresentation.hpp"
#include "treeBase.hpp"
#include <memory>
#include <stack>
#include <bit>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <queue>
#include <concepts>

namespace Tree
{
	// 前向声明
	template <typename U, template <typename> typename Container, typename Derived>
	class Iterator_base;

	namespace Inner_BinaryTree
	{
		// 迭代器概念定义
		template <typename Iter>
		concept Iterator = requires(Iter iter)
		{
			{ *iter } -> std::convertible_to<typename Iter::value_type&>;
			{ ++iter } -> std::same_as<Iter&>;
		};

		// 节点概念
		template <typename Node>
		concept TreeNode = requires
		{
			typename Node::value_type;
		}
		&& std::derived_from<Node, ::TreeNode::Node_BinaryRepresentation<typename Node::value_type>>;
	}

	template <typename T, typename Derived, Inner_BinaryTree::TreeNode tree_node, typename Allocator = std::allocator<tree_node>>
	class Tree_BinaryRepresentation : public TreeBase<T, Tree_BinaryRepresentation<T, Derived, tree_node, Allocator>, tree_node, Allocator>
	{
	public:
		using Base = TreeBase<T, Tree_BinaryRepresentation<T, Derived, tree_node, Allocator>, tree_node, Allocator>;
		using NodeBase = TreeNode::Node_BinaryRepresentation<T>;
		using Node = tree_node;
		using NodePtr = Node*;
		using const_NodePtr = const Node*;
		using value_type = T;
		using allocator_type = typename Base::allocator_type;
		using Tree = Tree_BinaryRepresentation;
		using Base::m_allocator;
		using Base::m_root;
		using Base::m_size;

	protected:
		// 迭代器基类
		template <typename U, template <typename...> typename Container, typename DerivedIterator>
			requires (std::is_same_v<Container<NodePtr>, std::stack<NodePtr>> || std::is_same_v<Container<NodePtr>, std::queue<NodePtr>>)
		class Iterator_base
		{
		public:
			using value_type = U;

			// 默认构造
			Iterator_base() :current(nullptr) {}

			// 有参构造
			explicit Iterator_base(NodePtr root)
			{
				if (root)
				{
					container.push(root);
					current = root;
				}
			}

			// 拷贝构造
			Iterator_base(const Iterator_base& other) :container(other.container), current(other.current) {}

			// 拷贝赋值
			Iterator_base& operator=(const Iterator_base& other)
			{
				if (this != &other)
				{
					container = other.container;
					current = other.current;
				}
				return *this;
			}

			// 移动构造
			Iterator_base(Iterator_base&& other)noexcept :container(std::move(other.container)), current(other.current)
			{
				other.current = nullptr;
			}

			// 移动赋值
			Iterator_base& operator=(Iterator_base&& other)noexcept
			{
				if (this != &other)
				{
					container = std::move(other.container);
					current = other.current;
					other.current = nullptr;
				}
				return *this;
			}

			U& operator*() const
			{
				return current->value().value().get();
			}

			U* operator->() const
			{
				return &(current->value().value().get());
			}

			DerivedIterator& operator++()
			{
				return static_cast<DerivedIterator*>(this)->operator++();
			}
			DerivedIterator operator++(int)
			{
				Iterator_base tmp = *this;
				++(*this);
				return tmp;
			}

			bool operator==(const Iterator_base& other) const
			{
				return current == other.current;
			}

			bool operator!=(const Iterator_base& other) const
			{
				return !(*this == other);
			}

			NodePtr get_current() const { return current; }
		protected:
			Container<NodePtr> container;
			NodePtr current = nullptr;
		};

		// 先序迭代器
		class PreIterator : public Iterator_base<T, std::stack, PreIterator>
		{
		private:
			using Base = Iterator_base<T, std::stack, PreIterator>;
			using Base::container;
			using Base::current;
		public:
			explicit PreIterator(NodePtr root) : Base(root) {}

			PreIterator& operator++()
			{
				if (container.empty())
				{
					current = nullptr;
					return *this;
				}
				current = container.top();
				container.pop();

				if (Tree::right_child(current))
					container.push(Tree::right_child(current));

				if (Tree::left_child(current))
					container.push(Tree::left_child(current));

				if (container.empty())
				{
					current = nullptr;
				}
				else
				{
					current = container.top();
				}

				return *this;
			}
		};

		// 中序迭代器
		class InIterator : public Iterator_base<T, std::stack, InIterator>
		{
		private:
			using Base = Iterator_base<T, std::stack, InIterator>;
			using Base::container;
			using Base::current;

		public:
			// 构造函数 - 完全重写基类的行为
			explicit InIterator(NodePtr root)
			{
				if (!root)
				{
					current = nullptr;
					return;
				}

				// 将所有左子节点压栈
				NodePtr node = root;
				while (node)
				{
					container.push(node);
					node = Tree::left_child(node);
				}

				// 设置第一个要访问的节点（最左节点）
				if (!container.empty())
				{
					current = container.top();
				}
			}

			InIterator& operator++()
			{
				if (!current) return *this;

				// 弹出当前节点（即将被访问）
				container.pop();

				// 如果有右子树，处理右子树
				if (Tree::right_child(current))
				{
					// 进入右子树，并找到其最左节点
					NodePtr node = Tree::right_child(current);
					while (node)
					{
						container.push(node);
						node = Tree::left_child(node);
					}
				}

				// 设置下一个要访问的节点
				if (!container.empty())
				{
					current = container.top();
				}
				else
				{
					current = nullptr;
				}

				return *this;
			}
		};

		// 后序迭代器
		class PostIterator : public Iterator_base<T, std::stack, PostIterator>
		{
		private:
			using Base = Iterator_base<T, std::stack, PostIterator>;
			using Base::container;
			using Base::current;
		public:
			// 构造函数
			explicit PostIterator(NodePtr root) : Base(root)
			{
				if (root)
				{
					// 清空基类构造函数可能放入的根节点
					while (!container.empty()) container.pop();

					std::stack<NodePtr> stack;
					stack.push(root);
					while (!stack.empty())
					{
						NodePtr top = stack.top();
						stack.pop();
						container.push(top);
						if (Tree::left_child(top)) stack.push(Tree::left_child(top));
						if (Tree::right_child(top))stack.push(Tree::right_child(top));
					}
					current = container.empty() ? nullptr : container.top();
				}
			}

			PostIterator& operator++()
			{
				if (!container.empty())
				{
					container.pop();
				}
				current = container.empty() ? nullptr : container.top();
				return *this;
			}
		};

		// 层序迭代器
		class LevelIterator : public Iterator_base<T, std::queue, LevelIterator>
		{
		private:
			using Base = Iterator_base<T, std::queue, LevelIterator>;
			using Base::container;
			using Base::current;
		public:
			LevelIterator(NodePtr root) : Base(root) {}

			LevelIterator& operator++()
			{
				container.pop();
				if (Tree::left_child(current)) container.push(Tree::left_child(current));
				if (Tree::right_child(current)) container.push(Tree::right_child(current));
				current = container.empty() ? nullptr : container.front();
				return *this;
			}
		};

	protected:
		// 常量迭代器别名
		using const_PreIterator = PreIterator;
		using const_InIterator = InIterator;
		using const_PostIterator = PostIterator;
		using const_LevelIterator = LevelIterator;

	private:
		// 获取begin/end迭代器
		template<Inner_BinaryTree::Iterator Iter>
		Iter begin() const
		{
			// 我们需要为不同类型的迭代器提供专门的实现
			// 这里通过SFINAE或if constexpr来区分
			return Iter(m_root);
		}

		template<Inner_BinaryTree::Iterator Iter>
		Iter end() const
		{
			return Iter(nullptr);
		}

	protected:
		// 先序遍历辅助函数，使用父指针，无栈无递归
		template <typename Func>
		void preOrder(Func visit, const Tree& tree)
		{
			NodePtr cur = tree.m_root;
			while (cur)
			{
				// 访问当前节点
				visit(cur);

				// 优先访问左子树
				if (left_child(cur))
				{
					cur = left_child(cur);
				}
				// 其次访问右子树
				else if (right_child(cur))
				{
					cur = right_child(cur);
				}
				// 叶子节点，向上回溯寻找未访问的右子树
				else
				{
					while (cur)
					{
						NodePtr p = parent(cur);
						if (!p)
						{
							cur = nullptr;
							break;
						}
						// 若当前节点是父节点的左孩子，且父节点有右孩子，则转向右孩子
						if (left_child(p) == cur && right_child(p))
						{
							cur = right_child(p);
							break;
						}
						// 否则继续向上回溯
						cur = p;
					}
				}
			}
		}

		// 深拷贝辅助函数
		void deepCopy(const Tree& other)
		{
			if (other.m_size == 0)
			{
				m_root = nullptr;
				return;
			}
			NodePtr other_root = other.m_root;
			m_root = this->create_node(other_root->value().value().get());
			std::stack<std::pair<NodePtr, NodePtr>> stack;// other->this
			stack.push({ static_cast<NodePtr>(other.m_root), static_cast<NodePtr>(m_root) });
			while (!stack.empty())
			{
				std::pair<NodePtr, NodePtr> top = stack.top();
				NodePtr orig = top.first;
				NodePtr copy = top.second;
				stack.pop();
				if (orig->child(1))
				{
					NodePtr rc = this->create_node(orig->child(1)->value().value().get());
					copy->set_child(1, rc);
					rc->set_parent(copy);
					stack.push({ static_cast<NodePtr>(orig->child(1)), rc });
				}
				if (orig->child(0))
				{
					NodePtr lc = this->create_node(orig->child(0)->value().value().get());
					copy->set_child(0, lc);
					lc->set_parent(copy);
					stack.push({ static_cast<NodePtr>(orig->child(0)), lc });
				}
			}
		}

	public:
		// 默认构造函数
		Tree_BinaryRepresentation(std::size_t size, const allocator_type& alloc = allocator_type()) :Base(size, alloc) {}

		// 拷贝构造函数
		Tree_BinaryRepresentation(const Tree& other) :Base(other.m_size, other.m_allocator)
		{
			deepCopy(other);
		}

		// 拷贝赋值运算符
		Tree_BinaryRepresentation& operator=(const Tree& other)
		{
			if (this != &other)
			{
				clear();
				m_allocator = other.m_allocator;
				m_size = other.m_size;
				deepCopy(other);
			}
			return *this;
		}

		// 移动构造函数
		Tree_BinaryRepresentation(Tree&& other) noexcept : Base(other.m_size, other.m_allocator)
		{
			m_root = other.m_root;
			other.m_root = nullptr;
			other.m_size = 0;
		}

		// 移动赋值运算符
		Tree_BinaryRepresentation& operator=(Tree&& other)noexcept
		{
			if (this != &other)
			{
				clear();
				m_size = other.m_size;
				m_root = other.m_root;
				other.m_root = nullptr;
				other.m_size = 0;
			}
			return *this;
		}

		// 清空树
		void clear()
		{
			if (!m_root) return;

			std::stack<NodePtr> stack;
			NodePtr current = m_root;
			NodePtr last_visited = nullptr;

			// 后序遍历销毁节点
			while (!stack.empty() || current)
			{
				if (current)
				{
					stack.push(current);
					current = left_child(current);
				}
				else
				{
					NodePtr peek_node = stack.top();

					// 如果右子节点存在且未被访问
					if (right_child(peek_node) && right_child(peek_node) != last_visited)
					{
						current = right_child(peek_node);
					}
					else
					{
						// 访问并销毁节点
						this->destroy_node(peek_node);
						stack.pop();
						last_visited = peek_node;
					}
				}
			}

			m_root = nullptr;
			m_size = 0;
		}

		// 析构函数
		~Tree_BinaryRepresentation()
		{
			clear();
		}

	protected:
		// 获得左孩子 - 静态版本
		static NodePtr left_child(NodePtr node)
		{
			return static_cast<NodePtr>(node->child(0));
		}

		// 获得右孩子 - 静态版本
		static NodePtr right_child(NodePtr node)
		{
			return static_cast<NodePtr>(node->child(1));
		}

		// 设置左孩子 - 静态版本
		static void set_lchild(NodePtr node, NodePtr child)
		{
			node->set_child(0, child);
		}

		// 设置右孩子 - 静态版本
		static void set_rchild(NodePtr node, NodePtr child)
		{
			node->set_child(1, child);
		}

		// 获取父节点 - 静态版本
		static NodePtr parent(NodePtr node)
		{
			return static_cast<NodePtr>(node->parent());
		}

		// 设置父节点 - 静态版本
		static void set_parent(NodePtr node, NodePtr parent)
		{
			node->set_parent(parent);
		}

		// 迭代器begin/end实际接口
	protected:
		// ===== 先序遍历 =====
		PreIterator begin_preorder() { return begin<PreIterator>(); }
		const_PreIterator begin_preorder() const { return begin<const_PreIterator>(); }

		PreIterator end_preorder() { return end<PreIterator>(); }
		const_PreIterator end_preorder() const { return end<const_PreIterator>(); }

		// ===== 中序遍历 =====
		InIterator begin_inorder() { return begin<InIterator>(); }
		const_InIterator begin_inorder() const { return begin<const_InIterator>(); }

		InIterator end_inorder() { return end<InIterator>(); }
		const_InIterator end_inorder() const { return end<const_InIterator>(); }

		// ===== 后序遍历 =====
		PostIterator begin_postorder() { return begin<PostIterator>(); }
		const_PostIterator begin_postorder() const { return begin<const_PostIterator>(); }

		PostIterator end_postorder() { return end<PostIterator>(); }
		const_PostIterator end_postorder() const { return end<const_PostIterator>(); }

		// ===== 层序遍历 =====
		LevelIterator begin_levelorder() { return begin<LevelIterator>(); }
		const_LevelIterator begin_levelorder() const { return begin<const_LevelIterator>(); }

		LevelIterator end_levelorder() { return end<LevelIterator>(); }
		const_LevelIterator end_levelorder() const { return end<const_LevelIterator>(); }

		// 范围for遍历支持
	private:
		// 先序遍历视图
		struct PreorderView
		{
			Tree& tree;
			PreorderView(Tree& t) : tree(t) {}
			auto begin() { return tree.begin_preorder(); }
			auto end() { return tree.end_preorder(); }
		};

		struct ConstPreorderView
		{
			const Tree& tree;
			ConstPreorderView(const Tree& t) : tree(t) {}
			auto begin() const { return tree.begin_preorder(); }
			auto end() const { return tree.end_preorder(); }
		};

		// 中序遍历视图
		struct InorderView
		{
			Tree& tree;
			InorderView(Tree& t) : tree(t) {}
			auto begin() { return tree.begin_inorder(); }
			auto end() { return tree.end_inorder(); }
		};

		struct ConstInorderView
		{
			const Tree& tree;
			ConstInorderView(const Tree& t) : tree(t) {}
			auto begin() const { return tree.begin_inorder(); }
			auto end() const { return tree.end_inorder(); }
		};

		// 后序遍历视图
		struct PostorderView
		{
			Tree& tree;
			PostorderView(Tree& t) : tree(t) {}
			auto begin() { return tree.begin_postorder(); }
			auto end() { return tree.end_postorder(); }
		};

		struct ConstPostorderView
		{
			const Tree& tree;
			ConstPostorderView(const Tree& t) : tree(t) {}
			auto begin() const { return tree.begin_postorder(); }
			auto end() const { return tree.end_postorder(); }
		};

		// 层序遍历视图
		struct LevelorderView
		{
			Tree& tree;
			LevelorderView(Tree& t) : tree(t) {}
			auto begin() { return tree.begin_levelorder(); }
			auto end() { return tree.end_levelorder(); }
		};

		struct ConstLevelorderView
		{
			const Tree& tree;
			ConstLevelorderView(const Tree& t) : tree(t) {}
			auto begin() const { return tree.begin_levelorder(); }
			auto end() const { return tree.end_levelorder(); }
		};

	protected:
		// 获取视图的接口
		PreorderView preorder() { return PreorderView(*this); }
		ConstPreorderView preorder() const { return ConstPreorderView(*this); }

		InorderView inorder() { return InorderView(*this); }
		ConstInorderView inorder() const { return ConstInorderView(*this); }

		PostorderView postorder() { return PostorderView(*this); }
		ConstPostorderView postorder() const { return ConstPostorderView(*this); }

		LevelorderView levelorder() { return LevelorderView(*this); }
		ConstLevelorderView levelorder() const { return ConstLevelorderView(*this); }

		// 平衡树旋转操作
	protected:
		// 左旋
		static void left_rotate(NodePtr& root, NodePtr x)
		{
			if (!x || !right_child(x)) return;

			NodePtr y = right_child(x);  // x的右子节点
			NodePtr beta = left_child(y);  // y的左子节点

			// 1. 连接 beta 到 x
			set_rchild(x, beta);
			if (beta) set_parent(beta, x);

			// 2. 连接 y 到 x 的父节点
			NodePtr parent_x = parent(x);
			set_parent(y, parent_x);

			if (!parent_x)
			{
				root = y;  // x是根节点
			}
			else if (x == left_child(parent_x))
			{
				set_lchild(parent_x, y);
			}
			else
			{
				set_rchild(parent_x, y);
			}

			// 3. 连接 x 到 y
			set_lchild(y, x);
			set_parent(x, y);
		}

		// 右旋
		static void right_rotate(NodePtr& root, NodePtr x)
		{
			if (!x || !left_child(x)) return;

			NodePtr y = left_child(x);  // x的左子节点
			NodePtr beta = right_child(y);  // y的右子节点

			// 1. 连接 beta 到 x
			set_lchild(x, beta);
			if (beta) set_parent(beta, x);

			// 2. 连接 y 到 x 的父节点
			NodePtr parent_x = parent(x);
			set_parent(y, parent_x);

			if (!parent_x)
			{
				root = y;  // x是根节点
			}
			else if (x == left_child(parent_x))
			{
				set_lchild(parent_x, y);
			}
			else
			{
				set_rchild(parent_x, y);
			}

			// 3. 连接 x 到 y
			set_rchild(y, x);
			set_parent(x, y);
		}

		// 左右旋
		static void left_right_rotate(NodePtr& root, NodePtr x)
		{
			if (!x || !left_child(x)) return;

			NodePtr y = left_child(x);

			// 1. 对y进行左旋
			left_rotate(root, y);

			// 2. 对x进行右旋
			right_rotate(root, x);
		}

		// 右左旋
		static void right_left_rotate(NodePtr& root, NodePtr x)
		{
			if (!x || !right_child(x)) return;

			NodePtr y = right_child(x);

			// 1. 对y进行右旋
			right_rotate(root, y);

			// 2. 对x进行左旋
			left_rotate(root, x);
		}
	};
}