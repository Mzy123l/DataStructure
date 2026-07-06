#include <gtest/gtest.h>

#include <algorithm>
#include <functional>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "../include/RBTree.hpp"

// ============================================================================
// 辅助函数
// ============================================================================

/** 验证中序遍历结果是否严格升序（按默认比较器） */
template <typename Tree>
static bool inorder_is_sorted(const Tree& tree)
{
    if (tree.empty()) return true;
    auto it = tree.begin();
    auto prev = *it;
    ++it;
    for (; it != tree.end(); ++it)
    {
        if (*it < prev) return false;
        prev = *it;
    }
    return true;
}

/** 将树的中序遍历结果收集到 vector 中 */
template <typename Tree>
static std::vector<typename Tree::value_type> collect_inorder(const Tree& tree)
{
    std::vector<typename Tree::value_type> result;
    for (const auto& val : tree)
    {
        result.push_back(val);
    }
    return result;
}

// ============================================================================
// 1. 构造
// ============================================================================

TEST(RBTreeConstruction, DefaultConstructor)
{
    Tree::RBTree<int> tree;
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
    EXPECT_EQ(tree.begin(), tree.end());
}

TEST(RBTreeConstruction, CopyConstructor)
{
    Tree::RBTree<int> orig;
    orig.insert(5);
    orig.insert(3);
    orig.insert(7);
    orig.insert(1);
    orig.insert(9);

    Tree::RBTree<int> copy(orig);
    EXPECT_EQ(copy.size(), orig.size());
    EXPECT_FALSE(copy.empty());

    auto v_orig = collect_inorder(orig);
    auto v_copy = collect_inorder(copy);
    EXPECT_EQ(v_orig, v_copy);

    // 深拷贝验证：修改副本不影响原树
    copy.insert(100);
    EXPECT_NE(copy.size(), orig.size());
    EXPECT_FALSE(orig.find(100).has_value());
}

TEST(RBTreeConstruction, MoveConstructor)
{
    Tree::RBTree<int> orig;
    orig.insert(10);
    orig.insert(20);
    orig.insert(5);

    const auto orig_size = orig.size();

    Tree::RBTree<int> moved(std::move(orig));

    EXPECT_EQ(moved.size(), orig_size);
    EXPECT_TRUE(moved.find(10).has_value());
    EXPECT_TRUE(moved.find(20).has_value());
    EXPECT_TRUE(moved.find(5).has_value());

    // 原树应处于有效但空的状态
    EXPECT_TRUE(orig.empty());
    EXPECT_EQ(orig.size(), 0u);
    EXPECT_EQ(orig.begin(), orig.end());
}

TEST(RBTreeConstruction, CopyConstructorEmpty)
{
    Tree::RBTree<int> empty_tree;
    Tree::RBTree<int> copy(empty_tree);
    EXPECT_TRUE(copy.empty());
    EXPECT_EQ(copy.size(), 0u);
}

TEST(RBTreeConstruction, MoveConstructorEmpty)
{
    Tree::RBTree<int> empty_tree;
    Tree::RBTree<int> moved(std::move(empty_tree));
    EXPECT_TRUE(moved.empty());
    EXPECT_EQ(moved.size(), 0u);
}

TEST(RBTreeConstruction, CustomComparator)
{
    Tree::RBTree<int, std::greater<int>> tree;  // 降序
    tree.insert(1);
    tree.insert(5);
    tree.insert(3);

    // 降序比较器 → 中序遍历应为 5, 3, 1（升序比较器的逆序）
    auto vec = collect_inorder(tree);
    ASSERT_EQ(vec.size(), 3u);
    // 默认 inorder 按 BST 性质走的是"左小右大"的结构存储，
    // 使用 greater 比较器后中序遍历应给出降序结果
    EXPECT_GE(vec[0], vec[1]);
    EXPECT_GE(vec[1], vec[2]);
}

// ============================================================================
// 2. 赋值
// ============================================================================

TEST(RBTreeAssignment, CopyAssignment)
{
    Tree::RBTree<int> orig;
    orig.insert(42);
    orig.insert(17);
    orig.insert(99);

    Tree::RBTree<int> assigned;
    assigned.insert(999);  // 预存数据，赋值时被覆盖

    assigned = orig;

    EXPECT_EQ(assigned.size(), orig.size());
    EXPECT_TRUE(assigned.find(42).has_value());
    EXPECT_TRUE(assigned.find(17).has_value());
    EXPECT_TRUE(assigned.find(99).has_value());
    EXPECT_FALSE(assigned.find(999).has_value());

    // 深拷贝
    assigned.insert(0);
    EXPECT_FALSE(orig.find(0).has_value());
}

TEST(RBTreeAssignment, MoveAssignment)
{
    Tree::RBTree<int> orig;
    orig.insert(1);
    orig.insert(2);
    orig.insert(3);
    const auto orig_sz = orig.size();

    Tree::RBTree<int> target;
    target.insert(-1);  // 预存数据

    target = std::move(orig);

    EXPECT_EQ(target.size(), orig_sz);
    EXPECT_TRUE(orig.empty());
    EXPECT_EQ(orig.size(), 0u);
    EXPECT_FALSE(target.find(-1).has_value());  // 原 target 数据被释放
}

