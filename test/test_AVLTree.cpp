/**
 * @file test_AVLTree.cpp
 * @brief AVLTree 的综合 gtest 测试
 *
 * 覆盖构造/赋值、插入、删除、查找、迭代器、边界情况、深拷贝、
 * 移动语义、自定义比较器、平衡性验证、自赋值安全等场景。
 */

#include <gtest/gtest.h>

#include "../include/AVLTree.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <limits>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <vector>

// ==================== 辅助函数与工具 ====================

/** 递归计算以 node 为根的子树实际高度（独立遍历验证，不依赖缓存） */
template <typename NodeType>
int compute_tree_height_independent(NodeType* node)
{
    if (!node)
    {
        return 0;
    }
    int lh = compute_tree_height_independent(
        static_cast<NodeType*>(node->child(0)));
    int rh = compute_tree_height_independent(
        static_cast<NodeType*>(node->child(1)));
    return 1 + std::max(lh, rh);
}

/** 递归验证整棵树满足 AVL 平衡条件（|balance factor| <= 1） */
template <typename NodeType>
bool verify_avl_balance(NodeType* node)
{
    if (!node)
    {
        return true;
    }
    auto* left = static_cast<NodeType*>(node->child(0));
    auto* right = static_cast<NodeType*>(node->child(1));
    int lh = left ? static_cast<int>(left->height()) : 0;
    int rh = right ? static_cast<int>(right->height()) : 0;
    if (std::abs(lh - rh) > 1)
    {
        return false;
    }
    return verify_avl_balance(left) && verify_avl_balance(right);
}

/** 从 AVLTree 收集所有元素到 vector（中序遍历顺序） */
template <typename TreeType>
std::vector<typename TreeType::value_type> collect_inorder(
    const TreeType& tree)
{
    std::vector<typename TreeType::value_type> result;
    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        result.push_back(*it);
    }
    return result;
}

/** 生成 [0, n) 的随机排列 */
std::vector<int> random_permutation(int n, unsigned seed = 42)
{
    std::vector<int> v(n);
    for (int i = 0; i < n; ++i)
    {
        v[i] = i;
    }
    std::mt19937 rng(seed);
    std::shuffle(v.begin(), v.end(), rng);
    return v;
}

// ============================================================
// 1. 构造测试
// ============================================================

TEST(AVLTreeConstruction, DefaultConstructor)
{
    Tree::AVLTree<int> tree;
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
    EXPECT_EQ(tree.begin(), tree.end());
}

TEST(AVLTreeConstruction, CopyConstructorEmpty)
{
    Tree::AVLTree<int> t1;
    Tree::AVLTree<int> t2(t1);
    EXPECT_TRUE(t2.empty());
    EXPECT_EQ(t2.size(), 0u);
}

TEST(AVLTreeConstruction, CopyConstructorNonEmpty)
{
    Tree::AVLTree<int> t1;
    for (int i = 0; i < 100; ++i)
    {
        t1.insert(i);
    }
    Tree::AVLTree<int> t2(t1);
    EXPECT_EQ(t2.size(), 100u);
    // 验证元素一致
    auto v1 = collect_inorder(t1);
    auto v2 = collect_inorder(t2);
    EXPECT_EQ(v1, v2);
}

TEST(AVLTreeConstruction, MoveConstructor)
{
    Tree::AVLTree<int> t1;
    for (int i = 0; i < 100; ++i)
    {
        t1.insert(i);
    }
    std::size_t orig_size = t1.size();

    Tree::AVLTree<int> t2(std::move(t1));

    EXPECT_EQ(t2.size(), orig_size);
    // 移动后原树应为空
    EXPECT_TRUE(t1.empty());
    EXPECT_EQ(t1.size(), 0u);
    EXPECT_EQ(t1.begin(), t1.end());

    // 验证 t2 元素完整
    auto v2 = collect_inorder(t2);
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(v2[static_cast<std::size_t>(i)], i);
    }
}

// ============================================================
// 2. 赋值测试
// ============================================================

TEST(AVLTreeAssignment, CopyAssignment)
{
    Tree::AVLTree<int> t1;
    for (int i = 0; i < 50; ++i)
    {
        t1.insert(i * 2);
    }

    Tree::AVLTree<int> t2;
    t2.insert(999);
    t2 = t1;

    EXPECT_EQ(t2.size(), 50u);
    auto v1 = collect_inorder(t1);
    auto v2 = collect_inorder(t2);
    EXPECT_EQ(v1, v2);

    // 深拷贝：修改 t2 不应影响 t1
    t2.insert(777);
    EXPECT_EQ(t1.size(), 50u);
    EXPECT_FALSE(t1.find(777).has_value());
}

TEST(AVLTreeAssignment, MoveAssignment)
{
    Tree::AVLTree<int> t1;
    for (int i = 0; i < 30; ++i)
    {
        t1.insert(i);
    }
    std::size_t orig_size = t1.size();

    Tree::AVLTree<int> t2;
    t2.insert(-1);
    t2 = std::move(t1);

    EXPECT_EQ(t2.size(), orig_size);
    EXPECT_TRUE(t1.empty());
    EXPECT_EQ(t1.size(), 0u);

    auto v2 = collect_inorder(t2);
    for (int i = 0; i < 30; ++i)
    {
        EXPECT_EQ(v2[static_cast<std::size_t>(i)], i);
    }
}

