#include <gtest/gtest.h>
#include <string>
#include <utility>

#include "../include/singlyLinkedList.hpp"

using namespace DateStructure;

// ============================================================
// 1. 构造测试 (Construction)
// ============================================================

TEST(SinglyLinkedListTest, DefaultConstructor)
{
    singlyLinkedList<int> list;
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

TEST(SinglyLinkedListTest, CopyConstructor)
{
    singlyLinkedList<int> list1;
    list1.push_back(1);
    list1.push_back(2);
    list1.push_back(3);

    singlyLinkedList<int> list2(list1);
    EXPECT_EQ(list2.size(), 3);
    EXPECT_FALSE(list2.isEmpty());

    // 验证拷贝元素正确
    auto it = list2.begin();
    EXPECT_EQ(*it++, 1);
    EXPECT_EQ(*it++, 2);
    EXPECT_EQ(*it++, 3);
    EXPECT_EQ(it, list2.end());
}

TEST(SinglyLinkedListTest, CopyConstructorEmpty)
{
    singlyLinkedList<int> list1;
    singlyLinkedList<int> list2(list1);
    EXPECT_TRUE(list2.isEmpty());
    EXPECT_EQ(list2.size(), 0);
}

TEST(SinglyLinkedListTest, MoveConstructor)
{
    singlyLinkedList<int> list1;
    list1.push_back(10);
    list1.push_back(20);
    list1.push_back(30);

    singlyLinkedList<int> list2(std::move(list1));
    EXPECT_EQ(list2.size(), 3);
    EXPECT_FALSE(list2.isEmpty());

    // 移动后原链表应为空
    EXPECT_TRUE(list1.isEmpty());
    EXPECT_EQ(list1.size(), 0);
    EXPECT_EQ(list1.begin(), list1.end());

    // 验证元素正确移动
    auto it = list2.begin();
    EXPECT_EQ(*it++, 10);
    EXPECT_EQ(*it++, 20);
    EXPECT_EQ(*it++, 30);
}

TEST(SinglyLinkedListTest, MoveConstructorEmpty)
{
    singlyLinkedList<int> list1;
    singlyLinkedList<int> list2(std::move(list1));
    EXPECT_TRUE(list2.isEmpty());
    EXPECT_EQ(list2.size(), 0);
    EXPECT_TRUE(list1.isEmpty());
}

TEST(SinglyLinkedListTest, CopyAssignment)
{
    singlyLinkedList<int> list1;
    list1.push_back(100);
    list1.push_back(200);

    singlyLinkedList<int> list2;
    list2.push_back(999);  // 预填数据，验证被完全覆盖

    list2 = list1;
    EXPECT_EQ(list2.size(), 2);
    EXPECT_EQ(list1.size(), 2);

    auto it = list2.begin();
    EXPECT_EQ(*it++, 100);
    EXPECT_EQ(*it++, 200);
}

TEST(SinglyLinkedListTest, CopyAssignmentSelf)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    // 自赋值不应破坏数据
    list = list;
    EXPECT_EQ(list.size(), 2);
    auto it = list.begin();
    EXPECT_EQ(*it++, 1);
    EXPECT_EQ(*it++, 2);
}

TEST(SinglyLinkedListTest, MoveAssignment)
{
    singlyLinkedList<int> list1;
    list1.push_back(100);
    list1.push_back(200);

    singlyLinkedList<int> list2;
    list2.push_back(999);

    list2 = std::move(list1);
    EXPECT_EQ(list2.size(), 2);
    EXPECT_TRUE(list1.isEmpty());
    EXPECT_EQ(list1.size(), 0);

    auto it = list2.begin();
    EXPECT_EQ(*it++, 100);
    EXPECT_EQ(*it++, 200);
}

TEST(SinglyLinkedListTest, MoveAssignmentSelf)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    // 自移动赋值不应破坏数据（because of this != &other check）
    list = std::move(list);
    EXPECT_EQ(list.size(), 2);
    auto it = list.begin();
    EXPECT_EQ(*it++, 1);
    EXPECT_EQ(*it++, 2);
}