TEST(RBTreeAssignment, SelfCopyAssignment)
{
    Tree::RBTree<int> tree;
    tree.insert(1);
    tree.insert(2);

    const auto sz = tree.size();
    // 自赋值：应该安全且不改变任何东西
    tree = tree;

    EXPECT_EQ(tree.size(), sz);
    EXPECT_TRUE(tree.find(1).has_value());
    EXPECT_TRUE(tree.find(2).has_value());
}

TEST(RBTreeAssignment, SelfMoveAssignment)
{
    Tree::RBTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    const auto sz = tree.size();

    // 自移动赋值：实现中有 if (this != &other) 保护
    tree = std::move(tree);

    EXPECT_EQ(tree.size(), sz);
    EXPECT_TRUE(tree.find(10).has_value());
    EXPECT_TRUE(tree.find(20).has_value());
}

TEST(RBTreeAssignment, CopyAssignmentEmpty)
{
    Tree::RBTree<int> src;
    Tree::RBTree<int> dst;
    dst.insert(99);

    dst = src;
    EXPECT_TRUE(dst.empty());
    EXPECT_EQ(dst.size(), 0u);
}

TEST(RBTreeAssignment, MoveAssignmentEmpty)
{
    Tree::RBTree<int> src;
    Tree::RBTree<int> dst;
    dst.insert(99);

    dst = std::move(src);
    EXPECT_TRUE(dst.empty());
    EXPECT_EQ(dst.size(), 0u);
}

// ============================================================================
// 3. Insert
// ============================================================================

TEST(RBTreeInsert, SingleInsert)
{
    Tree::RBTree<int> tree;
    EXPECT_TRUE(tree.insert(42));
    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(tree.size(), 1u);
    auto found = tree.find(42);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->get(), 42);
}

TEST(RBTreeInsert, MultipleInsert)
{
    Tree::RBTree<int> tree;
    std::vector<int> vals = {50, 30, 70, 20, 40, 60, 80};
    for (int v : vals)
    {
        EXPECT_TRUE(tree.insert(v));
    }
    EXPECT_EQ(tree.size(), vals.size());
    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeInsert, DuplicatesReturnFalse)
{
    Tree::RBTree<int> tree;
    EXPECT_TRUE(tree.insert(10));
    EXPECT_FALSE(tree.insert(10));  // 重复
    EXPECT_FALSE(tree.insert(10));  // 再次重复
    EXPECT_EQ(tree.size(), 1u);
}

TEST(RBTreeInsert, DuplicatesDoNotChangeTree)
{
    Tree::RBTree<int> tree;
    std::vector<int> vals = {5, 3, 8, 1, 4, 7, 9};
    for (int v : vals) tree.insert(v);

    const auto sz_before = tree.size();
    auto inorder_before = collect_inorder(tree);

    // 尝试重复插入所有值
    for (int v : vals)
    {
        EXPECT_FALSE(tree.insert(v));
    }

    EXPECT_EQ(tree.size(), sz_before);
    EXPECT_EQ(collect_inorder(tree), inorder_before);
}

TEST(RBTreeInsert, SequentialOrder)
{
    Tree::RBTree<int> tree;
    const int N = 1000;
    for (int i = 0; i < N; ++i)
    {
        EXPECT_TRUE(tree.insert(i));
    }
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(N));
    EXPECT_TRUE(inorder_is_sorted(tree));

    // 验证所有值都可找到
    for (int i = 0; i < N; ++i)
    {
        auto f = tree.find(i);
        ASSERT_TRUE(f.has_value());
        EXPECT_EQ(f->get(), i);
    }
}

TEST(RBTreeInsert, ReverseSequentialOrder)
{
    Tree::RBTree<int> tree;
    const int N = 1000;
    for (int i = N - 1; i >= 0; --i)
    {
        EXPECT_TRUE(tree.insert(i));
    }
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(N));
    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeInsert, RandomOrder10K)
{
    Tree::RBTree<int> tree;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(
        std::numeric_limits<int>::min(),
        std::numeric_limits<int>::max());

    const int N = 10000;
    std::set<int> ref;

    for (int i = 0; i < N; ++i)
    {
        int val = dist(rng);
        bool rb_ok = tree.insert(val);
        auto set_ok = ref.insert(val);
        EXPECT_EQ(rb_ok, set_ok.second);
    }

    EXPECT_EQ(tree.size(), ref.size());
    EXPECT_TRUE(inorder_is_sorted(tree));

    // 验证中序遍历与 std::set 一致
    auto rb_vec = collect_inorder(tree);
    std::vector<int> set_vec(ref.begin(), ref.end());
    EXPECT_EQ(rb_vec, set_vec);
}

TEST(RBTreeInsert, NegativeValues)
{
    Tree::RBTree<int> tree;
    std::vector<int> vals = {-50, -30, -70, -20, -40, 0, 10, -100};
    for (int v : vals) tree.insert(v);
    EXPECT_EQ(tree.size(), vals.size());
    EXPECT_TRUE(inorder_is_sorted(tree));
    for (int v : vals)
    {
        auto f = tree.find(v);
        ASSERT_TRUE(f.has_value());
        EXPECT_EQ(f->get(), v);
    }
}

