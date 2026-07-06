#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "../include/doublyLinkedList.hpp"

using DoublyLinkedList::doublyLinkedList;

// ============================================================================
// 1. 构造测试
// ============================================================================

TEST(ConstructionTest, DefaultConstructor)
{
    doublyLinkedList<int> list;
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.begin(), list.end());
}

TEST(ConstructionTest, InitializerListConstructor)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};
    EXPECT_EQ(list.size(), 5);
    EXPECT_FALSE(list.isEmpty());

    int expected = 1;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

TEST(ConstructionTest, InitializerListEmpty)
{
    doublyLinkedList<int> list{};
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.begin(), list.end());
}

TEST(ConstructionTest, CopyConstructor)
{
    doublyLinkedList<int> original{10, 20, 30};
    doublyLinkedList<int> copy(original);

    EXPECT_EQ(copy.size(), 3);
    EXPECT_EQ(original.size(), 3);

    // 验证值正确
    auto it_orig = original.begin();
    auto it_copy = copy.begin();
    for (; it_orig != original.end(); ++it_orig, ++it_copy)
    {
        EXPECT_EQ(*it_orig, *it_copy);
    }
}

TEST(ConstructionTest, CopyConstructorDeepCopy)
{
    doublyLinkedList<int> original{100, 200, 300};
    doublyLinkedList<int> copy(original);

    // 修改 copy，不应影响 original
    *copy.begin() = 999;
    EXPECT_EQ(*original.begin(), 100);
    EXPECT_EQ(*copy.begin(), 999);
}

