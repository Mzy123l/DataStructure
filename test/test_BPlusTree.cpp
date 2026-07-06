/**
 * @file test_BPlusTree.cpp
 * @brief BPlusTree 综合单元测试
 *
 * 测试覆盖：构造、插入、删除、查找、边界情况、移动语义、
 *           size 跟踪、叶节点链表、clear、大批量操作
 */

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <set>
#include <memory>

#include "../include/BPlusTree.hpp"

// ============================================================
// 1. 构造测试
// ============================================================

TEST(BPlusTreeConstruction, DefaultConstruction)
{
    Tree::BPlusTree<int, std::string> tree;
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.get_leftmost_leaf(), nullptr);
    EXPECT_EQ(tree.get_rightmost_leaf(), nullptr);
}

TEST(BPlusTreeConstruction, CustomOrderConstruction)
{
    // order=3 (默认最小值)
    Tree::BPlusTree<int, int> tree3(std::allocator<std::pair<const int, int>>{}, 3);
    EXPECT_TRUE(tree3.empty());

    // order=5
    Tree::BPlusTree<int, int> tree5(std::allocator<std::pair<const int, int>>{}, 5);
    EXPECT_TRUE(tree5.empty());

    // order=10
    Tree::BPlusTree<int, int> tree10(std::allocator<std::pair<const int, int>>{}, 10);
    EXPECT_TRUE(tree10.empty());

    // order 小于 3 时会被 clamp 到 3
    Tree::BPlusTree<int, int> tree2(std::allocator<std::pair<const int, int>>{}, 2);
    EXPECT_TRUE(tree2.empty());
}

TEST(BPlusTreeConstruction, MoveConstruction)
{
    Tree::BPlusTree<int, std::string> tree;
    tree.insert(1, "one");
    tree.insert(2, "two");
    tree.insert(3, "three");
    EXPECT_EQ(tree.size(), 3);

    Tree::BPlusTree<int, std::string> moved(std::move(tree));
    EXPECT_EQ(moved.size(), 3);
    EXPECT_FALSE(moved.empty());

    // 移动后原树为空
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.get_leftmost_leaf(), nullptr);
    EXPECT_EQ(tree.get_rightmost_leaf(), nullptr);

    // 验证 moved 中的元素
    auto* v1 = moved.find(1);
    auto* v2 = moved.find(2);
    auto* v3 = moved.find(3);
    EXPECT_NE(v1, nullptr);
    EXPECT_NE(v2, nullptr);
    EXPECT_NE(v3, nullptr);
    EXPECT_EQ(*v1, "one");
    EXPECT_EQ(*v2, "two");
    EXPECT_EQ(*v3, "three");
}

TEST(BPlusTreeConstruction, MoveAssignment)
{
    Tree::BPlusTree<int, std::string> tree1;
    tree1.insert(10, "ten");
    tree1.insert(20, "twenty");

    Tree::BPlusTree<int, std::string> tree2;
    tree2.insert(99, "ninety-nine");

    tree2 = std::move(tree1);

    // tree2 拥有原 tree1 的元素
    EXPECT_EQ(tree2.size(), 2);
    auto* v10 = tree2.find(10);
    auto* v20 = tree2.find(20);
    EXPECT_NE(v10, nullptr);
    EXPECT_NE(v20, nullptr);
    EXPECT_EQ(*v10, "ten");
    EXPECT_EQ(*v20, "twenty");

    // tree1 被清空
    EXPECT_EQ(tree1.size(), 0);
    EXPECT_TRUE(tree1.empty());
}

TEST(BPlusTreeConstruction, MoveAssignmentToSelf)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(1, 100);
    tree.insert(2, 200);

    // 自赋值应该安全（通过 if (this != &other) 保护）
    size_t old_size = tree.size();
    auto* old_leftmost = tree.get_leftmost_leaf();
    auto* old_rightmost = tree.get_rightmost_leaf();

    tree = std::move(tree);

    EXPECT_EQ(tree.size(), old_size);
    EXPECT_EQ(tree.get_leftmost_leaf(), old_leftmost);
    EXPECT_EQ(tree.get_rightmost_leaf(), old_rightmost);
    auto* v1 = tree.find(1);
    auto* v2 = tree.find(2);
    EXPECT_NE(v1, nullptr);
    EXPECT_NE(v2, nullptr);
    EXPECT_EQ(*v1, 100);
    EXPECT_EQ(*v2, 200);
}