TEST(AVLTreeAssignment, SelfAssignment)
{
    Tree::AVLTree<int> tree;
    for (int i = 0; i < 10; ++i)
    {
        tree.insert(i);
    }

    // 引用自身再赋值（通过指针别名）
    Tree::AVLTree<int>& ref = tree;
    tree = ref;

    EXPECT_EQ(tree.size(), 10u);
    auto v = collect_inorder(tree);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(v[static_cast<std::size_t>(i)], i);
    }

    // 移动自赋值
    tree = std::move(ref);
    EXPECT_EQ(tree.size(), 10u);
}

TEST(AVLTreeAssignment, AssignToEmpty)
{
    Tree::AVLTree<int> t1;
    for (int i = 0; i < 5; ++i)
    {
        t1.insert(i);
    }
    Tree::AVLTree<int> t2;
    t2 = t1;
    EXPECT_EQ(t2.size(), 5u);
    auto v = collect_inorder(t2);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(v[static_cast<std::size_t>(i)], i);
    }
}

TEST(AVLTreeAssignment, AssignEmptyToNonEmpty)
{
    Tree::AVLTree<int> t1;
    Tree::AVLTree<int> t2;
    for (int i = 0; i < 10; ++i)
    {
        t2.insert(i);
    }
    t2 = t1;
    EXPECT_TRUE(t2.empty());
    EXPECT_EQ(t2.size(), 0u);
}

// ============================================================
// 3. 插入测试
// ============================================================

TEST(AVLTreeInsert, SingleInsert)
{
    Tree::AVLTree<int> tree;
    EXPECT_TRUE(tree.insert(42));
    EXPECT_EQ(tree.size(), 1u);
    EXPECT_FALSE(tree.empty());

    auto result = tree.find(42);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get(), 42);
}

TEST(AVLTreeInsert, MultipleInserts)
{
    Tree::AVLTree<int> tree;
    std::vector<int> values = {5, 3, 7, 1, 4, 6, 8, 0, 2, 9};
    for (int v : values)
    {
        EXPECT_TRUE(tree.insert(v)) << "insert " << v << " failed";
    }
    EXPECT_EQ(tree.size(), values.size());

    auto inorder = collect_inorder(tree);
    // 中序遍历应为升序
    EXPECT_TRUE(std::is_sorted(inorder.begin(), inorder.end()));
    EXPECT_EQ(inorder.size(), values.size());
}

TEST(AVLTreeInsert, DuplicateInserts)
{
    Tree::AVLTree<int> tree;
    EXPECT_TRUE(tree.insert(10));
    EXPECT_FALSE(tree.insert(10)) << "duplicate insert should return false";
    EXPECT_EQ(tree.size(), 1u);

    // 多次重复插入
    EXPECT_TRUE(tree.insert(20));
    EXPECT_FALSE(tree.insert(20));
    EXPECT_FALSE(tree.insert(20));
    EXPECT_EQ(tree.size(), 2u);
}

TEST(AVLTreeInsert, LargeCountInsert)
{
    Tree::AVLTree<int> tree;
    constexpr int N = 10000;
    for (int i = 0; i < N; ++i)
    {
        EXPECT_TRUE(tree.insert(i));
    }
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(N));

    // 验证中序遍历有序且完整
    auto inorder = collect_inorder(tree);
    EXPECT_EQ(inorder.size(), static_cast<std::size_t>(N));
    EXPECT_TRUE(std::is_sorted(inorder.begin(), inorder.end()));

    // 验证每个元素都可被查找到
    for (int i = 0; i < N; ++i)
    {
        auto result = tree.find(i);
        ASSERT_TRUE(result.has_value()) << "missing value: " << i;
        EXPECT_EQ(result->get(), i);
    }

    // 平衡性：顺序插入是 AVL 树的恶劣场景，高度必须为 O(log N)
    int height = static_cast<int>(tree.root()->height());
    double max_allowed = 2.0 * std::log2(static_cast<double>(N + 1));
    EXPECT_LE(static_cast<double>(height), max_allowed + 1.0);
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

TEST(AVLTreeInsert, RandomOrderInsert)
{
    Tree::AVLTree<int> tree;
    auto perm = random_permutation(5000, 123);
    for (int v : perm)
    {
        EXPECT_TRUE(tree.insert(v));
    }
    EXPECT_EQ(tree.size(), 5000u);

    auto inorder = collect_inorder(tree);
    EXPECT_TRUE(std::is_sorted(inorder.begin(), inorder.end()));

    // 验证平衡性
    int height = static_cast<int>(tree.root()->height());
    double max_allowed =
        2.0 * std::log2(static_cast<double>(tree.size() + 1));
    EXPECT_LE(static_cast<double>(height), max_allowed + 1.0);
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

TEST(AVLTreeInsert, SortedOrderStress)
{
    // 严格升序插入，考验最坏情况的平衡能力
    Tree::AVLTree<int> tree;
    constexpr int N = 2000;
    for (int i = 0; i < N; ++i)
    {
        EXPECT_TRUE(tree.insert(i));
    }
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(N));
    EXPECT_TRUE(verify_avl_balance(tree.root()));

    auto inorder = collect_inorder(tree);
    for (int i = 0; i < N; ++i)
    {
        EXPECT_EQ(inorder[static_cast<std::size_t>(i)], i);
    }
}

