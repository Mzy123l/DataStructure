#pragma once

#include "treeBase.hpp"
#include "Node_ParentRepresentation.hpp"
#include <vector>
#include <algorithm>
#include <unordered_map>

/*
*	@brief	树的双亲表示法
*			此方法不适合维护迭代器
*	@brief  仅作为基础设施使用，可进一步拓展		
*	@brief  使用了大量裸指针，不安全！！！
*/


namespace Tree
{
	template <typename T, typename Derived, typename Allocator = std::allocator<TreeNode::Node_ParentRepresentation<T>>>
	class Tree_ParentRepresentation :public TreeBase<T, Tree_ParentRepresentation<T, Derived, Allocator>, TreeNode::Node_ParentRepresentation<T>, Allocator>
	{
	public:
		using Base = TreeBase<T, Tree_ParentRepresentation, TreeNode::Node_ParentRepresentation<T>, Allocator>;
		using Node = TreeNode::Node_ParentRepresentation<T>;
		using NodePtr = Node*;
		using const_NodePtr = const Node*;
		using value_type = typename Base::value_type;
		using allocator_type = typename Base::allocator_type;
		using Tree = Tree_ParentRepresentation;
		using Base::m_allocator;
		using Base::m_root;
		using Base::m_size;

		// 默认构造函数
		Tree_ParentRepresentation(const allocator_type& alloc = allocator_type()) : Base(alloc) {}

		// 拷贝构造& 赋值  --  delete
		Tree_ParentRepresentation(const Tree_ParentRepresentation& other): Base(other.m_allocator)
		{
			deep_copy(other);
		}
		Tree_ParentRepresentation operator=(const Tree_ParentRepresentation& other)
		{
			if (this != &other)
			{
				// 清理当前树的资源
				this->clear();

				deep_copy(other);
			}
			return *this;
		}

		// 移动构造
		Tree_ParentRepresentation(Tree_ParentRepresentation&& other) noexcept:
			Base(std::move(other.m_allocator)), m_root(other.m_root),m_size(other.m_size), m_all_nodes(std::move(other.m_all_nodes))
		{
			other.m_root = nullptr;
			other.m_size = 0;
		}

		// 移动赋值运算符
		Tree_ParentRepresentation& operator=(Tree_ParentRepresentation&& other) noexcept
		{
			if (this != &other)
			{
				// 清理当前树的资源
				this->clear();

				// 移动基类部分
				m_size = other.m_size;
				m_root = other.m_root;
				m_allocator = std::move(other.m_allocator);

				// 移动节点容器
				m_all_nodes = std::move(other.m_all_nodes);

				// 将 other 置为空树状态
				other.m_root = nullptr;
				other.m_size = 0;
			}
			return *this;
		}

		// 析构时清空
		~Tree_ParentRepresentation() { this->clear(); }

		/// 查找节点
		NodePtr find(const T& val)
		{
			for (NodePtr node : m_all_nodes)
			{
				if (node->value() == val)
					return node;
			}
			return nullptr;
		}

		/// @brief 在指定父节点下插入一个新节点。
		/// @brief 如果父指针是nullptr，则为创建根节点
		template <typename... Args>
		NodePtr insert(NodePtr parent, Args&&... args) 
		{
			NodePtr new_node = this->create_node(std::forward<Args>(args)...);
			new_node->set_parent(parent);
			m_all_nodes.push_back(new_node); // 记录节点
			++m_size;

			if (parent == nullptr) 
			{
				this->m_root = new_node;
				m_size = 1;
			}
			
			return new_node;
		}

		/// @brief 获取节点的根节点。
		NodePtr find_root(NodePtr node) const 
		{
			if (!node) return nullptr;
			while (node->has_parent())
			{
				node = node->parent();
			}
			return node;
		}

		/// @brief 获取节点深度。
		std::size_t depth(const_NodePtr node) const
		{
			return node->depth();
		}

		/// @brief 判断两节点是否连通（拥有相同根节点）。
		bool connected(const_NodePtr p, const_NodePtr q) const 
		{
			if (!p || !q) return false;
			return find_root(p) == find_root(q);
		}