// ============================================================
// 2. 插入测试
// ============================================================

TEST(BPlusTreeInsert, SingleInsert)
{
    Tree::BPlusTree<int, std::string> tree;
    tree.insert(42, "hello");

    EXPECT_EQ(tree.size(), 1);
    EXPECT_FALSE(tree.empty());

    auto* val = tree.find(42);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, "hello");
}

TEST(BPlusTreeInsert, MultipleInserts)
{
    Tree::BPlusTree<int, int> tree;
    constexpr int N = 100;

    for (int i = 0; i < N; ++i)
    {
        tree.insert(i, i * 10);
    }

    EXPECT_EQ(tree.size(), N);

    for (int i = 0; i < N; ++i)
    {
        auto* val = tree.find(i);
        ASSERT_NE(val, nullptr) << "i = " << i;
        EXPECT_EQ(*val, i * 10);
    }
}

TEST(BPlusTreeInsert, DuplicateKeyUpdatesValue)
{
    Tree::BPlusTree<int, std::string> tree;
    tree.insert(1, "first");
    tree.insert(1, "second");

    // size 不应增长
    EXPECT_EQ(tree.size(), 1);

    auto* val = tree.find(1);
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(*val, "second");
}

TEST(BPlusTreeInsert, DuplicateKeyDoesNotIncreaseSize)
{
    Tree::BPlusTree<int, int> tree;
    for (int i = 0; i < 10; ++i)
    {
        tree.insert(5, i);
    }
    EXPECT_EQ(tree.size(), 1);
}

TEST(BPlusTreeInsert, LargeSequentialInsert)
{
    Tree::BPlusTree<int, int> tree;
    constexpr int N = 1000;

    for (int i = 0; i < N; ++i)
    {
        tree.insert(i, i);
    }

    EXPECT_EQ(tree.size(), N);
    EXPECT_FALSE(tree.empty());

    // 验证所有元素
    for (int i = 0; i < N; ++i)
    {
        auto* val = tree.find(i);
        ASSERT_NE(val, nullptr) << "Missing key: " << i;
        EXPECT_EQ(*val, i);
    }
}

TEST(BPlusTreeInsert, ReverseOrderInsert)
{
    Tree::BPlusTree<int, int> tree;
    constexpr int N = 200;

    for (int i = N - 1; i >= 0; --i)
    {
        tree.insert(i, i * 2);
    }

    EXPECT_EQ(tree.size(), N);

    for (int i = 0; i < N; ++i)
    {
        auto* val = tree.find(i);
        ASSERT_NE(val, nullptr) << "Missing key: " << i;
        EXPECT_EQ(*val, i * 2);
    }
}

TEST(BPlusTreeInsert, RandomOrderInsert)
{
    Tree::BPlusTree<int, int> tree;
    constexpr int N = 500;

    std::vector<int> keys(N);
    for (int i = 0; i < N; ++i)
    {
        keys[i] = i;
    }

    std::mt19937 rng(42);
    std::shuffle(keys.begin(), keys.end(), rng);

    for (int k : keys)
    {
        tree.insert(k, k * 3);
    }

    EXPECT_EQ(tree.size(), N);

    for (int i = 0; i < N; ++i)
    {
        auto* val = tree.find(i);
        ASSERT_NE(val, nullptr) << "Missing key: " << i;
        EXPECT_EQ(*val, i * 3);
    }
}

TEST(BPlusTreeInsert, StringKeyInsert)
{
    Tree::BPlusTree<std::string, int> tree;
    tree.insert("apple", 1);
    tree.insert("banana", 2);
    tree.insert("cherry", 3);
    tree.insert("date", 4);
    tree.insert("elderberry", 5);

    EXPECT_EQ(tree.size(), 5);

    EXPECT_EQ(*tree.find("apple"), 1);
    EXPECT_EQ(*tree.find("banana"), 2);
    EXPECT_EQ(*tree.find("cherry"), 3);
    EXPECT_EQ(*tree.find("date"), 4);
    EXPECT_EQ(*tree.find("elderberry"), 5);
}

TEST(BPlusTreeInsert, NegativeKeys)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(-10, 100);
    tree.insert(-5, 25);
    tree.insert(0, 0);
    tree.insert(5, 25);
    tree.insert(10, 100);

    EXPECT_EQ(tree.size(), 5);
    EXPECT_EQ(*tree.find(-10), 100);
    EXPECT_EQ(*tree.find(-5), 25);
    EXPECT_EQ(*tree.find(0), 0);
    EXPECT_EQ(*tree.find(5), 25);
    EXPECT_EQ(*tree.find(10), 100);
}

