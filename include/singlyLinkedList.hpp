#pragma once
#include <memory>
#include <utility>
#include <functional>
#include <optional>


namespace DateStructure
{

	template<class T>
	class singlyLinkedList
	{
	private:

		// 节点结构体
		struct Node
		{
			T data;
			std::unique_ptr<Node> next;

			Node(const T& val, std::unique_ptr<Node>&& next_ptr = nullptr) :data(val), next(std::move(next_ptr)) {}
			Node(T&& val, std::unique_ptr<Node>&& next_ptr = nullptr) :data(std::move(val)), next(std::move(next_ptr)) {}
		};

		
		
	private:
		template<class U>
		class base_iterator
		{
		public:
			using node_type = std::conditional_t<std::is_const_v<U>, const Node, Node>;
			using node_value = std::remove_cv_t<T>;
			// 构造函数
			base_iterator() = default;

			explicit base_iterator(node_type* ptr = nullptr)
				: current(ptr ? std::optional(std::ref(*ptr)) : std::nullopt) {
			}

			// 拷贝构造函数
			explicit base_iterator(const Node& node) : current(std::ref(node)) {}
			// 单向转化
			template<typename OtherU, typename = std::enable_if_t<std::is_same_v<U, const T>&& std::is_same_v<OtherU, T>>>
			base_iterator(const base_iterator<OtherU>& other)
				: current(other.current) {
			}

			// 判断迭代器是否有效
			bool is_valid() const { return current.has_value(); }
			explicit operator bool() const { return current.has_value(); }

			// 前置递增运算符
			base_iterator<U>& operator++()
			{
				if (*this)
				{
					node_type& currNode = current.value().get();
					if (currNode.next)
					{
						current = std::ref(*(currNode.next));
					}
					else
					{
						current = std::nullopt;
					}
				}
				return *this;
			}

			// 后置递增运算符
			base_iterator<U> operator++(int)
			{
				base_iterator<U> temp = *this;
				++(*this);
				return temp;
			}

			// 重载解引用
			U& operator*() const
			{
				return current.value().get().data;
			}

			// 箭头运算符
			U* operator->() const
			{
				return &(current.value().get().data);
			}

			// 重载 ==
			template<typename OtherU>
			bool operator==(const base_iterator<OtherU>& other) const
			{
				if (!current && !other.current) return true;
				if (!current || !other.current) return false;
				return &(current.value().get()) == &(other.current.value().get());
			}

			// 重载 !=
			template<typename OtherU>
			bool operator!=(const base_iterator<OtherU>& other) const
			{

				return !(*this == other);
			}

			// 赋值运算符
			base_iterator<U>& operator=(const base_iterator<U>& other)
			{
				if (this != &other)
				{
					current = other.current;
				}
				return *this;
			}

		private:
			std::optional<std::reference_wrapper<node_type>> current;

			// 允许const_iterator访问iterator的current
			template<typename> friend class base_iterator;
		};
	public:
		using iterator = base_iterator<T>;
		using const_iterator = base_iterator<const T>;

		

	public:

		singlyLinkedList() = default;
		template<typename... Args>
		
		// 尾插(拷贝)
		void push_back(const T& val)
		{
			if (head == nullptr)
			{
				head = std::make_unique<Node>(val);
				tail = std::ref(*head);
				count = 1;
				return;
			}
			tail.value().get().next = std::make_unique<Node>(val);
			++count;
			tail = std::ref(*(tail.value().get().next));

		}

		// 拷贝构造函数
		singlyLinkedList(const singlyLinkedList<T>& other)
		{
			deepCopy(other.head);
		}

		// 移动构造函数	
		singlyLinkedList(singlyLinkedList<T>&& other) noexcept
		{
			head = std::move(other.head);
			count = other.count;
			other.count = 0;
			tail = std::move(other.tail);
			other.tail = std::nullopt;
		}

		// 拷贝赋值运算符
		singlyLinkedList<T>& operator=(const singlyLinkedList<T>& other)
		{
			if (this != &other)
			{  // 防止自赋值
				clear();           // 释放当前资源
				deepCopy(other.head);
			}
			return *this;
		}
		// 移动赋值运算符
		singlyLinkedList<T>& operator=(singlyLinkedList<T>&& other) noexcept
		{
			if (this != &other)
			{  // 防止自赋值
				clear();           // 释放当前资源
				head = std::move(other.head);
				count = other.count;
				other.count = 0;
				tail = std::move(other.tail);
				other.tail = std::nullopt;
			}
			return *this;
		}
		// 析构
		~singlyLinkedList() { clear(); }

		// 尾插(移动)
		void push_back(T&& val)
		{
			if (head == nullptr)
			{
				head = std::make_unique<Node>(std::move(val));
				tail = std::ref(*head);
				count = 1;
				return;
			}
			tail.value().get().next = std::make_unique<Node>(std::move(val));
			++count;
			tail = std::ref(*(tail.value().get().next));
		}

		// 头插(拷贝)
		void push_front(const T& val)
		{
			if (count == 0)
			{
				head = std::make_unique<Node>(val);
				tail = std::ref(*head);
				count = 1;
				return;
			}
			std::unique_ptr<Node> newHead = std::make_unique<Node>(val, std::move(head));
			head = std::move(newHead);
			++count;
		}