TEST(ConstructionTest, MoveConstructor)
{
    doublyLinkedList<int> original{1, 2, 3, 4};
    doublyLinkedList<int> moved(std::move(original));

    EXPECT_EQ(moved.size(), 4);
    EXPECT_FALSE(moved.isEmpty());

    // 移动后的原列表应为空
    EXPECT_EQ(original.size(), 0);
    EXPECT_TRUE(original.isEmpty());
    EXPECT_EQ(original.begin(), original.end());

    // 验证 moved 的值
    int expected = 1;
    for (auto it = moved.begin(); it != moved.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

TEST(ConstructionTest, MoveConstructorEmpty)
{
    doublyLinkedList<int> original;
    doublyLinkedList<int> moved(std::move(original));

    EXPECT_EQ(moved.size(), 0);
    EXPECT_TRUE(moved.isEmpty());
    EXPECT_EQ(original.size(), 0);
    EXPECT_TRUE(original.isEmpty());
}

// ============================================================================
// 2. 赋值测试
// ============================================================================

TEST(AssignmentTest, CopyAssignment)
{
    doublyLinkedList<int> list1{7, 8, 9};
    doublyLinkedList<int> list2{1, 2};

    list2 = list1;

    EXPECT_EQ(list2.size(), 3);
    EXPECT_EQ(list1.size(), 3);

    auto it1 = list1.begin();
    auto it2 = list2.begin();
    for (; it1 != list1.end(); ++it1, ++it2)
    {
        EXPECT_EQ(*it1, *it2);
    }
}

TEST(AssignmentTest, CopyAssignmentDeepCopy)
{
    doublyLinkedList<int> list1{10, 20};
    doublyLinkedList<int> list2{30, 40, 50};

    list2 = list1;

    *list2.begin() = 777;
    EXPECT_EQ(*list1.begin(), 10);
    EXPECT_EQ(*list2.begin(), 777);
}

TEST(AssignmentTest, CopyAssignmentSelf)
{
    doublyLinkedList<int> list{1, 2, 3};
    // 自赋值不应崩溃
    list = list;
    EXPECT_EQ(list.size(), 3);

    int expected = 1;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

TEST(AssignmentTest, CopyAssignmentEmpty)
{
    doublyLinkedList<int> list1;
    doublyLinkedList<int> list2{1, 2, 3};

    list2 = list1;
    EXPECT_EQ(list2.size(), 0);
    EXPECT_TRUE(list2.isEmpty());
}

TEST(AssignmentTest, CopyAssignmentToEmpty)
{
    doublyLinkedList<int> list1{10, 20, 30};
    doublyLinkedList<int> list2;

    list2 = list1;
    EXPECT_EQ(list2.size(), 3);
    EXPECT_FALSE(list2.isEmpty());
}

TEST(AssignmentTest, MoveAssignment)
{
    doublyLinkedList<int> list1{5, 6, 7, 8};
    doublyLinkedList<int> list2{1};

    list2 = std::move(list1);

    EXPECT_EQ(list2.size(), 4);
    EXPECT_EQ(list1.size(), 0);
    EXPECT_TRUE(list1.isEmpty());

    int expected = 5;
    for (auto it = list2.begin(); it != list2.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

TEST(AssignmentTest, MoveAssignmentSelf)
{
    doublyLinkedList<int> list{10, 20, 30};
    // 自移动赋值不应崩溃
    list = std::move(list);
    // 自移动赋值后，list 应该仍然有效（实现中检测了 this != &other）
    EXPECT_EQ(list.size(), 3);
}

TEST(AssignmentTest, MoveAssignmentEmpty)
{
    doublyLinkedList<int> list1;
    doublyLinkedList<int> list2{1, 2, 3};

    list2 = std::move(list1);
    EXPECT_EQ(list2.size(), 0);
    EXPECT_TRUE(list2.isEmpty());
    EXPECT_EQ(list1.size(), 0);
    EXPECT_TRUE(list1.isEmpty());
}

// ============================================================================
// 3. Push 操作测试
// ============================================================================

TEST(PushTest, PushBackSingle)
{
    doublyLinkedList<int> list;
    list.push_back(42);

    EXPECT_EQ(list.size(), 1);
    EXPECT_FALSE(list.isEmpty());
    EXPECT_EQ(list.front(), 42);
    EXPECT_EQ(list.back(), 42);
}

TEST(PushTest, PushBackMultiple)
{
    doublyLinkedList<int> list;
    for (int i = 0; i < 10; ++i)
    {
        list.push_back(i);
    }

    EXPECT_EQ(list.size(), 10);

    int expected = 0;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

TEST(PushTest, PushFrontSingle)
{
    doublyLinkedList<int> list;
    list.push_front(99);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.front(), 99);
    EXPECT_EQ(list.back(), 99);
}

TEST(PushTest, PushFrontMultiple)
{
    doublyLinkedList<int> list;
    for (int i = 0; i < 5; ++i)
    {
        list.push_front(i);
    }

    EXPECT_EQ(list.size(), 5);

    // push_front 逆序：最后 push 的在最前
    int expected[] = {4, 3, 2, 1, 0};
    int idx = 0;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected[idx++]);
    }
}

TEST(PushTest, PushBackFrontInterleaved)
{
    doublyLinkedList<int> list;

    list.push_back(1);    // [1]
    list.push_front(0);   // [0, 1]
    list.push_back(2);    // [0, 1, 2]
    list.push_front(-1);  // [-1, 0, 1, 2]
    list.push_back(3);    // [-1, 0, 1, 2, 3]

    EXPECT_EQ(list.size(), 5);

    int expected[] = {-1, 0, 1, 2, 3};
    int idx = 0;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected[idx++]);
    }
}

TEST(PushTest, PushBackReturnValue)
{
    doublyLinkedList<int> list;
    int& ref = list.push_back(10);
    EXPECT_EQ(ref, 10);
    ref = 20;
    EXPECT_EQ(list.back(), 20);
}

TEST(PushTest, PushFrontReturnValue)
{
    doublyLinkedList<int> list;
    int& ref = list.push_front(5);
    EXPECT_EQ(ref, 5);
    ref = 15;
    EXPECT_EQ(list.front(), 15);
}

TEST(PushTest, PushBackString)
{
    doublyLinkedList<std::string> list;
    list.push_back("hello");
    list.push_back("world");

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), "hello");
    EXPECT_EQ(list.back(), "world");
}

TEST(PushTest, PushFrontString)
{
    doublyLinkedList<std::string> list;
    list.push_front("first");
    list.push_front("second");

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), "second");
    EXPECT_EQ(list.back(), "first");
}

// ============================================================================
// 4. Emplace 操作测试
// ============================================================================

TEST(EmplaceTest, EmplaceBackSingle)
{
    doublyLinkedList<int> list;
    list.emplace_back(100);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.front(), 100);
    EXPECT_EQ(list.back(), 100);
}

TEST(EmplaceTest, EmplaceBackMultiple)
{
    doublyLinkedList<int> list;
    list.emplace_back(1);
    list.emplace_back(2);
    list.emplace_back(3);

    EXPECT_EQ(list.size(), 3);

    int expected = 1;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

TEST(EmplaceTest, EmplaceFrontSingle)
{
    doublyLinkedList<int> list;
    list.emplace_front(200);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.front(), 200);
}

TEST(EmplaceTest, EmplaceFrontMultiple)
{
    doublyLinkedList<int> list;
    list.emplace_front(3);
    list.emplace_front(2);
    list.emplace_front(1);

    EXPECT_EQ(list.size(), 3);

    int expected[] = {1, 2, 3};
    int idx = 0;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected[idx++]);
    }
}

TEST(EmplaceTest, EmplaceBackReturnValue)
{
    doublyLinkedList<int> list;
    int& ref = list.emplace_back(50);
    EXPECT_EQ(ref, 50);
    ref = 60;
    EXPECT_EQ(list.back(), 60);
}

