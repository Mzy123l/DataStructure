#include "../include/singlyLinkedList.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
using namespace DateStructure;
class TestObj {
public:
    int* data;
    explicit TestObj(int val) : data(new int(val)) {
        std::cout << "构造 TestObj(" << val << ")" << std::endl;
    }

    TestObj(const TestObj& other) : data(new int(*other.data)) {
        std::cout << "拷贝构造 TestObj(" << *data << ")" << std::endl;
    }

    TestObj(TestObj&& other) noexcept : data(other.data) {
        other.data = nullptr;
        std::cout << "移动构造 TestObj" << std::endl;
    }

    TestObj& operator=(const TestObj& other) {
        if (this != &other) {
            delete data;
            data = new int(*other.data);
        }
        std::cout << "拷贝赋值 TestObj" << std::endl;
        return *this;
    }

    TestObj& operator=(TestObj&& other) noexcept {
        if (this != &other) {
            delete data;
            data = other.data;
            other.data = nullptr;
        }
        std::cout << "移动赋值 TestObj" << std::endl;
        return *this;
    }

    ~TestObj() {
        std::cout << "析构 TestObj";
        if (data) {
            std::cout << "(data=" << *data << ")";
            delete data;
        }
        std::cout << std::endl;
    }

    bool operator==(const TestObj& other) const {
        return *data == *other.data;
    }

    friend std::ostream& operator<<(std::ostream& os, const TestObj& obj) {
        if (obj.data) os << *obj.data;
        else os << "null";
        return os;
    }
};
// 辅助打印函数
template<typename T>
void printList(const singlyLinkedList<T>& list) {
    std::cout << "List: ";
    for (const auto& val : list) {
        std::cout << val << " ";
    }
    std::cout << "(size: " << list.size() << ")" << std::endl;
}