		//头插(移动)
		void push_front(T&& val)
		{
			if (count == 0)
			{
				head = std::make_unique<Node>(val);
				tail = std::ref(*head);
				count = 1;
				return;
			}
			std::unique_ptr<Node> newHead = std::make_unique<Node>(std::move(val), std::move(head));
			head = std::move(newHead);
			++count;
		}

		//是否为空
		const bool isEmpty() const
		{
			return count == 0;
		}

		//获取长度
		const size_t size() const
		{
			return count;
		}
		// 返回指向第一个元素的迭代器
		iterator begin() 
		{
			return iterator(head.get());
		}

		// 返回指向尾后位置的迭代器
		iterator end()
		{
			return iterator(nullptr);
		}

		// 返回指向第一个元素的常量迭代器
		const_iterator begin() const
		{
			return const_iterator(head.get());
		}

		// 返回指向尾后位置的常量迭代器
		const_iterator end() const
		{
			return const_iterator(nullptr);
		}

		// 返回指向第一个元素的常量迭代器 
		const const_iterator cbegin() const
		{
			return const_iterator(head.get());
		}

		// 返回指向尾后位置的常量迭代器 
		const const_iterator cend() const
		{
			return const_iterator(nullptr);
		}
		// 查找特定元素，返回迭代器
		iterator find(const T& val)
		{
			auto it = begin();
			while (it != end()) 
			{
				if (*it == val) 
				{ 
					return it;
				}
				++it;
			}
			return end();
		}
		// 查找特定元素，返回常量迭代器
		const const_iterator cfind(const T& val) const
		{
			auto it = cbegin();
			while (it != cend())
			{
				if (*it == val)
				{
					return it;
				}
				++it;
			}
			return cend();
		}
		// 在指定迭代器位置插入新节点(拷贝)
		iterator insert(iterator position, const T& val)
		{
			if (position == begin())
			{
				push_front(val);
				return begin();
			}

			// 找到position的前一个节点
			Node* prev = nullptr;
			Node* current = head.get();
			while (current != nullptr && iterator(current) != position)
			{
				prev = current;
				current = current->next.get();
			}

			if (prev == nullptr)
			{
				// 没有找到位置，插入到末尾
				push_back(val);
				return iterator(nullptr);
			}
			
			std::unique_ptr<Node> newNode = std::make_unique<Node>(val, std::move(prev->next));
			prev->next = std::move(newNode);
			++count;
			return iterator(prev->next.get());
		}

		// 在指定迭代器位置插入新节点(移动)
		iterator insert(iterator position, T&& val)
		{
			if (position == begin())
			{
				push_front(val);
				return begin();
			}

			// 找到position的前一个节点
			Node* prev = nullptr;
			Node* current = head.get();
			while (current != nullptr && iterator(current) != position)
			{
				prev = current;
				current = current->next.get();
			}

			if (prev == nullptr)
			{
				// 没有找到位置，插入到末尾
				push_back(std::move(val));
				return iterator(nullptr);
			}
			
			std::unique_ptr<Node> newNode = std::make_unique<Node>(std::move(val), std::move(prev->next));
			prev->next = std::move(newNode);
			++count;
			return iterator(prev->next.get());
		}

		// 删除指定迭代器位置的节点
		iterator erase(iterator position)
		{
			if (position == end() || head == nullptr)
			{
				return end();
			}

			if (position == begin())
			{
				head = std::move(head->next);
				--count;
				if (count == 0)
				{
					tail = std::nullopt;
				}
				return iterator(head.get());
			}

			// 找到position的前一个节点
			Node* prev = head.get();
			while (prev != nullptr && prev->next != nullptr && iterator(prev->next.get()) != position)
			{
				prev = prev->next.get();
			}

			if (prev == nullptr || prev->next == nullptr)
			{
				return end();  // 没找到
			}

			
			prev->next = std::move(prev->next->next);
			iterator result(prev->next.get());
			if (prev->next == nullptr)
			{
				tail = std::ref(*prev);
			}
			--count;
			return result;
		}
	
		// 清空链表
		void clear()
		{
			// 使用迭代方式释放节点，避免递归析构导致的栈溢出
			while (head)
			{
				auto next = std::move(head->next);
				head.reset();  // 显式释放当前节点
				head = std::move(next);
			}
			tail = std::nullopt;
			count = 0;
		}
	private:

		std::unique_ptr<Node> head;//头指针
		std::optional<std::reference_wrapper<Node>> tail;//尾引用
		std::size_t count = 0;//节点数
		
		

		// 深拷贝函数
		void deepCopy(const std::unique_ptr<Node>& otherHead)
		{
			if (otherHead == nullptr)
			{
				count = 0;

				return; 
			}
			head = std::make_unique<Node>(otherHead->data);
			count = 1;
			tail = std::ref(*head);
			Node* current = (otherHead->next).get();
			while (current)
			{
				this->push_back(current->data);
				current = current->next.get();
			}
			
		}

	};
}