// ============================================================
// 2. 插入操作测试 (Push Operations)
// ============================================================

TEST(SinglyLinkedListTest, PushBack)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    EXPECT_EQ(list.size(), 1);
    EXPECT_FALSE(list.isEmpty());
    EXPECT_EQ(*list.begin(), 1);

    list.push_back(2);
    list.push_back(3);
    EXPECT_EQ(list.size(), 3);

    auto it = list.begin();
    EXPECT_EQ(*it++, 1);
    EXPECT_EQ(*it++, 2);
    EXPECT_EQ(*it++, 3);
}

TEST(SinglyLinkedListTest, PushBackMove)
{
    singlyLinkedList<std::string> list;
    std::string s = "hello";
    list.push_back(std::move(s));
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(*list.begin(), "hello");
    EXPECT_TRUE(s.empty());
}

TEST(SinglyLinkedListTest, PushFront)
{
    singlyLinkedList<int> list;
    list.push_front(1);
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(*list.begin(), 1);

    list.push_front(2);
    list.push_front(3);
    EXPECT_EQ(list.size(), 3);

    // push_front 逆序添加，结果应为 3, 2, 1
    auto it = list.begin();
    EXPECT_EQ(*it++, 3);
    EXPECT_EQ(*it++, 2);
    EXPECT_EQ(*it++, 1);
}

TEST(SinglyLinkedListTest, PushFrontMove)
{
    singlyLinkedList<std::string> list;
    std::string s = "world";
    list.push_front(std::move(s));
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(*list.begin(), "world");
    // 注意：std::move 后的 string 状态未指定，不检查 s.empty()
}

TEST(SinglyLinkedListTest, PushBackAndPushFrontInterleaved)
{
    singlyLinkedList<int> list;
    list.push_back(2);   // [2]
    list.push_front(1);  // [1, 2]
    list.push_back(3);   // [1, 2, 3]
    list.push_front(0);  // [0, 1, 2, 3]

    EXPECT_EQ(list.size(), 4);
    auto it = list.begin();
    EXPECT_EQ(*it++, 0);
    EXPECT_EQ(*it++, 1);
    EXPECT_EQ(*it++, 2);
    EXPECT_EQ(*it++, 3);
}

TEST(SinglyLinkedListTest, PushBackLargeCount)
{
    singlyLinkedList<int> list;
    const int N = 10000;
    for (int i = 0; i < N; ++i)
    {
        list.push_back(i);
    }
    EXPECT_EQ(list.size(), N);

    // 验证所有元素
    auto it = list.begin();
    for (int i = 0; i < N; ++i, ++it)
    {
        EXPECT_EQ(*it, i);
    }
    EXPECT_EQ(it, list.end());
}

TEST(SinglyLinkedListTest, PushFrontLargeCount)
{
    singlyLinkedList<int> list;
    const int N = 10000;
    for (int i = 0; i < N; ++i)
    {
        list.push_front(i);
    }
    EXPECT_EQ(list.size(), N);

    // push_front 逐个添加 → 最终顺序为逆序
    auto it = list.begin();
    for (int i = N - 1; i >= 0; --i, ++it)
    {
        EXPECT_EQ(*it, i);
    }
    EXPECT_EQ(it, list.end());
}

// ============================================================
// 3. 弹出 / 删除操作测试（erase 模拟 pop_front）
//    注意：singlyLinkedList 没有独立的 pop_front，
//    使用 erase(begin()) 达到相同效果。
// ============================================================

TEST(SinglyLinkedListTest, EraseFrontSimulatesPopFront)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    // erase(begin()) 等价于 pop_front
    auto it = list.erase(list.begin());
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(*it, 2);
    EXPECT_EQ(*list.begin(), 2);

    it = list.erase(list.begin());
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(*it, 3);
}