int main() {
    std::cout << "=== 单向链表测试 ===" << std::endl;

    // 测试1: 默认构造函数和基本操作
    {
        std::cout << "\n测试1: 默认构造和基本操作" << std::endl;
        singlyLinkedList<int> list;

        std::cout << "空链表测试:" << std::endl;
        assert(list.isEmpty());
        assert(list.size() == 0);
        assert(list.begin() == list.end());
        printList(list);

        // 尾插(拷贝)
        list.push_back(1);
        list.push_back(2);
        list.push_back(3);
        std::cout << "尾插1,2,3后:" << std::endl;
        assert(list.size() == 3);
        assert(!list.isEmpty());
        printList(list);

        // 头插(拷贝)
        list.push_front(0);
        list.push_front(-1);
        std::cout << "头插0,-1后:" << std::endl;
        assert(list.size() == 5);
        printList(list);

        // 尾插(移动)
        int value = 100;
        list.push_back(std::move(value));
        std::cout << "移动尾插100后 (value可能被移动):" << std::endl;
        printList(list);

        // 头插(移动)
        int value2 = -100;
        list.push_front(std::move(value2));
        std::cout << "移动头插-100后:" << std::endl;
        printList(list);
    }

    // 测试2: 迭代器
    {
        std::cout << "\n测试2: 迭代器" << std::endl;
        singlyLinkedList<int> list;
        for (int i = 1; i <= 5; i++)
        {
            list.push_back(i);
        }
        std::cout << "基于范围的for循环:" << std::endl;
        for (const auto& val : list) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        // 前向迭代
        std::cout << "迭代器遍历: ";
        for (auto it = list.begin(); it != list.end(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;

        // 常量迭代器
        std::cout << "常量迭代器遍历: ";
        for (auto it = list.cbegin(); it != list.cend(); ++it) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;

        // 修改元素
        std::cout << "修改第一个元素为100: ";
        if (!list.isEmpty()) {
            *list.begin() = 100;
        }
        printList(list);

        // 迭代器有效性
        auto it = list.begin();
        ++it; ++it;
        assert(*it == 3);  // 应该指向第三个元素
    }

    // 测试3: 拷贝构造和拷贝赋值
    {
        std::cout << "\n测试3: 拷贝构造和拷贝赋值" << std::endl;

        // 创建原链表
        singlyLinkedList<int> original;
        for (int i = 1; i <= 5; ++i) {
            original.push_back(i * 10);
        }
        std::cout << "原链表: ";
        printList(original);

        // 拷贝构造
        singlyLinkedList<int> copy1(original);
        std::cout << "拷贝构造后的链表: ";
        printList(copy1);
        assert(copy1.size() == original.size());

        // 修改原链表，确保深拷贝
        original.push_back(60);
        std::cout << "修改原链表后:" << std::endl;
        std::cout << "原链表: ";
        printList(original);
        std::cout << "拷贝链表(应不变): ";
        printList(copy1);
        assert(copy1.size() == 5);
        assert(original.size() == 6);

        // 拷贝赋值
        singlyLinkedList<int> copy2;
        copy2.push_back(100);
        copy2.push_back(200);
        std::cout << "赋值前copy2: ";
        printList(copy2);

        copy2 = original;
        std::cout << "拷贝赋值后的copy2: ";
        printList(copy2);
        assert(copy2.size() == original.size());

        // 自赋值
        copy2 = copy2;
        std::cout << "自赋值后: ";
        printList(copy2);
        assert(copy2.size() == original.size());
    }

    // 测试4: 移动构造和移动赋值
    {
        std::cout << "\n测试4: 移动构造和移动赋值" << std::endl;

        // 创建临时链表
        singlyLinkedList<int> temp;
        for (int i = 1; i <= 5; ++i) {
            temp.push_back(i);
        }
        std::cout << "移动前的临时链表: ";
        printList(temp);

        // 移动构造
        singlyLinkedList<int> moved1(std::move(temp));
        std::cout << "移动构造后的链表: ";
        printList(moved1);
        std::cout << "移动后的临时链表(应为空): ";
        printList(temp);
        assert(temp.size() == 0);
        assert(temp.isEmpty());

        // 移动赋值
        singlyLinkedList<int> moved2;
        moved2.push_back(100);
        moved2.push_back(200);
        std::cout << "移动赋值前moved2: ";
        printList(moved2);

        moved2 = std::move(moved1);
        std::cout << "移动赋值后的moved2: ";
        printList(moved2);
        std::cout << "移动赋值后的moved1(应为空): ";
        printList(moved1);
        assert(moved1.isEmpty());
    }

    // 测试5: 查找
    {
        std::cout << "\n测试5: 查找" << std::endl;
        singlyLinkedList<int> list;
        for (int i = 1; i <= 5; i++)
        {
            list.push_back(i * 10);
        }
        printList(list);

        // 查找存在的元素
        auto it1 = list.find(30);
        assert(it1 != list.end());
        std::cout << "找到30: " << *it1 << std::endl;

        // 查找不存在的元素
        auto it2 = list.find(99);
        assert(it2 == list.end());
        std::cout << "查找99: 未找到(返回end)" << std::endl;

        // 常量查找
        const auto& constList = list;
        auto it3 = constList.cfind(20);
        assert(it3 != constList.cend());
        std::cout << "常量查找20: " << *it3 << std::endl;
    }

    // 测试6: 插入
    {
        std::cout << "\n测试6: 插入操作" << std::endl;
        singlyLinkedList<int> list;
        for (int i = 1; i <= 5; i++)
        {
            list.push_back(i);
        }
        std::cout << "初始链表: ";
        printList(list);

        // 在头部插入
        list.insert(list.begin(), 0);
        std::cout << "在头部插入0后: ";
        printList(list);
        assert(*list.begin() == 0);

        // 在中间插入(拷贝)
        auto it = list.begin();
        ++it; ++it;  // 指向第二个元素(原链表中的1)
        list.insert(it, 99);
        std::cout << "在第二个位置插入99后: ";
        printList(list);

        // 在中间插入(移动)
        int value = 88;
        it = list.begin();
        for (int i = 0; i < 4; ++i) ++it;  // 指向第五个位置
        list.insert(it, std::move(value));
        std::cout << "在第五个位置移动插入88后: ";
        printList(list);

        // 在末尾插入(应通过push_back处理)
        list.insert(list.end(), 100);
        std::cout << "在末尾插入100后: ";
        printList(list);

        // 在空链表插入
        singlyLinkedList<int> emptyList;
        emptyList.insert(emptyList.begin(), 999);
        std::cout << "空链表中插入999后: ";
        printList(emptyList);
    }

    // 测试7: 删除
    {
        std::cout << "\n测试7: 删除操作" << std::endl;
        singlyLinkedList<int> list;
        for (int i = 1; i <= 5; i++)
        {
            list.push_back(i);
        }
        std::cout << "初始链表: ";
        printList(list);

        // 删除头部
        auto it = list.begin();
        it = list.erase(it);
        std::cout << "删除头部后: ";
        printList(list);
        assert(list.size() == 4);
        if (it != list.end()) {
            std::cout << "返回的迭代器指向: " << *it << std::endl;
        }

        // 删除中间
        it = list.begin();
        ++it;  // 指向第二个元素(原链表中的3)
        it = list.erase(it);
        std::cout << "删除第二个元素后: ";
        printList(list);
        assert(list.size() == 3);

        // 删除尾部
        it = list.begin();
        ++it; ++it;  // 指向最后一个元素
        it = list.erase(it);
        std::cout << "删除尾部后: ";
        printList(list);
        assert(it == list.end());  // 删除尾部应返回end()
        assert(list.size() == 2);

        // 删除不存在的元素(尾后位置)
        it = list.erase(list.end());
        assert(it == list.end());
        std::cout << "尝试删除尾后位置: 无变化" << std::endl;
        printList(list);

        // 删除所有元素
        while (!list.isEmpty()) {
            list.erase(list.begin());
        }
        std::cout << "删除所有元素后: ";
        printList(list);
        assert(list.isEmpty());
    }

    // 测试8: 复杂类型和边界情况
    {
        std::cout << "\n测试8: 字符串类型和边界情况" << std::endl;
        singlyLinkedList<std::string> strList;

        strList.push_back("Hello");
        strList.push_back("World");
        strList.push_front("Start");
        std::cout << "字符串链表: ";
        for (const auto& s : strList) {
            std::cout << s << " ";
        }
        std::cout << std::endl;

        // 查找字符串
        auto it = strList.find("World");
        if (it != strList.end()) {
            std::cout << "找到: " << *it << std::endl;
        }

        // 在中间插入字符串
        it = strList.begin();
        ++it;
        strList.insert(it, "Middle");
        std::cout << "插入后: ";
        for (const auto& s : strList) {
            std::cout << s << " ";
        }
        std::cout << std::endl;
    }

    // 测试9: 大容量测试
    {
        std::cout << "\n测试9: 大容量操作" << std::endl;
        singlyLinkedList<int> bigList;
        const int N = 1000;

        for (int i = 0; i < N; ++i) {
            bigList.push_back(i);
        }
        std::cout << "插入" << N << "个元素后大小: " << bigList.size() << std::endl;
        assert(bigList.size() == N);

        // 验证数据
        int count = 0;
        for (auto it = bigList.begin(); it != bigList.end(); ++it, ++count) {
            assert(*it == count);
        }
        std::cout << "验证" << count << "个元素，全部正确" << std::endl;
    }

    // 测试10: 组合操作
    {
        std::cout << "\n测试10: 组合操作测试" << std::endl;
        singlyLinkedList<int> list;

        // 混合操作
        list.push_back(1);
        list.push_front(0);
        list.push_back(2);
        list.push_back(3);
        list.push_front(-1);

        std::cout << "初始: ";
        printList(list);

        // 删除第二个元素
        auto it = list.begin();
        ++it;
        list.erase(it);
        std::cout << "删除第二个元素后: ";
        printList(list);

        // 在第二个位置插入
        it = list.begin();
        ++it;
        list.insert(it, 99);
        std::cout << "插入99后: ";
        printList(list);

        // 拷贝构造
        auto copy = list;
        std::cout << "拷贝: ";
        printList(copy);

        // 修改原链表
        *list.begin() = 100;
        std::cout << "修改原链表第一个元素为100: ";
        printList(list);
        std::cout << "拷贝链表应不变: ";
        printList(copy);
    }

    std::cout << "\n=== 所有测试通过! ===" << std::endl;
    std::cout << "\n=== 新增10个测试 ===" << std::endl;

    // 测试11: 测试 clear() 函数
    {
        std::cout << "\n测试11: clear() 函数测试" << std::endl;
        singlyLinkedList<int> list;
        for (int i = 0; i < 1000; ++i) {
            list.push_back(i);
        }
        std::cout << "清空前大小: " << list.size() << std::endl;
        assert(list.size() == 1000);
        assert(!list.isEmpty());

        // 调用 clear()
        list.clear();
        std::cout << "清空后大小: " << list.size() << std::endl;
        assert(list.size() == 0);
        assert(list.isEmpty());
        assert(list.begin() == list.end());

        // 清空后能否正常使用
        list.push_back(1);
        list.push_back(2);
        std::cout << "清空后重新插入元素: ";
        printList(list);
        assert(list.size() == 2);
    }

    // 测试12: 自拷贝和自移动
    {
        std::cout << "\n测试12: 自赋值测试" << std::endl;
        singlyLinkedList<int> list;
        for (int i = 1; i <= 5; ++i) {
            list.push_back(i);
        }

        // 自拷贝赋值
        auto& list_ref = list;
        list = list_ref;
        std::cout << "自拷贝赋值后: ";
        printList(list);
        assert(list.size() == 5);

        // 自移动赋值
        list = std::move(list);
        std::cout << "自移动赋值后: ";
        printList(list);
        assert(list.size() == 5);

        // 拷贝构造自引用
        singlyLinkedList<int> list2(list);
        std::cout << "拷贝构造后: ";
        printList(list2);
        assert(list2.size() == 5);
    }

    // 测试13: 边界条件 - 在单个元素链表上操作
    {
        std::cout << "\n测试13: 单元素链表边界测试" << std::endl;
        singlyLinkedList<int> list;
        list.push_back(42);

        // 测试迭代器
        auto it = list.begin();
        assert(*it == 42);
        ++it;
        assert(it == list.end());

        // 在头部插入
        list.push_front(21);
        std::cout << "头部插入后: ";
        printList(list);
        assert(list.size() == 2);

        // 在头部删除
        list.erase(list.begin());
        std::cout << "头部删除后: ";
        printList(list);
        assert(list.size() == 1);

        // 删除最后一个元素
        list.erase(list.begin());
        std::cout << "删除所有元素后: ";
        printList(list);
        assert(list.isEmpty());

        // 在空链表上插入
        list.insert(list.begin(), 100);
        std::cout << "空链表插入: ";
        printList(list);
        assert(list.size() == 1);
    }

    // 测试14: 复杂对象的移动语义
    {
        std::cout << "\n测试14: 复杂对象移动语义测试" << std::endl;
      

        std::cout << "创建 TestObj 链表..." << std::endl;
        singlyLinkedList<TestObj> list;
        list.push_back(TestObj(1));
        list.push_back(TestObj(2));
        list.push_front(TestObj(0));

        std::cout << "当前链表: ";
        for (const auto& obj : list) {
            std::cout << obj << " ";
        }
        std::cout << std::endl;

        // 移动构造
        singlyLinkedList<TestObj> moved_list(std::move(list));
        std::cout << "移动构造后，原链表大小: " << list.size()
            << ", 新链表大小: " << moved_list.size() << std::endl;
    }

    // 测试15: 迭代器失效测试
    {
        std::cout << "\n测试15: 迭代器失效测试" << std::endl;
        singlyLinkedList<int> list;
        for (int i = 1; i <= 5; ++i) {
            list.push_back(i * 10);
        }

        std::cout << "初始链表: ";
        printList(list);

        // 获取中间迭代器
        auto it = list.begin();
        ++it; ++it;  // 指向第三个元素(30)
        std::cout << "获取迭代器指向: " << *it << std::endl;

        // 在迭代器前插入元素
        auto insert_it = list.begin();
        ++insert_it;  // 指向第二个元素
        list.insert(insert_it, 25);
        std::cout << "在第二个位置插入25后: ";
        printList(list);

        // 原迭代器应失效，但这里我们设计允许继续使用
        std::cout << "原迭代器现在指向: " << *it << std::endl;

        // 删除元素
        auto erase_it = list.begin();
        ++erase_it;  // 指向第二个元素(25)
        list.erase(erase_it);
        std::cout << "删除第二个元素后: ";
        printList(list);
    }

    // 测试16: 随机访问模式(虽然链表不支持随机访问，但可以模拟)
    {
        std::cout << "\n测试16: 随机插入/删除测试" << std::endl;
        singlyLinkedList<int> list;
        std::mt19937 rng(42);  // 固定种子，可重复
        std::uniform_int_distribution<int> dist(1, 100);

        // 随机插入
        for (int i = 0; i < 20; ++i) {
            int val = dist(rng);
            if (list.isEmpty() || dist(rng) % 2 == 0) {
                list.push_back(val);
            }
            else {
                list.push_front(val);
            }
        }
        std::cout << "随机插入20个元素后: ";
        printList(list);

        // 随机查找和删除
        int target = 50;  // 查找接近中位数的值
        auto it = list.find(target);
        if (it != list.end()) {
            std::cout << "找到" << target << "，删除它" << std::endl;
            list.erase(it);
        }
        else {
            std::cout << "未找到" << target << std::endl;
        }

        std::cout << "删除后链表: ";
        printList(list);
    }

    // 测试17: 混合操作压力测试
    {
        std::cout << "\n测试17: 混合操作压力测试" << std::endl;
        singlyLinkedList<int> list;
        const int NUM_OPERATIONS = 10000;
        int expected_size = 0;

        for (int i = 0; i < NUM_OPERATIONS; ++i) {
            int op = i % 10;
            if (op < 4) {  // 40% push_back
                list.push_back(i);
                ++expected_size;
            }
            else if (op < 8) {  // 40% push_front
                list.push_front(i);
                ++expected_size;
            }
            else if (op < 9 && !list.isEmpty()) {  // 10% erase
                list.erase(list.begin());
                --expected_size;
            }
            else {  // 10% insert
                if (!list.isEmpty()) {
                    auto it = list.begin();
                    for (int j = 0; j < list.size() / 2; ++j) ++it;
                    list.insert(it, i);
                    ++expected_size;
                }
            }

            // 定期检查size
            if (i % 1000 == 0) {
                assert(list.size() == static_cast<size_t>(expected_size));
            }
        }

        std::cout << "完成" << NUM_OPERATIONS << "次混合操作后大小: "
            << list.size() << std::endl;
        assert(list.size() == static_cast<size_t>(expected_size));
    }

    // 测试18: 拷贝语义完整性
    {
        std::cout << "\n测试18: 深拷贝完整性测试" << std::endl;
        singlyLinkedList<std::vector<int>> listOfVectors;

        // 创建包含vector的链表
        for (int i = 0; i < 5; ++i) {
            std::vector<int> vec = { i, i * 2, i * 3 };
            listOfVectors.push_back(vec);
        }

        // 深拷贝
        singlyLinkedList<std::vector<int>> copy = listOfVectors;

        // 修改原链表中的一个vector
        if (!listOfVectors.isEmpty()) {
            // 获取第二个元素的迭代器
            auto it = listOfVectors.begin();
            ++it;  // 移动到第二个元素

            if (it != listOfVectors.end()) {
                it->push_back(999);  // 修改原链表的第二个元素
            }
        }

        std::cout << "原链表大小: " << listOfVectors.size()
            << ", 拷贝链表大小: " << copy.size() << std::endl;

        // 验证深拷贝 - 通过值验证而非引用验证
        auto it_orig = listOfVectors.begin();
        auto it_copy = copy.begin();

        if (it_orig != listOfVectors.end() && it_copy != copy.end()) {
            // 只比较第一个元素的大小
            std::cout << "原链表第一个vector大小: " << it_orig->size() << std::endl;
            std::cout << "拷贝链表第一个vector大小: " << it_copy->size() << std::endl;

            // 由于修改的是第二个元素，第一个元素应该相同
            if (listOfVectors.size() > 0 && copy.size() > 0) {
                // 获取第二个元素的迭代器进行比较
                auto it_orig2 = listOfVectors.begin();
                auto it_copy2 = copy.begin();
                ++it_orig2;
                ++it_copy2;

                if (it_orig2 != listOfVectors.end() && it_copy2 != copy.end()) {
                    std::cout << "原链表第二个vector大小: " << it_orig2->size() << std::endl;
                    std::cout << "拷贝链表第二个vector大小: " << it_copy2->size() << std::endl;
                    // 修改后，原链表的第二个vector应该比拷贝的大1
                    assert(it_orig2->size() == it_copy2->size() + 1);
                }
            }
        }

        // 更安全的验证：遍历并比较所有元素
        std::cout << "遍历验证深拷贝: " << std::endl;
        auto it1 = listOfVectors.begin();
        auto it2 = copy.begin();
        int count = 0;

        while (it1 != listOfVectors.end() && it2 != copy.end()) {
            std::cout << "元素 " << count << ": ";
            if (count == 1) {
                std::cout << "原链表vector大小=" << it1->size()
                    << ", 拷贝链表vector大小=" << it2->size();
                assert(it1->size() != it2->size());
            }
            else {
                // 其他元素应该相同
                assert(it1->size() == it2->size());
            }
            std::cout << std::endl;
            ++it1;
            ++it2;
            ++count;
        }
    }

    // 测试19: 异常安全 - 基本保证
    {
        std::cout << "\n测试19: 异常安全测试" << std::endl;

        // 测试拷贝构造的异常安全
        try {
            singlyLinkedList<int> list1;
            for (int i = 0; i < 100; ++i) {
                list1.push_back(i);
            }

            // 正常情况下应该能成功拷贝
            singlyLinkedList<int> list2 = list1;
            std::cout << "拷贝构造成功，大小: " << list2.size() << std::endl;
            assert(list2.size() == 100);

        }
        catch (const std::exception& e) {
            std::cout << "拷贝构造异常: " << e.what() << std::endl;
        }

        // 测试移动构造(应保证不抛异常)
        singlyLinkedList<int> list3;
        for (int i = 0; i < 50; ++i) {
            list3.push_back(i);
        }

        singlyLinkedList<int> list4(std::move(list3));
        std::cout << "移动构造成功，原大小: " << list3.size()
            << ", 新大小: " << list4.size() << std::endl;
        assert(list3.isEmpty());
        assert(list4.size() == 50);
    }

    // 测试20: 性能基准测试(不抛异常)
    {
        std::cout << "\n测试20: 性能基准测试" << std::endl;
        const int BIG_SIZE = 10000;

        // 测试大容量插入
        singlyLinkedList<int> bigList;
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < BIG_SIZE; ++i) {
            bigList.push_back(i);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "插入" << BIG_SIZE << "个元素耗时: "
            << duration.count() << "ms" << std::endl;
        assert(bigList.size() == BIG_SIZE);

        // 测试遍历性能
        start = std::chrono::high_resolution_clock::now();
        int sum = 0;
        for (const auto& val : bigList) {
            sum += val;
        }
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "遍历" << BIG_SIZE << "个元素求和耗时: "
            << duration.count() << "ms" << std::endl;
        std::cout << "求和结果: " << sum << std::endl;
        assert(sum == (BIG_SIZE - 1) * BIG_SIZE / 2);

        // 测试拷贝性能
        start = std::chrono::high_resolution_clock::now();
        singlyLinkedList<int> copy = bigList;
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "拷贝" << BIG_SIZE << "个元素耗时: "
            << duration.count() << "ms" << std::endl;
        assert(copy.size() == BIG_SIZE);

        // 测试清空性能
        start = std::chrono::high_resolution_clock::now();
        bigList.clear();
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "清空" << BIG_SIZE << "个元素耗时: "
            << duration.count() << "ms" << std::endl;
        assert(bigList.isEmpty());
    }

    std::cout << "\n=== 所有新增测试通过! ===" << std::endl;
    // 最终完整性测试
    {
        std::cout << "\n=== 最终完整性检查 ===" << std::endl;

        // 检查是否所有资源都被正确释放
        {
            singlyLinkedList<std::string> finalTest;
            for (int i = 0; i < 100; ++i) {
                finalTest.push_back("String number " + std::to_string(i));
            }

            // 自动析构应该没有内存泄漏
            std::cout << "创建并销毁100个字符串元素的链表 - 应无内存泄漏" << std::endl;
        }

        // 检查自包含性
        {
            singlyLinkedList<singlyLinkedList<int>> nestedList;
            singlyLinkedList<int> innerList;
            innerList.push_back(1);
            innerList.push_back(2);
            nestedList.push_back(innerList);

            std::cout << "嵌套链表测试 - 自包含数据结构正常" << std::endl;
        }

        std::cout << "=== 所有测试和检查完成！链表实现稳定可靠 ===" << std::endl;
    }
    return 0;
}