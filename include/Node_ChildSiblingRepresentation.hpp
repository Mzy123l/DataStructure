#pragma once
#include "treeNodeBase.hpp"
#include <stdexcept>

namespace TreeNode
{
	// 孩子兄弟表示法——节点
	template <typename T>
	class Node_ChildSiblingRepresentation: public NodeBase<T, Node_ChildSiblingRepresentation<T>>
	{
	public:
		using Node = Node_ChildSiblingRepresentation;  // 类名
		using Base = NodeBase<T, Node_ChildSiblingRepresentation>;  // 基类
		using NodePtr = Node_ChildSiblingRepresentation*;  // 节点指针
		using constNodePtr = const Node_ChildSiblingRepresentation*;  // 常量节点指针
	protected:
		using Base::data;

	public:

		/*
		*	@brief	构造函数
		*/
		// 默认构造函数
		Node_ChildSiblingRepresentation() : Base(), m_first_child(nullptr), m_next_sibling(nullptr) {}


		// 移动& 拷贝构造
		template <typename U>
			requires std::is_convertible_v<U, T>
		Node_ChildSiblingRepresentation(U&& u) : Base(std::forward<U>(u)), m_first_child(nullptr), m_next_sibling(nullptr) {}

		// 原地构造
		template <typename... Args>
		Node_ChildSiblingRepresentation(std::in_place_t, Args&&... args) : Base(std::in_place, std::forward<Args>(args)...), m_first_child(nullptr), m_next_sibling(nullptr) {}

		// ========== 直接实现的方法 ==========
		
		// 判断当前节点是否为叶子节点
		bool is_leaf()const { return !m_first_child; }

		// 判断当前节点是否为内部节点
		bool is_internal()const { return m_first_child; }

		// 获取节点的度(子节点数量)
		std::size_t degree()const
		{
			std::size_t count = 0;
			NodePtr child = m_first_child;
			while (child)
			{
				++count;
				child = child->m_next_sibling;
			}
			return count;
		}

		// 获取以该节点为根的子树高度
		std::size_t height()const
		{
			if (is_leaf())
				return 1;
			std::size_t max_height = 0;
			NodePtr child = m_first_child;
			while (child)
			{
				std::size_t child_height = child->height();
				if (child_height > max_height)
				{
					max_height = child_height;
				}
				child = child->m_next_sibling;
			}
			return max_height + 1;
		}

		// 获取节点数据的引用
		std::optional<std::reference_wrapper<T>> value() 
		{
			if (data) 
				return std::ref(data.value());
			return std::nullopt;
		}

		// 判断节点是否包含数据
		bool has_value() const { return data.has_value(); }

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

		// 获取子节点数量
		std::size_t child_count() const { return degree(); }

		// 获取指定索引的子节点指针
		NodePtr child(std::size_t index)
		{
			NodePtr current = m_first_child;
			for (std::size_t i = 0; i < index && current; ++i)
			{
				current = current->m_next_sibling;
			}
			return current;
			
		}

		// 根据指针获取子节点索引
		std::size_t child_index(NodePtr ptr)
		{
			
			NodePtr current = m_first_child;
			if (!ptr)
			{
				throw std::invalid_argument("Cannot search for null pointer");
			}

			std::size_t index = 0;
			while (current)
			{
				if (current == ptr)
				{
					return index;
				}
				current = current->m_next_sibling;
				++index;
			}
			throw std::out_of_range("Node is not a child of this node");
		}

		// 设置指定位置的子节点指针
		void set_child(std::size_t index, NodePtr ptr)
		{
			
			if (index == 0) 
			{
				if (ptr) 
				{
					ptr->m_next_sibling = m_first_child ? m_first_child->m_next_sibling : nullptr;
				}
				m_first_child = ptr;
			}
			else 
			{
				NodePtr prev = child(index - 1);
				if (prev) 
				{
					if (ptr) 
					{
						ptr->m_next_sibling = prev->m_next_sibling ? prev->m_next_sibling->m_next_sibling : nullptr;
					}
					prev->m_next_sibling = ptr;
				}
			}
		}

		// 在子节点链表末尾添加子节点
		void add_child(NodePtr ptr)
		{
			if (!m_first_child)
			{
				m_first_child = ptr;
			}
			else
			{
				NodePtr last_child = m_first_child;
				while (last_child->m_next_sibling)
				{
					last_child = last_child->m_next_sibling;
				}
				last_child->m_next_sibling = ptr;
				
			}
		}

		// 在指定位置插入子节点
		void insert_child(std::size_t pos, NodePtr ptr)
		{
			if (pos == 0) 
			{
				if (ptr) 
				{
					ptr->m_next_sibling = m_first_child;
				}
				m_first_child = ptr;
			}
			else 
			{
				NodePtr prev = child(pos - 1);
				if (prev) 
				{
					if (ptr) 
					{
						ptr->m_next_sibling = prev->m_next_sibling;
					}
					prev->m_next_sibling = ptr;
				}
			}
		}

		// 从子节点链表中移除指定的子节点
		bool remove_child(NodePtr ptr)
		{
			if (!m_first_child || !ptr) return false;
			if (ptr == m_first_child)
			{
				m_first_child = m_first_child->m_next_sibling;
				return true;
			}
			else
			{
				NodePtr current = m_first_child;
				while (current->m_next_sibling)
				{
					if (current->m_next_sibling == ptr)
					{
						current->m_next_sibling = ptr->m_next_sibling;
						return true;
					}
					current = current->m_next_sibling;
				}
				return false;
			}
		}

		// 判断是否存在下一个兄弟节点
		bool has_nextSibling() const { return m_next_sibling; }

		// 获取下一个兄弟节点指针
		NodePtr next_sibling()  { return m_next_sibling; }

		// ======  这些方法需要树结构支持，无法在当前类中直接实现  ======
		// 抛出异常

		// 判断是否为根节点(需要父节点信息)
		bool is_root() const 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 获取节点深度(需要根节点信息)
		std::size_t depth() const 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 获取节点层级(需要根节点信息)
		std::size_t level() const 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 获取父节点指针(当前表示法不支持)
		NodePtr parent()
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 判断是否有父节点(当前表示法不支持)
		bool has_parent() 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 设置父节点指针(当前表示法不支持)
		void set_parent(NodePtr ptr) 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 从父节点分离(需要父节点信息)
		void detach() 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 判断是否有前一个兄弟节点(需要父节点或双向链表)
		bool has_prevSibling() const 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 获取前一个兄弟节点指针(需要父节点或双向链表)
		NodePtr prev_sibling() 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 判断是否有兄弟节点(需要父节点信息)
		bool has_sibling() const
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}


	protected:
		NodePtr m_first_child; //左孩子
		NodePtr m_next_sibling; //右兄弟
	};
}