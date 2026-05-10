#pragma once
#include <optional>
#include <memory>
#include <initializer_list>

namespace DoublyLinkedList
{
	template<typename T>
	class doublyLinkedList
	{
	private:
		// 节点
		struct Node
		{
			std::optional<T> data;
			Node* prev;
			Node* next;

			Node() = delete;

			// 1. 单参数版本（支持隐式转换）
			template<typename U>
				requires std::convertible_to<U, T>
			Node(Node* p, Node* n, U&& u) : data(std::in_place, std::forward<U>(u)), prev(p), next(n) {}

			// 2. 多参数版本（用于 emplace_）
			template<typename... Args>
			Node(Node* p, Node* n, Args&&... args) : data(std::in_place, std::forward<Args>(args)...), prev(p), next(n) {}

			// 3. 哨兵版本
			Node(Node* p, Node* n, std::nullopt_t) : data(std::nullopt), prev(p), next(n) {}
		};

	private:
		// 迭代器
		template <typename U>
		class base_iterator
		{
		public:
			using node_type = std::conditional_t<std::is_const_v<U>, const Node, Node>;
			using value_type = U;
			using difference_type = std::ptrdiff_t;
			using pointer = U*;
			using reference = U&;
			using iterator_category = std::bidirectional_iterator_tag;

			// 默认构造函数
			base_iterator() = default;

			// 节点指针构造
			explicit base_iterator(node_type* ptr) : current(ptr) {}

			// 拷贝构造函数
			base_iterator(const base_iterator&) = default;

			// 从iterator构造const_iterator
			template<typename OtherU, typename = std::enable_if_t<std::is_same_v<OtherU, T>&& std::is_same_v<U, const T>> >
			base_iterator(const base_iterator<OtherU>& other) : current(other.current) {}

			// 析构函数
			~base_iterator() = default;

			// 重载赋值运算符
			template<typename OtherU>
				requires (!std::is_const_v<U>)
			base_iterator& operator=(const base_iterator<OtherU>& other) noexcept
			{
				if (current != other.current)
				{
					current = other.current;
				}
				return *this;
			}

			// 前置递增运算符
			base_iterator& operator++() noexcept
			{
				if (current && current->next)
				{
					current = current->next;
				}
				return *this;
			}

			// 后置递增运算符
			base_iterator operator++(int)
			{
				base_iterator temp = *this;
				++(*this);
				return temp;
			}

			// 前置递减运算符
			base_iterator& operator--() noexcept
			{
				if (current)
				{
					// 当current是sentinel时，应该移动到tail
					if (current->prev)
					{
						current = current->prev;
					}
				}
				return *this;
			}

			// 后置递减运算符
			base_iterator operator--(int)
			{
				base_iterator temp = *this;
				--(*this);
				return temp;
			}

			// 重载==
			template<typename OtherU>
			bool operator==(const base_iterator<OtherU>& other) const noexcept
			{
				return current == other.current;
			}

			// 重载!=
			template<typename OtherU>
			bool operator!=(const base_iterator<OtherU>& other) const noexcept
			{
				return !((*this) == other);
			}

			// 解引用运算符
			U& operator*()
			{
				return current->data.value();
			}

			// 重载->
			U* operator->()
			{
				return &(current->data.value());
			}

		private:
			node_type* current;
			template<typename> friend class base_iterator;
			friend class doublyLinkedList;
		};
	public:
		using iterator = base_iterator<T>;
		using const_iterator = base_iterator<const T>;

	private:
		Node* head;
		Node* tail;
		Node* sentinel;  // 哨兵节点，表示end()
		std::size_t length;

		// 深拷贝函数
		void deepCopy(Node* otherHead, std::size_t otherLength)
		{
			length = otherLength;
			if (!otherHead || otherLength == 0)
			{
				head = nullptr;
				tail = nullptr;
				sentinel->prev = nullptr;
				return;
			}

			// 创建头节点
			head = new Node(nullptr, nullptr, otherHead->data.value());

			Node* otherCurrent = otherHead->next;
			Node* thisPrev = head;

			// 循环复制剩余节点
			for (size_t i = 1; i < length; ++i)
			{
				thisPrev->next = new Node(thisPrev, nullptr, otherCurrent->data.value());
				thisPrev = thisPrev->next;
				otherCurrent = otherCurrent->next;
			}

			// 设置尾节点和哨兵
			tail = thisPrev;
			tail->next = sentinel;
			sentinel->prev = tail;
		}

	public:
		// size()
		const std::size_t size() const
		{
			return length;
		}

		// isEmpty
		const bool isEmpty() const
		{
			return length == 0;
		}

		// begin()迭代器
		iterator begin()
		{
			return isEmpty() ? end() : iterator(head);
		}

		// end()迭代器
		iterator end()
		{
			return iterator(sentinel);
		}

		// cbegin()迭代器
		const_iterator cbegin() const
		{
			return isEmpty() ? cend() : const_iterator(head);
		}

		// cend()迭代器
		const_iterator cend() const
		{
			return const_iterator(sentinel);
		}

		// 清空链表
		void clear()
		{
			Node* current = head;
			while (current && current != sentinel)
			{
				Node* temp = current;
				current = current->next;
				delete temp;
			}

			head = nullptr;
			tail = nullptr;
			length = 0;

			// 重置哨兵节点
			if (sentinel)
			{
				sentinel->prev = nullptr;
				sentinel->next = nullptr;
			}
		}

		// 析构函数
		~doublyLinkedList()
		{
			clear();
			delete sentinel;
		}