		/// @brief 获取从节点到根节点的路径。
		std::vector<NodePtr> path_to_root(NodePtr node) 
		{
			std::vector<NodePtr> path;
			while (node) 
			{
				path.push_back(node);
				node = node->parent();
			}
			return path;
		}

	

		/// @brief 从树中移除并销毁一个节点及其所有后代。
		/// 警告：此操作复杂度为 O(n)，因为需要从 m_all_nodes 中查找并移除该节点及其所有后代。
		bool erase(NodePtr node) 
		{
			if (!node) return false;

			// 1. 收集所有需要移除的后代节点（需要遍历整个容器判断）
			std::vector<NodePtr> to_remove;
			for (NodePtr n : m_all_nodes) 
			{
				// 如果 node 是 n 的祖先，则 n 需要被移除，node也需要被删除
				if (is_ancestor_of(node, n) || node == n)
					to_remove.push_back(n);
			}

			// 2. 从容器中移除这些节点指针
			for (NodePtr rm : to_remove) 
			{
				m_all_nodes.erase(std::remove(m_all_nodes.begin(), m_all_nodes.end(), rm), m_all_nodes.end());
			}

			// 3. 如果被删的是根，更新根指针
			if (node == this->m_root) 
			{
				this->m_root = nullptr;
			}

			// 4. 更新大小并销毁节点
			this->m_size -= to_remove.size();
			for (NodePtr rm : to_remove) 
			{
				this->destroy_node(rm);
			}
			return true;
		}

		/// @brief 判断祖先-后代关系
		bool is_ancestor_of(NodePtr ancestor, NodePtr descendant) const
		{
			NodePtr current = descendant;
			while (current->parent())
			{
				if (current->parent() == ancestor)
					return true;
				current = current->parent();
			}
			return false;
		}


		/// @brief 清空整棵树，释放所有节点。
		void clear()
		{
			for (NodePtr node : m_all_nodes) 
			{
				this->destroy_node(node);
			}
			m_all_nodes.clear();
			this->m_root = nullptr;
			this->m_size = 0;
		}


		/*
		*	==== 子树操作 ====
		*/
		// 将指定节点从树中分离，并且返回一棵新树
		Tree detach(NodePtr node)
		{
			if (!node)
				return Tree_ParentRepresentation(m_allocator);
			// 收集需要移除的节点
			std::vector<NodePtr> to_remove;
			for (auto n : m_all_nodes)
			{
				if (is_ancestor_of(node, n) || node == n)
					to_remove.push_back(n);
			}
			// 移除节点
			for (NodePtr rm : to_remove)
			{
				m_all_nodes.erase(std::remove(m_all_nodes.begin(), m_all_nodes.end(), rm), m_all_nodes.end());
			}
			// 更新当前树的root和size
			if (node == m_root)
			{
				m_root = nullptr;
			}
			m_size -= to_remove.size();
			// 创建新树
			Tree_ParentRepresentation new_tree(m_allocator);
			new_tree.m_root = node;
			new_tree.m_size = to_remove.size();
			return new_tree;
		}
		// 在指定节点下插入一棵子树   @brief 警告: 该操作将使被插入的子树不可操作
		void insert_subTree(NodePtr node, Tree&& subTree)
		{
			if (subTree.m_size)
			{
				node->add_child(subTree.m_root);
				m_size += subTree.m_size;
			}
			subTree.m_size = 0;
			subTree.m_root = nullptr;
		}
	private:
		void deep_copy(const Tree& other)
		{
			Tree new_tree(m_allocator);
			if (other.m_all_nodes.empty())
				return;
			std::unordered_map<NodePtr, NodePtr> node_map;// 映射：原节点(other) -> 新节点(this)
			for (const NodePtr& node : other.m_all_nodes)
			{
				NodePtr new_node = this->create_node(node->value());
				m_all_nodes.push_back(new_node);
				node_map[node] = new_node;
			}
			for (const NodePtr& node : other.m_all_nodes)
			{
				NodePtr this_node = node_map[node];
				this_node->set_parent(node_map[node->parent()]);
			}
			m_size = other.m_size;
			m_root = node_map[other.m_root];
			m_allocator = other.m_allocator;
		}

	protected:
		std::vector<NodePtr> m_all_nodes; // 节点存储容器
	};
} 