// ============================================================
// 3. 删除测试
// ============================================================

TEST(BPlusTreeErase, EraseExistingKey)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(1, 10);
    tree.insert(2, 20);
    tree.insert(3, 30);

    EXPECT_TRUE(tree.erase(2));
    EXPECT_EQ(tree.size(), 2);
    EXPECT_EQ(tree.find(2), nullptr);

    // 其余键仍在
    EXPECT_NE(tree.find(1), nullptr);
    EXPECT_NE(tree.find(3), nullptr);
}

TEST(BPlusTreeErase, EraseNonexistentKey)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(1, 10);
    tree.insert(2, 20);

    // 删除不存在的键
    EXPECT_FALSE(tree.erase(99));
    EXPECT_EQ(tree.size(), 2);
}

TEST(BPlusTreeErase, EraseFromEmptyTree)
{
    Tree::BPlusTree<int, int> tree;
    EXPECT_FALSE(tree.erase(1));
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.empty());
}

TEST(BPlusTreeErase, EraseAllKeysOneByOne)
{
    Tree::BPlusTree<int, int> tree;
    constexpr int N = 50;

    for (int i = 0; i < N; ++i)
    {
        tree.insert(i, i * 10);
    }
    EXPECT_EQ(tree.size(), N);

    // 逐个删除
    for (int i = 0; i < N; ++i)
    {
        EXPECT_TRUE(tree.erase(i)) << "Failed to erase key: " << i;
        EXPECT_EQ(tree.size(), N - i - 1);
    }

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.get_leftmost_leaf(), nullptr);
    EXPECT_EQ(tree.get_rightmost_leaf(), nullptr);
}

TEST(BPlusTreeErase, EraseCausingUnderflowAndMerge)
{
    // 用较小 order (3) 插入足量数据以触发分裂，然后删除触发下溢/合并
    Tree::BPlusTree<int, int> tree(std::allocator<std::pair<const int, int>>{}, 3);
    constexpr int N = 30;

    for (int i = 0; i < N; ++i)
    {
        tree.insert(i, i);
    }
    EXPECT_EQ(tree.size(), N);

    // 逆序删除以触发下溢和合并
    for (int i = N - 1; i >= 0; --i)
    {
        EXPECT_TRUE(tree.erase(i)) << "Failed to erase key: " << i;
    }

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
}

TEST(BPlusTreeErase, EraseAndReinsert)
{
    Tree::BPlusTree<int, int> tree;

    for (int i = 0; i < 20; ++i)
    {
        tree.insert(i, i);
    }

    // 删除一半
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_TRUE(tree.erase(i));
    }
    EXPECT_EQ(tree.size(), 10);

    // 重新插入已删除的键
    for (int i = 0; i < 10; ++i)
    {
        tree.insert(i, i * 100);
    }
    EXPECT_EQ(tree.size(), 20);

    // 验证所有键
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(*tree.find(i), i * 100);
    }
    for (int i = 10; i < 20; ++i)
    {
        EXPECT_EQ(*tree.find(i), i);
    }
}

TEST(BPlusTreeErase, EraseSingleElement)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(42, 99);

    EXPECT_TRUE(tree.erase(42));
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.find(42), nullptr);
    EXPECT_EQ(tree.get_leftmost_leaf(), nullptr);
    EXPECT_EQ(tree.get_rightmost_leaf(), nullptr);
}

// ============================================================
// 4. 查找测试
// ============================================================

TEST(BPlusTreeFind, FindExisting)
{
    Tree::BPlusTree<int, std::string> tree;
    tree.insert(1, "alpha");
    tree.insert(2, "beta");
    tree.insert(3, "gamma");

    auto* v = tree.find(2);
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(*v, "beta");
}

TEST(BPlusTreeFind, FindNonexistent)
{
    Tree::BPlusTree<int, std::string> tree;
    tree.insert(1, "alpha");

    EXPECT_EQ(tree.find(0), nullptr);
    EXPECT_EQ(tree.find(2), nullptr);
    EXPECT_EQ(tree.find(100), nullptr);
}

TEST(BPlusTreeFind, FindInEmptyTree)
{
    Tree::BPlusTree<int, std::string> tree;
    EXPECT_EQ(tree.find(1), nullptr);
}