TEST(SinglyLinkedListTest, EraseFrontOnEmpty)
{
    singlyLinkedList<int> list;
    auto it = list.erase(list.begin());  // 对空链表 pop_front 应为安全操作
    EXPECT_EQ(it, list.end());
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

TEST(SinglyLinkedListTest, EraseOnlyElement)
{
    singlyLinkedList<int> list;
    list.push_back(42);

    auto it = list.erase(list.begin());
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
    EXPECT_EQ(it, list.end());
}

TEST(SinglyLinkedListTest, EraseAtEnd)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    // 删除 end() 位置无效，链表不变
    auto it = list.erase(list.end());
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(it, list.end());
}

TEST(SinglyLinkedListTest, EraseMiddle)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    auto pos = list.begin();
    ++pos;  // 指向 2

    auto it = list.erase(pos);
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(*it, 3);  // 返回被删元素的后继

    // 验证链表为 [1, 3]
    auto check = list.begin();
    EXPECT_EQ(*check++, 1);
    EXPECT_EQ(*check++, 3);
}

TEST(SinglyLinkedListTest, EraseLastElement)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    // 指向最后一个元素
    auto pos = list.begin();
    ++pos;
    ++pos;  // 指向 3

    auto it = list.erase(pos);
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(it, list.end());  // 删除尾元素后返回 end()

    auto check = list.begin();
    EXPECT_EQ(*check++, 1);
    EXPECT_EQ(*check++, 2);
}

TEST(SinglyLinkedListTest, EraseAfterErase)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    // 连续删除
    auto it = list.erase(list.begin());
    EXPECT_EQ(*it, 2);
    it = list.erase(it);
    EXPECT_EQ(*it, 3);
    it = list.erase(it);
    EXPECT_EQ(it, list.end());
    EXPECT_TRUE(list.isEmpty());
}

// ============================================================
// 4. Insert 测试
//    注意：insert 在指定位置之前插入，而非之后。
// ============================================================

TEST(SinglyLinkedListTest, InsertAtBegin)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    auto it = list.insert(list.begin(), 0);
    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(*it, 0);

    auto check = list.begin();
    EXPECT_EQ(*check++, 0);
    EXPECT_EQ(*check++, 1);
    EXPECT_EQ(*check++, 2);
}

TEST(SinglyLinkedListTest, InsertAtMiddle)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(3);

    auto pos = list.begin();
    ++pos;  // 指向 3，在 3 前插入 2

    auto it = list.insert(pos, 2);
    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(*it, 2);

    auto check = list.begin();
    EXPECT_EQ(*check++, 1);
    EXPECT_EQ(*check++, 2);
    EXPECT_EQ(*check++, 3);
}

TEST(SinglyLinkedListTest, InsertAtEnd)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    // 在 end() 前插入 ≈ push_back
    auto it = list.insert(list.end(), 3);
    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(*it, 3);

    auto check = list.begin();
    EXPECT_EQ(*check++, 1);
    EXPECT_EQ(*check++, 2);
    EXPECT_EQ(*check++, 3);
}

TEST(SinglyLinkedListTest, InsertAtEndEmpty)
{
    singlyLinkedList<int> list;
    auto it = list.insert(list.end(), 42);
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(*it, 42);
}

TEST(SinglyLinkedListTest, InsertMove)
{
    singlyLinkedList<std::string> list;
    list.push_back("a");
    list.push_back("c");

    std::string s = "b";
    auto pos = list.begin();
    ++pos;
    list.insert(pos, std::move(s));
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(list.size(), 3);

    auto check = list.begin();
    EXPECT_EQ(*check++, "a");
    EXPECT_EQ(*check++, "b");
    EXPECT_EQ(*check++, "c");
}

TEST(SinglyLinkedListTest, InsertIntoEmptyList)
{
    singlyLinkedList<int> list;
    auto it = list.insert(list.begin(), 42);
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(*it, 42);
    EXPECT_FALSE(list.isEmpty());
}

// ============================================================
// 5. Find 测试
// ============================================================