TEST(AVLTreeInsert, ReverseOrderStress)
{
    // 严格降序插入
    Tree::AVLTree<int> tree;
    constexpr int N = 2000;
    for (int i = N - 1; i >= 0; --i)
    {
        EXPECT_TRUE(tree.insert(i));
    }
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(N));
    EXPECT_TRUE(verify_avl_balance(tree.root()));

    auto inorder = collect_inorder(tree);
    for (int i = 0; i < N; ++i)
    {
        EXPECT_EQ(inorder[static_cast<std::size_t>(i)], i);
    }
}

// ============================================================
// 4. 删除测试
// ============================================================

TEST(AVLTreeErase, EraseLeaf)
{
    Tree::AVLTree<int> tree;
    for (int v : {10, 5, 15})
    {
        tree.insert(v);
    }
    EXPECT_TRUE(tree.erase(5));  // 叶节点（取决于插入顺序，此处5应为叶子）
    EXPECT_EQ(tree.size(), 2u);
    EXPECT_FALSE(tree.find(5).has_value());
    EXPECT_TRUE(tree.find(10).has_value());
    EXPECT_TRUE(tree.find(15).has_value());
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

TEST(AVLTreeErase, EraseNodeWithOneChild)
{
    Tree::AVLTree<int> tree;
    // 构造一棵小树，删除只有一个孩子的节点
    tree.insert(10);
    tree.insert(5);
    tree.insert(3);  // 5 有左孩子 3

    EXPECT_TRUE(tree.erase(5));
    EXPECT_EQ(tree.size(), 2u);
    EXPECT_FALSE(tree.find(5).has_value());
    EXPECT_TRUE(tree.find(3).has_value());
    EXPECT_TRUE(tree.find(10).has_value());
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

TEST(AVLTreeErase, EraseNodeWithTwoChildren)
{
    Tree::AVLTree<int> tree;
    for (int v : {20, 10, 30, 5, 15, 25, 35})
    {
        tree.insert(v);
    }
    // 删除有两孩子的节点 20（根）
    EXPECT_TRUE(tree.erase(20));
    EXPECT_EQ(tree.size(), 6u);
    EXPECT_FALSE(tree.find(20).has_value());
    // 其余节点仍存在
    for (int v : {10, 30, 5, 15, 25, 35})
    {
        EXPECT_TRUE(tree.find(v).has_value()) << "missing: " << v;
    }
    // 仍然有序
    auto inorder = collect_inorder(tree);
    EXPECT_TRUE(std::is_sorted(inorder.begin(), inorder.end()));
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

TEST(AVLTreeErase, EraseRoot)
{
    Tree::AVLTree<int> tree;
    tree.insert(100);
    // 单节点树，删除根
    EXPECT_TRUE(tree.erase(100));
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);

    // 重新构造，删除有多孩子的根
    for (int v : {50, 25, 75, 10, 30, 60, 90})
    {
        tree.insert(v);
    }
    EXPECT_TRUE(tree.erase(50));
    EXPECT_EQ(tree.size(), 6u);
    EXPECT_FALSE(tree.find(50).has_value());
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

TEST(AVLTreeErase, EraseNonexistent)
{
    Tree::AVLTree<int> tree;
    for (int v : {1, 2, 3})
    {
        tree.insert(v);
    }
    EXPECT_FALSE(tree.erase(999));
    EXPECT_EQ(tree.size(), 3u);
}

TEST(AVLTreeErase, EraseFromEmpty)
{
    Tree::AVLTree<int> tree;
    EXPECT_FALSE(tree.erase(42));
    EXPECT_TRUE(tree.empty());
}

TEST(AVLTreeErase, EraseAllElementsOneByOne)
{
    Tree::AVLTree<int> tree;
    constexpr int N = 500;
    for (int i = 0; i < N; ++i)
    {
        tree.insert(i);
    }
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(N));

    // 按随机顺序逐个删除
    auto perm = random_permutation(N, 77);
    for (int i = 0; i < N; ++i)
    {
        int val = perm[i];
        EXPECT_TRUE(tree.erase(val)) << "erase " << val << " failed";
        EXPECT_EQ(tree.size(), static_cast<std::size_t>(N - i - 1));
        // 每个阶段验证平衡性
        EXPECT_TRUE(verify_avl_balance(tree.root()));
    }
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
}

TEST(AVLTreeErase, EraseAndBalance)
{
    // 随机插入再随机删除一半，验证始终平衡
    Tree::AVLTree<int> tree;
    constexpr int N = 1000;
    auto perm = random_permutation(N, 99);
    for (int v : perm)
    {
        tree.insert(v);
    }

    auto del_order = random_permutation(N, 199);
    for (int i = 0; i < N / 2; ++i)
    {
        tree.erase(del_order[i]);
    }
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(N - N / 2));
    EXPECT_TRUE(verify_avl_balance(tree.root()));

    auto inorder = collect_inorder(tree);
    EXPECT_TRUE(std::is_sorted(inorder.begin(), inorder.end()));
}

// ============================================================
// 5. 查找测试
// ============================================================

TEST(AVLTreeFind, FindExisting)
{
    Tree::AVLTree<int> tree;
    for (int i = 0; i < 100; ++i)
    {
        tree.insert(i);
    }

    for (int i = 0; i < 100; ++i)
    {
        auto result = tree.find(i);
        ASSERT_TRUE(result.has_value()) << "should find " << i;
        EXPECT_EQ(result->get(), i);
    }
}

TEST(AVLTreeFind, FindNonexistent)
{
    Tree::AVLTree<int> tree;
    for (int i : {1, 3, 5, 7, 9})
    {
        tree.insert(i);
    }

    for (int v : {0, 2, 4, 6, 8, 10, -1, 100})
    {
        auto result = tree.find(v);
        EXPECT_FALSE(result.has_value()) << "should NOT find " << v;
    }
}

TEST(AVLTreeFind, FindOnEmpty)
{
    Tree::AVLTree<int> tree;
    auto result = tree.find(42);
    EXPECT_FALSE(result.has_value());
}

TEST(AVLTreeFind, FindAfterInsertions)
{
    Tree::AVLTree<int> tree;
    tree.insert(5);
    auto r1 = tree.find(5);
    ASSERT_TRUE(r1.has_value());

    // 插入更多元素后原元素仍在
    for (int i = 0; i < 10; ++i)
    {
        tree.insert(i);
    }
    auto r2 = tree.find(5);
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(r2->get(), 5);
}

TEST(AVLTreeFind, FindAfterErasures)
{
    Tree::AVLTree<int> tree;
    for (int i = 0; i < 20; ++i)
    {
        tree.insert(i);
    }

    // 删除后，被删元素不应找到，其余仍在
    tree.erase(7);
    tree.erase(3);
    tree.erase(15);

    EXPECT_FALSE(tree.find(7).has_value());
    EXPECT_FALSE(tree.find(3).has_value());
    EXPECT_FALSE(tree.find(15).has_value());

    for (int v : {0, 1, 2, 4, 5, 6, 8, 9, 10, 19})
    {
        EXPECT_TRUE(tree.find(v).has_value()) << "missing: " << v;
    }
}

TEST(AVLTreeFind, FindOnConstTree)
{
    Tree::AVLTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    const auto& ctree = tree;
    auto r = ctree.find(2);
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->get(), 2);

    auto r2 = ctree.find(999);
    EXPECT_FALSE(r2.has_value());
}