TEST(RBTreeInsert, LargeValues)
{
    Tree::RBTree<long long> tree;
    tree.insert(std::numeric_limits<long long>::max());
    tree.insert(std::numeric_limits<long long>::min());
    tree.insert(0LL);

    EXPECT_EQ(tree.size(), 3u);
    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeInsert, StringType)
{
    Tree::RBTree<std::string> tree;
    tree.insert("hello");
    tree.insert("world");
    tree.insert("aaaa");
    tree.insert("zzzz");

    EXPECT_EQ(tree.size(), 4u);
    EXPECT_TRUE(inorder_is_sorted(tree));
}

// ============================================================================
// 4. Erase
// ============================================================================

TEST(RBTreeErase, EraseFromEmpty)
{
    Tree::RBTree<int> tree;
    EXPECT_FALSE(tree.erase(10));
    EXPECT_TRUE(tree.empty());
}

TEST(RBTreeErase, EraseNonexistent)
{
    Tree::RBTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    const auto sz = tree.size();
    EXPECT_FALSE(tree.erase(999));
    EXPECT_EQ(tree.size(), sz);
    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeErase, EraseLeaf)
{
    Tree::RBTree<int> tree;
    // 构建：      10
    //           /    \
    //          5     15
    //         / \   /  \
    //        3   7 12  18
    for (int v : {10, 5, 15, 3, 7, 12, 18}) tree.insert(v);

    // 删除叶子节点 3
    EXPECT_TRUE(tree.erase(3));
    EXPECT_EQ(tree.size(), 6u);
    EXPECT_FALSE(tree.find(3).has_value());
    EXPECT_TRUE(inorder_is_sorted(tree));

    // 删除叶子节点 18
    EXPECT_TRUE(tree.erase(18));
    EXPECT_EQ(tree.size(), 5u);
    EXPECT_FALSE(tree.find(18).has_value());
    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeErase, EraseNodeWithOneChild)
{
    Tree::RBTree<int> tree;
    // 构建：      10
    //           /    \
    //          5     15
    //           \   /
    //           7  12
    for (int v : {10, 5, 15, 7, 12}) tree.insert(v);

    // 删除 5（只有右子 7）
    EXPECT_TRUE(tree.erase(5));
    EXPECT_EQ(tree.size(), 4u);
    EXPECT_FALSE(tree.find(5).has_value());
    EXPECT_TRUE(tree.find(7).has_value());
    EXPECT_TRUE(inorder_is_sorted(tree));

    // 删除 15（只有左子 12）
    EXPECT_TRUE(tree.erase(15));
    EXPECT_EQ(tree.size(), 3u);
    EXPECT_FALSE(tree.find(15).has_value());
    EXPECT_TRUE(tree.find(12).has_value());
    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeErase, EraseNodeWithTwoChildren)
{
    Tree::RBTree<int> tree;
    // 构建：      10
    //           /    \
    //          5     15
    //         / \   /  \
    //        3   7 12  18
    for (int v : {10, 5, 15, 3, 7, 12, 18}) tree.insert(v);

    // 删除 10（根，有两个子节点）
    EXPECT_TRUE(tree.erase(10));
    EXPECT_EQ(tree.size(), 6u);
    EXPECT_FALSE(tree.find(10).has_value());
    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeErase, EraseRoot)
{
    Tree::RBTree<int> tree;

    // 单节点树
    tree.insert(42);
    EXPECT_TRUE(tree.erase(42));
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
    EXPECT_FALSE(tree.find(42).has_value());

    // 重新填充多节点，然后删根
    for (int v : {20, 10, 30, 5, 15, 25, 35}) tree.insert(v);
    EXPECT_TRUE(tree.erase(20));  // 根节点
    EXPECT_EQ(tree.size(), 6u);
    EXPECT_FALSE(tree.find(20).has_value());
    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeErase, EraseAllOneByOne)
{
    Tree::RBTree<int> tree;
    std::vector<int> vals = {50, 30, 70, 20, 40, 60, 80, 10, 90};
    for (int v : vals) tree.insert(v);
    EXPECT_EQ(tree.size(), vals.size());

    for (int v : vals)
    {
        EXPECT_TRUE(tree.erase(v));
        EXPECT_FALSE(tree.find(v).has_value());
        if (!tree.empty())
        {
            EXPECT_TRUE(inorder_is_sorted(tree));
        }
    }

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
}

TEST(RBTreeErase, EraseCausingFixupCases)
{
    // 插入大量顺序元素再逐个删除，触发删除修复的多种情况
    Tree::RBTree<int> tree;
    const int N = 1000;
    for (int i = 0; i < N; ++i)
    {
        tree.insert(i);
    }
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(N));
    EXPECT_TRUE(inorder_is_sorted(tree));

    // 逐序删除（从 0 到 N-1，每次都触发修复）
    for (int i = 0; i < N; ++i)
    {
        EXPECT_TRUE(tree.erase(i));
        if (i % 100 == 99 && !tree.empty())
        {
            EXPECT_TRUE(inorder_is_sorted(tree));
        }
    }

    EXPECT_TRUE(tree.empty());
}

TEST(RBTreeErase, EraseReverseOrderFixup)
{
    Tree::RBTree<int> tree;
    const int N = 500;
    for (int i = 0; i < N; ++i) tree.insert(i);

    // 从大到小删除
    for (int i = N - 1; i >= 0; --i)
    {
        EXPECT_TRUE(tree.erase(i));
    }
    EXPECT_TRUE(tree.empty());
}

