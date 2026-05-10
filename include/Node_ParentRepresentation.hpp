#pragma once
#include "treeNodeBase.hpp"
#include <stdexcept>

namespace TreeNode
{
	// 双亲表示法——节点
	template <typename T>
	class Node_ParentRepresentation : public NodeBase<T, Node_ParentRepresentation<T>>
	{
	public:
		using Node = Node_ParentRepresentation;  // 类名
		using Base = NodeBase<T, Node_ParentRepresentation>;  // 基类
		using NodePtr = Node_ParentRepresentation*;  // 节点指针
		using constNodePtr = const Node_ParentRepresentation*;  // 常量节点指针
	protected:
		using Base::data;

	public:
		/*
		*	@brief	构造函数
		*/
		// 默认构造函数
		Node_ParentRepresentation() : Base(), m_parent(nullptr) {}

		// 移动& 拷贝构造
		template <typename U>
			requires std::is_convertible_v<U, T>
		Node_ParentRepresentation(U&& u) : Base(std::forward<U>(u)), m_parent(nullptr) {}

		// 原地构造
		template <typename... Args>
		Node_ParentRepresentation(std::in_place_t, Args&&... args) : Base(std::in_place, std::forward<Args>(args)...), parent(nullptr) {}

		// ========== 直接实现的方法 ==========

		// 判断节点是否包含数据
		bool has_value()const { return data.has_value(); }

		// 获取节点数据的引用
		std::optional<std::reference_wrapper<T>> value()
		{
			if (data)
				return std::ref(data.value());
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
		NodePtr parent()
		{
			return m_parent;
		}

		// 判断是否有父节点
		bool has_parent() const { return parent; }

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

		// 判断是否为根节点
		bool is_root()const { return !m_parent; }

		
		// 获取节点深度(需要根节点信息)
		std::size_t depth() const
		{
			if (!m_parent)
				return 0;
			return m_parent->depth() + 1;

		}
		
		// 获取节点层级(需要根节点信息)
		std::size_t level() const
		{
			return depth();
		}

		// ======  这些方法需要树结构支持，无法在当前类中直接实现  ======
		// 抛出异常

		// 判断是否为叶子节点(需要子节点信息)
		bool is_leaf() const
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 判断是否为内部节点(需要子节点信息)
		bool is_internal() const
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 获取节点度(需要子节点信息)
		std::size_t degree() const
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		

		// 获取子树高度(需要子节点信息)
		std::size_t height() const
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		

		// 获取子节点数量(需要子节点信息)
		std::size_t child_count() const
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 查找子节点索引(需要子节点信息)
		std::size_t child_index(NodePtr ptr) const
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 获取指定索引子节点指针(需要子节点信息)
		NodePtr child(std::size_t index)
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 设置指定位置子节点指针(需要子节点信息)
		void set_child(std::size_t index, NodePtr ptr)
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 添加子节点(需要子节点信息)
		void add_child(NodePtr ptr)
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 在指定位置插入子节点(需要子节点信息)
		void insert_child(std::size_t pos, NodePtr ptr)
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 移除子节点(需要子节点信息)
		bool remove_child(NodePtr ptr)
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 判断是否有兄弟节点(需要父节点和兄弟节点信息)
		bool has_sibling() const
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 判断是否有前一个兄弟节点(需要父节点和兄弟节点信息)
		bool has_prevSibling() const
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 判断是否有下一个兄弟节点(需要父节点和兄弟节点信息)
		bool has_nextSibling() const
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 获取前一个兄弟节点指针(需要父节点和兄弟节点信息)
		NodePtr prev_sibling()
		{
			throw std::logic_error("Method not supported in parent representation");
		}

		// 获取下一个兄弟节点指针(需要父节点和兄弟节点信息)
		NodePtr next_sibling()
		{
			throw std::logic_error("Method not supported in parent representation");
		}
	

	protected:
		/*
		 *	如果是根节点，父节点指针nullptr
		 */
		NodePtr m_parent;  // 父节点指针

		

	};
}