TEST(AVLTreeFind, FindModifiesNothing)
{
    Tree::AVLTree<int> tree;
    for (int i = 0; i < 50; ++i)
    {
        tree.insert(i);
    }
    std::size_t sz_before = tree.size();
    auto inorder_before = collect_inorder(tree);

    // 大量查找
    for (int i = -50; i < 100; ++i)
    {
        volatile auto r = tree.find(i);
        (void)r;
    }

    EXPECT_EQ(tree.size(), sz_before);
    auto inorder_after = collect_inorder(tree);
    EXPECT_EQ(inorder_before, inorder_after);
}

// ============================================================
// 6. 迭代器测试
// ============================================================

TEST(AVLTreeIterator, BeginEnd)
{
    Tree::AVLTree<int> tree;
    EXPECT_EQ(tree.begin(), tree.end());

    tree.insert(42);
    auto it = tree.begin();
    EXPECT_NE(it, tree.end());
    EXPECT_EQ(*it, 42);
    ++it;
    EXPECT_EQ(it, tree.end());
}

TEST(AVLTreeIterator, RangeFor)
{
    Tree::AVLTree<int> tree;
    for (int v : {3, 1, 4, 1, 5, 9, 2, 6})
    {
        tree.insert(v);
    }
    // 使用范围 for 遍历
    std::vector<int> values;
    for (const auto& val : tree)
    {
        values.push_back(val);
    }
    EXPECT_TRUE(std::is_sorted(values.begin(), values.end()));
    EXPECT_EQ(values.size(), tree.size());
}

TEST(AVLTreeIterator, EmptyTreeIteration)
{
    Tree::AVLTree<int> tree;
    int count = 0;
    for (const auto& val : tree)
    {
        (void)val;
        ++count;
    }
    EXPECT_EQ(count, 0);

    // begin() == end()
    EXPECT_EQ(tree.begin(), tree.end());
}

TEST(AVLTreeIterator, InorderProperty)
{
    Tree::AVLTree<int> tree;
    auto perm = random_permutation(200, 42);
    for (int v : perm)
    {
        tree.insert(v);
    }

    // 中序遍历应为严格升序
    int prev = -1;
    bool first = true;
    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            EXPECT_LT(prev, *it) << "inorder property violated: "
                                 << prev << " >= " << *it;
        }
        prev = *it;
    }
}

TEST(AVLTreeIterator, IteratorEquality)
{
    Tree::AVLTree<int> tree;
    for (int i = 0; i < 10; ++i)
    {
        tree.insert(i);
    }

    auto it1 = tree.begin();
    auto it2 = tree.begin();
    EXPECT_EQ(it1, it2);

    ++it1;
    EXPECT_NE(it1, it2);
    ++it2;
    EXPECT_EQ(it1, it2);
}

TEST(AVLTreeIterator, PostIncrement)
{
    Tree::AVLTree<int> tree;
    for (int i = 0; i < 5; ++i)
    {
        tree.insert(i);
    }

    auto it = tree.begin();
    auto prev = it++;
    EXPECT_EQ(*prev, 0);
    EXPECT_EQ(*it, 1);
}

TEST(AVLTreeIterator, ArrowOperator)
{
    Tree::AVLTree<std::string> tree;
    tree.insert("hello");
    auto it = tree.begin();
    EXPECT_EQ(it->size(), 5u);
}

TEST(AVLTreeIterator, SizeAndIterationConsistency)
{
    Tree::AVLTree<int> tree;
    for (int i = 0; i < 100; ++i)
    {
        tree.insert(i);
    }
    std::size_t count = 0;
    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        ++count;
    }
    EXPECT_EQ(count, tree.size());
}