TEST(EmplaceTest, EmplaceFrontReturnValue)
{
    doublyLinkedList<int> list;
    int& ref = list.emplace_front(70);
    EXPECT_EQ(ref, 70);
    ref = 80;
    EXPECT_EQ(list.front(), 80);
}

// 测试 emplace 支持多参数构造（如 std::pair）
TEST(EmplaceTest, EmplaceBackMultiArg)
{
    struct Point
    {
        int x, y;
        Point(int x_, int y_) : x(x_), y(y_) {}
        bool operator==(const Point& other) const
        {
            return x == other.x && y == other.y;
        }
    };

    doublyLinkedList<Point> list;
    list.emplace_back(10, 20);
    list.emplace_back(30, 40);

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front().x, 10);
    EXPECT_EQ(list.front().y, 20);
    EXPECT_EQ(list.back().x, 30);
    EXPECT_EQ(list.back().y, 40);
}

TEST(EmplaceTest, EmplaceFrontMultiArg)
{
    struct Point
    {
        int x, y;
        Point(int x_, int y_) : x(x_), y(y_) {}
        bool operator==(const Point& other) const
        {
            return x == other.x && y == other.y;
        }
    };

    doublyLinkedList<Point> list;
    list.emplace_front(1, 1);
    list.emplace_front(0, 0);

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front().x, 0);
    EXPECT_EQ(list.front().y, 0);
    EXPECT_EQ(list.back().x, 1);
    EXPECT_EQ(list.back().y, 1);
}

// ============================================================================
// 5. Pop 操作测试（通过 erase 模拟，API 中无 pop_back/pop_front）
// ============================================================================

TEST(PopTest, PopBackSingle)
{
    doublyLinkedList<int> list{10, 20, 30};
    // 没有独立的 pop_back()，通过 --end() + erase 模拟
    list.erase(--list.end());

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.back(), 20);
    EXPECT_EQ(list.front(), 10);
}

TEST(PopTest, PopBackToEmpty)
{
    doublyLinkedList<int> list{1};
    list.erase(--list.end());

    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.begin(), list.end());
}

TEST(PopTest, PopFrontSingle)
{
    doublyLinkedList<int> list{100, 200, 300};
    list.erase(list.begin());

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), 200);
}

TEST(PopTest, PopFrontToEmpty)
{
    doublyLinkedList<int> list{42};
    list.erase(list.begin());

    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.isEmpty());
}

TEST(PopTest, EraseReturnsCorrectIterator)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};

    // 删除第二个元素 (2)
    auto it = list.begin();
    ++it;
    auto result = list.erase(it);

    EXPECT_EQ(*result, 3);  // 应返回被删元素之后的元素
    EXPECT_EQ(list.size(), 4);

    // 验证顺序：1, 3, 4, 5
    int expected[] = {1, 3, 4, 5};
    int idx = 0;
    for (auto iter = list.begin(); iter != list.end(); ++iter)
    {
        EXPECT_EQ(*iter, expected[idx++]);
    }
}

// ============================================================================
// 6. Insert 操作测试
// ============================================================================

TEST(InsertTest, InsertAtBegin)
{
    doublyLinkedList<int> list{2, 3, 4};
    auto it = list.insert(list.begin(), 1);

    EXPECT_EQ(*it, 1);
    EXPECT_EQ(list.size(), 4);

    int expected[] = {1, 2, 3, 4};
    int idx = 0;
    for (auto iter = list.begin(); iter != list.end(); ++iter)
    {
        EXPECT_EQ(*iter, expected[idx++]);
    }
}

TEST(InsertTest, InsertAtEnd)
{
    doublyLinkedList<int> list{1, 2, 3};
    auto it = list.insert(list.end(), 4);

    EXPECT_EQ(*it, 4);
    EXPECT_EQ(list.size(), 4);

    int expected[] = {1, 2, 3, 4};
    int idx = 0;
    for (auto iter = list.begin(); iter != list.end(); ++iter)
    {
        EXPECT_EQ(*iter, expected[idx++]);
    }
}

TEST(InsertTest, InsertAtMiddle)
{
    doublyLinkedList<int> list{1, 3, 4};

    // 在第二个位置插入 2
    auto pos = list.begin();
    ++pos;
    auto it = list.insert(pos, 2);

    EXPECT_EQ(*it, 2);
    EXPECT_EQ(list.size(), 4);

    int expected[] = {1, 2, 3, 4};
    int idx = 0;
    for (auto iter = list.begin(); iter != list.end(); ++iter)
    {
        EXPECT_EQ(*iter, expected[idx++]);
    }
}