TEST(BPlusTreeFind, FindAfterInsert)
{
    Tree::BPlusTree<int, int> tree;
    EXPECT_EQ(tree.find(1), nullptr);

    tree.insert(1, 100);
    EXPECT_NE(tree.find(1), nullptr);
    EXPECT_EQ(*tree.find(1), 100);
}

TEST(BPlusTreeFind, FindAfterErase)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(1, 100);
    tree.insert(2, 200);
    tree.insert(3, 300);

    tree.erase(2);
    EXPECT_EQ(tree.find(2), nullptr);
    EXPECT_NE(tree.find(1), nullptr);
    EXPECT_NE(tree.find(3), nullptr);
}

TEST(BPlusTreeFind, ConstVersion)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(10, 100);
    tree.insert(20, 200);

    const Tree::BPlusTree<int, int>& cref = tree;

    const int* v10 = cref.find(10);
    const int* v20 = cref.find(20);
    const int* v99 = cref.find(99);

    ASSERT_NE(v10, nullptr);
    ASSERT_NE(v20, nullptr);
    EXPECT_EQ(*v10, 100);
    EXPECT_EQ(*v20, 200);
    EXPECT_EQ(v99, nullptr);
}

TEST(BPlusTreeFind, ModifyThroughPointer)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(5, 50);

    int* val = tree.find(5);
    ASSERT_NE(val, nullptr);
    *val = 999;

    // 再次查找应当看到修改后的值
    int* val2 = tree.find(5);
    ASSERT_NE(val2, nullptr);
    EXPECT_EQ(*val2, 999);
}

// ============================================================
// 5. 边界情况
// ============================================================

TEST(BPlusTreeEdgeCases, EmptyTreeOperations)
{
    Tree::BPlusTree<int, int> tree;

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.find(0), nullptr);
    EXPECT_FALSE(tree.erase(0));
    EXPECT_EQ(tree.get_leftmost_leaf(), nullptr);
    EXPECT_EQ(tree.get_rightmost_leaf(), nullptr);

    // clear on empty tree 应该安全
    tree.clear();
    EXPECT_TRUE(tree.empty());
}

TEST(BPlusTreeEdgeCases, SingleElement)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(42, 100);

    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(tree.size(), 1);
    EXPECT_EQ(*tree.find(42), 100);

    // 叶节点链表
    auto* left = tree.get_leftmost_leaf();
    auto* right = tree.get_rightmost_leaf();
    EXPECT_NE(left, nullptr);
    EXPECT_EQ(left, right);

    // 删除唯一的元素
    tree.erase(42);
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
}

TEST(BPlusTreeEdgeCases, RepeatedInsertErase)
{
    Tree::BPlusTree<int, int> tree;

    for (int cycle = 0; cycle < 10; ++cycle)
    {
        // 插入
        for (int i = 0; i < 10; ++i)
        {
            tree.insert(i, i + cycle);
        }
        EXPECT_EQ(tree.size(), 10);

        // 验证
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_NE(tree.find(i), nullptr);
        }

        // 删除
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_TRUE(tree.erase(i));
        }
        EXPECT_TRUE(tree.empty());
    }
}

TEST(BPlusTreeEdgeCases, DifferentOrders)
{
    for (int order : {3, 4, 5, 10})
    {
        Tree::BPlusTree<int, int> tree(
            std::allocator<std::pair<const int, int>>{}, order);

        constexpr int N = 200;
        for (int i = 0; i < N; ++i)
        {
            tree.insert(i, i);
        }
        EXPECT_EQ(tree.size(), N);

        for (int i = 0; i < N; ++i)
        {
            auto* val = tree.find(i);
            ASSERT_NE(val, nullptr)
                << "order=" << order << " key=" << i;
            EXPECT_EQ(*val, i);
        }

        // 倒序删除
        for (int i = N - 1; i >= 0; --i)
        {
            EXPECT_TRUE(tree.erase(i));
        }
        EXPECT_TRUE(tree.empty());
    }
}

TEST(BPlusTreeEdgeCases, FirstAndLastKeyAccess)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(50, 500);
    tree.insert(10, 100);
    tree.insert(30, 300);
    tree.insert(1, 10);
    tree.insert(99, 990);

    // find 操作应该能找到所有键
    EXPECT_EQ(*tree.find(1), 10);
    EXPECT_EQ(*tree.find(99), 990);
    EXPECT_EQ(*tree.find(10), 100);
    EXPECT_EQ(*tree.find(30), 300);
    EXPECT_EQ(*tree.find(50), 500);

    // 删除最小键
    EXPECT_TRUE(tree.erase(1));
    EXPECT_EQ(tree.find(1), nullptr);
    EXPECT_EQ(tree.size(), 4);

    // 删除最大键
    EXPECT_TRUE(tree.erase(99));
    EXPECT_EQ(tree.find(99), nullptr);
    EXPECT_EQ(tree.size(), 3);
}

