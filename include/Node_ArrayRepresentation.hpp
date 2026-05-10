#pragma once
#include "treeNodeBase.hpp"
#include <stdexcept>


namespace TreeNode
{
	// 数组表示法——节点
	template <typename T>
	class Node_ArrayRepresentation : public NodeBase<T, Node_ArrayRepresentation<T>>
	{
	public:
		using Node = Node_ArrayRepresentation;  // 类名
		using Base = NodeBase<T, Node_ArrayRepresentation>;  // 基类
		using NodePtr = Node_ArrayRepresentation*;  // 节点指针
		using constNodePtr = const Node_ArrayRepresentation*;  // 常量节点指针
	protected:
		using Base::data;

	public:

		/*
		*	@brief	构造函数
		*/

		// 默认构造函数
		Node_ArrayRepresentation() : Base() {}


		// 移动& 拷贝构造
		template <typename U>
			requires std::is_convertible_v<U, T>
		Node_ArrayRepresentation(U&& u) : Base(std::forward<U>(u)) {}

		// 原地构造
		template <typename... Args>
		Node_ArrayRepresentation(std::in_place_t, Args&&... args) : Base(std::in_place, std::forward<Args>(args)...) {}


		// ===== 下列方法可直接实现 =====

		// 获取是否有值
		bool has_value() const { return data.has_value(); }

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

		
		

		// ===== 下列方法需要依赖树，无法直接实现 =====
		// 抛出异常

		// 是否是叶子节点
		bool is_leaf() const
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 是否是根节点
		bool is_root() const 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 是否是内部节点——是否有子节点
		bool is_internal() const 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 节点的度(子节点数)
		std::size_t degree() const 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 节点深度
		std::size_t depth() const 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 以该节点为根的子树高度
		std::size_t height() const 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 节点层级
		std::size_t level() const 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 获取父节点指针
		NodePtr parent() 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 是否有父节点
		bool has_parent()
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 设置父节点指针
		void set_parent(NodePtr ptr) 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 从父节点分离
		void detach() 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 获取子节点数量
		std::size_t child_count() 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 获取子节点index
		std::size_t child_index(NodePtr ptr) 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 获取子节点指针  0 ~ child_count() - 1
		NodePtr child(std::size_t index) 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 设置子节点指针
		void set_child(std::size_t index, NodePtr ptr) 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 添加子节点
		void add_child(NodePtr ptr) 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 在指定位置插入子节点
		void insert_child(std::size_t pos, NodePtr ptr) 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 移除子节点 -- 若找不到子节点,则返回false
		bool remove_child(NodePtr ptr)
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 是否有兄弟
		bool has_sibling() 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 是否有左兄弟
		bool has_prevSibling() 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 是否有右兄弟
		bool has_nextSibling() 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 左兄弟指针
		NodePtr prev_sibling() 
		{
			throw std::logic_error("Method not supported in array representation");
		}

		// 右兄弟指针
		NodePtr next_sibling()
		{
			throw std::logic_error("Method not supported in array representation");
		}
	};
}