TEST(InsertTest, InsertIntoEmptyAtBegin)
{
    doublyLinkedList<int> list;
    auto it = list.insert(list.begin(), 100);

    EXPECT_EQ(*it, 100);
    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.front(), 100);
    EXPECT_EQ(list.back(), 100);
}

TEST(InsertTest, InsertIntoEmptyAtEnd)
{
    doublyLinkedList<int> list;
    auto it = list.insert(list.end(), 200);

    EXPECT_EQ(*it, 200);
    EXPECT_EQ(list.size(), 1);
}

TEST(InsertTest, InsertMultipleAtBegin)
{
    doublyLinkedList<int> list;
    for (int i = 5; i >= 1; --i)
    {
        list.insert(list.begin(), i);
    }

    EXPECT_EQ(list.size(), 5);

    int expected = 1;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

TEST(InsertTest, InsertString)
{
    doublyLinkedList<std::string> list{"world"};
    list.insert(list.begin(), "hello");

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), "hello");
    EXPECT_EQ(list.back(), "world");
}

// ============================================================================
// 7. Erase 操作测试
// ============================================================================

TEST(EraseTest, EraseBegin)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};
    list.erase(list.begin());

    EXPECT_EQ(list.size(), 4);
    EXPECT_EQ(list.front(), 2);
}

TEST(EraseTest, EraseEnd)
{
    doublyLinkedList<int> list{1, 2, 3};
    // erase(end()) 应该无效果，返回 end()
    auto result = list.erase(list.end());

    EXPECT_EQ(result, list.end());
    EXPECT_EQ(list.size(), 3);
}

TEST(EraseTest, EraseTail)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};
    auto tail_it = --list.end();
    auto result = list.erase(tail_it);

    EXPECT_EQ(result, list.end());
    EXPECT_EQ(list.size(), 4);
    EXPECT_EQ(list.back(), 4);
}

TEST(EraseTest, EraseMiddle)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};

    // 删除中间元素 3
    auto mid = list.begin();
    ++mid;
    ++mid;  // 指向 3
    list.erase(mid);

    EXPECT_EQ(list.size(), 4);

    int expected[] = {1, 2, 4, 5};
    int idx = 0;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected[idx++]);
    }
}

TEST(EraseTest, EraseSingleElement)
{
    doublyLinkedList<int> list{42};
    list.erase(list.begin());

    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.begin(), list.end());
}

TEST(EraseTest, EraseFromEmpty)
{
    doublyLinkedList<int> list;
    auto result = list.erase(list.begin());

    EXPECT_EQ(result, list.end());
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.isEmpty());
}

TEST(EraseTest, EraseAllOneByOne)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};

    while (!list.isEmpty())
    {
        list.erase(list.begin());
    }

    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.isEmpty());
}

// ============================================================================
// 8. Find 操作测试
// ============================================================================

TEST(FindTest, FindExisting)
{
    doublyLinkedList<int> list{10, 20, 30, 40, 50};

    auto it = list.find(30);
    EXPECT_NE(it, list.end());
    EXPECT_EQ(*it, 30);
}

TEST(FindTest, FindFirst)
{
    doublyLinkedList<int> list{7, 8, 9};

    auto it = list.find(7);
    EXPECT_NE(it, list.end());
    EXPECT_EQ(*it, 7);
    EXPECT_EQ(it, list.begin());
}

TEST(FindTest, FindLast)
{
    doublyLinkedList<int> list{7, 8, 9};

    auto it = list.find(9);
    EXPECT_NE(it, list.end());
    EXPECT_EQ(*it, 9);

    auto last = --list.end();
    EXPECT_EQ(it, last);
}

TEST(FindTest, FindNonexistent)
{
    doublyLinkedList<int> list{1, 2, 3};

    auto it = list.find(999);
    EXPECT_EQ(it, list.end());
}

TEST(FindTest, FindInEmpty)
{
    doublyLinkedList<int> list;

    auto it = list.find(1);
    EXPECT_EQ(it, list.end());
}

TEST(FindTest, FindDuplicateReturnsFirst)
{
    doublyLinkedList<int> list{1, 2, 2, 3};

    auto it = list.find(2);
    EXPECT_NE(it, list.end());
    EXPECT_EQ(*it, 2);

    // 应该指向第一个 2
    EXPECT_EQ(it, ++list.begin());
}

TEST(FindTest, CFindExisting)
{
    const doublyLinkedList<int> list{5, 10, 15};

    auto it = list.cfind(10);
    EXPECT_NE(it, list.cend());
    EXPECT_EQ(*it, 10);
}

TEST(FindTest, CFindNonexistent)
{
    const doublyLinkedList<int> list{1, 2, 3};

    auto it = list.cfind(0);
    EXPECT_EQ(it, list.cend());
}

// ============================================================================
// 9. 迭代器测试
// ============================================================================