TEST(RBTreeErase, InterleavedInsertErase)
{
    Tree::RBTree<int> tree;
    std::mt19937 rng(888);
    std::uniform_int_distribution<int> val_dist(1, 1000);
    std::uniform_int_distribution<int> op_dist(0, 3);

    const int ops = 2000;
    for (int i = 0; i < ops; ++i)
    {
        int val = val_dist(rng);
        if (op_dist(rng) <= 2)  // 75% 插入
        {
            tree.insert(val);
        }
        else  // 25% 删除
        {
            tree.erase(val);
        }
    }
    EXPECT_TRUE(inorder_is_sorted(tree));
}

// ============================================================================
// 5. Find
// ============================================================================

TEST(RBTreeFind, FindExisting)
{
    Tree::RBTree<int> tree;
    for (int v : {5, 3, 8, 1, 4, 7, 9}) tree.insert(v);

    for (int v : {5, 3, 8, 1, 4, 7, 9})
    {
        auto result = tree.find(v);
        ASSERT_TRUE(result.has_value()) << "v = " << v;
        EXPECT_EQ(result->get(), v);
    }
}

TEST(RBTreeFind, FindNonexistent)
{
    Tree::RBTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    EXPECT_FALSE(tree.find(0).has_value());
    EXPECT_FALSE(tree.find(4).has_value());
    EXPECT_FALSE(tree.find(100).has_value());
    EXPECT_FALSE(tree.find(-1).has_value());
}

TEST(RBTreeFind, FindInEmptyTree)
{
    Tree::RBTree<int> tree;
    EXPECT_FALSE(tree.find(0).has_value());
    EXPECT_FALSE(tree.find(42).has_value());
}

TEST(RBTreeFind, FindAfterErase)
{
    Tree::RBTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);

    EXPECT_TRUE(tree.find(20).has_value());
    tree.erase(20);
    EXPECT_FALSE(tree.find(20).has_value());
    EXPECT_TRUE(tree.find(10).has_value());
    EXPECT_TRUE(tree.find(30).has_value());
}

TEST(RBTreeFind, FindAfterClear)
{
    Tree::RBTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    tree.clear();
    EXPECT_FALSE(tree.find(1).has_value());
    EXPECT_FALSE(tree.find(2).has_value());
    EXPECT_FALSE(tree.find(3).has_value());
}

TEST(RBTreeFind, FindStringType)
{
    Tree::RBTree<std::string> tree;
    tree.insert("apple");
    tree.insert("banana");
    tree.insert("cherry");

    auto r = tree.find(std::string("banana"));
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->get(), "banana");

    // 用 const char* 查找（依赖 std::less<...> 的异构比较能力？）
    // 注意：Comp = std::less<std::string>，find 接受 const std::string&
}

// ============================================================================
// 6. 迭代器
// ============================================================================

TEST(RBTreeIterator, BeginEndOnEmpty)
{
    Tree::RBTree<int> tree;
    EXPECT_EQ(tree.begin(), tree.end());
}

TEST(RBTreeIterator, BeginEndOnNonEmpty)
{
    Tree::RBTree<int> tree;
    tree.insert(42);
    EXPECT_NE(tree.begin(), tree.end());
}

TEST(RBTreeIterator, ForwardIteration)
{
    Tree::RBTree<int> tree;
    std::vector<int> vals = {30, 20, 40, 10, 25, 35, 50};
    for (int v : vals) tree.insert(v);

    std::sort(vals.begin(), vals.end());
    std::vector<int> result;
    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        result.push_back(*it);
    }
    EXPECT_EQ(result, vals);
}

TEST(RBTreeIterator, RangeFor)
{
    Tree::RBTree<int> tree;
    for (int v : {5, 3, 8, 1}) tree.insert(v);

    std::vector<int> result;
    for (const auto& val : tree)
    {
        result.push_back(val);
    }

    std::vector<int> expected = {1, 3, 5, 8};
    EXPECT_EQ(result, expected);
}

TEST(RBTreeIterator, ConstIteration)
{
    Tree::RBTree<int> tree;
    for (int v : {5, 3, 8, 1, 4}) tree.insert(v);

    const auto& ctree = tree;
    std::vector<int> result;
    for (auto it = ctree.begin(); it != ctree.end(); ++it)
    {
        result.push_back(*it);
    }

    std::vector<int> expected = {1, 3, 4, 5, 8};
    EXPECT_EQ(result, expected);
}

TEST(RBTreeIterator, ConstRangeFor)
{
    Tree::RBTree<int> tree;
    for (int v : {7, 2, 9, 1}) tree.insert(v);

    const auto& ctree = tree;
    std::vector<int> result;
    for (const auto& val : ctree)
    {
        result.push_back(val);
    }

    std::vector<int> expected = {1, 2, 7, 9};
    EXPECT_EQ(result, expected);
}

TEST(RBTreeIterator, PostIncrement)
{
    Tree::RBTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    auto it = tree.begin();
    auto prev = it++;
    EXPECT_EQ(*prev, 1);
    EXPECT_EQ(*it, 2);
}

