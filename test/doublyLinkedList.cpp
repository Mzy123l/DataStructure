#include <iostream>
#include <cassert>
#include <string>
#include <utility>
#include "../include/doublyLinkedList.hpp"

void test_basic_operations() {
	std::cout << "=== 测试基本操作 ===\n";

	DoublyLinkedList::doublyLinkedList<int> list;
	assert(list.isEmpty());
	assert(list.size() == 0);
	std::cout << "默认构造函数测试通过\n";

	// push_back
	list.push_back(1);
	assert(list.size() == 1);
	assert(list.front() == 1);
	assert(list.back() == 1);
	std::cout << "push_back(1) 测试通过\n";

	// push_front
	list.push_front(2);
	assert(list.size() == 2);
	assert(list.front() == 2);
	assert(list.back() == 1);
	std::cout << "push_front(2) 测试通过\n";

	// push_back
	list.push_back(3);
	assert(list.size() == 3);
	assert(list.front() == 2);
	assert(list.back() == 3);
	std::cout << "push_back(3) 测试通过\n";

	// 迭代器测试
	auto it = list.begin();
	assert(*it == 2);
	++it;
	assert(*it == 1);
	++it;
	assert(*it == 3);
	++it;
	assert(it == list.end());
	std::cout << "正向迭代器测试通过\n";

	// 反向遍历
	auto rit = list.end();
	--rit;
	assert(*rit == 3);
	--rit;
	assert(*rit == 1);
	--rit;
	assert(*rit == 2);
	std::cout << "反向迭代器测试通过\n";

	// 范围for循环
	int sum = 0;
	for (int num : list) {
		sum += num;
	}
	assert(sum == 6);
	std::cout << "范围for循环测试通过\n";
}

void test_emplace_operations() {
	std::cout << "\n=== 测试原地构造操作 ===\n";

	DoublyLinkedList::doublyLinkedList<std::string> str_list;

	// emplace_back
	str_list.emplace_back("Hello");
	assert(str_list.size() == 1);
	assert(str_list.front() == "Hello");
	std::cout << "emplace_back(\"Hello\") 测试通过\n";

	// emplace_front
	str_list.emplace_front("World");
	assert(str_list.size() == 2);
	assert(str_list.front() == "World");
	assert(str_list.back() == "Hello");
	std::cout << "emplace_front(\"World\") 测试通过\n";

	// 多参数emplace
	str_list.emplace_back(3, 'X');  // 构造 "XXX"
	assert(str_list.back() == "XXX");
	std::cout << "emplace_back(3, 'X') 测试通过\n";
}

void test_find_and_erase() {
	std::cout << "\n=== 测试查找和删除 ===\n";

	DoublyLinkedList::doublyLinkedList<int> list = { 1, 2, 3, 4, 5 };

	// find
	auto it = list.find(3);
	assert(it != list.end());
	assert(*it == 3);
	std::cout << "find(3) 测试通过\n";

	// 删除中间元素
	auto next_it = list.erase(it);
	assert(list.size() == 4);
	assert(*next_it == 4);
	std::cout << "erase(3) 测试通过\n";

	// 删除头部
	auto new_begin = list.erase(list.begin());
	assert(list.size() == 3);
	assert(*new_begin == 2);
	std::cout << "erase(begin()) 测试通过\n";

	// 删除尾部
	auto end_it = list.end();
	--end_it;  // 指向最后一个元素
	auto new_end = list.erase(end_it);
	assert(list.size() == 2);
	assert(new_end == list.end());
	std::cout << "erase(end()-1) 测试通过\n";

	// 查找不存在的元素
	auto not_found = list.find(100);
	assert(not_found == list.end());
	std::cout << "find(不存在的值) 测试通过\n";
}

void test_insert() {
	std::cout << "\n=== 测试插入操作 ===\n";

	DoublyLinkedList::doublyLinkedList<int> list = { 1, 2, 4, 5 };

	// 在指定位置插入
	auto it = list.begin();
	++it;  // 指向2
	++it;  // 指向4
	auto new_it = list.insert(it, 3);

	assert(list.size() == 5);
	assert(*new_it == 3);

	// 验证顺序
	int expected[] = { 1, 2, 3, 4, 5 };
	int i = 0;
	for (int num : list) {
		assert(num == expected[i++]);
	}
	std::cout << "insert(iterator, 3) 测试通过\n";

	// 在头部插入
	auto front_it = list.insert(list.begin(), 0);
	assert(list.front() == 0);
	assert(*front_it == 0);
	std::cout << "insert(begin(), 0) 测试通过\n";

	// 在尾部插入
	auto back_it = list.end();
	auto inserted_it = list.insert(back_it, 6);
	assert(list.back() == 6);
	assert(*inserted_it == 6);
	std::cout << "insert(end(), 6) 测试通过\n";
}