		// 默认构造函数
		doublyLinkedList() : head(nullptr), tail(nullptr), sentinel(new Node(nullptr, nullptr, std::nullopt)), length(0) {}

		// 初始化列表构造
		doublyLinkedList(std::initializer_list<T> list) : head(nullptr), tail(nullptr), sentinel(new Node(nullptr, nullptr, std::nullopt)), length(0)
		{
			for (auto const& it : list)
			{
				emplace_back(it);
			}
		}

		// 拷贝构造
		doublyLinkedList(const doublyLinkedList& other) : sentinel(new Node(nullptr, nullptr, std::nullopt)), head(nullptr), tail(nullptr), length(0)
		{
			deepCopy(other.head, other.length);
		}

		// 移动构造
		doublyLinkedList(doublyLinkedList&& other) noexcept
		{
			length = other.length;
			head = other.head;
			tail = other.tail;
			sentinel = other.sentinel;

			// 为 other 创建新的哨兵节点，使其处于有效状态
			other.head = nullptr;
			other.tail = nullptr;
			other.sentinel = new Node(nullptr, nullptr, std::nullopt);
			other.length = 0;
		}

		// 拷贝赋值
		doublyLinkedList& operator=(const doublyLinkedList& other)
		{
			if (this != &other)
			{
				if (!isEmpty()) clear();
				deepCopy(other.head, other.length);
			}
			return *this;
		}

		// 移动赋值
		doublyLinkedList& operator=(doublyLinkedList&& other) noexcept
		{
			if (this != &other)
			{
				if (!isEmpty()) clear();
				delete sentinel;

				length = other.length;
				head = other.head;
				tail = other.tail;
				sentinel = other.sentinel;

				// 为 other 创建新的哨兵节点，使其处于有效状态
				other.head = nullptr;
				other.tail = nullptr;
				other.sentinel = new Node(nullptr, nullptr, std::nullopt);
				other.length = 0;
			}
			return *this;
		}

		// emplace_front()
		template<typename... Args>
		T& emplace_front(Args&&... args)
		{
			Node* newHead(new Node(nullptr, head, std::forward<Args>(args)...));
			if (head)
			{
				head->prev = newHead;
			}
			else
			{
				sentinel->prev = tail = newHead;
				tail->next = sentinel;
			}

			head = newHead;
			++length;
			return head->data.value();
		}

		// emplace_back()
		template<typename... Args>
		T& emplace_back(Args&&... args)
		{
			Node* newTail(new Node(tail, sentinel, std::forward<Args>(args)...));
			if (!head)
			{
				head = newTail;
			}
			if (tail)
			{
				tail->next = newTail;
			}
			tail = newTail;
			++length;
			sentinel->prev = newTail;
			return tail->data.value();
		}

		// push_front()
		template<typename U>
			requires std::convertible_to<U, T>
		T& push_front(U&& val)
		{
			emplace_front(std::forward<U>(val));
			return head->data.value();
		}

		// push_back()
		template<typename U>
			requires std::convertible_to<U, T>
		T& push_back(U&& val)
		{
			emplace_back(std::forward<U>(val));
			return tail->data.value();
		}

		// 查找指定元素 非const
		iterator find(const T& val)
		{
			for (auto it = begin(); it != end(); ++it)
			{
				if (val == (*it))
				{
					return it;
				}
			}
			return end();
		}

		// 查找指定元素 const
		const const_iterator cfind(const T& val) const
		{
			for (auto it = cbegin(); it != cend(); ++it)
			{
				if (val == (*it))
				{
					return it;
				}
			}
			return cend();
		}

		// front()
		T& front()
		{
			if (head)
				return head->data.value();
		}

		// back()
		T& back()
		{
			if (tail)
				return tail->data.value();
		}

		// 指定位置插入
		template <typename U>
			requires std::convertible_to<U, T>
		iterator insert(iterator pos, U&& val)
		{
			if (pos == begin())
			{
				emplace_front(std::forward<U>(val));
				return begin();
			}
			else if (pos == end())
			{
				emplace_back(std::forward<U>(val));
				return iterator(tail);
			}
			else
			{
				Node* newNode = new Node(pos.current->prev, pos.current, std::forward<U>(val));
				if (pos.current->prev)
				{
					pos.current->prev->next = newNode;
				}
				pos.current->prev = newNode;
				++length;
				return iterator(pos.current->prev);
			}
		}

		// 删除指定元素
		iterator erase(iterator pos)
		{
			if (pos == end() || isEmpty())
			{
				return end();
			}

			if (pos == begin())
			{
				// 删除头节点
				Node* temp = head;
				head = head->next;

				if (head)
				{
					head->prev = nullptr;
				}
				else
				{
					sentinel->prev = tail = nullptr;
				}
				delete temp;
				--length;
				if (length == 0)
				{
					tail = sentinel->prev = nullptr;
				}
				return begin();
			}
			else if (pos == iterator(tail))
			{
				// 删除尾节点
				Node* temp = tail;
				tail = tail->prev;
				if (tail)
				{
					tail->next = sentinel;
				}
				else
				{
					head = nullptr;
				}

				sentinel->prev = tail;
				iterator result = end();
				delete temp;
				--length;
				return result;
			}
			else
			{
				// 删除中间节点
				if (pos.current)
				{
					Node* toDelete = pos.current;
					Node* prevNode = toDelete->prev;
					Node* nextNode = toDelete->next;

					prevNode->next = nextNode;
					nextNode->prev = prevNode;

					iterator result(nextNode);
					delete toDelete;
					--length;
					return result;
				}
				else
				{
					return end();
				}
			}
		}
	};
}