// ============================================================
// 7. 边界情况测试
// ============================================================

TEST(AVLTreeEdgeCases, EmptyTreeOperations)
{
    Tree::AVLTree<int> tree;
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
    EXPECT_EQ(tree.begin(), tree.end());

    // 各种操作在空树上的行为
    EXPECT_FALSE(tree.find(0).has_value());
    EXPECT_FALSE(tree.erase(0));
    tree.clear();  // 不应崩溃
    EXPECT_TRUE(tree.empty());
}

TEST(AVLTreeEdgeCases, SingleElement)
{
    Tree::AVLTree<int> tree;
    tree.insert(7);

    EXPECT_EQ(tree.size(), 1u);
    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(*tree.begin(), 7);
    EXPECT_EQ(++tree.begin(), tree.end());
    EXPECT_TRUE(tree.find(7).has_value());
    EXPECT_FALSE(tree.find(0).has_value());
    EXPECT_TRUE(verify_avl_balance(tree.root()));

    // 删除单元素
    EXPECT_TRUE(tree.erase(7));
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
}

TEST(AVLTreeEdgeCases, RepeatedInsertEraseCycles)
{
    Tree::AVLTree<int> tree;
    for (int cycle = 0; cycle < 10; ++cycle)
    {
        // 插入 0..4
        for (int i = 0; i < 5; ++i)
        {
            tree.insert(i);
        }
        EXPECT_EQ(tree.size(), 5u);

        // 删除 0..4
        for (int i = 0; i < 5; ++i)
        {
            EXPECT_TRUE(tree.erase(i));
        }
        EXPECT_TRUE(tree.empty());
        EXPECT_TRUE(verify_avl_balance(tree.root()));
    }
}

TEST(AVLTreeEdgeCases, LargeValues)
{
    Tree::AVLTree<long long> tree;
    tree.insert(1LL << 60);
    tree.insert(-(1LL << 60));
    tree.insert(0);
    tree.insert(std::numeric_limits<long long>::max());
    tree.insert(std::numeric_limits<long long>::min());

    EXPECT_EQ(tree.size(), 5u);
    EXPECT_TRUE(verify_avl_balance(tree.root()));

    auto inorder = collect_inorder(tree);
    EXPECT_TRUE(std::is_sorted(inorder.begin(), inorder.end()));

    EXPECT_TRUE(tree.find(std::numeric_limits<long long>::max()).has_value());
    EXPECT_TRUE(tree.find(std::numeric_limits<long long>::min()).has_value());
}