void test_copy_and_move() {
	std::cout << "\n=== 测试拷贝和移动语义 ===\n";

	DoublyLinkedList::doublyLinkedList<int> list1 = { 1, 2, 3 };

	// 拷贝构造
	DoublyLinkedList::doublyLinkedList<int> list2(list1);
	auto it = list2.begin();
	it = --list2.end();
	std::cout << (*it);
	assert(list2.size() == 3);
	assert(list2.front() == 1);
	assert(list2.back() == 3);
	std::cout << "拷贝构造函数测试通过\n";

	// 修改list1不应影响list2
	list1.erase(list1.begin());
	assert(list1.size() == 2);
	assert(list2.size() == 3);
	std::cout << "深拷贝测试通过\n";

	// 拷贝赋值
	DoublyLinkedList::doublyLinkedList<int> list3;
	list3 = list1;
	assert(list3.size() == 2);
	assert(list3.front() == 2);
	std::cout << "拷贝赋值运算符测试通过\n";

	// 移动构造
	DoublyLinkedList::doublyLinkedList<int> list4(std::move(list3));
	assert(list4.size() == 2);
	assert(list3.size() == 0);
	assert(list3.isEmpty());
	std::cout << "移动构造函数测试通过\n";

	// 移动赋值
	DoublyLinkedList::doublyLinkedList<int> list5 = { 4, 5, 6 };
	list5 = std::move(list4);
	assert(list5.size() == 2);
	assert(list4.size() == 0);
	assert(list4.isEmpty());
	std::cout << "移动赋值运算符测试通过\n";
}

void test_clear() {
	std::cout << "\n=== 测试清空操作 ===\n";

	DoublyLinkedList::doublyLinkedList<int> list = { 1, 2, 3, 4, 5 };
	assert(list.size() == 5);

	list.clear();
	assert(list.isEmpty());
	assert(list.size() == 0);
	assert(list.begin() == list.end());
	std::cout << "clear() 测试通过\n";

	// 清空后可以重新使用
	list.push_back(10);
	assert(list.size() == 1);
	assert(list.front() == 10);
	assert(list.back() == 10);
	std::cout << "清空后重新使用测试通过\n";
}

void test_initializer_list() {
	std::cout << "\n=== 测试初始化列表 ===\n";

	DoublyLinkedList::doublyLinkedList<int> list = { 10, 20, 30, 40 };
	assert(list.size() == 4);

	int expected[] = { 10, 20, 30, 40 };
	int i = 0;
	for (int num : list) {
		assert(num == expected[i++]);
	}
	std::cout << "初始化列表构造测试通过\n";
}

void test_const_iterators() {
	std::cout << "\n=== 测试const迭代器 ===\n";

	const DoublyLinkedList::doublyLinkedList<int> list = { 1, 2, 3, 4, 5 };

	auto cit = list.cbegin();
	assert(*cit == 1);
	++cit;
	assert(*cit == 2);

	int sum = 0;
	for (auto it = list.cbegin(); it != list.cend(); ++it) {
		sum += *it;
	}
	assert(sum == 15);
	std::cout << "const迭代器测试通过\n";

	// 从iterator构造const_iterator
	DoublyLinkedList::doublyLinkedList<int> non_const_list = { 1, 2, 3 };
	auto it = non_const_list.begin();
	DoublyLinkedList::doublyLinkedList<int>::const_iterator cit2 = it;
	assert(*cit2 == 1);
	std::cout << "iterator到const_iterator转换测试通过\n";
}

// 定义没有默认构造函数的测试类
class NoDefaultConstructor {
private:
	int value;
	std::string name;

public:
	// 删除默认构造函数
	NoDefaultConstructor() = delete;

	// 带参数的构造函数
	NoDefaultConstructor(int v, const std::string& n) : value(v), name(n) {}

	// 拷贝构造函数
	NoDefaultConstructor(const NoDefaultConstructor& other) = default;

	// 移动构造函数
	NoDefaultConstructor(NoDefaultConstructor&& other) = default;

	// 拷贝赋值运算符
	NoDefaultConstructor& operator=(const NoDefaultConstructor& other) = default;

	// 移动赋值运算符
	NoDefaultConstructor& operator=(NoDefaultConstructor&& other) = default;

	// 比较运算符
	bool operator==(const NoDefaultConstructor& other) const {
		return value == other.value && name == other.name;
	}