// ============================================================
// 6. 移动语义测试
// ============================================================

TEST(BPlusTreeMoveSemantics, MovedFromTreeIsEmpty)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(1, 100);
    tree.insert(2, 200);
    tree.insert(3, 300);

    Tree::BPlusTree<int, int> moved(std::move(tree));

    // 原树被重置
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.get_leftmost_leaf(), nullptr);
    EXPECT_EQ(tree.get_rightmost_leaf(), nullptr);
    EXPECT_EQ(tree.find(1), nullptr);

    // 新树有效
    EXPECT_EQ(moved.size(), 3);
    EXPECT_NE(moved.find(1), nullptr);
    EXPECT_NE(moved.find(2), nullptr);
    EXPECT_NE(moved.find(3), nullptr);
}

TEST(BPlusTreeMoveSemantics, MoveAssignAfterInsert)
{
    Tree::BPlusTree<int, int> tree1;
    for (int i = 0; i < 50; ++i)
    {
        tree1.insert(i, i * i);
    }

    Tree::BPlusTree<int, int> tree2;
    tree2 = std::move(tree1);

    EXPECT_TRUE(tree1.empty());
    EXPECT_EQ(tree2.size(), 50);

    for (int i = 0; i < 50; ++i)
    {
        auto* val = tree2.find(i);
        ASSERT_NE(val, nullptr) << "i = " << i;
        EXPECT_EQ(*val, i * i);
    }
}

TEST(BPlusTreeMoveSemantics, ChainedMove)
{
    Tree::BPlusTree<int, std::string> t1;
    t1.insert(1, "a");
    t1.insert(2, "b");

    Tree::BPlusTree<int, std::string> t2(std::move(t1));
    Tree::BPlusTree<int, std::string> t3(std::move(t2));

    EXPECT_TRUE(t1.empty());
    EXPECT_TRUE(t2.empty());
    EXPECT_EQ(t3.size(), 2);
    EXPECT_EQ(*t3.find(1), "a");
    EXPECT_EQ(*t3.find(2), "b");
}

TEST(BPlusTreeMoveSemantics, ReuseAfterMove)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(1, 10);

    Tree::BPlusTree<int, int> moved(std::move(tree));

    // 在移动后的原树中插入新数据
    tree.insert(100, 1000);
    tree.insert(200, 2000);

    EXPECT_EQ(tree.size(), 2);
    EXPECT_NE(tree.find(100), nullptr);
    EXPECT_NE(tree.find(200), nullptr);

    // moved 树仍持有原数据
    EXPECT_EQ(moved.size(), 1);
    EXPECT_NE(moved.find(1), nullptr);
}

// ============================================================
// 7. Size 跟踪测试
// ============================================================

TEST(BPlusTreeSizeTracking, SizeIncreasesOnInsert)
{
    Tree::BPlusTree<int, int> tree;

    for (int i = 0; i < 100; ++i)
    {
        tree.insert(i, i);
        EXPECT_EQ(tree.size(), i + 1);
    }
}

TEST(BPlusTreeSizeTracking, SizeNotChangedOnDuplicateInsert)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(1, 100);
    EXPECT_EQ(tree.size(), 1);

    tree.insert(1, 200);
    EXPECT_EQ(tree.size(), 1);
}

TEST(BPlusTreeSizeTracking, SizeDecreasesOnErase)
{
    Tree::BPlusTree<int, int> tree;
    constexpr int N = 50;

    for (int i = 0; i < N; ++i)
    {
        tree.insert(i, i);
    }

    for (int i = 0; i < N; ++i)
    {
        tree.erase(i);
        EXPECT_EQ(tree.size(), N - i - 1);
    }
}

TEST(BPlusTreeSizeTracking, SizeUnchangedOnFailedErase)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(1, 10);
    tree.insert(2, 20);

    size_t old_size = tree.size();
    EXPECT_FALSE(tree.erase(99));
    EXPECT_EQ(tree.size(), old_size);
}