TEST(SinglyLinkedListTest, FindExisting)
{
    singlyLinkedList<int> list;
    list.push_back(10);
    list.push_back(20);
    list.push_back(30);
    list.push_back(40);

    auto it = list.find(20);
    EXPECT_NE(it, list.end());
    EXPECT_EQ(*it, 20);

    // 找到后可继续迭代
    ++it;
    EXPECT_EQ(*it, 30);
}

TEST(SinglyLinkedListTest, FindFirstElement)
{
    singlyLinkedList<int> list;
    list.push_back(10);
    list.push_back(20);

    auto it = list.find(10);
    EXPECT_NE(it, list.end());
    EXPECT_EQ(*it, 10);
    EXPECT_EQ(it, list.begin());
}

TEST(SinglyLinkedListTest, FindLastElement)
{
    singlyLinkedList<int> list;
    list.push_back(10);
    list.push_back(20);
    list.push_back(30);

    auto it = list.find(30);
    EXPECT_NE(it, list.end());
    EXPECT_EQ(*it, 30);

    // 尾元素++后应等于 end()
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST(SinglyLinkedListTest, FindNonexistent)
{
    singlyLinkedList<int> list;
    list.push_back(10);
    list.push_back(20);

    auto it = list.find(99);
    EXPECT_EQ(it, list.end());
}

TEST(SinglyLinkedListTest, FindInEmptyList)
{
    singlyLinkedList<int> list;
    auto it = list.find(42);
    EXPECT_EQ(it, list.end());
}

TEST(SinglyLinkedListTest, CFindExisting)
{
    const singlyLinkedList<int> list = []()
    {
        singlyLinkedList<int> l;
        l.push_back(10);
        l.push_back(20);
        l.push_back(30);
        return l;
    }();

    auto it = list.cfind(20);
    EXPECT_NE(it, list.cend());
    EXPECT_EQ(*it, 20);
}

TEST(SinglyLinkedListTest, CFindNonexistent)
{
    const singlyLinkedList<int> list = []()
    {
        singlyLinkedList<int> l;
        l.push_back(10);
        return l;
    }();

    auto it = list.cfind(99);
    EXPECT_EQ(it, list.cend());
}

// ============================================================
// 6. 迭代器测试 (Iterator)
// ============================================================

TEST(SinglyLinkedListTest, ForwardIteration)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    list.push_back(4);
    list.push_back(5);

    int expected = 1;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
    EXPECT_EQ(expected, 6);
}

TEST(SinglyLinkedListTest, RangeForLoop)
{
    singlyLinkedList<int> list;
    list.push_back(10);
    list.push_back(20);
    list.push_back(30);

    int expected = 10;
    for (const auto& val : list)
    {
        EXPECT_EQ(val, expected);
        expected += 10;
    }
    EXPECT_EQ(expected, 40);
}

TEST(SinglyLinkedListTest, IteratorEmptyList)
{
    singlyLinkedList<int> list;
    EXPECT_EQ(list.begin(), list.end());

    int count = 0;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        ++count;
    }
    EXPECT_EQ(count, 0);
}

TEST(SinglyLinkedListTest, ConstIteration)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    const auto& clist = list;
    int expected = 1;
    for (auto it = clist.begin(); it != clist.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

TEST(SinglyLinkedListTest, CbeginCend)
{
    singlyLinkedList<int> list;
    list.push_back(5);
    list.push_back(10);

    const auto& clist = list;
    auto it = clist.cbegin();
    EXPECT_EQ(*it, 5);
    ++it;
    EXPECT_EQ(*it, 10);
    ++it;
    EXPECT_EQ(it, clist.cend());
}

TEST(SinglyLinkedListTest, PostfixIncrement)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    auto it = list.begin();
    auto old = it++;
    EXPECT_EQ(*old, 1);
    EXPECT_EQ(*it, 2);
}

TEST(SinglyLinkedListTest, IteratorArrowOperator)
{
    singlyLinkedList<std::string> list;
    list.push_back("hello");

    auto it = list.begin();
    EXPECT_EQ(it->length(), 5);
    EXPECT_EQ(it->size(), 5);
}

