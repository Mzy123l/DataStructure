#pragma once
#include "treeNodeBase.hpp"
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <optional>
#include <functional>
#include <iterator>

namespace TreeNode
{
	// 多孩子表示法——节点
	template <typename T>
	class Node_MultiChildRepresentation : public NodeBase<T, Node_MultiChildRepresentation<T>>
	{


	public:
		using Node = Node_MultiChildRepresentation;  // 类名
		using Base = NodeBase<T, Node_MultiChildRepresentation>;  // 基类
		using NodePtr = Node_MultiChildRepresentation*;  // 节点指针
		using constNodePtr = const Node_MultiChildRepresentation*;  // 常量节点指针
	protected:
		using Base::data;

	public:
		/*
		*	@brief	构造函数
		*/
		// 默认构造函数
		Node_MultiChildRepresentation() :Base(), children() {}

		// 移动& 拷贝构造
		template <typename U>
			requires std::is_convertible_v<U, T>
		Node_MultiChildRepresentation(U&& u) : Base(std::forward<U>(u)), children() {}

		// 原地构造
		template <typename... Args>
		Node_MultiChildRepresentation(std::in_place_t, Args&&... args) : Base(std::in_place, std::forward<Args>(args)...), children() {}


		/// 特殊方法--预留孩子数量大小
		void reserve(std::size_t size)
		{
			children.reserve(size);
		}

		void resize(std::size_t size)
		{
			children.resize(size, nullptr);
		}
		// ========== 直接实现的方法 ==========

		// 是否是叶子节点
		bool is_leaf() const { return children.empty(); }

		// 是否是内部节点——是否有子节点
		bool is_internal() const { return !children.empty(); }


		// 节点的度(子节点数)
		std::size_t degree() const { return children.size(); }

		// 以该节点为根的子树高度
		std::size_t height() const 
		{ 
			if (is_leaf())return 1;

			std::size_t max_height = 0;
			for (const auto& child : children)
			{
				std::size_t child_height = child->height();
				if (child_height > max_height)
				{
					max_height = child_height;
				}

			}
			return max_height + 1;
		}

		// 获取是否有值
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
				return std::cref(data.value()); // 注意这里使用 cref 返回常量引用
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

		// 获取子节点数量
		std::size_t child_count() const { return degree(); }

		// 获取指定索引的子节点指针
		NodePtr child(std::size_t index)
		{
			return children[index];
		}
		constNodePtr child(std::size_t index) const
		{
			return children[index];
		}

		// 根据指针获取子节点索引
		std::size_t child_index(NodePtr ptr)
		{
			auto it = std::find(child.begin(), child.end(), ptr);
			if (it == children.end())
			{
				throw std::out_of_range("Node is not a child of this node");
			}
			return std::distance(children.begin(), it);

		}

		// 设置子节点指针
		void set_child(std::size_t index, NodePtr ptr) 
		{ 
			children[index] = ptr;
		}

		// 添加子节点
		void add_child(NodePtr ptr) 
		{ 
			children.push_back(ptr);
		}

		// 在指定位置插入子节点
		void insert_child(std::size_t pos, NodePtr ptr) 
		{ 
			children.insert(children.begin() + pos, ptr);
		}

		// 移除子节点 -- 若找不到子节点,则返回false
		bool remove_child(NodePtr ptr) 
		{ 
			auto it = std::find(children.begin(), children.end(), ptr);
			if (it == children.end())
				return false;
			children.erase(it);
			return true;
		}

		// ======  这些方法需要树结构支持，无法在当前类中直接实现  ======
		// 抛出异常

		// 节点层级
		std::size_t level() const 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 获取父节点指针
		NodePtr parent() 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 是否有父节点
		bool has_parent() 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 设置父节点指针
		void set_parent(NodePtr ptr) 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 从父节点分离
		void detach() 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 是否是根节点
		bool is_root() const 
		{ 
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 节点深度
		std::size_t depth() const 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 是否有兄弟
		bool has_sibling()const 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 是否有左兄弟
		bool has_prevSibling()const 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 是否有右兄弟
		bool has_nextSibling()const 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 左兄弟指针
		NodePtr prev_sibling() 
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

		// 右兄弟指针
		NodePtr next_sibling()
		{
			throw std::logic_error("Method not supported in child-sibling representation");
		}

	protected:
		std::vector<NodePtr> children;
	};
}