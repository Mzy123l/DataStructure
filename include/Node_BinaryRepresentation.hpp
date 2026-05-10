#pragma once
#include "treeNodeBase.hpp"
#include <stdexcept>
#include <cassert>
namespace TreeNode
{

	// 二叉树表示法——节点
	template <typename T>
	class Node_BinaryRepresentation : public NodeBase<T, Node_BinaryRepresentation<T>>
	{
	public:
		using Node = Node_BinaryRepresentation;  // 类名
		using Base = NodeBase<T, Node_BinaryRepresentation>;  // 基类
		using NodePtr = Node_BinaryRepresentation*;  // 节点指针
		using constNodePtr = const Node_BinaryRepresentation*;  // 常量节点指针
	protected:
		using Base::data;

	public:
		/*
		*	@brief	构造函数
		*/

		// 默认构造函数
		Node_BinaryRepresentation() : Base(), m_parent(nullptr), m_lchild(nullptr), m_rchild(nullptr) {}


		// 移动& 拷贝构造
		template <typename U>
			requires std::is_convertible_v<U, T>
		Node_BinaryRepresentation(U&& u) : Base(std::forward<U>(u)), m_parent(nullptr), m_lchild(nullptr), m_rchild(nullptr) {}

		// 原地构造
		template <typename... Args>
		Node_BinaryRepresentation(std::in_place_t, Args&&... args) : Base(std::in_place, std::forward<Args>(args)...), m_parent(nullptr), m_lchild(nullptr), m_rchild(nullptr) {}

		// ====  下列方法可直接实现 =====

		// 是否是叶子节点
		bool is_leaf() const { return !(m_lchild || m_rchild); }

		// 是否是根节点
		bool is_root() const { return !m_parent; }

		// 是否是内部节点——是否有子节点
		bool is_internal() const { return !is_leaf(); }

		// 节点的度(子节点数)
		std::size_t degree() const { return is_leaf() ? 0 : ((m_lchild && m_rchild) ? 2 : 1); }

		// 节点深度
		std::size_t depth() const 
		{
			std::size_t dep = 0;
			NodePtr current = this;
			while (current->m_parent)
			{
				++dep;
				current = current->m_parent;
			}
			return dep;
		}

		// 以该节点为根的子树高度
		std::size_t height() const
		{
			if (is_leaf())
				return 1;

			std::size_t left_height = 0;
			std::size_t right_height = 0;

			if (m_lchild) 
				left_height = m_lchild->height();

			if (m_rchild) 
				right_height = m_rchild->height();

			// 树的高度 = max(左子树高度, 右子树高度) + 1
			return std::max(left_height, right_height) + 1;

		}

		// 节点层级
		std::size_t level() const { return depth(); }

		// 判断节点是否包含数据
		bool has_value() const { return data.has_value(); }

		// 获取节点数据的引用
		std::optional<std::reference_wrapper<T>> value()
		{
			if (data)
				return std::ref(data.value());
			return std::nullopt;
		}
		std::optional<std::reference_wrapper<const T>> value() const
		{
			if (data)
				return std::cref(data.value());
			return std::nullopt;
		}
		// 设置节点数据(拷贝/移动)
		template <typename U>
			requires std::is_convertible_v<U, T>
		void set_value(U&& u)
		{
			if (!data.has_value())
			{
				data = std::optional<T>(std::in_place, std::forward<U>(u));
			}
			else
			{
				data.value() = std::forward<U>(u);
			}
		}
		// 设置节点数据(原地构造)
		template <typename... Args>
		void set_value(Args&&... args)
		{
			if (!data.has_value())
			{
				data.emplace(std::forward<Args>(args)...);
			}
			else
			{
				data.value() = T(std::forward<Args>(args)...);
			}
		}

		// 获取父节点指针
		NodePtr parent() { return m_parent; }

		// 是否有父节点
		bool has_parent() { return m_parent; }

		// 设置父节点指针
		void set_parent(NodePtr ptr)
		{
			m_parent = ptr;
		}

		// 从父节点分离
		void detach() 
		{ 
			m_parent = nullptr;
		}

		// 获取子节点数量
		std::size_t child_count() const { return degree(); }

		// 获取子节点index
		std::size_t child_index(NodePtr ptr) const 
		{ 
			if (ptr == m_lchild)
				return 0;
			if (ptr == m_rchild)
				return 1;
			throw std::out_of_range("Node is not a child of this node");
		}


		// 获取子节点指针  0 ~ child_count() - 1
		NodePtr child(std::size_t index) 
		{
			assert(index == 0 || index == 1);
			if (index == 0)
				return m_lchild;
			return m_rchild;
		}
		const NodePtr child(std::size_t index) const
		{
			assert(index == 0 || index == 1);
			if (index == 0)
				return m_lchild;
			return m_rchild;
		}
		// 设置子节点指针
		void set_child(std::size_t index, NodePtr ptr) 
		{ 
			assert(index == 0 || index == 1);
			if (index == 0)
				m_lchild = ptr;
			else
				m_rchild = ptr;
		}
		// 移除子节点 -- 若找不到子节点,则返回false
		bool remove_child(NodePtr ptr)
		{
			if (ptr)
			{
				if (ptr == m_lchild)
				{
					m_lchild = nullptr;
					return true;
				}
				if (ptr == m_rchild)
				{
					m_rchild = nullptr;
					return true;
				}
			}
			return false;
		}

		// 是否有左兄弟
		bool has_prevSibling()const
		{
			if (!m_parent)
				return false;
			return m_parent->m_lchild && m_parent->m_lchild != this;
		}

		// 是否有右兄弟
		bool has_nextSibling()const
		{
			if (!m_parent)
				return false;
			return m_parent->m_rchild && m_parent->m_rchild != this;
		}

		// 是否有兄弟
		bool has_sibling()const
		{
			if (!m_parent)
				return false;
			return (m_parent->m_lchild && m_parent->m_lchild != this) || (m_parent->m_rchild && m_parent->m_rchild != this);
		}

		// 左兄弟指针
		NodePtr prev_sibling() { return m_parent->m_lchild; }

		// 右兄弟指针
		NodePtr next_sibling() { return m_parent->m_rchild; }

		// ====== 因二叉树表示法限制，下列方法无法实现 ====
		// 抛出异常

		// 添加子节点
		void add_child(NodePtr ptr) 
		{
			throw std::logic_error("Method not supported in binary representation");
		}

		// 在指定位置插入子节点
		void insert_child(std::size_t pos, NodePtr ptr)
		{
			throw std::logic_error("Method not supported in binary representation");
		}


	protected:

		NodePtr m_parent;//父节点指针
		NodePtr m_lchild;//左孩子指针
		NodePtr m_rchild;//右孩子指针


	};
}