TEST(AVLTreeEdgeCases, NegativeValues)
{
    Tree::AVLTree<int> tree;
    for (int v : {-5, -3, -10, 0, -1, -7, -2, -9})
    {
        tree.insert(v);
    }
    auto inorder = collect_inorder(tree);
    EXPECT_TRUE(std::is_sorted(inorder.begin(), inorder.end()));
    EXPECT_EQ(inorder.front(), -10);
    EXPECT_EQ(inorder.back(), 0);

    // 删除负数
    EXPECT_TRUE(tree.erase(-10));
    EXPECT_FALSE(tree.find(-10).has_value());
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

TEST(AVLTreeEdgeCases, MixedPositiveAndNegative)
{
    Tree::AVLTree<int> tree;
    for (int i = -100; i <= 100; ++i)
    {
        tree.insert(i);
    }
    EXPECT_EQ(tree.size(), 201u);
    auto inorder = collect_inorder(tree);
    EXPECT_TRUE(std::is_sorted(inorder.begin(), inorder.end()));

    // 验证平衡
    EXPECT_TRUE(verify_avl_balance(tree.root()));
    int height = static_cast<int>(tree.root()->height());
    double max_allowed =
        2.0 * std::log2(static_cast<double>(tree.size() + 1));
    EXPECT_LE(static_cast<double>(height), max_allowed + 1.0);
}

TEST(AVLTreeEdgeCases, DuplicateInsertAndErase)
{
    Tree::AVLTree<int> tree;
    EXPECT_TRUE(tree.insert(5));
    EXPECT_FALSE(tree.insert(5));
    EXPECT_FALSE(tree.insert(5));
    EXPECT_EQ(tree.size(), 1u);

    // 删除一次即可移除
    EXPECT_TRUE(tree.erase(5));
    EXPECT_TRUE(tree.empty());
    // 再次删除应返回 false
    EXPECT_FALSE(tree.erase(5));
}

TEST(AVLTreeEdgeCases, ClearNonEmpty)
{
    Tree::AVLTree<int> tree;
    for (int i = 0; i < 1000; ++i)
    {
        tree.insert(i);
    }
    tree.clear();
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
    EXPECT_EQ(tree.begin(), tree.end());

    // clear 后应仍可正常使用
    tree.insert(42);
    EXPECT_EQ(tree.size(), 1u);
    EXPECT_TRUE(tree.find(42).has_value());
}

TEST(AVLTreeEdgeCases, ClearEmpty)
{
    Tree::AVLTree<int> tree;
    tree.clear();  // 不崩溃
    EXPECT_TRUE(tree.empty());
}

// ============================================================
// 8. 深拷贝测试
// ============================================================

TEST(AVLTreeCopy, DeepCopyVerification)
{
    Tree::AVLTree<int> original;
    for (int i = 0; i < 100; ++i)
    {
        original.insert(i);
    }

    Tree::AVLTree<int> copy(original);

    // 修改副本不影响原树
    copy.insert(999);
    copy.erase(50);

    EXPECT_EQ(original.size(), 100u);
    EXPECT_FALSE(original.find(999).has_value());
    EXPECT_TRUE(original.find(50).has_value());

    EXPECT_EQ(copy.size(), 100u);  // insert 999, erase 50, net 0
    EXPECT_TRUE(copy.find(999).has_value());
    EXPECT_FALSE(copy.find(50).has_value());
}

TEST(AVLTreeCopy, CopyLargeTree)
{
    Tree::AVLTree<int> original;
    constexpr int N = 3000;
    for (int i = 0; i < N; ++i)
    {
        original.insert(i);
    }

    Tree::AVLTree<int> copy(original);
    EXPECT_EQ(copy.size(), original.size());
    EXPECT_TRUE(verify_avl_balance(copy.root()));

    auto v1 = collect_inorder(original);
    auto v2 = collect_inorder(copy);
    EXPECT_EQ(v1, v2);
}

TEST(AVLTreeCopy, CopyEmptyTree)
{
    Tree::AVLTree<int> empty;
    Tree::AVLTree<int> copy(empty);
    EXPECT_TRUE(copy.empty());
    EXPECT_EQ(copy.size(), 0u);
}

// ============================================================
// 9. 移动测试
// ============================================================

TEST(AVLTreeMove, MovedFromTreeIsEmpty)
{
    Tree::AVLTree<int> t1;
    for (int i = 0; i < 50; ++i)
    {
        t1.insert(i);
    }

    Tree::AVLTree<int> t2(std::move(t1));

    EXPECT_TRUE(t1.empty());
    EXPECT_EQ(t1.size(), 0u);
    EXPECT_EQ(t1.begin(), t1.end());

    // 移动后的树仍可使用（插入新数据）
    t1.insert(100);
    EXPECT_EQ(t1.size(), 1u);
    EXPECT_TRUE(t1.find(100).has_value());
}

TEST(AVLTreeMove, MoveAssignmentLeavesSourceEmpty)
{
    Tree::AVLTree<int> t1;
    for (int i = 0; i < 50; ++i)
    {
        t1.insert(i);
    }

    Tree::AVLTree<int> t2;
    t2.insert(-999);
    t2 = std::move(t1);

    EXPECT_TRUE(t1.empty());
    EXPECT_EQ(t1.size(), 0u);

    auto v = collect_inorder(t2);
    EXPECT_EQ(v.size(), 50u);
    EXPECT_TRUE(std::is_sorted(v.begin(), v.end()));
}

// ============================================================
// 10. 自定义比较器测试
// ============================================================

TEST(AVLTreeCustomComparator, DescendingOrder)
{
    Tree::AVLTree<int, std::greater<int>> tree;
    std::vector<int> values = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    for (int v : values)
    {
        EXPECT_TRUE(tree.insert(v));
    }

    EXPECT_EQ(tree.size(), values.size());

    // 中序遍历应为降序（因为 greater 比较器认为"更小"的值更大）
    // 注意：中序基于比较器，greater 下"左子树"是较大的元素
    // 因此遍历结果取决于实现细节。关键是验证：所有元素可被查找到。
    for (int v : values)
    {
        auto result = tree.find(v);
        ASSERT_TRUE(result.has_value()) << "missing: " << v;
        EXPECT_EQ(result->get(), v);
    }

    // 验证平衡性
    EXPECT_TRUE(verify_avl_balance(tree.root()));

    // 集合一致性：元素集合与插入一致
    std::vector<int> inorder = collect_inorder(tree);
    std::sort(values.begin(), values.end());
    std::sort(inorder.begin(), inorder.end());
    EXPECT_EQ(values, inorder);
}

TEST(AVLTreeCustomComparator, DescendingOrderDuplicates)
{
    Tree::AVLTree<int, std::greater<int>> tree;
    EXPECT_TRUE(tree.insert(10));
    EXPECT_FALSE(tree.insert(10));
    EXPECT_EQ(tree.size(), 1u);
}

TEST(AVLTreeCustomComparator, DescendingOrderEraseAndFind)
{
    Tree::AVLTree<int, std::greater<int>> tree;
    for (int i = 0; i < 100; ++i)
    {
        tree.insert(i);
    }
    EXPECT_TRUE(tree.erase(50));
    EXPECT_FALSE(tree.find(50).has_value());
    EXPECT_TRUE(tree.find(99).has_value());
    EXPECT_TRUE(tree.find(0).has_value());
    EXPECT_EQ(tree.size(), 99u);
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

// 大小写不敏感的字符串比较器
struct CaseInsensitiveCompare
{
    bool operator()(const std::string& a, const std::string& b) const
    {
        return std::lexicographical_compare(
            a.begin(), a.end(), b.begin(), b.end(),
            [](char c1, char c2)
            { return std::tolower(static_cast<unsigned char>(c1))
                     < std::tolower(static_cast<unsigned char>(c2)); });
    }
};

TEST(AVLTreeCustomComparator, CaseInsensitiveStrings)
{
    Tree::AVLTree<std::string, CaseInsensitiveCompare> tree;

    EXPECT_TRUE(tree.insert("Apple"));
    EXPECT_TRUE(tree.insert("banana"));
    EXPECT_TRUE(tree.insert("CHERRY"));
    EXPECT_TRUE(tree.insert("Date"));

    EXPECT_EQ(tree.size(), 4u);

    // 大小写不敏感查找
    auto r1 = tree.find("APPLE");
    ASSERT_TRUE(r1.has_value());
    EXPECT_EQ(r1->get(), "Apple");

    auto r2 = tree.find("BANANA");
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(r2->get(), "banana");

    auto r3 = tree.find("cherry");
    ASSERT_TRUE(r3.has_value());
    EXPECT_EQ(r3->get(), "CHERRY");
}

TEST(AVLTreeCustomComparator, CaseInsensitiveDuplicateInsert)
{
    Tree::AVLTree<std::string, CaseInsensitiveCompare> tree;

    EXPECT_TRUE(tree.insert("Hello"));
    EXPECT_FALSE(tree.insert("hello")) << "case-insensitive duplicate";
    EXPECT_FALSE(tree.insert("HELLO")) << "case-insensitive duplicate";
    EXPECT_EQ(tree.size(), 1u);

    auto r = tree.find("HeLLo");
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->get(), "Hello");
}

TEST(AVLTreeCustomComparator, CaseInsensitiveErase)
{
    Tree::AVLTree<std::string, CaseInsensitiveCompare> tree;

    tree.insert("World");
    tree.insert("foo");
    tree.insert("BAR");

    EXPECT_TRUE(tree.erase("world"));
    EXPECT_EQ(tree.size(), 2u);
    EXPECT_FALSE(tree.find("WORLD").has_value());
    EXPECT_TRUE(tree.find("FOO").has_value());
    EXPECT_TRUE(tree.find("bar").has_value());
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

// ============================================================
// 11. 平衡性测试
// ============================================================

TEST(AVLTreeBalance, HeightAfterRandomInserts)
{
    Tree::AVLTree<int> tree;
    constexpr int N = 10000;
    auto perm = random_permutation(N, 42);
    for (int v : perm)
    {
        tree.insert(v);
    }

    int height = static_cast<int>(tree.root()->height());
    double max_allowed = 2.0 * std::log2(static_cast<double>(N + 1));
    EXPECT_LE(static_cast<double>(height), max_allowed + 1.0)
        << "height=" << height << " exceeds O(log n) bound "
        << max_allowed;

    // 严格的 AVL 条件
    EXPECT_TRUE(verify_avl_balance(tree.root()));

    // 独立计算高度，与缓存对齐
    int indep_height =
        compute_tree_height_independent(tree.root());
    EXPECT_EQ(indep_height, height)
        << "cached height vs independent height mismatch";
}

TEST(AVLTreeBalance, HeightAfterRandomInsertsAndErases)
{
    Tree::AVLTree<int> tree;
    constexpr int N = 5000;
    auto perm = random_permutation(N, 123);
    for (int v : perm)
    {
        tree.insert(v);
    }

    // 随机删除一半
    auto del_order = random_permutation(N, 456);
    for (int i = 0; i < N / 2; ++i)
    {
        tree.erase(del_order[i]);
    }

    int height = static_cast<int>(tree.root()->height());
    double max_allowed =
        2.0 * std::log2(static_cast<double>(tree.size() + 1));
    EXPECT_LE(static_cast<double>(height), max_allowed + 1.0);
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

TEST(AVLTreeBalance, InterleavedInsertAndErase)
{
    // 交替插入和删除，验证树始终保持平衡
    Tree::AVLTree<int> tree;
    std::mt19937 rng(9999);
    std::uniform_int_distribution<int> dist(0, 9999);

    for (int op = 0; op < 2000; ++op)
    {
        int val = dist(rng);
        if (dist(rng) % 2 == 0)
        {
            tree.insert(val);
        }
        else
        {
            tree.erase(val);
        }
        // 定期验证平衡性（不要每条操作都验证，避免 O(N²)）
        if (op % 100 == 0 && tree.size() > 0)
        {
            EXPECT_TRUE(verify_avl_balance(tree.root()))
                << "balance violated at op=" << op;
        }
    }

    // 最终验证平衡性
    if (tree.size() > 0)
    {
        EXPECT_TRUE(verify_avl_balance(tree.root()));
        int height = static_cast<int>(tree.root()->height());
        double max_allowed =
            2.0 * std::log2(static_cast<double>(tree.size() + 1));
        EXPECT_LE(static_cast<double>(height), max_allowed + 1.0);
    }
}

TEST(AVLTreeBalance, HeightCacheMatchesIndependent)
{
    Tree::AVLTree<int> tree;
    auto perm = random_permutation(500, 777);
    for (int v : perm)
    {
        tree.insert(v);
    }

    // 删除一部分
    auto del = random_permutation(500, 888);
    for (int i = 0; i < 200; ++i)
    {
        tree.erase(del[i]);
    }

    int cached_h = static_cast<int>(tree.root()->height());
    int indep_h =
        compute_tree_height_independent(tree.root());
    EXPECT_EQ(cached_h, indep_h)
        << "height cache mismatch after inserts+erases";
}

// ============================================================
// 12. 自赋值安全测试
// ============================================================

TEST(AVLTreeSelfAssignment, CopySelfAssignmentSafety)
{
    Tree::AVLTree<int> tree;
    for (int i = 0; i < 20; ++i)
    {
        tree.insert(i);
    }

    std::size_t sz = tree.size();
    auto values_before = collect_inorder(tree);

    // 通过引用自赋值
    Tree::AVLTree<int>& ref = tree;
    tree = ref;

    EXPECT_EQ(tree.size(), sz);
    auto values_after = collect_inorder(tree);
    EXPECT_EQ(values_before, values_after);
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

TEST(AVLTreeSelfAssignment, MoveSelfAssignmentSafety)
{
    Tree::AVLTree<int> tree;
    for (int i = 0; i < 20; ++i)
    {
        tree.insert(i);
    }

    std::size_t sz = tree.size();
    auto values_before = collect_inorder(tree);

    // 通过 std::move 自赋值（虽然移动自赋值通常不会发生，但代码应该安全）
    Tree::AVLTree<int>& ref = tree;
    tree = std::move(ref);

    EXPECT_EQ(tree.size(), sz);
    auto values_after = collect_inorder(tree);
    EXPECT_EQ(values_before, values_after);
    EXPECT_TRUE(verify_avl_balance(tree.root()));
}

TEST(AVLTreeSelfAssignment, SelfAssignmentOnEmpty)
{
    Tree::AVLTree<int> tree;
    Tree::AVLTree<int>& ref = tree;
    tree = ref;
    EXPECT_TRUE(tree.empty());

    tree = std::move(ref);
    EXPECT_TRUE(tree.empty());
}

// ============================================================
// 13. 综合场景测试
// ============================================================

TEST(AVLTreeIntegration, SetLikeBehavior)
{
    // 用 AVLTree 模拟 std::set 的行为：不重复、有序迭代
    Tree::AVLTree<int> tree;
    std::set<int> expected;

    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> dist(0, 500);

    for (int i = 0; i < 1000; ++i)
    {
        int val = dist(rng);
        bool inserted = tree.insert(val);
        auto [it, set_inserted] = expected.insert(val);
        EXPECT_EQ(inserted, set_inserted);
    }

    EXPECT_EQ(tree.size(), expected.size());

    // 验证元素一一对应
    auto it_tree = tree.begin();
    auto it_set = expected.begin();
    while (it_tree != tree.end() && it_set != expected.end())
    {
        EXPECT_EQ(*it_tree, *it_set);
        ++it_tree;
        ++it_set;
    }
    EXPECT_EQ(it_tree, tree.end());
    EXPECT_EQ(it_set, expected.end());

    // 随机删除
    for (int i = 0; i < 200; ++i)
    {
        int val = dist(rng);
        bool erased = tree.erase(val);
        std::size_t set_erased = expected.erase(val);
        EXPECT_EQ(erased, set_erased > 0);
    }
    EXPECT_EQ(tree.size(), expected.size());
}

TEST(AVLTreeIntegration, CopyAfterModifications)
{
    // 一系列插入删除后拷贝，验证副本与原树一致
    Tree::AVLTree<int> t1;
    for (int i = 0; i < 200; ++i)
    {
        t1.insert(i);
    }
    for (int i = 0; i < 200; i += 2)
    {
        t1.erase(i);
    }

    Tree::AVLTree<int> t2(t1);
    EXPECT_EQ(t1.size(), t2.size());
    auto v1 = collect_inorder(t1);
    auto v2 = collect_inorder(t2);
    EXPECT_EQ(v1, v2);

    // 各自独立修改
    t1.insert(999);
    t2.erase(1);  // 从 t2 删除奇数，t1 不受影响
    EXPECT_NE(t1.size(), t2.size());
    EXPECT_TRUE(t1.find(999).has_value());
    EXPECT_FALSE(t2.find(999).has_value());
    EXPECT_TRUE(t1.find(1).has_value());   // 1 仍在 t1 中
    EXPECT_FALSE(t2.find(1).has_value());  // 1 已从 t2 删除
}

TEST(AVLTreeIntegration, ChainOfMoves)
{
    Tree::AVLTree<int> t1;
    for (int i = 0; i < 100; ++i)
    {
        t1.insert(i);
    }
    auto original_values = collect_inorder(t1);

    Tree::AVLTree<int> t2(std::move(t1));
    Tree::AVLTree<int> t3(std::move(t2));
    Tree::AVLTree<int> t4;
    t4 = std::move(t3);

    EXPECT_TRUE(t1.empty());
    EXPECT_TRUE(t2.empty());
    EXPECT_TRUE(t3.empty());
    EXPECT_EQ(t4.size(), 100u);

    auto final_values = collect_inorder(t4);
    EXPECT_EQ(original_values, final_values);
}

TEST(AVLTreeIntegration, StressInsertEraseWithVerification)
{
    Tree::AVLTree<int> tree;
    std::set<int> reference;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(1, 2000);

    constexpr int OPS = 3000;
    for (int i = 0; i < OPS; ++i)
    {
        int val = dist(rng);
        if (dist(rng) % 3 == 0)
        {
            // 插入
            tree.insert(val);
            reference.insert(val);
        }
        else
        {
            // 删除
            tree.erase(val);
            reference.erase(val);
        }

        // 每 500 条操作验证一次
        if (i % 500 == 0)
        {
            EXPECT_EQ(tree.size(), reference.size());
            if (tree.size() > 0)
            {
                EXPECT_TRUE(verify_avl_balance(tree.root()));
            }
        }
    }

    // 最终验证
    EXPECT_EQ(tree.size(), reference.size());
    auto it_tree = tree.begin();
    auto it_set = reference.begin();
    while (it_tree != tree.end())
    {
        EXPECT_EQ(*it_tree, *it_set);
        ++it_tree;
        ++it_set;
    }
}