TEST(RBTreeIterator, InorderProperty)
{
    Tree::RBTree<int> tree;
    std::mt19937 rng(123);
    std::uniform_int_distribution<int> dist(-10000, 10000);

    for (int i = 0; i < 500; ++i)
    {
        tree.insert(dist(rng));
    }
    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeIterator, EmptyTreeIteration)
{
    Tree::RBTree<std::string> tree;
    int count = 0;
    for (const auto& val : tree)
    {
        (void)val;
        ++count;
    }
    EXPECT_EQ(count, 0);

    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        ++count;
    }
    EXPECT_EQ(count, 0);
}

TEST(RBTreeIterator, SingleElementIteration)
{
    Tree::RBTree<int> tree;
    tree.insert(99);

    auto it = tree.begin();
    ASSERT_NE(it, tree.end());
    EXPECT_EQ(*it, 99);

    ++it;
    EXPECT_EQ(it, tree.end());
}

// ============================================================================
// 7. 边界情况
// ============================================================================

TEST(RBTreeEdgeCases, EmptyTreeOperations)
{
    Tree::RBTree<int> tree;
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
    EXPECT_EQ(tree.begin(), tree.end());
    EXPECT_FALSE(tree.find(0).has_value());
    EXPECT_FALSE(tree.erase(0));

    // clear 空树
    tree.clear();
    EXPECT_TRUE(tree.empty());

    // 获取比较器和分配器
    auto comp = tree.key_comp();
    auto alloc = tree.get_allocator();
    (void)comp;
    (void)alloc;
}

TEST(RBTreeEdgeCases, SingleElementTree)
{
    Tree::RBTree<int> tree;
    tree.insert(42);

    EXPECT_FALSE(tree.empty());
    EXPECT_EQ(tree.size(), 1u);

    auto it = tree.begin();
    EXPECT_NE(it, tree.end());
    EXPECT_EQ(*it, 42);
    ++it;
    EXPECT_EQ(it, tree.end());

    // 插入重复
    EXPECT_FALSE(tree.insert(42));
    EXPECT_EQ(tree.size(), 1u);

    // 查找
    auto f = tree.find(42);
    ASSERT_TRUE(f.has_value());
    EXPECT_EQ(f->get(), 42);

    // 删除
    EXPECT_TRUE(tree.erase(42));
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.begin(), tree.end());
}

TEST(RBTreeEdgeCases, RepeatedInsertEraseCycles)
{
    Tree::RBTree<int> tree;
    for (int cycle = 0; cycle < 10; ++cycle)
    {
        for (int i = 0; i < 20; ++i)
        {
            tree.insert(i);
        }
        EXPECT_EQ(tree.size(), 20u);
        EXPECT_TRUE(inorder_is_sorted(tree));

        for (int i = 0; i < 20; ++i)
        {
            tree.erase(i);
        }
        EXPECT_TRUE(tree.empty());
    }
}

TEST(RBTreeEdgeCases, AllSameValueInsert)
{
    Tree::RBTree<int> tree;
    for (int i = 0; i < 100; ++i)
    {
        if (i == 0)
        {
            EXPECT_TRUE(tree.insert(7));
        }
        else
        {
            EXPECT_FALSE(tree.insert(7));
        }
    }
    EXPECT_EQ(tree.size(), 1u);
}

TEST(RBTreeEdgeCases, ExtremeValues)
{
    Tree::RBTree<int> tree;
    tree.insert(std::numeric_limits<int>::max());
    tree.insert(std::numeric_limits<int>::min());
    tree.insert(0);
    tree.insert(1);
    tree.insert(-1);

    EXPECT_EQ(tree.size(), 5u);
    EXPECT_TRUE(inorder_is_sorted(tree));
    EXPECT_TRUE(tree.find(std::numeric_limits<int>::max()).has_value());
    EXPECT_TRUE(tree.find(std::numeric_limits<int>::min()).has_value());
}

// ============================================================================
// 8. 深拷贝
// ============================================================================

TEST(RBTreeCopy, DeepCopyModifyDoesNotAffectOriginal)
{
    Tree::RBTree<int> orig;
    for (int v : {10, 5, 15, 3, 7, 12, 18}) orig.insert(v);

    Tree::RBTree<int> copy(orig);

    // 修改副本
    copy.insert(99);
    copy.erase(5);

    // 原树应不变
    EXPECT_EQ(orig.size(), 7u);
    EXPECT_TRUE(orig.find(5).has_value());
    EXPECT_FALSE(orig.find(99).has_value());
}

TEST(RBTreeCopy, DeepCopyAfterAssignment)
{
    Tree::RBTree<int> orig;
    orig.insert(1);
    orig.insert(2);

    Tree::RBTree<int> assigned;
    assigned = orig;

    assigned.insert(3);
    EXPECT_FALSE(orig.find(3).has_value());
}

TEST(RBTreeCopy, CopyOfEmptyTree)
{
    Tree::RBTree<int> empty;
    Tree::RBTree<int> copy(empty);
    EXPECT_TRUE(copy.empty());

    copy.insert(1);
    EXPECT_FALSE(empty.find(1).has_value());
}

// ============================================================================
// 9. 移动后状态
// ============================================================================

TEST(RBTreeMove, MovedFromIsEmpty)
{
    Tree::RBTree<int> src;
    src.insert(1);
    src.insert(2);
    src.insert(3);

    Tree::RBTree<int> dst(std::move(src));

    EXPECT_TRUE(src.empty());
    EXPECT_EQ(src.size(), 0u);
    EXPECT_EQ(src.begin(), src.end());

    // dst 应包含数据
    EXPECT_EQ(dst.size(), 3u);
    EXPECT_TRUE(dst.find(1).has_value());
    EXPECT_TRUE(dst.find(2).has_value());
    EXPECT_TRUE(dst.find(3).has_value());
}