TEST(SinglyLinkedListTest, IteratorEquality)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    auto it1 = list.begin();
    auto it2 = list.begin();
    EXPECT_EQ(it1, it2);

    ++it2;
    EXPECT_NE(it1, it2);

    ++it1;
    EXPECT_EQ(it1, it2);

    ++it1;
    ++it2;
    EXPECT_EQ(it1, list.end());
    EXPECT_EQ(it2, list.end());
    EXPECT_EQ(it1, it2);
}

TEST(SinglyLinkedListTest, IteratorIsValid)
{
    singlyLinkedList<int> list;
    EXPECT_FALSE(list.begin().is_valid());

    list.push_back(1);
    EXPECT_TRUE(list.begin().is_valid());
    EXPECT_FALSE(list.end().is_valid());
}

TEST(SinglyLinkedListTest, IteratorBoolConversion)
{
    singlyLinkedList<int> list;
    EXPECT_FALSE(static_cast<bool>(list.begin()));

    list.push_back(42);
    EXPECT_TRUE(static_cast<bool>(list.begin()));
    EXPECT_FALSE(static_cast<bool>(list.end()));
}

// ============================================================
// 7. Size 测试
// ============================================================

TEST(SinglyLinkedListTest, SizeInitial)
{
    singlyLinkedList<int> list;
    EXPECT_EQ(list.size(), 0);
}

TEST(SinglyLinkedListTest, SizeAfterPushBack)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    EXPECT_EQ(list.size(), 1);
    list.push_back(2);
    EXPECT_EQ(list.size(), 2);
    list.push_back(3);
    EXPECT_EQ(list.size(), 3);
}

TEST(SinglyLinkedListTest, SizeAfterPushFront)
{
    singlyLinkedList<int> list;
    list.push_front(1);
    EXPECT_EQ(list.size(), 1);
    list.push_front(2);
    EXPECT_EQ(list.size(), 2);
}

TEST(SinglyLinkedListTest, SizeAfterErase)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    list.erase(list.begin());
    EXPECT_EQ(list.size(), 2);
    list.erase(list.begin());
    EXPECT_EQ(list.size(), 1);
    list.erase(list.begin());
    EXPECT_EQ(list.size(), 0);
}

TEST(SinglyLinkedListTest, SizeAfterInsert)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(3);

    auto pos = list.begin();
    ++pos;
    list.insert(pos, 2);
    EXPECT_EQ(list.size(), 3);
}

TEST(SinglyLinkedListTest, SizeAfterClear)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    list.clear();
    EXPECT_EQ(list.size(), 0);
}

// ============================================================
// 8. isEmpty 测试
// ============================================================

TEST(SinglyLinkedListTest, IsEmptyInitial)
{
    singlyLinkedList<int> list;
    EXPECT_TRUE(list.isEmpty());
}

TEST(SinglyLinkedListTest, IsEmptyAfterInsert)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    EXPECT_FALSE(list.isEmpty());

    list.push_back(2);
    EXPECT_FALSE(list.isEmpty());
}

TEST(SinglyLinkedListTest, IsEmptyAfterClear)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    list.clear();
    EXPECT_TRUE(list.isEmpty());
}

TEST(SinglyLinkedListTest, IsEmptyAfterEraseAll)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    while (!list.isEmpty())
    {
        list.erase(list.begin());
    }
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

// ============================================================
// 9. Clear 测试
// ============================================================

TEST(SinglyLinkedListTest, ClearNonEmpty)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    list.clear();
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
    EXPECT_EQ(list.begin(), list.end());
}

TEST(SinglyLinkedListTest, ClearEmpty)
{
    singlyLinkedList<int> list;
    list.clear();  // 清空空链表不应出错
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

TEST(SinglyLinkedListTest, ClearThenReuse)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);

    list.clear();
    EXPECT_TRUE(list.isEmpty());

    // 清空后可重新使用
    list.push_back(10);
    list.push_back(20);
    EXPECT_EQ(list.size(), 2);
    EXPECT_FALSE(list.isEmpty());

    auto it = list.begin();
    EXPECT_EQ(*it++, 10);
    EXPECT_EQ(*it++, 20);
}