	bool operator!=(const NoDefaultConstructor& other) const {
		return !(*this == other);
	}

	// 获取值
	int getValue() const { return value; }
	const std::string& getName() const { return name; }

	// 用于输出
	friend std::ostream& operator<<(std::ostream& os, const NoDefaultConstructor& obj) {
		os << "NoDefaultConstructor{value=" << obj.value << ", name=\"" << obj.name << "\"}";
		return os;
	}
};

void test_no_default_constructor() {
	std::cout << "\n=== 测试无默认构造函数的类 ===" << std::endl;

	// 测试emplace_front和emplace_back
	{
		std::cout << "测试emplace_front和emplace_back..." << std::endl;
		DoublyLinkedList::doublyLinkedList<NoDefaultConstructor> list;

		// 使用emplace_back原地构造
		auto& back_ref = list.emplace_back(1, "First");
		assert(back_ref.getValue() == 1);
		assert(back_ref.getName() == "First");
		assert(list.size() == 1);
		std::cout << "emplace_back(1, \"First\") 测试通过" << std::endl;

		// 使用emplace_front原地构造
		auto& front_ref = list.emplace_front(2, "Second");
		assert(front_ref.getValue() == 2);
		assert(front_ref.getName() == "Second");
		assert(list.size() == 2);
		std::cout << "emplace_front(2, \"Second\") 测试通过" << std::endl;

		// 再添加一个
		list.emplace_back(3, "Third");
		assert(list.size() == 3);
		std::cout << "emplace_back(3, \"Third\") 测试通过" << std::endl;

		// 验证顺序
		auto it = list.begin();
		assert(it->getValue() == 2 && it->getName() == "Second");
		++it;
		assert(it->getValue() == 1 && it->getName() == "First");
		++it;
		assert(it->getValue() == 3 && it->getName() == "Third");
		std::cout << "顺序验证通过" << std::endl;
	}

	// 测试push_front和push_back
	{
		std::cout << "\n测试push_front和push_back..." << std::endl;
		DoublyLinkedList::doublyLinkedList<NoDefaultConstructor> list;

		// 创建对象
		NoDefaultConstructor obj1(10, "Obj1");
		NoDefaultConstructor obj2(20, "Obj2");
		NoDefaultConstructor obj3(30, "Obj3");

		// 使用push_back
		list.push_back(obj1);
		assert(list.size() == 1);
		assert(list.back().getValue() == 10);
		std::cout << "push_back(obj1) 测试通过" << std::endl;

		// 使用push_front
		list.push_front(obj2);
		assert(list.size() == 2);
		assert(list.front().getValue() == 20);
		std::cout << "push_front(obj2) 测试通过" << std::endl;

		// 再push_back一个
		list.push_back(obj3);
		assert(list.size() == 3);
		assert(list.back().getValue() == 30);
		std::cout << "push_back(obj3) 测试通过" << std::endl;

		// 验证顺序
		auto it = list.begin();
		assert(it->getValue() == 20);
		++it;
		assert(it->getValue() == 10);
		++it;
		assert(it->getValue() == 30);
		std::cout << "顺序验证通过" << std::endl;
	}

	// 测试插入操作
	{
		std::cout << "\n测试insert操作..." << std::endl;
		DoublyLinkedList::doublyLinkedList<NoDefaultConstructor> list;

		// 添加一些元素
		list.emplace_back(1, "A");
		list.emplace_back(2, "B");
		list.emplace_back(4, "D");

		// 在指定位置插入
		auto it = list.begin();
		++it; ++it; // 指向第三个元素(值为4)
		NoDefaultConstructor newObj(3, "C");
		auto inserted_it = list.insert(it, newObj);

		assert(list.size() == 4);
		assert(inserted_it->getValue() == 3);
		assert(inserted_it->getName() == "C");
		std::cout << "insert操作测试通过" << std::endl;

		// 验证顺序
		int expectedValues[] = { 1, 2, 3, 4 };
		int i = 0;
		for (const auto& obj : list) {
			assert(obj.getValue() == expectedValues[i++]);
		}
		std::cout << "插入后顺序验证通过" << std::endl;
	}

	// 测试拷贝构造函数
	{
		std::cout << "\n测试拷贝构造函数..." << std::endl;
		DoublyLinkedList::doublyLinkedList<NoDefaultConstructor> list1;
		list1.emplace_back(100, "One");
		list1.emplace_back(200, "Two");
		list1.emplace_back(300, "Three");

		// 拷贝构造
		DoublyLinkedList::doublyLinkedList<NoDefaultConstructor> list2(list1);
		assert(list1.size() == 3);
		assert(list2.size() == 3);

		// 验证内容相同
		auto it1 = list1.begin();
		auto it2 = list2.begin();
		for (; it1 != list1.end() && it2 != list2.end(); ++it1, ++it2) {
			assert(it1->getValue() == it2->getValue());
			assert(it1->getName() == it2->getName());
		}
		std::cout << "拷贝构造函数测试通过" << std::endl;

		// 修改原链表不影响拷贝
		list1.erase(list1.begin());
		assert(list1.size() == 2);
		assert(list2.size() == 3);
		std::cout << "深拷贝测试通过" << std::endl;
	}

	// 测试移动语义
	{
		std::cout << "\n测试移动语义..." << std::endl;
		DoublyLinkedList::doublyLinkedList<NoDefaultConstructor> list1;
		list1.emplace_back(1, "Move1");
		list1.emplace_back(2, "Move2");

		// 移动构造
		DoublyLinkedList::doublyLinkedList<NoDefaultConstructor> list2(std::move(list1));
		assert(list1.isEmpty());
		assert(list1.size() == 0);
		assert(list2.size() == 2);

		// 验证移动后的内容
		int i = 1;
		for (const auto& obj : list2) {
			assert(obj.getValue() == i);
			++i;
		}
		std::cout << "移动构造函数测试通过" << std::endl;

		// 移动赋值
		DoublyLinkedList::doublyLinkedList<NoDefaultConstructor> list3;
		list3 = std::move(list2);
		assert(list2.isEmpty());
		assert(list3.size() == 2);
		std::cout << "移动赋值运算符测试通过" << std::endl;
	}

	// 测试初始化列表
	{
		std::cout << "\n测试初始化列表..." << std::endl;
		// 由于NoDefaultConstructor没有默认构造函数，我们不能使用初始化列表构造函数
		// 因为初始化列表要求元素类型可默认构造
		// 这里我们只测试emplace
		std::cout << "注意: 无默认构造函数的类不能使用初始化列表构造" << std::endl;
	}

	// 测试查找
	{
		std::cout << "\n测试查找操作..." << std::endl;
		DoublyLinkedList::doublyLinkedList<NoDefaultConstructor> list;

		list.emplace_back(1, "Alice");
		list.emplace_back(2, "Bob");
		list.emplace_back(3, "Charlie");

		// 查找存在的元素
		NoDefaultConstructor target1(2, "Bob");
		auto it = list.find(target1);
		assert(it != list.end());
		assert(it->getValue() == 2);
		assert(it->getName() == "Bob");
		std::cout << "查找存在的元素测试通过" << std::endl;

		// 查找不存在的元素
		NoDefaultConstructor target2(4, "David");
		auto not_found = list.find(target2);
		assert(not_found == list.end());
		std::cout << "查找不存在的元素测试通过" << std::endl;
	}

	// 测试emplace的多参数
	{
		std::cout << "\n测试emplace多参数..." << std::endl;
		DoublyLinkedList::doublyLinkedList<NoDefaultConstructor> list;

		// 使用emplace_back，传递两个参数构造NoDefaultConstructor
		list.emplace_back(42, "The Answer");
		assert(list.size() == 1);
		assert(list.front().getValue() == 42);
		assert(list.front().getName() == "The Answer");
		std::cout << "emplace_back多参数测试通过" << std::endl;

		// 使用emplace_front，传递两个参数
		list.emplace_front(99, "Important");
		assert(list.size() == 2);
		assert(list.front().getValue() == 99);
		assert(list.front().getName() == "Important");
		std::cout << "emplace_front多参数测试通过" << std::endl;

		// 在中间插入
		auto it = list.begin();
		++it;
		list.insert(it, NoDefaultConstructor(50, "Middle"));
		assert(list.size() == 3);

		// 验证
		int i = 0;
		int expectedValues[] = { 99, 50, 42 };
		for (const auto& obj : list) {
			assert(obj.getValue() == expectedValues[i++]);
		}
		std::cout << "emplace多参数验证通过" << std::endl;
	}

	std::cout << "=== 无默认构造函数类测试完成 ===" << std::endl;
}

int main() {
	std::cout << "开始测试双向链表...\n";

	test_basic_operations();
	test_emplace_operations();
	test_find_and_erase();
	test_insert();
	test_copy_and_move();
	test_clear();
	test_initializer_list();
	test_const_iterators();
	test_no_default_constructor();

	std::cout << "\n🎉 所有测试通过！\n";
	return 0;
}