TEST(IteratorTest, ForwardTraversal)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};

    int expected = 1;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
    EXPECT_EQ(expected, 6);  // 所有 5 个元素都被访问
}

TEST(IteratorTest, BackwardTraversal)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};

    auto it = list.end();
    --it;  // 指向最后一个元素 (5)

    int expected = 5;
    while (it != list.begin())
    {
        EXPECT_EQ(*it, expected--);
        --it;
    }
    EXPECT_EQ(*it, expected);  // 1
}

TEST(IteratorTest, RangeFor)
{
    doublyLinkedList<int> list{10, 20, 30};

    int expected = 10;
    for (const auto& val : list)
    {
        EXPECT_EQ(val, expected);
        expected += 10;
    }
}

TEST(IteratorTest, EmptyListIteration)
{
    doublyLinkedList<int> list;

    int count = 0;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        ++count;
    }
    EXPECT_EQ(count, 0);

    // range-for on empty
    for (const auto& val : list)
    {
        (void)val;
        ++count;
    }
    EXPECT_EQ(count, 0);
}

TEST(IteratorTest, ConstIteration)
{
    const doublyLinkedList<int> list{5, 10, 15};

    int expected = 5;
    for (auto it = list.cbegin(); it != list.cend(); ++it)
    {
        EXPECT_EQ(*it, expected);
        expected += 5;
    }
}

TEST(IteratorTest, PostIncrement)
{
    doublyLinkedList<int> list{1, 2, 3};

    auto it = list.begin();
    auto old = it++;
    EXPECT_EQ(*old, 1);
    EXPECT_EQ(*it, 2);
}

TEST(IteratorTest, PostDecrement)
{
    doublyLinkedList<int> list{1, 2, 3};

    auto it = list.end();
    --it;
    auto old = it--;
    EXPECT_EQ(*old, 3);
    EXPECT_EQ(*it, 2);
}

TEST(IteratorTest, ArrowOperator)
{
    doublyLinkedList<std::string> list;
    list.emplace_back("hello");

    auto it = list.begin();
    EXPECT_EQ(it->size(), 5);
    EXPECT_EQ(it->at(0), 'h');
}

TEST(IteratorTest, IteratorEquality)
{
    doublyLinkedList<int> list{1, 2, 3};

    auto a = list.begin();
    auto b = list.begin();
    EXPECT_EQ(a, b);

    ++a;
    EXPECT_NE(a, b);

    ++b;
    EXPECT_EQ(a, b);
}

TEST(IteratorTest, IteratorCopy)
{
    doublyLinkedList<int> list{1, 2, 3};

    auto it1 = list.begin();
    auto it2(it1);  // 拷贝构造
    EXPECT_EQ(it1, it2);
    EXPECT_EQ(*it1, *it2);
}

// ============================================================================
// 10. Size 测试
// ============================================================================

TEST(SizeTest, InitialSize)
{
    doublyLinkedList<int> list;
    EXPECT_EQ(list.size(), 0);
}

TEST(SizeTest, AfterPushBack)
{
    doublyLinkedList<int> list;
    list.push_back(1);
    EXPECT_EQ(list.size(), 1);

    list.push_back(2);
    EXPECT_EQ(list.size(), 2);
}

TEST(SizeTest, AfterPushFront)
{
    doublyLinkedList<int> list;
    list.push_front(1);
    EXPECT_EQ(list.size(), 1);

    list.push_front(2);
    EXPECT_EQ(list.size(), 2);
}

TEST(SizeTest, AfterInsert)
{
    doublyLinkedList<int> list{1, 2};
    EXPECT_EQ(list.size(), 2);

    list.insert(list.begin(), 0);
    EXPECT_EQ(list.size(), 3);

    list.insert(list.end(), 3);
    EXPECT_EQ(list.size(), 4);
}

TEST(SizeTest, AfterErase)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};
    EXPECT_EQ(list.size(), 5);

    list.erase(list.begin());
    EXPECT_EQ(list.size(), 4);

    list.erase(--list.end());
    EXPECT_EQ(list.size(), 3);
}

TEST(SizeTest, AfterClear)
{
    doublyLinkedList<int> list{1, 2, 3};
    list.clear();
    EXPECT_EQ(list.size(), 0);
}

TEST(SizeTest, AfterMove)
{
    doublyLinkedList<int> list1{1, 2, 3};
    doublyLinkedList<int> list2(std::move(list1));

    EXPECT_EQ(list2.size(), 3);
    EXPECT_EQ(list1.size(), 0);
}

// ============================================================================
// 11. Empty (isEmpty) 测试
// ============================================================================

TEST(EmptyTest, NewListIsEmpty)
{
    doublyLinkedList<int> list;
    EXPECT_TRUE(list.isEmpty());
}