TEST(BPlusTreeSizeTracking, SizeZeroAfterClear)
{
    Tree::BPlusTree<int, int> tree;
    for (int i = 0; i < 30; ++i)
    {
        tree.insert(i, i);
    }
    EXPECT_GT(tree.size(), 0);

    tree.clear();
    EXPECT_EQ(tree.size(), 0);
    EXPECT_TRUE(tree.empty());
}

// ============================================================
// 8. 叶节点链表测试
// ============================================================

TEST(BPlusTreeLeafList, SingleLeafNode)
{
    Tree::BPlusTree<int, int> tree;

    // 插入少量元素（不触发分裂），应该只有单个叶节点
    tree.insert(1, 10);
    tree.insert(2, 20);

    auto* left = tree.get_leftmost_leaf();
    auto* right = tree.get_rightmost_leaf();

    ASSERT_NE(left, nullptr);
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(left, right) << "Single leaf: leftmost should equal rightmost";

    EXPECT_TRUE(left->is_leaf());
    EXPECT_EQ(left->leaf_next(), nullptr);
    EXPECT_EQ(left->leaf_prev(), nullptr);
}

TEST(BPlusTreeLeafList, MultipleLeafNodesLinkedCorrectly)
{
    // 使用 order=3，插入足够多的数据触发分裂
    Tree::BPlusTree<int, int> tree(std::allocator<std::pair<const int, int>>{}, 3);
    constexpr int N = 20;

    for (int i = 0; i < N; ++i)
    {
        tree.insert(i, i);
    }

    // 遍历叶节点链表
    auto* leaf = tree.get_leftmost_leaf();
    ASSERT_NE(leaf, nullptr);
    ASSERT_TRUE(leaf->is_leaf());

    int total_keys = 0;
    int prev_key = -1;
    auto* prev_leaf = static_cast<decltype(leaf)>(nullptr);

    while (leaf)
    {
        EXPECT_TRUE(leaf->is_leaf());
        EXPECT_EQ(leaf->leaf_prev(), prev_leaf);

        auto& data = leaf->leaf_data();
        for (auto& kv : data)
        {
            EXPECT_EQ(kv.first, prev_key + 1)
                << "Keys should be sequential in leaf list";
            prev_key = kv.first;
            ++total_keys;
        }

        prev_leaf = leaf;
        leaf = leaf->leaf_next();
    }

    EXPECT_EQ(total_keys, N);

    // verify rightmost
    EXPECT_EQ(tree.get_rightmost_leaf(), prev_leaf);
}

TEST(BPlusTreeLeafList, LeftmostAndRightmostAfterInsertion)
{
    Tree::BPlusTree<int, int> tree(std::allocator<std::pair<const int, int>>{}, 3);

    tree.insert(5, 50);
    EXPECT_NE(tree.get_leftmost_leaf(), nullptr);
    EXPECT_EQ(tree.get_leftmost_leaf(), tree.get_rightmost_leaf());

    tree.insert(1, 10);  // 新最小值
    tree.insert(10, 100); // 新最大值

    EXPECT_NE(tree.get_leftmost_leaf(), nullptr);
    EXPECT_NE(tree.get_rightmost_leaf(), nullptr);

    // 获取最左和最右叶节点的第一个和最后一个数据
    auto* left_leaf = tree.get_leftmost_leaf();
    auto* right_leaf = tree.get_rightmost_leaf();
    ASSERT_TRUE(left_leaf->is_leaf());
    ASSERT_TRUE(right_leaf->is_leaf());

    EXPECT_EQ(left_leaf->leaf_data().front().first, 1);
    EXPECT_EQ(right_leaf->leaf_data().back().first, 10);
}

TEST(BPlusTreeLeafList, LeafListAfterErase)
{
    Tree::BPlusTree<int, int> tree(std::allocator<std::pair<const int, int>>{}, 3);

    // 插入足够多的数据
    for (int i = 0; i < 15; ++i)
    {
        tree.insert(i, i);
    }

    // 删除中间一些键
    tree.erase(5);
    tree.erase(6);
    tree.erase(7);

    // 遍历叶节点链表验证连续性
    auto* leaf = tree.get_leftmost_leaf();
    int total = 0;
    int last_key = -1;

    while (leaf)
    {
        for (auto& kv : leaf->leaf_data())
        {
            EXPECT_GT(kv.first, last_key)
                << "Keys should be strictly increasing";
            last_key = kv.first;
            ++total;
        }
        leaf = leaf->leaf_next();
    }

    EXPECT_EQ(total, 12); // 15 - 3
}