TEST(RBTreeMove, MoveAssignLeavesSourceEmpty)
{
    Tree::RBTree<int> src;
    src.insert(100);
    src.insert(200);

    Tree::RBTree<int> dst;
    dst = std::move(src);

    EXPECT_TRUE(src.empty());
    EXPECT_EQ(src.size(), 0u);
    EXPECT_EQ(dst.size(), 2u);
    EXPECT_TRUE(dst.find(100).has_value());
}

TEST(RBTreeMove, MoveFromEmpty)
{
    Tree::RBTree<int> empty;
    Tree::RBTree<int> dst(std::move(empty));

    EXPECT_TRUE(dst.empty());
    EXPECT_EQ(dst.size(), 0u);
}

TEST(RBTreeMove, MoveAssignmentFromEmpty)
{
    Tree::RBTree<int> empty;
    Tree::RBTree<int> dst;
    dst.insert(42);

    dst = std::move(empty);
    EXPECT_TRUE(dst.empty());
}

// ============================================================================
// 10. 自定义比较器
// ============================================================================

TEST(RBTreeCustomComparator, DescendingOrder)
{
    Tree::RBTree<int, std::greater<int>> tree;
    tree.insert(1);
    tree.insert(5);
    tree.insert(3);
    tree.insert(7);
    tree.insert(2);

    EXPECT_EQ(tree.size(), 5u);

    auto vec = collect_inorder(tree);
    ASSERT_EQ(vec.size(), 5u);
    // 降序比较器 → 中序遍历应产生降序序列
    for (size_t i = 1; i < vec.size(); ++i)
    {
        EXPECT_GE(vec[i - 1], vec[i]);
    }
}

TEST(RBTreeCustomComparator, FindWithDescendingComparator)
{
    Tree::RBTree<int, std::greater<int>> tree;
    tree.insert(10);
    tree.insert(20);
    tree.insert(5);

    auto r1 = tree.find(10);
    ASSERT_TRUE(r1.has_value());
    EXPECT_EQ(r1->get(), 10);

    auto r2 = tree.find(20);
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(r2->get(), 20);

    EXPECT_FALSE(tree.find(15).has_value());
}

TEST(RBTreeCustomComparator, EraseWithDescendingComparator)
{
    Tree::RBTree<int, std::greater<int>> tree;
    for (int v : {50, 30, 70, 20, 40}) tree.insert(v);

    EXPECT_TRUE(tree.erase(30));
    EXPECT_FALSE(tree.find(30).has_value());
    EXPECT_EQ(tree.size(), 4u);

    auto vec = collect_inorder(tree);
    for (size_t i = 1; i < vec.size(); ++i)
    {
        EXPECT_GE(vec[i - 1], vec[i]);
    }
}

TEST(RBTreeCustomComparator, CopyWithComparator)
{
    Tree::RBTree<int, std::greater<int>> orig;
    orig.insert(1);
    orig.insert(2);
    orig.insert(3);

    Tree::RBTree<int, std::greater<int>> copy(orig);
    EXPECT_EQ(copy.size(), orig.size());

    auto v_orig = collect_inorder(orig);
    auto v_copy = collect_inorder(copy);
    EXPECT_EQ(v_orig, v_copy);
}

TEST(RBTreeCustomComparator, MoveWithComparator)
{
    Tree::RBTree<int, std::greater<int>> orig;
    orig.insert(10);
    orig.insert(5);

    Tree::RBTree<int, std::greater<int>> moved(std::move(orig));
    EXPECT_EQ(moved.size(), 2u);
    EXPECT_TRUE(orig.empty());
}

// ============================================================================
// 11. RBTree 性质验证
// ============================================================================

TEST(RBTreeProperties, InorderIsSortedAfterRandomInserts)
{
    Tree::RBTree<int> tree;
    std::mt19937 rng(777);
    std::uniform_int_distribution<int> dist(-50000, 50000);

    const int N = 2000;
    std::set<int> ref;

    for (int i = 0; i < N; ++i)
    {
        int val = dist(rng);
        tree.insert(val);
        ref.insert(val);
    }

    auto rb_vec = collect_inorder(tree);
    std::vector<int> set_vec(ref.begin(), ref.end());
    EXPECT_EQ(rb_vec, set_vec);
    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeProperties, InorderIsSortedAfterErases)
{
    Tree::RBTree<int> tree;
    const int N = 500;
    for (int i = 0; i < N; ++i) tree.insert(i);

    // 随机删除一半
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, N - 1);
    int to_erase = N / 2;

    for (int i = 0; i < to_erase; ++i)
    {
        tree.erase(dist(rng));
    }

    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeProperties, SizeConsistency)
{
    Tree::RBTree<int> tree;
    EXPECT_EQ(tree.size(), 0u);

    int count = 0;
    for (int v : {5, 3, 8, 1, 4, 7, 9})
    {
        tree.insert(v);
        ++count;
        EXPECT_EQ(tree.size(), static_cast<std::size_t>(count));
    }

    for (int v : {3, 7, 1, 9, 4, 8, 5})
    {
        tree.erase(v);
        --count;
        EXPECT_EQ(tree.size(), static_cast<std::size_t>(count));
    }
}

