#include "../include/UnionFind.hpp"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

// 测试基础并查集 basicUnionFind
void test_basic_union_find()
{
    std::cout << "测试 basicUnionFind..." << std::endl;

    // 1. 默认构造和基本操作
    Forest::basicUnionFind<> uf(5);
    assert(uf.elementCount() == 5);
    assert(uf.count() == 5);

    // 2. 查找操作
    for (std::size_t i = 0; i < 5; ++i)
    {
        assert(uf.find(i) == i);
    }

    // 3. 合并操作
    assert(uf.unite(0, 1) == true);  // 成功合并
    assert(uf.find(0) == uf.find(1));
    assert(uf.connected(0, 1) == true);
    assert(uf.count() == 4);

    // 4. 重复合并应返回false
    assert(uf.unite(0, 1) == false);
    assert(uf.count() == 4);  // 数量不变

    // 5. 链式合并
    assert(uf.unite(1, 2) == true);
    assert(uf.connected(0, 2) == true);
    assert(uf.count() == 3);

    // 6. 添加新元素
    std::size_t new_id = uf.add_element();
    assert(new_id == 5);
    assert(uf.elementCount() == 6);
    assert(uf.count() == 4);  // 新元素是一个独立的集合

    // 7. 批量添加元素
    uf.add_elements(3);
    assert(uf.elementCount() == 9);
    assert(uf.count() == 7);  // 5,6,7,8 各是一个新集合

    // 8. 获取根节点集合
    std::vector<std::size_t> roots = uf.get_roots();
    assert(roots.size() == uf.count());

    // 9. 容量管理
    uf.reserve(100);
    uf.shrink_to_fit();

    // 10. 清空操作
    uf.clear();
    assert(uf.elementCount() == 0);
    assert(uf.count() == 0);

    std::cout << "basicUnionFind 所有测试通过!" << std::endl;
}

// 测试异常情况
void test_basic_union_find_exceptions()
{
    std::cout << "测试 basicUnionFind 异常情况..." << std::endl;

    Forest::basicUnionFind<> uf(3);

    try
    {
        uf.find(10);  // 越界访问
        assert(false && "应抛出异常");
    }
    catch (const std::out_of_range& e)
    {
        assert(std::string(e.what()) == "Element not found in UnionFind");
    }

    std::cout << "异常测试通过!" << std::endl;
}

// 测试移动语义
void test_basic_union_find_move_semantics()
{
    std::cout << "测试 basicUnionFind 移动语义..." << std::endl;

    Forest::basicUnionFind<> uf1(5);
    uf1.unite(0, 1);
    uf1.unite(2, 3);

    std::size_t count_before_move = uf1.count();
    std::size_t element_count_before_move = uf1.elementCount();

    // 移动构造
    Forest::basicUnionFind<> uf2(std::move(uf1));
    assert(uf2.count() == count_before_move);
    assert(uf2.elementCount() == element_count_before_move);
    assert(uf2.connected(0, 1) == true);
    assert(uf2.connected(2, 3) == true);
    assert(uf1.count() == 0);  // 移动后源对象应为空

    // 移动赋值
    Forest::basicUnionFind<> uf3(2);
    uf3 = std::move(uf2);
    assert(uf3.count() == count_before_move);
    assert(uf3.elementCount() == element_count_before_move);

    std::cout << "移动语义测试通过!" << std::endl;
}