TEST(BPlusTreeLeafList, LeafListAfterClear)
{
    Tree::BPlusTree<int, int> tree(std::allocator<std::pair<const int, int>>{}, 3);

    for (int i = 0; i < 20; ++i)
    {
        tree.insert(i, i);
    }

    tree.clear();

    EXPECT_EQ(tree.get_leftmost_leaf(), nullptr);
    EXPECT_EQ(tree.get_rightmost_leaf(), nullptr);
}

// ============================================================
// 9. Clear 测试
// ============================================================

TEST(BPlusTreeClear, ClearNonEmptyTree)
{
    Tree::BPlusTree<int, int> tree;

    for (int i = 0; i < 100; ++i)
    {
        tree.insert(i, i);
    }
    EXPECT_FALSE(tree.empty());

    tree.clear();

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.get_leftmost_leaf(), nullptr);
    EXPECT_EQ(tree.get_rightmost_leaf(), nullptr);
    EXPECT_EQ(tree.find(0), nullptr);
    EXPECT_EQ(tree.find(50), nullptr);
}

TEST(BPlusTreeClear, ClearEmptyTree)
{
    Tree::BPlusTree<int, int> tree;
    tree.clear();  // 应该安全
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
}

TEST(BPlusTreeClear, InsertAfterClear)
{
    Tree::BPlusTree<int, int> tree;

    for (int i = 0; i < 50; ++i)
    {
        tree.insert(i, i);
    }
    tree.clear();

    // 清空后重新插入
    for (int i = 0; i < 30; ++i)
    {
        tree.insert(i * 2, i * 20);
    }

    EXPECT_EQ(tree.size(), 30);
    for (int i = 0; i < 30; ++i)
    {
        auto* val = tree.find(i * 2);
        ASSERT_NE(val, nullptr);
        EXPECT_EQ(*val, i * 20);
    }
}

// ============================================================
// 10. 大批量测试
// ============================================================

TEST(BPlusTreeLargeTest, Insert5000SequentialThenFindAll)
{
    Tree::BPlusTree<int, int> tree;
    constexpr int N = 5000;

    for (int i = 0; i < N; ++i)
    {
        tree.insert(i, i * 2);
    }

    EXPECT_EQ(tree.size(), N);
    EXPECT_FALSE(tree.empty());

    // 验证所有键
    for (int i = 0; i < N; ++i)
    {
        auto* val = tree.find(i);
        ASSERT_NE(val, nullptr) << "Missing key: " << i;
        EXPECT_EQ(*val, i * 2);
    }

    // 验证叶节点链表
    auto* leaf = tree.get_leftmost_leaf();
    ASSERT_NE(leaf, nullptr);
    int total = 0;
    while (leaf)
    {
        total += leaf->leaf_data().size();
        leaf = leaf->leaf_next();
    }
    EXPECT_EQ(total, N);
}

TEST(BPlusTreeLargeTest, Insert5000ThenEraseAll)
{
    Tree::BPlusTree<int, int> tree;
    constexpr int N = 5000;

    for (int i = 0; i < N; ++i)
    {
        tree.insert(i, i);
    }
    EXPECT_EQ(tree.size(), N);

    // 逐个删除
    for (int i = 0; i < N; ++i)
    {
        EXPECT_TRUE(tree.erase(i)) << "Failed to erase key: " << i;
    }

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
    EXPECT_EQ(tree.get_leftmost_leaf(), nullptr);
    EXPECT_EQ(tree.get_rightmost_leaf(), nullptr);
}

TEST(BPlusTreeLargeTest, Insert5000ReverseThenEraseAll)
{
    Tree::BPlusTree<int, int> tree;
    constexpr int N = 5000;

    // 逆序插入
    for (int i = N - 1; i >= 0; --i)
    {
        tree.insert(i, i * 3);
    }

    EXPECT_EQ(tree.size(), N);

    // 验证所有键
    for (int i = 0; i < N; ++i)
    {
        auto* val = tree.find(i);
        ASSERT_NE(val, nullptr) << "Missing key: " << i;
        EXPECT_EQ(*val, i * 3);
    }

    // 逆序删除
    for (int i = N - 1; i >= 0; --i)
    {
        EXPECT_TRUE(tree.erase(i));
    }

    EXPECT_TRUE(tree.empty());
}