TEST(RBTreeProperties, BlackRoot)
{
    // 通过公有接口，我们只能验证 BST 性质（排序性）。
    // 红黑颜色属性的严格验证需要访问内部节点指针。
    // 此处验证：大量随机插入/删除后树仍保持 BST 性质。
    Tree::RBTree<int> tree;
    std::mt19937 rng(555);
    std::uniform_int_distribution<int> val_dist(1, 5000);
    std::uniform_int_distribution<int> op_dist(0, 3);

    const int ops = 3000;
    for (int i = 0; i < ops; ++i)
    {
        int val = val_dist(rng);
        if (op_dist(rng) <= 2)
            tree.insert(val);
        else
            tree.erase(val);
    }
    EXPECT_TRUE(inorder_is_sorted(tree));
}

// ============================================================================
// 12. 自赋值安全
// ============================================================================

TEST(RBTreeSelfAssignment, CopySelfAssignPreservesData)
{
    Tree::RBTree<int> tree;
    for (int v : {1, 2, 3, 4, 5}) tree.insert(v);

    auto before = collect_inorder(tree);
    tree = tree;
    auto after = collect_inorder(tree);

    EXPECT_EQ(before, after);
    EXPECT_EQ(tree.size(), 5u);
}

TEST(RBTreeSelfAssignment, MoveSelfAssignPreservesData)
{
    Tree::RBTree<int> tree;
    for (int v : {10, 20, 30}) tree.insert(v);

    auto before = collect_inorder(tree);
    tree = std::move(tree);
    auto after = collect_inorder(tree);

    EXPECT_EQ(before, after);
    EXPECT_EQ(tree.size(), 3u);
}

TEST(RBTreeSelfAssignment, SelfAssignEmpty)
{
    Tree::RBTree<int> tree;
    tree = tree;
    EXPECT_TRUE(tree.empty());

    tree = std::move(tree);
    EXPECT_TRUE(tree.empty());
}

// ============================================================================
// 13. 压力测试
// ============================================================================

TEST(RBTreeStress, Insert10KThenEraseAll)
{
    Tree::RBTree<int> tree;
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> dist(
        std::numeric_limits<int>::min(),
        std::numeric_limits<int>::max());

    const int N = 10000;
    std::set<int> ref;

    // 插入 10000 个随机值
    for (int i = 0; i < N; ++i)
    {
        int val = dist(rng);
        tree.insert(val);
        ref.insert(val);
    }

    EXPECT_EQ(tree.size(), ref.size());
    EXPECT_TRUE(inorder_is_sorted(tree));

    // 逐个删除所有元素
    for (int val : ref)
    {
        EXPECT_TRUE(tree.erase(val));
    }

    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
    EXPECT_EQ(tree.begin(), tree.end());
}

TEST(RBTreeStress, Insert10KSequentialThenEraseReverse)
{
    Tree::RBTree<int> tree;
    const int N = 10000;

    // 顺序插入
    for (int i = 0; i < N; ++i) tree.insert(i);
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(N));
    EXPECT_TRUE(inorder_is_sorted(tree));

    // 逆序删除
    for (int i = N - 1; i >= 0; --i) tree.erase(i);
    EXPECT_TRUE(tree.empty());
}

TEST(RBTreeStress, Insert10KSequentialThenEraseRandomOrder)
{
    Tree::RBTree<int> tree;
    const int N = 10000;

    for (int i = 0; i < N; ++i) tree.insert(i);
    EXPECT_EQ(tree.size(), static_cast<std::size_t>(N));

    // 随机打乱删除顺序
    std::vector<int> vals(N);
    std::iota(vals.begin(), vals.end(), 0);
    std::mt19937 rng(42);
    std::shuffle(vals.begin(), vals.end(), rng);

    for (int v : vals)
    {
        tree.erase(v);
    }
    EXPECT_TRUE(tree.empty());
}

TEST(RBTreeStress, LargeNumberOfOperations)
{
    Tree::RBTree<int> tree;
    std::mt19937 rng(9999);
    std::uniform_int_distribution<int> val_dist(1, 10000);
    std::uniform_real_distribution<double> op_dist(0.0, 1.0);

    const int ops = 10000;
    int expected_size = 0;
    std::set<int> ref;

    for (int i = 0; i < ops; ++i)
    {
        int val = val_dist(rng);
        double op = op_dist(rng);

        if (op < 0.6)  // 60% 插入
        {
            bool rb_ok = tree.insert(val);
            auto set_ok = ref.insert(val);
            EXPECT_EQ(rb_ok, set_ok.second);
        }
        else if (op < 0.95)  // 35% 删除
        {
            bool rb_ok = tree.erase(val);
            size_t set_erased = ref.erase(val);
            EXPECT_EQ(rb_ok, set_erased > 0);
        }
        else  // 5% 查找
        {
            auto rb_found = tree.find(val);
            auto set_found = ref.find(val);
            EXPECT_EQ(rb_found.has_value(), set_found != ref.end());
            if (rb_found.has_value())
            {
                EXPECT_EQ(rb_found->get(), *set_found);
            }
        }

        EXPECT_EQ(tree.size(), ref.size());
    }

    EXPECT_TRUE(inorder_is_sorted(tree));
    auto rb_vec = collect_inorder(tree);
    std::vector<int> set_vec(ref.begin(), ref.end());
    EXPECT_EQ(rb_vec, set_vec);
}

