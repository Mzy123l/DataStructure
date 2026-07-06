/**
 * @file test_UnionFind.cpp
 * @brief basicUnionFind 与 UnionFind<T> 的 GoogleTest 测试套件
 *
 * 覆盖构造、查找、合并、连通性、计数、添加元素、移动语义、
 * 路径压缩、根节点获取、容量管理、清空、边界条件及异常场景。
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "../include/UnionFind.hpp"

// ============================================================================
// 辅助：自定义可哈希类型
// ============================================================================

struct Person
{
    std::string name;
    int age;

    bool operator==(const Person& other) const
    {
        return name == other.name && age == other.age;
    }
};

namespace std
{
template <>
struct hash<Person>
{
    std::size_t operator()(const Person& p) const
    {
        return std::hash<std::string>{}(p.name)
               ^ (std::hash<int>{}(p.age) << 1);
    }
};
}  // namespace std

// ============================================================================
// basicUnionFind 测试
// ============================================================================

// ---------- 1. 构造 ----------

TEST(BasicUnionFindTest, ConstructWithN)
{
    Forest::basicUnionFind<> uf(5);
    EXPECT_EQ(uf.elementCount(), 5);
    EXPECT_EQ(uf.count(), 5);
}

TEST(BasicUnionFindTest, DefaultConstruct)
{
    Forest::basicUnionFind<> uf;
    EXPECT_EQ(uf.elementCount(), 0);
    EXPECT_EQ(uf.count(), 0);
}

// ---------- 2. Find ----------

TEST(BasicUnionFindTest, FindExistingElement)
{
    Forest::basicUnionFind<> uf(5);
    for (std::size_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(uf.find(i), i);
    }
}

TEST(BasicUnionFindTest, FindOutOfRangeThrows)
{
    Forest::basicUnionFind<> uf(3);
    EXPECT_THROW(uf.find(10), std::out_of_range);
    EXPECT_THROW(uf.find(3), std::out_of_range);
}

TEST(BasicUnionFindTest, FindOnEmptyUFThrows)
{
    Forest::basicUnionFind<> uf;
    EXPECT_THROW(uf.find(0), std::out_of_range);
}

// ---------- 3. Unite ----------

TEST(BasicUnionFindTest, UniteTwoElements)
{
    Forest::basicUnionFind<> uf(5);
    EXPECT_TRUE(uf.unite(0, 1));
    EXPECT_EQ(uf.find(0), uf.find(1));
    EXPECT_EQ(uf.count(), 4);
}

TEST(BasicUnionFindTest, UniteAlreadyUnitedReturnsFalse)
{
    Forest::basicUnionFind<> uf(5);
    ASSERT_TRUE(uf.unite(0, 1));
    EXPECT_FALSE(uf.unite(0, 1));
    EXPECT_FALSE(uf.unite(1, 0));
    EXPECT_EQ(uf.count(), 4);  // 数量不变
}

TEST(BasicUnionFindTest, UniteChain)
{
    Forest::basicUnionFind<> uf(10);
    // 链式合并 0-1-2-3-4
    EXPECT_TRUE(uf.unite(0, 1));
    EXPECT_TRUE(uf.unite(1, 2));
    EXPECT_TRUE(uf.unite(2, 3));
    EXPECT_TRUE(uf.unite(3, 4));
    EXPECT_EQ(uf.count(), 10 - 4);  // 6
}

TEST(BasicUnionFindTest, UniteAllIntoOneSet)
{
    constexpr std::size_t N = 20;
    Forest::basicUnionFind<> uf(N);
    for (std::size_t i = 1; i < N; ++i)
    {
        EXPECT_TRUE(uf.unite(0, i));
    }
    EXPECT_EQ(uf.count(), 1);
    // 所有元素连通
    for (std::size_t i = 1; i < N; ++i)
    {
        EXPECT_TRUE(uf.connected(0, i));
    }
}

// ---------- 4. Connected ----------

TEST(BasicUnionFindTest, ConnectedSameSet)
{
    Forest::basicUnionFind<> uf(5);
    uf.unite(0, 1);
    uf.unite(1, 2);
    EXPECT_TRUE(uf.connected(0, 2));
    EXPECT_TRUE(uf.connected(2, 0));
}

TEST(BasicUnionFindTest, ConnectedDifferentSets)
{
    Forest::basicUnionFind<> uf(5);
    uf.unite(0, 1);
    uf.unite(2, 3);
    EXPECT_FALSE(uf.connected(0, 2));
    EXPECT_FALSE(uf.connected(1, 3));
    EXPECT_FALSE(uf.connected(0, 4));
}

// ---------- 5. Count ----------

TEST(BasicUnionFindTest, CountInitial)
{
    Forest::basicUnionFind<> uf(7);
    EXPECT_EQ(uf.count(), 7);
}

TEST(BasicUnionFindTest, CountDecreasesAfterUnite)
{
    Forest::basicUnionFind<> uf(10);
    EXPECT_EQ(uf.count(), 10);
    uf.unite(0, 1);
    EXPECT_EQ(uf.count(), 9);
    uf.unite(2, 3);
    EXPECT_EQ(uf.count(), 8);
    uf.unite(0, 2);
    EXPECT_EQ(uf.count(), 7);
    // 同一集合再次合并不减少
    uf.unite(1, 3);
    EXPECT_EQ(uf.count(), 7);
}

// ---------- 6. Add element ----------

TEST(BasicUnionFindTest, AddSingleElement)
{
    Forest::basicUnionFind<> uf(3);
    std::size_t id = uf.add_element();
    EXPECT_EQ(id, 3);
    EXPECT_EQ(uf.elementCount(), 4);
    EXPECT_EQ(uf.count(), 4);  // 新元素独立成集
    EXPECT_EQ(uf.find(id), id);
}

TEST(BasicUnionFindTest, AddElementsBatch)
{
    Forest::basicUnionFind<> uf(2);
    uf.add_elements(5);
    EXPECT_EQ(uf.elementCount(), 7);
    EXPECT_EQ(uf.count(), 7);  // 5 个新独立集合
    // 新元素自环
    for (std::size_t i = 2; i < 7; ++i)
    {
        EXPECT_EQ(uf.find(i), i);
    }
}

TEST(BasicUnionFindTest, AddElementsToEmpty)
{
    Forest::basicUnionFind<> uf;
    uf.add_elements(4);
    EXPECT_EQ(uf.elementCount(), 4);
    EXPECT_EQ(uf.count(), 4);
    EXPECT_EQ(uf.find(0), 0);
    EXPECT_EQ(uf.find(3), 3);
}

// ---------- 7. 移动语义 ----------

TEST(BasicUnionFindTest, MoveConstruction)
{
    Forest::basicUnionFind<> uf1(5);
    uf1.unite(0, 1);
    uf1.unite(2, 3);
    std::size_t cnt = uf1.count();
    std::size_t elem = uf1.elementCount();

    Forest::basicUnionFind<> uf2(std::move(uf1));
    EXPECT_EQ(uf2.count(), cnt);
    EXPECT_EQ(uf2.elementCount(), elem);
    EXPECT_TRUE(uf2.connected(0, 1));
    EXPECT_TRUE(uf2.connected(2, 3));

    // 移动后源对象 count == 0
    EXPECT_EQ(uf1.count(), 0);
}

TEST(BasicUnionFindTest, MoveAssignment)
{
    Forest::basicUnionFind<> uf1(5);
    uf1.unite(0, 1);
    std::size_t cnt = uf1.count();
    std::size_t elem = uf1.elementCount();

    Forest::basicUnionFind<> uf2(2);  // 有数据的接收方
    uf2 = std::move(uf1);
    EXPECT_EQ(uf2.count(), cnt);
    EXPECT_EQ(uf2.elementCount(), elem);
    EXPECT_TRUE(uf2.connected(0, 1));

    EXPECT_EQ(uf1.count(), 0);
}

TEST(BasicUnionFindTest, MoveAssignmentSelfAssign)
{
    // 自赋值不应崩溃，但不做特别要求（移动自赋值理论上不应发生）
    Forest::basicUnionFind<> uf(5);
    uf.unite(0, 1);
    std::size_t cnt = uf.count();
    // 自赋值通过 if(this != &other) 保护
    uf = std::move(uf);  // NOLINT: 自赋值测试
    EXPECT_EQ(uf.count(), cnt);
    EXPECT_TRUE(uf.connected(0, 1));
}

// ---------- 8. 路径压缩 ----------

TEST(BasicUnionFindTest, PathCompressionAfterFind)
{
    Forest::basicUnionFind<> uf(10);
    // 构造一条长链 0<-1<-2<-3<-4 (parent[child] = parent)
    // 通过合并实现: unite(4,3) -> unite(3,2) -> ...
    // 按秩合并会使 root 成为 parent，链方向取决于秩
    //
    // 更可靠的方式：通过添加元素 + 直接检查 find 前后的路径
    // 由于按秩合并，先合并的形成秩更大的一侧，无法保证链的精确方向。
    // 这里改为验证多次 find 后所有节点路径压缩到同一根。
    uf.unite(0, 1);
    uf.unite(1, 2);
    uf.unite(2, 3);
    uf.unite(3, 4);

    // 查找最深节点，触发路径压缩
    std::size_t root = uf.find(4);
    // 再次查找所有节点，根应一致
    for (std::size_t i = 0; i <= 4; ++i)
    {
        EXPECT_EQ(uf.find(i), root);
    }
}

TEST(BasicUnionFindTest, PathCompressionLargeChain)
{
    constexpr std::size_t N = 1000;
    Forest::basicUnionFind<> uf(N);
    // 链式合并 0-1-2-3-...-N-1
    for (std::size_t i = 1; i < N; ++i)
    {
        uf.unite(0, i);
    }
    EXPECT_EQ(uf.count(), 1);
    // 查找最后一个元素触发路径压缩后，所有元素应快速找到
    std::size_t root = uf.find(N - 1);
    for (std::size_t i = 0; i < N; ++i)
    {
        EXPECT_EQ(uf.find(i), root);
    }
}

// ---------- 9. 边界条件 ----------

TEST(BasicUnionFindTest, SingleElement)
{
    Forest::basicUnionFind<> uf(1);
    EXPECT_EQ(uf.count(), 1);
    EXPECT_EQ(uf.elementCount(), 1);
    EXPECT_EQ(uf.find(0), 0);
    EXPECT_FALSE(uf.unite(0, 0));  // 自合并返回 false
    EXPECT_EQ(uf.count(), 1);
}

TEST(BasicUnionFindTest, ZeroElements)
{
    Forest::basicUnionFind<> uf(0);
    EXPECT_EQ(uf.count(), 0);
    EXPECT_EQ(uf.elementCount(), 0);
}

TEST(BasicUnionFindTest, LargeN)
{
    constexpr std::size_t N = 10000;
    Forest::basicUnionFind<> uf(N);
    EXPECT_EQ(uf.count(), N);
    EXPECT_EQ(uf.elementCount(), N);

    // 两两合并
    for (std::size_t i = 0; i < N; i += 2)
    {
        uf.unite(i, i + 1);
    }
    EXPECT_EQ(uf.count(), N / 2);
}

TEST(BasicUnionFindTest, LargeNPartition)
{
    constexpr std::size_t N = 10000;
    constexpr std::size_t PART = N / 5;
    Forest::basicUnionFind<> uf(N);

    // 分成 5 组各自合并
    for (std::size_t g = 0; g < 5; ++g)
    {
        std::size_t base = g * PART;
        for (std::size_t i = 1; i < PART; ++i)
        {
            uf.unite(base, base + i);
        }
    }
    EXPECT_EQ(uf.count(), 5);
}

// ---------- 10. Get roots ----------

TEST(BasicUnionFindTest, GetRootsAllSingletons)
{
    Forest::basicUnionFind<> uf(5);
    auto roots = uf.get_roots();
    EXPECT_EQ(roots.size(), 5);
    for (std::size_t i = 0; i < 5; ++i)
    {
        // 每个元素都是自己的根
        EXPECT_NE(
            std::find(roots.begin(), roots.end(), i), roots.end());
    }
}

TEST(BasicUnionFindTest, GetRootsAfterUnites)
{
    Forest::basicUnionFind<> uf(10);
    uf.unite(0, 1);
    uf.unite(2, 3);
    uf.unite(4, 5);
    uf.unite(0, 2);  // {0,1,2,3}

    auto roots = uf.get_roots();
    EXPECT_EQ(roots.size(), uf.count());
    // 所有 root 应对应自环
    for (auto r : roots)
    {
        EXPECT_EQ(uf.find(r), r);
    }
}

TEST(BasicUnionFindTest, GetRootsEmpty)
{
    Forest::basicUnionFind<> uf;
    auto roots = uf.get_roots();
    EXPECT_TRUE(roots.empty());
}

// ---------- 11. 容量管理 & 清空 ----------

TEST(BasicUnionFindTest, ReserveAndShrink)
{
    Forest::basicUnionFind<> uf(3);
    uf.reserve(100);
    EXPECT_GE(uf.elementCount(), 3);  // 预留不改变元素数
    uf.shrink_to_fit();
    EXPECT_EQ(uf.elementCount(), 3);
    EXPECT_EQ(uf.count(), 3);
}

TEST(BasicUnionFindTest, Clear)
{
    Forest::basicUnionFind<> uf(5);
    uf.unite(0, 1);
    uf.clear();
    EXPECT_EQ(uf.elementCount(), 0);
    EXPECT_EQ(uf.count(), 0);
}

TEST(BasicUnionFindTest, ReuseAfterClear)
{
    Forest::basicUnionFind<> uf(3);
    uf.clear();
    EXPECT_EQ(uf.count(), 0);
    std::size_t id = uf.add_element();
    EXPECT_EQ(id, 0);
    EXPECT_EQ(uf.elementCount(), 1);
    EXPECT_EQ(uf.count(), 1);
    EXPECT_EQ(uf.find(0), 0);
}

// ============================================================================
// UnionFind<T> 测试
// ============================================================================

// ---------- 1. 构造 ----------

TEST(UnionFindTest, DefaultConstruct)
{
    Forest::UnionFind<int> uf;
    EXPECT_EQ(uf.elementCount(), 0);
    EXPECT_EQ(uf.count(), 0);
}

// ---------- 2. Add element ----------

TEST(UnionFindTest, AddElementInt)
{
    Forest::UnionFind<int> uf;
    EXPECT_TRUE(uf.add_element(10));
    EXPECT_TRUE(uf.add_element(20));
    EXPECT_TRUE(uf.add_element(30));
    EXPECT_EQ(uf.elementCount(), 3);
    EXPECT_EQ(uf.count(), 3);
}

TEST(UnionFindTest, AddElementDuplicateReturnsFalse)
{
    Forest::UnionFind<int> uf;
    ASSERT_TRUE(uf.add_element(42));
    EXPECT_FALSE(uf.add_element(42));
    EXPECT_EQ(uf.elementCount(), 1);
    EXPECT_EQ(uf.count(), 1);
}

TEST(UnionFindTest, AddElementString)
{
    Forest::UnionFind<std::string> uf;
    EXPECT_TRUE(uf.add_element("hello"));
    EXPECT_TRUE(uf.add_element("world"));
    EXPECT_EQ(uf.elementCount(), 2);
}

TEST(UnionFindTest, AddElementCStrOverload)
{
    Forest::UnionFind<std::string> uf;
    EXPECT_TRUE(uf.add_element("alpha"));
    EXPECT_TRUE(uf.add_element("beta"));
    EXPECT_EQ(uf.elementCount(), 2);
    EXPECT_FALSE(uf.add_element("alpha"));
}

TEST(UnionFindTest, AddElementViaPerfectForwarding)
{
    Forest::UnionFind<std::string> uf;
    // 使用完美转发构造
    EXPECT_TRUE(uf.add_element(std::string("gamma")));
    EXPECT_EQ(uf.elementCount(), 1);
}

// ---------- 3. Find ----------

TEST(UnionFindTest, FindExisting)
{
    Forest::UnionFind<int> uf;
    uf.add_element(100);
    uf.add_element(200);
    EXPECT_EQ(uf.find(100), 100);
    EXPECT_EQ(uf.find(200), 200);
}

TEST(UnionFindTest, FindAfterUniteReturnsRoot)
{
    Forest::UnionFind<int> uf;
    uf.add_element(1);
    uf.add_element(2);
    uf.unite(1, 2);
    // 根可以是 1 或 2 对应的值，二者 find 结果应一致
    EXPECT_EQ(uf.find(1), uf.find(2));
}

TEST(UnionFindTest, FindNonexistentThrows)
{
    Forest::UnionFind<int> uf;
    uf.add_element(1);
    EXPECT_THROW(uf.find(999), std::out_of_range);
}

TEST(UnionFindTest, FindOnEmptyThrows)
{
    Forest::UnionFind<int> uf;
    EXPECT_THROW(uf.find(0), std::out_of_range);
}

// ---------- 4. Unite ----------

TEST(UnionFindTest, UniteTwoElements)
{
    Forest::UnionFind<std::string> uf;
    uf.add_element("A");
    uf.add_element("B");
    EXPECT_TRUE(uf.unite("A", "B"));
    EXPECT_EQ(uf.find("A"), uf.find("B"));
    EXPECT_EQ(uf.count(), 1);
}

TEST(UnionFindTest, UniteAlreadyUnitedReturnsFalse)
{
    Forest::UnionFind<int> uf;
    uf.add_element(10);
    uf.add_element(20);
    ASSERT_TRUE(uf.unite(10, 20));
    EXPECT_FALSE(uf.unite(10, 20));
    EXPECT_FALSE(uf.unite(20, 10));
}

// ---------- 5. Connected ----------

TEST(UnionFindTest, ConnectedSameSet)
{
    Forest::UnionFind<int> uf;
    uf.add_element(1);
    uf.add_element(2);
    uf.add_element(3);
    uf.unite(1, 2);
    uf.unite(2, 3);
    EXPECT_TRUE(uf.connected(1, 3));
    EXPECT_TRUE(uf.connected(3, 1));
}

TEST(UnionFindTest, ConnectedDifferentSets)
{
    Forest::UnionFind<int> uf;
    for (int v : {1, 2, 3, 4})
    {
        uf.add_element(v);
    }
    uf.unite(1, 2);
    uf.unite(3, 4);
    EXPECT_FALSE(uf.connected(1, 3));
    EXPECT_FALSE(uf.connected(2, 4));
}

// ---------- 6. Count ----------

TEST(UnionFindTest, CountInitialZero)
{
    Forest::UnionFind<int> uf;
    EXPECT_EQ(uf.count(), 0);
}

TEST(UnionFindTest, CountGrowsWithAddElement)
{
    Forest::UnionFind<int> uf;
    uf.add_element(1);
    EXPECT_EQ(uf.count(), 1);
    uf.add_element(2);
    EXPECT_EQ(uf.count(), 2);
    uf.add_element(3);
    EXPECT_EQ(uf.count(), 3);
}

TEST(UnionFindTest, CountDecreasesWithUnite)
{
    Forest::UnionFind<int> uf;
    uf.add_element(1);
    uf.add_element(2);
    uf.add_element(3);
    EXPECT_EQ(uf.count(), 3);
    uf.unite(1, 2);
    EXPECT_EQ(uf.count(), 2);
    uf.unite(1, 3);
    EXPECT_EQ(uf.count(), 1);
}

// ---------- 7. 移动语义 ----------

TEST(UnionFindTest, MoveConstruction)
{
    Forest::UnionFind<std::string> uf1;
    uf1.add_element("X");
    uf1.add_element("Y");
    uf1.add_element("Z");
    uf1.unite("X", "Y");
    std::size_t cnt = uf1.count();
    std::size_t elem = uf1.elementCount();

    Forest::UnionFind<std::string> uf2(std::move(uf1));
    EXPECT_EQ(uf2.count(), cnt);
    EXPECT_EQ(uf2.elementCount(), elem);
    EXPECT_TRUE(uf2.connected("X", "Y"));
    EXPECT_FALSE(uf2.connected("X", "Z"));

    // 移动后源对象 count == 0
    EXPECT_EQ(uf1.count(), 0);
}

TEST(UnionFindTest, MoveAssignment)
{
    Forest::UnionFind<int> uf1;
    uf1.add_element(10);
    uf1.add_element(20);
    uf1.unite(10, 20);
    std::size_t cnt = uf1.count();
    std::size_t elem = uf1.elementCount();

    Forest::UnionFind<int> uf2;
    uf2.add_element(99);  // 接收方已有数据
    uf2 = std::move(uf1);
    EXPECT_EQ(uf2.count(), cnt);
    EXPECT_EQ(uf2.elementCount(), elem);
    EXPECT_TRUE(uf2.connected(10, 20));

    EXPECT_EQ(uf1.count(), 0);
}

TEST(UnionFindTest, MoveAssignmentSelfAssign)
{
    Forest::UnionFind<int> uf;
    uf.add_element(1);
    uf.add_element(2);
    uf.unite(1, 2);
    std::size_t cnt = uf.count();

    uf = std::move(uf);  // NOLINT: 自赋值测试
    EXPECT_EQ(uf.count(), cnt);
    EXPECT_TRUE(uf.connected(1, 2));
}

// ---------- 8. 边界 / 压力 ----------

TEST(UnionFindTest, MultipleElementsWithStrings)
{
    Forest::UnionFind<std::string> uf;
    std::vector<std::string> words = {
        "one", "two", "three", "four", "five",
        "six", "seven", "eight", "nine", "ten"};
    for (const auto& w : words)
    {
        EXPECT_TRUE(uf.add_element(w));
    }
    EXPECT_EQ(uf.elementCount(), 10);
    EXPECT_EQ(uf.count(), 10);
}

TEST(UnionFindTest, ChainUnite)
{
    Forest::UnionFind<int> uf;
    constexpr int N = 50;
    for (int i = 0; i < N; ++i)
    {
        uf.add_element(i);
    }
    for (int i = 1; i < N; ++i)
    {
        uf.unite(0, i);
    }
    EXPECT_EQ(uf.count(), 1);
    for (int i = 1; i < N; ++i)
    {
        EXPECT_EQ(uf.find(i), uf.find(0));
    }
}

TEST(UnionFindTest, AllToAllUnite)
{
    Forest::UnionFind<int> uf;
    constexpr int N = 100;
    for (int i = 0; i < N; ++i)
    {
        uf.add_element(i);
    }
    // 全连接：以 0 为中心星形合并所有元素
    for (int i = 1; i < N; ++i)
    {
        uf.unite(0, i);
    }
    EXPECT_EQ(uf.count(), 1);
}

// ---------- 9. 自定义可哈希类型 ----------

TEST(UnionFindTest, CustomHashableTypePerson)
{
    Forest::UnionFind<Person> uf;

    Person alice{"Alice", 25};
    Person bob{"Bob", 30};
    Person charlie{"Charlie", 35};

    EXPECT_TRUE(uf.add_element(alice));
    EXPECT_TRUE(uf.add_element(bob));
    EXPECT_TRUE(uf.add_element(charlie));
    EXPECT_EQ(uf.elementCount(), 3);
    EXPECT_EQ(uf.count(), 3);

    // 合并 Alice-Bob
    EXPECT_TRUE(uf.unite(alice, bob));
    EXPECT_TRUE(uf.connected(alice, bob));
    EXPECT_FALSE(uf.connected(alice, charlie));
    EXPECT_EQ(uf.count(), 2);

    // 合并 Bob-Charlie
    EXPECT_TRUE(uf.unite(bob, charlie));
    EXPECT_TRUE(uf.connected(alice, charlie));
    EXPECT_EQ(uf.count(), 1);
}

TEST(UnionFindTest, CustomHashableDuplicateDetection)
{
    Forest::UnionFind<Person> uf;
    Person p{"Alice", 25};
    EXPECT_TRUE(uf.add_element(p));
    EXPECT_FALSE(uf.add_element(Person{"Alice", 25}));
    EXPECT_EQ(uf.elementCount(), 1);
}

// ---------- 10. Get roots ----------

TEST(UnionFindTest, GetRootsAllSingletons)
{
    Forest::UnionFind<int> uf;
    uf.add_element(10);
    uf.add_element(20);
    uf.add_element(30);
    auto roots = uf.get_roots();
    EXPECT_EQ(roots.size(), 3);
    for (auto v : {10, 20, 30})
    {
        EXPECT_NE(
            std::find(roots.begin(), roots.end(), v), roots.end());
    }
}

TEST(UnionFindTest, GetRootsAfterUnites)
{
    Forest::UnionFind<int> uf;
    uf.add_element(1);
    uf.add_element(2);
    uf.add_element(3);
    uf.add_element(4);
    uf.unite(1, 2);
    uf.unite(3, 4);
    auto roots = uf.get_roots();
    EXPECT_EQ(roots.size(), 2);
    // 两个根分别来自 {1,2} 和 {3,4}
    for (auto r : roots)
    {
        EXPECT_EQ(uf.find(r), r);
    }
}

TEST(UnionFindTest, GetRootsEmpty)
{
    Forest::UnionFind<int> uf;
    auto roots = uf.get_roots();
    EXPECT_TRUE(roots.empty());
}

// ---------- 11. 容量管理 & 清空 ----------

TEST(UnionFindTest, ReserveAndShrink)
{
    Forest::UnionFind<int> uf;
    uf.add_element(1);
    uf.add_element(2);
    uf.reserve(100);
    EXPECT_EQ(uf.elementCount(), 2);  // 预留不改变元素数
    uf.shrink_to_fit();
    EXPECT_EQ(uf.elementCount(), 2);
}

TEST(UnionFindTest, Clear)
{
    Forest::UnionFind<std::string> uf;
    uf.add_element("A");
    uf.add_element("B");
    uf.unite("A", "B");
    uf.clear();
    EXPECT_EQ(uf.elementCount(), 0);
    EXPECT_EQ(uf.count(), 0);
}

TEST(UnionFindTest, ReuseAfterClear)
{
    Forest::UnionFind<int> uf;
    uf.add_element(1);
    uf.clear();
    EXPECT_EQ(uf.count(), 0);
    EXPECT_TRUE(uf.add_element(2));
    EXPECT_EQ(uf.elementCount(), 1);
    EXPECT_EQ(uf.count(), 1);
    EXPECT_EQ(uf.find(2), 2);
}

// ============================================================================
// 交叉测试：两种 UnionFind 在同一场景下的行为一致性
// ============================================================================

TEST(CrossTest, EquivalentUniteBehavior)
{
    // basicUnionFind: 直接使用索引
    Forest::basicUnionFind<> bu(5);
    bu.unite(0, 1);
    bu.unite(1, 2);

    // UnionFind<int>: 值即索引
    Forest::UnionFind<int> gu;
    for (int i = 0; i < 5; ++i)
    {
        gu.add_element(i);
    }
    gu.unite(0, 1);
    gu.unite(1, 2);

    // 二者连通性一致
    EXPECT_TRUE(bu.connected(0, 2));
    EXPECT_TRUE(gu.connected(0, 2));
    EXPECT_FALSE(bu.connected(0, 3));
    EXPECT_FALSE(gu.connected(0, 3));
    // 集合数一致
    EXPECT_EQ(bu.count(), gu.count());
}