TEST(EmptyTest, NotEmptyAfterInsert)
{
    doublyLinkedList<int> list;
    list.push_back(1);
    EXPECT_FALSE(list.isEmpty());
}

TEST(EmptyTest, EmptyAfterClear)
{
    doublyLinkedList<int> list{1, 2, 3};
    EXPECT_FALSE(list.isEmpty());

    list.clear();
    EXPECT_TRUE(list.isEmpty());
}

TEST(EmptyTest, EmptyAfterMove)
{
    doublyLinkedList<int> list1{1, 2};
    doublyLinkedList<int> list2(std::move(list1));

    EXPECT_FALSE(list2.isEmpty());
    EXPECT_TRUE(list1.isEmpty());
}

// ============================================================================
// 12. Clear 测试
// ============================================================================

TEST(ClearTest, ClearNonEmpty)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};
    list.clear();

    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.begin(), list.end());
}

TEST(ClearTest, ClearEmpty)
{
    doublyLinkedList<int> list;
    list.clear();

    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.begin(), list.end());
}

TEST(ClearTest, ClearThenReuse)
{
    doublyLinkedList<int> list{1, 2, 3};
    list.clear();

    // 清空后可以继续使用
    list.push_back(10);
    list.push_back(20);

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), 10);
    EXPECT_EQ(list.back(), 20);
}

TEST(ClearTest, ClearTwice)
{
    doublyLinkedList<int> list{1, 2, 3};
    list.clear();
    list.clear();  // 第二次 clear 不应崩溃

    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.isEmpty());
}

// ============================================================================
// 13. 边界情况测试
// ============================================================================

TEST(EdgeCaseTest, SingleElementOperations)
{
    doublyLinkedList<int> list;
    list.push_back(42);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.front(), 42);
    EXPECT_EQ(list.back(), 42);
    EXPECT_EQ(list.begin(), --list.end());  // begin 和 --end 应指向同一元素

    // 修改唯一元素
    list.front() = 100;
    EXPECT_EQ(list.front(), 100);

    // 删除唯一元素
    list.erase(list.begin());
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

TEST(EdgeCaseTest, SelfAssignmentCopy)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};

    // 自赋值不应影响数据
    list = list;

    EXPECT_EQ(list.size(), 5);

    int expected = 1;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

TEST(EdgeCaseTest, SelfAssignmentMove)
{
    doublyLinkedList<int> list{10, 20, 30};

    // 自移动赋值（实际代码检测了 this != &other，所以安全）
    list = std::move(list);

    // 自移动赋值后列表仍应有效
    EXPECT_EQ(list.size(), 3);
}

TEST(EdgeCaseTest, LargeNumberOfElements)
{
    doublyLinkedList<int> list;
    const std::size_t N = 10000;

    for (std::size_t i = 0; i < N; ++i)
    {
        list.push_back(static_cast<int>(i));
    }

    EXPECT_EQ(list.size(), N);

    // 验证顺序
    int expected = 0;
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
    EXPECT_EQ(expected, static_cast<int>(N));

    // 验证反向遍历
    auto it = list.end();
    --it;
    expected = static_cast<int>(N) - 1;
    while (it != list.begin())
    {
        EXPECT_EQ(*it, expected--);
        --it;
    }
    EXPECT_EQ(*it, 0);
}

TEST(EdgeCaseTest, OperationsOnEmptyList)
{
    doublyLinkedList<int> list;

    // find on empty
    EXPECT_EQ(list.find(1), list.end());

    // erase on empty
    EXPECT_EQ(list.erase(list.begin()), list.end());
    EXPECT_EQ(list.erase(list.end()), list.end());

    // front/back on empty — 这是未定义行为，不测试
    // 只验证 size 和 isEmpty
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.isEmpty());
}

TEST(EdgeCaseTest, InsertAndEraseAtAllPositions)
{
    doublyLinkedList<int> list;

    // 交替插入和删除
    list.push_back(1);   // [1]
    list.push_back(2);   // [1, 2]
    list.push_front(0);  // [0, 1, 2]
    list.erase(list.begin());  // [1, 2]
    list.insert(list.end(), 3);  // [1, 2, 3]
    list.erase(--list.end());   // [1, 2]

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), 1);
    EXPECT_EQ(list.back(), 2);
}

TEST(EdgeCaseTest, LargeStringElements)
{
    doublyLinkedList<std::string> list;

    list.push_back(std::string(1000, 'a'));
    list.push_back(std::string(2000, 'b'));
    list.push_front(std::string(500, 'c'));

    EXPECT_EQ(list.size(), 3);
    EXPECT_EQ(list.front().size(), 500);
    EXPECT_EQ(list.back().size(), 2000);
}