// 测试泛型并查集 UnionFind
void test_union_find_generic()
{
    std::cout << "测试泛型 UnionFind..." << std::endl;

    // 1. 构造和添加元素
    Forest::UnionFind<std::string> uf;
    assert(uf.elementCount() == 0);
    assert(uf.count() == 0);

    // 2. 添加字符串元素
    assert(uf.add_element("A") == true);
    assert(uf.add_element("B") == true);
    assert(uf.add_element("C") == true);
    assert(uf.elementCount() == 3);
    assert(uf.count() == 3);

    // 3. 重复添加应返回false
    assert(uf.add_element("A") == false);
    assert(uf.elementCount() == 3);

    // 4. 查找操作
    assert(uf.find("A") == "A");
    assert(uf.find("B") == "B");

    // 5. 合并操作
    assert(uf.unite("A", "B") == true);
    assert(uf.find("A") == uf.find("B"));
    assert(uf.connected("A", "B") == true);
    assert(uf.count() == 2);

    // 6. 链式合并
    assert(uf.unite("B", "C") == true);
    assert(uf.connected("A", "C") == true);
    assert(uf.count() == 1);

    // 7. 获取根节点集合
    std::vector<std::string> roots = uf.get_roots();
    assert(roots.size() == 1);

    // 8. 通过构造函数参数添加元素
    Forest::UnionFind<std::string> uf2;
    assert(uf2.add_element("Hello") == true);
    assert(uf2.add_element("World") == true);
    assert(uf2.elementCount() == 2);

    // 9. 容量管理
    uf.reserve(100);
    uf.shrink_to_fit();

    // 10. 清空操作
    uf.clear();
    assert(uf.elementCount() == 0);
    assert(uf.count() == 0);

    std::cout << "泛型 UnionFind 所有测试通过!" << std::endl;
}



// 测试泛型并查集的移动语义
void test_union_find_generic_move_semantics()
{
    std::cout << "测试泛型 UnionFind 移动语义..." << std::endl;

    Forest::UnionFind<std::string> uf1;
    uf1.add_element("X");
    uf1.add_element("Y");
    uf1.add_element("Z");
    uf1.unite("X", "Y");

    std::size_t count_before_move = uf1.count();
    std::size_t element_count_before_move = uf1.elementCount();

    // 移动构造
    Forest::UnionFind<std::string> uf2(std::move(uf1));
    assert(uf2.count() == count_before_move);
    assert(uf2.elementCount() == element_count_before_move);
    assert(uf2.connected("X", "Y") == true);

    // 移动赋值
    Forest::UnionFind<std::string> uf3;
    uf3 = std::move(uf2);
    assert(uf3.count() == count_before_move);
    assert(uf3.elementCount() == element_count_before_move);

    std::cout << "泛型移动语义测试通过!" << std::endl;
}

// 测试自定义类型（需可哈希）
struct Person
{
    std::string name;
    int age;

    bool operator==(const Person& other) const
    {
        return name == other.name && age == other.age;
    }
};

// 为Person提供hash特化
namespace std {
    template<>
    struct hash<Person>
    {
        std::size_t operator()(const Person& p) const
        {
            return std::hash<std::string>{}(p.name) ^ (std::hash<int>{}(p.age) << 1);
        }
    };
}

void test_union_find_custom_type()
{
    std::cout << "测试自定义类型 UnionFind..." << std::endl;

    Forest::UnionFind<Person> uf;

    Person p1{ "Alice", 25 };
    Person p2{ "Bob", 30 };
    Person p3{ "Charlie", 35 };

    assert(uf.add_element(p1) == true);
    assert(uf.add_element(p2) == true);
    assert(uf.add_element(p3) == true);

    assert(uf.unite(p1, p2) == true);
    assert(uf.connected(p1, p2) == true);
    assert(uf.connected(p1, p3) == false);

    std::cout << "自定义类型测试通过!" << std::endl;
}

int main()
{
    try
    {
        test_basic_union_find();
        std::cout << std::endl;

        test_basic_union_find_exceptions();
        std::cout << std::endl;

        test_basic_union_find_move_semantics();
        std::cout << std::endl;

        test_union_find_generic();
        std::cout << std::endl;

       
        std::cout << std::endl;

        test_union_find_generic_move_semantics();
        std::cout << std::endl;

        test_union_find_custom_type();
        std::cout << std::endl;

        std::cout << "所有测试用例通过!" << std::endl;
        return 0;

    }
    catch (const std::exception& e)
    {
        std::cerr << "测试失败，异常: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "测试失败，未知异常!" << std::endl;
        return 1;
    }
}