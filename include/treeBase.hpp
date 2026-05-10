#pragma once
#include <memory>
#include <cstddef>

namespace Tree
{
	// 树基类
	template <typename T, typename Derived, typename Node, typename Allocator = std::allocator<Node>>
	class TreeBase
	{
	public:
		using value_type = T;
		using node_type = Node;
		using node_pointer = node_type*;
		using const_node_pointer = const node_type*;
		using allocator_type = Allocator;
		using traits = std::allocator_traits<allocator_type>;

		// 默认构造函数
		explicit TreeBase(std::size_t size = 0, const allocator_type& alloc = allocator_type()): m_size(size), m_allocator(alloc){}

		// 默认析构
		// 在基类不作实现，派生类中各自实现
		~TreeBase() = default;

		// 获取根节点指针
		node_pointer root() { return m_root; }
		const_node_pointer root()const { return m_root; }

		// 树是否为空
		bool empty() const { return m_size == 0; }

		// 获取树的节点总数
		std::size_t size() const { return m_size; }

		// 获取分配器
		allocator_type get_allocator() const noexcept { return m_allocator; }

	

		// 清空树
		void clear() { static_cast<Derived*>(this)->clear(); }

		

		
	protected:

		/*
		*	@brief	节点内存分配管理
		*	=====   不涉及size()的增减   =====
		*/

		// 创建并构造一个节点
		template<typename... Args>
		node_pointer create_node(Args&&... args) 
		{
			node_pointer p = traits::allocate(m_allocator, 1);
			try 
			{
				traits::construct(m_allocator, p, std::forward<Args>(args)...);
				return p; // 注意：此时 m_size 未增加，节点还未链接入树
			}
			catch (...) 
			{
				traits::deallocate(m_allocator, p, 1);
				throw;
			}
		}

		// 析构并释放一个节点
		void destroy_node(node_pointer p) noexcept 
		{
			if (p) 
			{
				traits::destroy(m_allocator, p);
				traits::deallocate(m_allocator, p, 1);
			}
		}

	protected:
		node_pointer m_root = nullptr; // 根节点
		std::size_t m_size; // 大小
		allocator_type m_allocator; // 分配器
	};
}