TEST(EdgeCaseTest, MoveAwareType)
{
    // 使用 std::unique_ptr 等 move-only 类型测试移动语义
    // 注意：find() 需要比较，unique_ptr 不支持，故此测试用简单的值类型
    doublyLinkedList<std::vector<int>> list;

    std::vector<int> v1{1, 2, 3};
    std::vector<int> v2{4, 5, 6};

    list.push_back(std::move(v1));
    list.push_front(std::move(v2));

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.back().size(), 3);
    EXPECT_EQ(list.front().size(), 3);
}

// ============================================================================
// 14. 双向迭代器验证（-- on end() 返回最后一个元素）
// ============================================================================

TEST(BidirectionalTest, DecrementEndReturnsLast)
{
    doublyLinkedList<int> list{1, 2, 3, 4, 5};

    auto it = list.end();
    --it;
    EXPECT_EQ(*it, 5);

    --it;
    EXPECT_EQ(*it, 4);

    --it;
    EXPECT_EQ(*it, 3);

    --it;
    EXPECT_EQ(*it, 2);

    --it;
    EXPECT_EQ(*it, 1);

    // 此时 it == begin()
    EXPECT_EQ(it, list.begin());
}

TEST(BidirectionalTest, DecrementEndOnSingleElement)
{
    doublyLinkedList<int> list{42};

    auto it = list.end();
    --it;
    EXPECT_EQ(*it, 42);
    EXPECT_EQ(it, list.begin());
}

TEST(BidirectionalTest, IncrementToEnd)
{
    doublyLinkedList<int> list{1, 2, 3};

    auto it = list.begin();
    ++it;
    ++it;
    ++it;
    EXPECT_EQ(it, list.end());
}

TEST(BidirectionalTest, FullRoundTrip)
{
    doublyLinkedList<int> list{10, 20, 30};

    // 前进到头
    auto it = list.begin();
    ++it;
    ++it;
    ++it;
    EXPECT_EQ(it, list.end());

    // 后退回 begin
    --it;
    EXPECT_EQ(*it, 30);
    --it;
    EXPECT_EQ(*it, 20);
    --it;
    EXPECT_EQ(*it, 10);
    EXPECT_EQ(it, list.begin());
}

TEST(BidirectionalTest, DecrementEndEmptyList)
{
    doublyLinkedList<int> list;

    auto it = list.end();
    auto original = it;
    --it;
    // 空列表 --end() 应该仍是 end()（因为 sentinel->prev 为 nullptr）
    EXPECT_EQ(it, original);
}

// ============================================================================
// 15. 拷贝/移动后的链表可修改性测试
// ============================================================================

TEST(ModificationAfterCopyTest, CopyThenModify)
{
    doublyLinkedList<int> original{1, 2, 3};
    doublyLinkedList<int> copy = original;

    copy.push_back(4);
    copy.push_front(0);

    EXPECT_EQ(copy.size(), 5);
    EXPECT_EQ(original.size(), 3);

    // original 不变
    int expected_orig = 1;
    for (auto it = original.begin(); it != original.end(); ++it)
    {
        EXPECT_EQ(*it, expected_orig++);
    }

    // copy 已修改
    int expected_copy[] = {0, 1, 2, 3, 4};
    int idx = 0;
    for (auto it = copy.begin(); it != copy.end(); ++it)
    {
        EXPECT_EQ(*it, expected_copy[idx++]);
    }
}

TEST(ModificationAfterMoveTest, MoveThenModify)
{
    doublyLinkedList<int> original{10, 20, 30};
    doublyLinkedList<int> moved = std::move(original);

    // moved-from list 可以被重新赋值或使用
    original.push_back(100);
    EXPECT_EQ(original.size(), 1);
    EXPECT_EQ(original.front(), 100);

    // moved 列表独立可修改
    moved.push_back(40);
    EXPECT_EQ(moved.size(), 4);
    EXPECT_EQ(moved.back(), 40);
}

// ============================================================================
// 16. 值类型测试（非平凡类型）
// ============================================================================

TEST(ValueTypeTest, StringList)
{
    doublyLinkedList<std::string> list;

    list.push_back("alpha");
    list.push_back("beta");
    list.push_back("gamma");

    EXPECT_EQ(list.size(), 3);

    // 修改某个元素
    auto it = list.begin();
    ++it;
    *it = "CHANGED";

    EXPECT_EQ(list.front(), "alpha");
    auto mid = ++list.begin();
    EXPECT_EQ(*mid, "CHANGED");
    EXPECT_EQ(list.back(), "gamma");
}

TEST(ValueTypeTest, StringFind)
{
    doublyLinkedList<std::string> list{"apple", "banana", "cherry"};

    auto it = list.find("banana");
    EXPECT_NE(it, list.end());
    EXPECT_EQ(*it, "banana");

    auto not_found = list.find("durian");
    EXPECT_EQ(not_found, list.end());
}