TEST(SinglyLinkedListTest, ClearMultipleTimes)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.clear();
    list.clear();  // 连续两次 clear 不应出错
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

// ============================================================
// 10. 深拷贝测试 (Deep Copy)
// ============================================================

TEST(SinglyLinkedListTest, DeepCopyModifyCopyDoesNotAffectOriginal)
{
    singlyLinkedList<int> list1;
    list1.push_back(1);
    list1.push_back(2);

    singlyLinkedList<int> list2 = list1;

    list2.push_back(3);
    list2.push_back(4);

    // 原链表不应受影响
    EXPECT_EQ(list1.size(), 2);
    EXPECT_EQ(list2.size(), 4);

    auto it1 = list1.begin();
    EXPECT_EQ(*it1++, 1);
    EXPECT_EQ(*it1++, 2);
    EXPECT_EQ(it1, list1.end());
}

TEST(SinglyLinkedListTest, DeepCopyModifyOriginalDoesNotAffectCopy)
{
    singlyLinkedList<int> list1;
    list1.push_back(1);
    list1.push_back(2);

    singlyLinkedList<int> list2 = list1;

    list1.push_back(99);

    EXPECT_EQ(list1.size(), 3);
    EXPECT_EQ(list2.size(), 2);

    auto it2 = list2.begin();
    EXPECT_EQ(*it2++, 1);
    EXPECT_EQ(*it2++, 2);
    EXPECT_EQ(it2, list2.end());
}

TEST(SinglyLinkedListTest, DeepCopyEraseFromCopy)
{
    singlyLinkedList<int> list1;
    list1.push_back(1);
    list1.push_back(2);
    list1.push_back(3);

    singlyLinkedList<int> list2 = list1;

    list2.erase(list2.begin());

    EXPECT_EQ(list1.size(), 3);
    EXPECT_EQ(list2.size(), 2);

    auto it = list2.begin();
    EXPECT_EQ(*it++, 2);
    EXPECT_EQ(*it++, 3);
}

// ============================================================
// 11. 移动测试（Moved-from 状态）
// ============================================================

TEST(SinglyLinkedListTest, MovedFromListIsEmpty)
{
    singlyLinkedList<int> list1;
    list1.push_back(1);
    list1.push_back(2);
    list1.push_back(3);

    singlyLinkedList<int> list2 = std::move(list1);

    EXPECT_TRUE(list1.isEmpty());
    EXPECT_EQ(list1.size(), 0);
    EXPECT_EQ(list1.begin(), list1.end());
}

TEST(SinglyLinkedListTest, MoveAssignThenReuse)
{
    singlyLinkedList<int> list1;
    list1.push_back(10);
    list1.push_back(20);

    singlyLinkedList<int> list2;
    list2 = std::move(list1);

    EXPECT_TRUE(list1.isEmpty());
    EXPECT_EQ(list2.size(), 2);

    // 移动后的链表可重新使用
    list1.push_back(99);
    EXPECT_EQ(list1.size(), 1);
    EXPECT_EQ(*list1.begin(), 99);
}

TEST(SinglyLinkedListTest, MoveConstructThenReuse)
{
    singlyLinkedList<int> list1;
    list1.push_back(5);

    singlyLinkedList<int> list2(std::move(list1));

    list1.push_back(100);
    EXPECT_EQ(list1.size(), 1);
    EXPECT_EQ(*list1.begin(), 100);
}

// ============================================================
// 12. 边界情况测试 (Edge Cases)
// ============================================================