TEST(RBTreeStress, InsertDeleteCycle10K)
{
    // 反复构造和销毁树，测试内存管理
    for (int cycle = 0; cycle < 10; ++cycle)
    {
        Tree::RBTree<int> tree;
        for (int i = 0; i < 1000; ++i)
        {
            tree.insert(i);
        }
        EXPECT_EQ(tree.size(), 1000u);
        EXPECT_TRUE(inorder_is_sorted(tree));
        // tree 在此析构
    }
    // 到达此处说明析构无异常
    SUCCEED();
}

// ============================================================================
// 14. Clear 测试
// ============================================================================

TEST(RBTreeClear, ClearNonEmpty)
{
    Tree::RBTree<int> tree;
    tree.insert(1);
    tree.insert(2);
    tree.insert(3);

    tree.clear();
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
    EXPECT_EQ(tree.begin(), tree.end());
}

TEST(RBTreeClear, ClearEmpty)
{
    Tree::RBTree<int> tree;
    tree.clear();  // 不应崩溃
    EXPECT_TRUE(tree.empty());
}

TEST(RBTreeClear, ReuseAfterClear)
{
    Tree::RBTree<int> tree;
    tree.insert(10);
    tree.insert(20);
    tree.clear();

    // 清空后可以继续使用
    tree.insert(30);
    tree.insert(40);
    EXPECT_EQ(tree.size(), 2u);
    EXPECT_TRUE(tree.find(30).has_value());
    EXPECT_TRUE(inorder_is_sorted(tree));
}

// ============================================================================
// 15. 比较器概念与 key_comp 测试
// ============================================================================

TEST(RBTreeComparator, KeyComp)
{
    Tree::RBTree<int, std::less<int>> tree;
    auto comp = tree.key_comp();
    EXPECT_TRUE(comp(1, 2));
    EXPECT_FALSE(comp(2, 1));
    EXPECT_FALSE(comp(2, 2));
}

TEST(RBTreeComparator, KeyCompGreater)
{
    Tree::RBTree<int, std::greater<int>> tree;
    auto comp = tree.key_comp();
    EXPECT_TRUE(comp(2, 1));
    EXPECT_FALSE(comp(1, 2));
    EXPECT_FALSE(comp(1, 1));
}

// ============================================================================
// 16. 分配器测试
// ============================================================================

TEST(RBTreeAllocator, GetAllocator)
{
    Tree::RBTree<int> tree;
    auto alloc = tree.get_allocator();
    (void)alloc;  // 验证 get_allocator 可以调用
    SUCCEED();
}

TEST(RBTreeAllocator, DefaultAllocatorWorks)
{
    // 使用默认分配器的基本功能
    Tree::RBTree<double> tree;
    for (double v : {1.5, 3.14, 2.71}) tree.insert(v);
    EXPECT_EQ(tree.size(), 3u);
    EXPECT_TRUE(inorder_is_sorted(tree));
}

// ============================================================================
// 17. const 正确性
// ============================================================================

TEST(RBTreeConst, ConstFind)
{
    Tree::RBTree<int> tree;
    tree.insert(42);

    const auto& ctree = tree;
    auto result = ctree.find(42);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get(), 42);

    auto missing = ctree.find(99);
    EXPECT_FALSE(missing.has_value());
}

TEST(RBTreeConst, ConstBeginEnd)
{
    Tree::RBTree<int> tree;
    tree.insert(5);
    tree.insert(3);
    tree.insert(7);

    const auto& ctree = tree;
    std::vector<int> result;
    for (auto it = ctree.begin(); it != ctree.end(); ++it)
    {
        result.push_back(*it);
    }
    EXPECT_EQ(result, std::vector<int>({3, 5, 7}));
}

TEST(RBTreeConst, ConstEmpty)
{
    const Tree::RBTree<int> tree;
    EXPECT_TRUE(tree.empty());
    EXPECT_EQ(tree.size(), 0u);
    EXPECT_EQ(tree.begin(), tree.end());
}

// ============================================================================
// 18. 类型多样性
// ============================================================================

TEST(RBTreeTypes, DoubleType)
{
    Tree::RBTree<double> tree;
    tree.insert(3.14);
    tree.insert(1.59);
    tree.insert(2.65);

    EXPECT_EQ(tree.size(), 3u);
    EXPECT_TRUE(inorder_is_sorted(tree));

    auto f = tree.find(2.65);
    ASSERT_TRUE(f.has_value());
    EXPECT_DOUBLE_EQ(f->get(), 2.65);
}

TEST(RBTreeTypes, LongType)
{
    Tree::RBTree<long> tree;
    tree.insert(10000000000L);
    tree.insert(-5000000000L);
    tree.insert(0L);

    EXPECT_EQ(tree.size(), 3u);
    EXPECT_TRUE(inorder_is_sorted(tree));
}

TEST(RBTreeTypes, UnsignedType)
{
    Tree::RBTree<unsigned int> tree;
    tree.insert(100u);
    tree.insert(50u);
    tree.insert(200u);

    EXPECT_EQ(tree.size(), 3u);
    EXPECT_TRUE(inorder_is_sorted(tree));
}
