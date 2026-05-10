#pragma once
#include <optional>
#include <utility>

namespace TreeNode
{
	
	// 树节点基类
	template <typename T, class Derived>
	class NodeBase
	{
	public:
		/*
		*	@brief	构造函数
		*/
		// 默认构造函数
		NodeBase() :data(std::nullopt) {}


		// 移动& 拷贝构造
		template <typename U>
			requires std::is_convertible_v<U, T>
		NodeBase(U&& u) : data(std::in_place, std::forward<U>(u)) {}

		// 原地构造
		template <typename... Args>
		NodeBase(std::in_place_t, Args&&... args) :data(std::in_place, std::forward<Args>(args)...) {}
		/*
		*	@brief	析构函数
		*/

		~NodeBase() = default;

		/*
		*  @brief	节点基本信息
		*/
		// 是否是叶子节点
		bool is_leaf() const { return static_cast<Derived*>(this)->is_leaf(); }

		// 是否是根节点
		bool is_root() const { return static_cast<Derived*>(this)->is_root(); }

		// 是否是内部节点——是否有子节点
		bool is_internal() const { return static_cast<Derived*>(this)->is_internal(); }

		// 节点的度(子节点数)
		std::size_t degree() const { return static_cast<Derived*>(this)->degree(); }

		// 节点深度
		std::size_t depth() const { return static_cast<Derived*>(this)->depth(); }

		// 以该节点为根的子树高度
		std::size_t height() const { return static_cast<Derived*>(this)->height(); }

		// 节点层级
		std::size_t level() const { return static_cast<Derived*>(this)->level(); }

		/*
		*	@brief	节点数据信息
		*/

		// 获取是否有值
		bool has_value() const { return static_cast<Derived*>(this)->has_value(); }

		// 获取自身引用  安全引用--零开销
		std::optional<std::reference_wrapper<T>> value() { return static_cast<Derived*>(this)->value(); }

		// set_value()
		template <typename U>
			requires std::is_convertible_v<U, T>
		void set_value(U&& u) { static_cast<Derived*>(this)->set_value(std::forward<U>(u)); }

		template <typename... Args>
		void set_value(Args&&... args) { static_cast<Derived*>(this)->set_value(std::forward<Args>(args)...); }
		/*
		*	@brief	父节点访问
		*/
		// 获取父节点指针
		auto parent()-> decltype(auto) { return static_cast<Derived*>(this)->parent(); }

		// 是否有父节点
		bool has_parent() { return static_cast<Derived*>(this)->has_parent(); }
		
		// 设置父节点指针
		void set_parent(NodeBase* ptr) { static_cast<Derived*>(this)->set_parent(static_cast<Derived*>(ptr)); }

		// 从父节点分离
		void detach() { static_cast<Derived*>(this)->detach(); }

		/*
		*	@brief	子节点访问
		*/
		// 获取子节点数量
		std::size_t child_count() const { return static_cast<Derived*>(this)->child_count(); }

		// 获取子节点index
		std::size_t child_index(NodeBase* ptr) const { return static_cast<Derived*>(this)->child_index(); }

		// 获取子节点指针  0 ~ child_count() - 1
		auto child(std::size_t index) -> decltype(auto) { return static_cast<Derived*>(this)->child(index); }

		// 设置子节点指针
		void set_child(std::size_t index, NodeBase* ptr) { static_cast<Derived*>(this)->set_child(index, static_cast<Derived*>(ptr)); }

		// 添加子节点
		void add_child(NodeBase* ptr) { static_cast<Derived*>(this)->add_child(static_cast<Derived*>(ptr)); }

		// 在指定位置插入子节点
		void insert_child(std::size_t pos, NodeBase* ptr) { static_cast<Derived*>(this)->insert_child(pos, static_cast<Derived*>(ptr)); }

		// 移除子节点 -- 若找不到子节点,则返回false
		bool remove_child(NodeBase* ptr) { return static_cast<Derived*>(this)->remove_child(static_cast<Derived*>(ptr)); }

		/*
		*	@brief	兄弟节点访问
		*/

		// 是否有兄弟
		bool has_sibling() const { return static_cast<Derived*>(this)->has_sibling(); }

		// 是否有左兄弟
		bool has_prevSibling() const { return static_cast<Derived*>(this)->has_prevSibling(); }

		// 是否有右兄弟
		bool has_nextSibling() const { return static_cast<Derived*>(this)->has_nextSibling(); }

		// 左兄弟指针
		auto prev_sibling() -> decltype(auto) { return static_cast<Derived*>(this)->prev_sibling(); }

		// 右兄弟指针
		auto next_sibling() -> decltype(auto) { return static_cast<Derived*>(this)->next_sibling(); }

	protected:

		/*
		*	普通节点有值
		*	哨兵节点std::nullopt
		*/
		std::optional<T> data;



		
	};


}