TEST(SinglyLinkedListTest, SingleElementPushBack)
{
    singlyLinkedList<int> list;
    list.push_back(42);

    EXPECT_EQ(list.size(), 1);
    EXPECT_FALSE(list.isEmpty());
    EXPECT_EQ(*list.begin(), 42);

    auto it = list.begin();
    ++it;
    EXPECT_EQ(it, list.end());

    // 删除单元素
    list.erase(list.begin());
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

TEST(SinglyLinkedListTest, SingleElementPushFront)
{
    singlyLinkedList<int> list;
    list.push_front(99);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(*list.begin(), 99);

    list.push_front(100);
    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(*list.begin(), 100);
}

TEST(SinglyLinkedListTest, LargeElementCount)
{
    singlyLinkedList<int> list;
    const int N = 50000;

    for (int i = 0; i < N; ++i)
    {
        list.push_back(i);
    }
    EXPECT_EQ(list.size(), N);

    // 查找中间元素
    auto it = list.find(N / 2);
    ASSERT_NE(it, list.end());
    EXPECT_EQ(*it, N / 2);

    // 查找尾部元素
    it = list.find(N - 1);
    ASSERT_NE(it, list.end());
    EXPECT_EQ(*it, N - 1);
}

TEST(SinglyLinkedListTest, ClearLargeList)
{
    singlyLinkedList<int> list;
    const int N = 50000;

    for (int i = 0; i < N; ++i)
    {
        list.push_back(i);
    }

    list.clear();
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

TEST(SinglyLinkedListTest, RepeatedPushFrontAndErase)
{
    singlyLinkedList<int> list;

    for (int i = 0; i < 100; ++i)
    {
        list.push_front(i);
    }
    EXPECT_EQ(list.size(), 100);

    for (int i = 0; i < 100; ++i)
    {
        list.erase(list.begin());
    }
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

TEST(SinglyLinkedListTest, InsertEraseSequence)
{
    singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(5);

    // 逐步插入直至 [1, 2, 3, 4, 5]
    auto pos = list.begin();
    ++pos;
    list.insert(pos, 2);

    pos = list.begin();
    ++pos;
    ++pos;
    list.insert(pos, 3);

    pos = list.begin();
    ++pos;
    ++pos;
    ++pos;
    list.insert(pos, 4);

    EXPECT_EQ(list.size(), 5);
    int expected = 1;
    for (const auto& v : list)
    {
        EXPECT_EQ(v, expected++);
    }
}

TEST(SinglyLinkedListTest, CopyAssignChain)
{
    singlyLinkedList<int> list1;
    list1.push_back(1);
    list1.push_back(2);

    singlyLinkedList<int> list2;
    singlyLinkedList<int> list3;

    list3 = list2 = list1;

    EXPECT_EQ(list2.size(), 2);
    EXPECT_EQ(list3.size(), 2);
    EXPECT_EQ(list1.size(), 2);

    auto it2 = list2.begin();
    EXPECT_EQ(*it2++, 1);
    EXPECT_EQ(*it2++, 2);

    auto it3 = list3.begin();
    EXPECT_EQ(*it3++, 1);
    EXPECT_EQ(*it3++, 2);
}

// ============================================================
// 13. 自定义类型测试 (Custom Types)
// ============================================================

TEST(SinglyLinkedListTest, StringType)
{
    singlyLinkedList<std::string> list;
    list.push_back("hello");
    list.push_back("world");
    list.push_back("foo");

    EXPECT_EQ(list.size(), 3);

    auto it = list.begin();
    EXPECT_EQ(*it++, "hello");
    EXPECT_EQ(*it++, "world");
    EXPECT_EQ(*it++, "foo");

    // 查找
    auto found = list.find("world");
    ASSERT_NE(found, list.end());
    EXPECT_EQ(*found, "world");

    // 删除
    list.erase(found);
    EXPECT_EQ(list.size(), 2);

    it = list.begin();
    EXPECT_EQ(*it++, "hello");
    EXPECT_EQ(*it++, "foo");
}

TEST(SinglyLinkedListTest, StringTypeMove)
{
    singlyLinkedList<std::string> list;
    std::string s1 = "aaa";
    std::string s2 = "bbb";
    std::string s3 = "ccc";

    list.push_back(std::move(s1));
    list.push_front(std::move(s2));
    list.push_back(std::move(s3));

    EXPECT_TRUE(s1.empty());
    EXPECT_TRUE(s2.empty());
    EXPECT_TRUE(s3.empty());

    EXPECT_EQ(list.size(), 3);
    auto it = list.begin();
    EXPECT_EQ(*it++, "bbb");
    EXPECT_EQ(*it++, "aaa");
    EXPECT_EQ(*it++, "ccc");
}

TEST(SinglyLinkedListTest, PairType)
{
    singlyLinkedList<std::pair<int, std::string>> list;
    list.push_back({1, "one"});
    list.push_back({2, "two"});
    list.push_back({3, "three"});

    EXPECT_EQ(list.size(), 3);

    auto it = list.begin();
    EXPECT_EQ(it->first, 1);
    EXPECT_EQ(it->second, "one");
    ++it;
    EXPECT_EQ(it->first, 2);
    EXPECT_EQ(it->second, "two");
    ++it;
    EXPECT_EQ(it->first, 3);
    EXPECT_EQ(it->second, "three");
}

TEST(SinglyLinkedListTest, DoubleType)
{
    singlyLinkedList<double> list;
    list.push_back(1.5);
    list.push_back(2.7);
    list.push_back(3.14);

    EXPECT_EQ(list.size(), 3);

    auto it = list.begin();
    EXPECT_DOUBLE_EQ(*it++, 1.5);
    EXPECT_DOUBLE_EQ(*it++, 2.7);
    EXPECT_DOUBLE_EQ(*it++, 3.14);
}

TEST(SinglyLinkedListTest, BoolType)
{
    singlyLinkedList<bool> list;
    list.push_back(true);
    list.push_back(false);
    list.push_back(true);

    EXPECT_EQ(list.size(), 3);
    auto it = list.begin();
    EXPECT_EQ(*it++, true);
    EXPECT_EQ(*it++, false);
    EXPECT_EQ(*it++, true);
}

// ============================================================
// 14. const 正确性测试
// ============================================================

TEST(SinglyLinkedListTest, ConstListBeginEnd)
{
    const singlyLinkedList<int> list = []()
    {
        singlyLinkedList<int> l;
        l.push_back(1);
        l.push_back(2);
        l.push_back(3);
        return l;
    }();

    EXPECT_FALSE(list.isEmpty());
    EXPECT_EQ(list.size(), 3);

    int expected = 1;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

TEST(SinglyLinkedListTest, ConstListCbeginCend)
{
    const singlyLinkedList<int> list = []()
    {
        singlyLinkedList<int> l;
        l.push_back(10);
        l.push_back(20);
        l.push_back(30);
        return l;
    }();

    int expected = 10;
    for (auto it = list.cbegin(); it != list.cend(); ++it)
    {
        EXPECT_EQ(*it, expected);
        expected += 10;
    }
}

// ============================================================
// 15. 析构 / 生命周期测试
// ============================================================

TEST(SinglyLinkedListTest, DestructorReleasesMemory)
{
    // 创建临时链表，离开作用域后应正常析构
    {
        singlyLinkedList<int> list;
        for (int i = 0; i < 1000; ++i)
        {
            list.push_back(i);
        }
        EXPECT_EQ(list.size(), 1000);
    }
    // 如果没有崩溃/泄漏，测试通过
    SUCCEED();
}

TEST(SinglyLinkedListTest, DestructorOnEmptyList)
{
    {
        singlyLinkedList<int> list;
        EXPECT_TRUE(list.isEmpty());
    }
    SUCCEED();
}

TEST(SinglyLinkedListTest, DestructorOnMovedFromList)
{
    {
        singlyLinkedList<int> list1;
        list1.push_back(1);
        list1.push_back(2);

        singlyLinkedList<int> list2(std::move(list1));
        // list1 现在为空，离开作用域应安全析构
    }
    SUCCEED();
}