TEST(ValueTypeTest, StringInsertErase)
{
    doublyLinkedList<std::string> list{"hello", "world"};

    // 中间插入
    auto pos = list.begin();
    ++pos;
    list.insert(pos, "beautiful");

    EXPECT_EQ(list.size(), 3);

    // 删除中间
    pos = list.begin();
    ++pos;
    list.erase(pos);

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.front(), "hello");
    EXPECT_EQ(list.back(), "world");
}

// ============================================================================
// 17. 综合场景测试
// ============================================================================

TEST(IntegrationTest, BuildAndTearDown)
{
    doublyLinkedList<int> list;

    // 阶段 1：构建
    for (int i = 0; i < 100; ++i)
    {
        list.push_back(i);
    }
    EXPECT_EQ(list.size(), 100);

    // 阶段 2：查找和修改
    auto it50 = list.find(50);
    ASSERT_NE(it50, list.end());
    *it50 = -50;

    // 阶段 3：部分删除
    // 删除前 10 个
    for (int i = 0; i < 10; ++i)
    {
        list.erase(list.begin());
    }
    EXPECT_EQ(list.size(), 90);
    EXPECT_EQ(list.front(), 10);

    // 阶段 4：在 head 和 tail 插入
    list.push_front(-1);
    list.push_back(100);
    EXPECT_EQ(list.size(), 92);

    // 阶段 5：清空
    list.clear();
    EXPECT_TRUE(list.isEmpty());
    EXPECT_EQ(list.size(), 0);
}

TEST(IntegrationTest, AlternatingInsertErase)
{
    doublyLinkedList<int> list;

    // 先插入多个元素，再交替删除
    for (int i = 0; i < 25; ++i)
    {
        list.push_back(i);
    }
    EXPECT_EQ(list.size(), 25);

    // 交替删除首尾元素
    for (int i = 0; i < 10; ++i)
    {
        list.erase(list.begin());
        if (!list.isEmpty())
        {
            auto last = list.end();
            --last;
            list.erase(last);
        }
    }
    // 25 - 10*2 = 5
    EXPECT_EQ(list.size(), 5);
}

TEST(IntegrationTest, CopyChain)
{
    doublyLinkedList<int> list1{1, 2, 3};
    doublyLinkedList<int> list2 = list1;
    doublyLinkedList<int> list3 = list2;
    doublyLinkedList<int> list4(list3);

    EXPECT_EQ(list1.size(), 3);
    EXPECT_EQ(list2.size(), 3);
    EXPECT_EQ(list3.size(), 3);
    EXPECT_EQ(list4.size(), 3);

    // 修改 list1 不影响其他
    *list1.begin() = 999;
    EXPECT_EQ(*list2.begin(), 1);
    EXPECT_EQ(*list3.begin(), 1);
    EXPECT_EQ(*list4.begin(), 1);
}

TEST(IntegrationTest, MoveChain)
{
    doublyLinkedList<int> list1{1, 2, 3, 4, 5};
    doublyLinkedList<int> list2 = std::move(list1);
    doublyLinkedList<int> list3 = std::move(list2);
    doublyLinkedList<int> list4(std::move(list3));

    EXPECT_EQ(list4.size(), 5);
    EXPECT_EQ(list1.size(), 0);
    EXPECT_EQ(list2.size(), 0);
    EXPECT_EQ(list3.size(), 0);

    int expected = 1;
    for (auto it = list4.begin(); it != list4.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

// ============================================================================
// 18. front() / back() 测试
// ============================================================================

TEST(FrontBackTest, FrontOnNonEmpty)
{
    doublyLinkedList<int> list{10, 20, 30};
    EXPECT_EQ(list.front(), 10);
}

TEST(FrontBackTest, BackOnNonEmpty)
{
    doublyLinkedList<int> list{10, 20, 30};
    EXPECT_EQ(list.back(), 30);
}

TEST(FrontBackTest, FrontAfterPushFront)
{
    doublyLinkedList<int> list;
    list.push_front(5);
    EXPECT_EQ(list.front(), 5);
    list.push_front(3);
    EXPECT_EQ(list.front(), 3);
}

TEST(FrontBackTest, BackAfterPushBack)
{
    doublyLinkedList<int> list;
    list.push_back(5);
    EXPECT_EQ(list.back(), 5);
    list.push_back(10);
    EXPECT_EQ(list.back(), 10);
}

TEST(FrontBackTest, FrontEqualsBackForSingleElement)
{
    doublyLinkedList<int> list;
    list.push_back(42);
    EXPECT_EQ(list.front(), list.back());
    EXPECT_EQ(list.front(), 42);
}