TEST(BPlusTreeLargeTest, Insert5000RandomThenEraseAll)
{
    Tree::BPlusTree<int, int> tree;
    constexpr int N = 5000;

    std::vector<int> keys(N);
    for (int i = 0; i < N; ++i)
    {
        keys[i] = i;
    }
    std::mt19937 rng(12345);
    std::shuffle(keys.begin(), keys.end(), rng);

    for (int k : keys)
    {
        tree.insert(k, k);
    }
    EXPECT_EQ(tree.size(), N);

    // 再次 shuffle 来随机删除顺序
    std::shuffle(keys.begin(), keys.end(), rng);

    for (int k : keys)
    {
        EXPECT_TRUE(tree.erase(k)) << "Failed to erase key: " << k;
    }

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0);
}

// ============================================================
// 11. 分配器相关测试
// ============================================================

TEST(BPlusTreeAllocator, CustomAllocator)
{
    // 使用自定义分配器（标准分配器的 rebind）
    using CustomAlloc = std::allocator<std::pair<const int, std::string>>;
    Tree::BPlusTree<int, std::string, CustomAlloc> tree(CustomAlloc{}, 4);

    tree.insert(1, "one");
    tree.insert(2, "two");

    EXPECT_EQ(tree.size(), 2);
    EXPECT_EQ(*tree.find(1), "one");
    EXPECT_EQ(*tree.find(2), "two");
}

TEST(BPlusTreeAllocator, GetAllocator)
{
    using CustomAlloc = std::allocator<std::pair<const int, int>>;
    CustomAlloc alloc;
    Tree::BPlusTree<int, int, CustomAlloc> tree(alloc, 5);

    auto returned_alloc = tree.get_allocator();
    EXPECT_EQ(returned_alloc, alloc);
}

// ============================================================
// 12. 根节点访问测试
// ============================================================

TEST(BPlusTreeRoot, RootNotNullAfterInsert)
{
    Tree::BPlusTree<int, int> tree;
    EXPECT_EQ(tree.root(), nullptr);

    tree.insert(1, 10);
    EXPECT_NE(tree.root(), nullptr);

    tree.erase(1);
    EXPECT_EQ(tree.root(), nullptr);
}

TEST(BPlusTreeRoot, ConstRootAccess)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(1, 10);
    tree.insert(2, 20);

    const auto& cref = tree;
    EXPECT_NE(cref.root(), nullptr);
}

// ============================================================
// 13. 内存使用测试
// ============================================================

TEST(BPlusTreeMemoryUsage, MemoryUsageWithData)
{
Tree::BPlusTree<int, int> tree;

    EXPECT_GT(tree.memory_usage(), 0);  // 至少包含对象自身

    tree.insert(1, 10);
    EXPECT_GT(tree.memory_usage(), sizeof(tree));
}

TEST(BPlusTreeMemoryUsage, MemoryUsageAfterClear)
{
    Tree::BPlusTree<int, int> tree;
    tree.insert(1, 10);

    tree.clear();
    // clear 后内存使用应只包含对象自身
    EXPECT_GE(tree.memory_usage(), sizeof(tree));
}

// ============================================================
// 14. 压力测试：交错插入/删除
// ============================================================

TEST(BPlusTreeStress, InterleavedInsertErase)
{
    Tree::BPlusTree<int, int> tree(std::allocator<std::pair<const int, int>>{}, 4);
    std::mt19937 rng(9999);

    constexpr int OPS = 500;
    std::set<int> reference;  // 用于验证正确性

    std::uniform_int_distribution<int> key_dist(0, 200);
    std::uniform_int_distribution<int> op_dist(0, 1);

    for (int i = 0; i < OPS; ++i)
    {
        int key = key_dist(rng);

        if (op_dist(rng) == 0 || reference.empty())
        {
            // 插入
            tree.insert(key, key * 10);
            reference.insert(key);
        }
        else
        {
            // 删除
            // 从 reference 中随机选一个存在的键
            auto it = reference.lower_bound(key);
            if (it == reference.end())
            {
                it = reference.begin();
            }
            int to_erase = *it;

            bool erased = tree.erase(to_erase);
            EXPECT_TRUE(erased) << "Failed to erase key: " << to_erase;
            reference.erase(to_erase);
        }

        EXPECT_EQ(tree.size(), reference.size());
    }

    // 最终验证：tree 中的所有键与 reference 一致
    for (int k : reference)
    {
        auto* val = tree.find(k);
        ASSERT_NE(val, nullptr) << "Missing key in tree: " << k;
        EXPECT_EQ(*val, k * 10);
    }
}
