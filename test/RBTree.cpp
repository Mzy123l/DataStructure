#include "../include/RBTree.hpp"
#include <cassert>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <random>
#include <set>
#include <list>
#include <memory>
#include <map>
#include <functional>
#include <chrono>

// 计时辅助宏
#define TIMING_START auto start = std::chrono::high_resolution_clock::now();
#define TIMING_END(msg) \
    auto end = std::chrono::high_resolution_clock::now(); \
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start); \
    std::cout << msg << " 用时: " << duration.count() << "ms" << std::endl;

// 测试辅助函数：验证中序遍历结果是否为升序
template <typename Tree>
bool is_inorder_sorted(const Tree& tree)
{
    if (tree.empty()) return true;
    auto it = tree.begin();
    auto prev_val = *it;
    ++it;
    for (; it != tree.end(); ++it)
    {
        if (!(prev_val < *it) && !(*it < prev_val))
        {
            // 允许相等（如果比较器允许，但红黑树通常不存储重复键）
        }
        else if (*it < prev_val)
        {
            return false; // 顺序错误
        }
        prev_val = *it;
    }
    return true;
}

// 测试辅助函数：验证红黑树的五个性质
// 注意：由于只能通过公有接口访问，此验证是间接的，主要通过观察行为和迭代器。
// 更严格的性质验证（如黑高一致）需要访问内部指针，此处仅做基本检查。
template <typename Tree>
bool basic_rb_property_check(const Tree& tree)
{
    // 通过中序遍历验证二叉搜索树性质（排序性）
    if (!is_inorder_sorted(tree))
    {
        std::cerr << "BST property violated (not sorted)." << std::endl;
        return false;
    }
    return true;
}

void test_constructor_and_basic_properties()
{
    std::cout << "=== Test 1: Constructors & Basic Properties ===" << std::endl;
    Tree::RBTree<int> tree1;
    assert(tree1.empty() == true);
    assert(tree1.size() == 0);
    // 验证 begin() 等于 end() 当树为空时
    assert(tree1.begin() == tree1.end());

    // 测试插入后，empty 和 size 变化
    auto insert_ok = tree1.insert(10);
    assert(insert_ok == true);
    assert(tree1.empty() == false);
    assert(tree1.size() == 1);
    std::cout << "Test 1 passed.\n" << std::endl;
}

void test_insert_and_find()
{
    std::cout << "=== Test 2: Insert & Find ===" << std::endl;
    Tree::RBTree<int> tree;

    // 插入一系列值
    std::vector<int> values_to_insert = { 5, 3, 8, 1, 4, 7, 9, 2, 6 };
    for (int v : values_to_insert)
    {
        bool ok = tree.insert(v);
        assert(ok == true);
    }
    assert(tree.size() == values_to_insert.size());

    // 验证所有插入的值都能被找到
    for (int v : values_to_insert)
    {
        auto found = tree.find(v);
        assert(found.has_value() == true);
        assert(found.value().get() == v);
    }

    // 查找不存在的值
    auto not_found = tree.find(999);
    assert(not_found.has_value() == false);

    // 验证中序遍历是有序的
    assert(basic_rb_property_check(tree) == true);

    // 测试重复插入应返回 false
    bool dup_insert_ok = tree.insert(5);
    assert(dup_insert_ok == false);
    assert(tree.size() == values_to_insert.size()); // 大小不应改变

    std::cout << "Test 2 passed.\n" << std::endl;
}

void test_erase()
{
    std::cout << "=== Test 3: Erase ===" << std::endl;
    Tree::RBTree<std::string> tree;

    // 准备数据
    std::vector<std::string> words = { "apple", "banana", "cherry", "date", "fig", "grape" };
    for (const auto& w : words)
    {
        tree.insert(w);
    }
    assert(tree.size() == words.size());

    // 1. 删除叶子节点（或单子节点）
    bool erased = tree.erase("fig");
    assert(erased == true);
    assert(tree.size() == words.size() - 1);
    assert(tree.find("fig").has_value() == false);
    assert(basic_rb_property_check(tree) == true);

    // 2. 删除有两个子节点的节点
    erased = tree.erase("banana");
    assert(erased == true);
    assert(tree.size() == words.size() - 2);
    assert(tree.find("banana").has_value() == false);
    // 验证剩下的元素仍可找到
    assert(tree.find("apple").has_value() == true);
    assert(tree.find("cherry").has_value() == true);
    assert(basic_rb_property_check(tree) == true);

    // 3. 删除根节点
    std::string root_val = *tree.begin(); // 中序遍历第一个元素可能是最小节点，不一定是插入的根，这里仅为示例。更严谨的做法是多次插入特定序列使根已知。
    // 我们直接删除一个存在的值（可能是根）
    erased = tree.erase("date");
    assert(erased == true);
    assert(tree.find("date").has_value() == false);
    assert(basic_rb_property_check(tree) == true);

    // 4. 尝试删除不存在的元素
    erased = tree.erase("zoo");
    assert(erased == false);
    assert(tree.size() == words.size() - 3); // 大小不变

    // 5. 删除所有剩余元素
    for (const auto& w : words)
    {
        tree.erase(w); // erase 在元素不存在时返回 false，但可以安全调用
    }
    assert(tree.empty() == true);
    assert(tree.begin() == tree.end());

    std::cout << "Test 3 passed.\n" << std::endl;
}

void test_iterator()
{
    std::cout << "=== Test 4: Iterator ===" << std::endl;
    Tree::RBTree<int> tree;
    std::vector<int> vals = { 30, 20, 40, 10, 25, 35, 50 };
    std::vector<int> sorted_vals = vals;
    std::sort(sorted_vals.begin(), sorted_vals.end());

    for (int v : vals)
    {
        tree.insert(v);
    }

    // 测试前向迭代
    std::vector<int> iteration_result;
    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        iteration_result.push_back(*it);
    }
    assert(iteration_result == sorted_vals);

    // 测试基于范围的 for 循环
    iteration_result.clear();
    for (int value : tree)
    {
        iteration_result.push_back(value);
    }
    assert(iteration_result == sorted_vals);

    // 测试常量迭代器
    const Tree::RBTree<int>& const_tree_ref = tree;
    iteration_result.clear();
    for (auto it = const_tree_ref.begin(); it != const_tree_ref.end(); ++it)
    {
        iteration_result.push_back(*it);
    }
    assert(iteration_result == sorted_vals);

    std::cout << "Test 4 passed.\n" << std::endl;
}

void test_copy_and_move()
{
    std::cout << "=== Test 5: Copy & Move Semantics ===" << std::endl;
    Tree::RBTree<int> original;
    for (int i : {9, 5, 12, 3, 7, 10, 15})
    {
        original.insert(i);
    }
    auto original_size = original.size();

    // 1. 测试拷贝构造
    Tree::RBTree<int> copy_constructed(original);
    assert(copy_constructed.size() == original_size);
    // 验证内容一致
    for (auto it_orig = original.begin(), it_copy = copy_constructed.begin();
        it_orig != original.end(); ++it_orig, ++it_copy)
    {
        assert(*it_orig == *it_copy);
    }
    // 验证是深拷贝：修改原树不影响副本
    original.erase(7);
    assert(copy_constructed.find(7).has_value() == true); // 副本中应存在
    assert(basic_rb_property_check(copy_constructed) == true);

    // 2. 测试拷贝赋值
    Tree::RBTree<int> copy_assigned;
    copy_assigned = original; // 此时 original 已删除7
    std::cout << "copy_size:" << copy_assigned.size() << "\n" << "orig_size:" << original.size() << "\n";
    assert(copy_assigned.size() == original.size());
    for (auto it_orig = original.begin(), it_assign = copy_assigned.begin();
        it_orig != original.end(); ++it_orig, ++it_assign)
    {
        assert(*it_orig == *it_assign);
    }
    assert(copy_assigned.find(7).has_value() == false); // 应与 original 状态一致
    assert(basic_rb_property_check(copy_assigned) == true);

    // 3. 测试移动构造 (从临时对象)
    Tree::RBTree<int> temp;
    temp.insert(100);
    temp.insert(50);
    Tree::RBTree<int> move_constructed(std::move(temp));
    assert(move_constructed.size() == 2);
    assert(move_constructed.find(100).has_value() == true);


    std::cout << "Test 5 passed.\n" << std::endl;
}

// ==================== 新增测试：大量随机插入 ====================
void test_random_insertion_small()
{
    std::cout << "=== Test 6: Random Insertion (Small Scale) ===" << std::endl;
    TIMING_START
        std::mt19937 rng(42); // 固定种子以保证测试可重复
    std::uniform_int_distribution<int> dist(-10000, 10000);

    Tree::RBTree<int> tree;
    std::set<int> ref_set; // 用于参考比较
    int num_elements = 1000;

    // 随机插入
    for (int i = 0; i < num_elements; ++i)
    {
        int val = dist(rng);
        auto rb_result = tree.insert(val);
        auto set_result = ref_set.insert(val);

        // 验证插入结果的一致性
        assert(rb_result == set_result.second);
    }

    // 验证大小
    assert(tree.size() == ref_set.size());

    // 验证所有元素都能被找到
    for (int val : ref_set)
    {
        auto found = tree.find(val);
        assert(found.has_value() == true);
        assert(found.value().get() == val);
    }

    // 验证BST性质
    assert(basic_rb_property_check(tree) == true);

    // 验证有序性（与std::set比较）
    std::vector<int> rb_inorder, set_inorder;
    for (int val : tree) rb_inorder.push_back(val);
    for (int val : ref_set) set_inorder.push_back(val);
    assert(rb_inorder == set_inorder);

    TIMING_END("小规模随机插入测试")
        std::cout << "Test 6 passed.\n" << std::endl;
}

void test_random_insertion_large()
{
    std::cout << "=== Test 7: Random Insertion (Large Scale) ===" << std::endl;
    TIMING_START
        std::mt19937 rng(123456);
    std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(),
        std::numeric_limits<int>::max());

    Tree::RBTree<int> tree;
    std::set<int> ref_set;
    int num_elements = 10000;

    // 随机插入
    for (int i = 0; i < num_elements; ++i)
    {
        int val = dist(rng);
        auto rb_result = tree.insert(val);
        auto set_result = ref_set.insert(val);

        assert(rb_result == set_result.second);

        // 每插入1000个元素进行一次完整性检查
        if (i % 1000 == 999)
        {
            assert(tree.size() == ref_set.size());
            assert(basic_rb_property_check(tree) == true);
        }
    }

    // 最终验证
    assert(tree.size() == ref_set.size());
    assert(basic_rb_property_check(tree) == true);

    // 随机查询验证
    std::uniform_int_distribution<int> query_dist(0, 1);
    for (int i = 0; i < 100; ++i)
    {
        int val = dist(rng);
        auto rb_found = tree.find(val);
        auto set_found = ref_set.find(val);

        bool rb_has = rb_found.has_value();
        bool set_has = (set_found != ref_set.end());
        assert(rb_has == set_has);

        if (rb_has && set_has)
        {
            assert(rb_found.value().get() == *set_found);
        }
    }

    TIMING_END("大规模随机插入测试")
        std::cout << "Test 7 passed.\n" << std::endl;
}

void test_random_insertion_with_duplicates()
{
    std::cout << "=== Test 8: Random Insertion with Duplicates ===" << std::endl;
    std::mt19937 rng(777);
    std::uniform_int_distribution<int> dist(1, 1000);

    Tree::RBTree<int> tree;
    int num_elements = 5000;
    int insert_count = 0;
    int duplicate_count = 0;

    for (int i = 0; i < num_elements; ++i)
    {
        int val = dist(rng);
        bool inserted = tree.insert(val);
        if (inserted)
        {
            insert_count++;
        }
        else
        {
            duplicate_count++;
        }
    }

    std::cout << "插入总数: " << num_elements << std::endl;
    std::cout << "成功插入: " << insert_count << std::endl;
    std::cout << "重复元素: " << duplicate_count << std::endl;

    assert(tree.size() == static_cast<size_t>(insert_count));
    assert(basic_rb_property_check(tree) == true);

    // 验证无重复元素
    std::set<int> unique_elements;
    for (int val : tree)
    {
        auto result = unique_elements.insert(val);
        assert(result.second == true); // 不应该有重复
    }

    std::cout << "Test 8 passed.\n" << std::endl;
}

void test_random_insertion_and_erase()
{
    std::cout << "=== Test 9: Random Insertion and Erase ===" << std::endl;
    TIMING_START
        std::mt19937 rng(888);
    std::uniform_int_distribution<int> val_dist(1, 10000);
    std::uniform_int_distribution<int> op_dist(0, 3); // 0-3: 0-2插入，3删除

    Tree::RBTree<int> tree;
    std::set<int> ref_set;
    int operations = 5000;

    for (int i = 0; i < operations; ++i)
    {
        int op = op_dist(rng);
        int val = val_dist(rng);

        if (op <= 2) // 插入操作
        {
            auto rb_result = tree.insert(val);
            auto set_result = ref_set.insert(val);
            assert(rb_result == set_result.second);
        }
        else // 删除操作
        {
            auto rb_result = tree.erase(val);
            size_t set_result = ref_set.erase(val);
            assert(rb_result == (set_result > 0));
        }

        // 定期验证
        if (i % 1000 == 999)
        {
            assert(tree.size() == ref_set.size());
            if (tree.size() > 0)
            {
                assert(basic_rb_property_check(tree) == true);

                // 验证有序性
                std::vector<int> rb_vec, set_vec;
                for (int v : tree) rb_vec.push_back(v);
                for (int v : ref_set) set_vec.push_back(v);
                assert(rb_vec == set_vec);
            }
        }
    }

    TIMING_END("随机插入删除混合测试")
        std::cout << "Test 9 passed.\n" << std::endl;
}

// ==================== 新增测试：分配器测试 ====================
// 简单计数分配器
template<typename T>
class CountingAllocator
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;

    static int construct_count;
    static int destroy_count;
    static int allocate_count;
    static int deallocate_count;
    static int total_bytes_allocated;

    T* allocate(size_type n)
    {
        allocate_count++;
        total_bytes_allocated += n * sizeof(T);
        std::cout << "Allocating " << n << " elements of size " << sizeof(T)
            << " (" << n * sizeof(T) << " bytes)" << std::endl;
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, size_type n)
    {
        deallocate_count++;
        total_bytes_allocated -= n * sizeof(T);
        std::cout << "Deallocating " << n << " elements (" << n * sizeof(T) << " bytes)" << std::endl;
        ::operator delete(p);
    }

    template<typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {
        construct_count++;
        std::cout << "Constructing element at " << static_cast<void*>(p) << std::endl;
        new(p) U(std::forward<Args>(args)...);
    }

    template<typename U>
    void destroy(U* p)
    {
        destroy_count++;
        std::cout << "Destroying element at " << static_cast<void*>(p) << std::endl;
        p->~U();
    }

    static void reset_counts()
    {
        construct_count = destroy_count = allocate_count = deallocate_count = total_bytes_allocated = 0;
    }

    static void print_stats()
    {
        std::cout << "Allocator Statistics:" << std::endl;
        std::cout << "  Allocations: " << allocate_count << std::endl;
        std::cout << "  Deallocations: " << deallocate_count << std::endl;
        std::cout << "  Constructions: " << construct_count << std::endl;
        std::cout << "  Destructions: " << destroy_count << std::endl;
        std::cout << "  Total bytes allocated: " << total_bytes_allocated << std::endl;
    }

    template<typename U>
    struct rebind
    {
        using other = CountingAllocator<U>;
    };

    // 复制构造函数
    CountingAllocator() = default;
    template<typename U>
    CountingAllocator(const CountingAllocator<U>&) {}

    bool operator==(const CountingAllocator&) const { return true; }
    bool operator!=(const CountingAllocator&) const { return false; }
};

template<typename T>
int CountingAllocator<T>::construct_count = 0;
template<typename T>
int CountingAllocator<T>::destroy_count = 0;
template<typename T>
int CountingAllocator<T>::allocate_count = 0;
template<typename T>
int CountingAllocator<T>::deallocate_count = 0;
template<typename T>
int CountingAllocator<T>::total_bytes_allocated = 0;

// 状态化分配器：记录分配次数
template<typename T>
class StatefulAllocator
{
public:
    using value_type = T;
    int* counter; // 非静态成员，每个分配器实例有自己的计数器

    StatefulAllocator(int* c = nullptr) : counter(c) {}

    template<typename U>
    StatefulAllocator(const StatefulAllocator<U>& other) : counter(other.counter) {}

    T* allocate(std::size_t n)
    {
        if (counter) (*counter)++;
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n)
    {
        ::operator delete(p);
    }

    template<typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {
        new(p) U(std::forward<Args>(args)...);
    }

    template<typename U>
    void destroy(U* p)
    {
        p->~U();
    }

    template<typename U>
    struct rebind
    {
        using other = StatefulAllocator<U>;
    };

    bool operator==(const StatefulAllocator& other) const
    {
        return counter == other.counter;
    }

    bool operator!=(const StatefulAllocator& other) const
    {
        return !(*this == other);
    }
};

void test_allocator_basic()
{
    std::cout << "=== Test 10: Allocator (Basic) ===" << std::endl;
    CountingAllocator<int>::reset_counts();

    {
        using CustomTree = Tree::RBTree<int, std::less<int>, CountingAllocator<int>>;
        CustomTree tree(CountingAllocator<int>{});

        // 验证get_allocator返回正确的类型
        auto alloc = tree.get_allocator();
        static_assert(std::is_same<decltype(alloc), CountingAllocator<int>>::value,
            "Allocator type mismatch");

        // 插入一些元素
        for (int i = 0; i < 10; ++i)
        {
            tree.insert(i * 5);
        }

        // 删除一些元素
        tree.erase(0);
        tree.erase(25);

        // 验证树的基本功能
        assert(tree.size() == 8);
        assert(tree.find(5).has_value() == true);
        assert(tree.find(0).has_value() == false);
        assert(basic_rb_property_check(tree) == true);

        // 打印分配器统计
        CountingAllocator<int>::print_stats();

   
    } // tree析构，内存应该被释放

    // 析构后，构造和析构次数应该匹配
    assert(CountingAllocator<int>::construct_count == CountingAllocator<int>::destroy_count);
    assert(CountingAllocator<int>::allocate_count == CountingAllocator<int>::deallocate_count);
    assert(CountingAllocator<int>::total_bytes_allocated == 0);

    std::cout << "Test 10 passed.\n" << std::endl;
}

void test_allocator_with_complex_type()
{
    std::cout << "=== Test 11: Allocator with Complex Type ===" << std::endl;

    // 使用std::string作为值类型
    using StringAllocator = CountingAllocator<std::string>;
    StringAllocator::reset_counts();

    {
        using StringTree = Tree::RBTree<std::string, std::less<std::string>, StringAllocator>;
        StringTree tree(StringAllocator{});

        // 插入字符串
        std::vector<std::string> words = { "apple", "banana", "cherry", "date", "fig" };
        for (const auto& word : words)
        {
            tree.insert(word);
        }

        // 验证
        assert(tree.size() == words.size());
        for (const auto& word : words)
        {
            assert(tree.find(word).has_value() == true);
        }

        // 拷贝构造（应该使用相同的分配器类型）
        StringTree tree2 = tree;
        assert(tree2.size() == tree.size());

        // 移动构造
        StringTree tree3 = std::move(tree2);
        assert(tree3.size() == words.size());

        // 拷贝赋值
        StringTree tree4(StringAllocator{});
        tree4 = tree3;
        assert(tree4.size() == words.size());

        // 打印统计
        StringAllocator::print_stats();

        // 基本功能验证
        assert(basic_rb_property_check(tree3) == true);
        assert(basic_rb_property_check(tree4) == true);
    } // 析构所有树

    // 验证内存释放
    assert(StringAllocator::construct_count == StringAllocator::destroy_count);
    assert(StringAllocator::allocate_count == StringAllocator::deallocate_count);
    assert(StringAllocator::total_bytes_allocated == 0);

    std::cout << "Test 11 passed.\n" << std::endl;
}

void test_allocator_stateful()
{
    std::cout << "=== Test 12: Stateful Allocator ===" << std::endl;

    int allocation_count = 0;

    {
        using StatefulTree = Tree::RBTree<int, std::less<int>, StatefulAllocator<int>>;
        StatefulTree tree(StatefulAllocator<int>{&allocation_count});

        // 插入元素触发分配
        for (int i = 0; i < 100; ++i)
        {
            tree.insert(i);
        }

        std::cout << "Allocation count during insertion: " << allocation_count << std::endl;
        assert(allocation_count > 0);

        // 验证树的功能
        assert(tree.size() == 100);
        for (int i = 0; i < 100; ++i)
        {
            assert(tree.find(i).has_value() == true);
        }

        // 删除一半元素
        for (int i = 0; i < 50; ++i)
        {
            tree.erase(i);
        }

        assert(tree.size() == 50);
        assert(basic_rb_property_check(tree) == true);
    } // 析构时会释放内存

    std::cout << "Test 12 passed.\n" << std::endl;
}

void test_allocator_copy_move()
{
    std::cout << "=== Test 13: Allocator Copy/Move Semantics ===" << std::endl;

    // 测试分配器在拷贝/移动时的行为
    using DefaultAlloc = std::allocator<int>;
    using DefaultTree = Tree::RBTree<int, std::less<int>, DefaultAlloc>;

    DefaultTree tree1;
    for (int i = 0; i < 10; ++i)
    {
        tree1.insert(i * 3);
    }

    // 拷贝构造 - 应该使用相同的分配器类型
    DefaultTree tree2 = tree1;
    assert(tree2.size() == tree1.size());
    // 注意：get_allocator() 返回的分配器比较可能不总是相等，取决于分配器的实现
    // assert(tree2.get_allocator() == tree1.get_allocator());

    // 验证内容相同
    auto it1 = tree1.begin();
    auto it2 = tree2.begin();
    while (it1 != tree1.end() && it2 != tree2.end())
    {
        assert(*it1 == *it2);
        ++it1;
        ++it2;
    }

    // 移动构造
    DefaultTree tree3 = std::move(tree1);
    assert(tree3.size() == 10);


    // 拷贝赋值
    DefaultTree tree4;
    tree4 = tree3;
    assert(tree4.size() == 10);

    // 移动赋值
    DefaultTree tree5;
    tree5 = std::move(tree3);
    assert(tree5.size() == 10);


    // 验证所有树的功能正常
    assert(basic_rb_property_check(tree2) == true);
    assert(basic_rb_property_check(tree4) == true);
    assert(basic_rb_property_check(tree5) == true);

    std::cout << "Test 13 passed.\n" << std::endl;
}

void test_allocator_large_allocation()
{
    std::cout << "=== Test 14: Allocator with Large Allocation ===" << std::endl;
    TIMING_START

        // 测试大量元素插入时的分配器行为
        using LargeTree = Tree::RBTree<int, std::less<int>, std::allocator<int>>;
    LargeTree tree;

    int num_elements = 10000;
    std::vector<int> elements;
    elements.reserve(num_elements);

    // 生成不重复的随机数
    std::mt19937 rng(999);
    std::uniform_int_distribution<int> dist(1, num_elements * 10);
    std::set<int> unique_set;

    while (elements.size() < static_cast<size_t>(num_elements))
    {
        int val = dist(rng);
        if (unique_set.insert(val).second)
        {
            elements.push_back(val);
        }
    }

    // 插入所有元素
    for (int val : elements)
    {
        tree.insert(val);
    }

    assert(tree.size() == static_cast<size_t>(num_elements));
    assert(basic_rb_property_check(tree) == true);

    // 验证所有元素都存在
    for (int val : elements)
    {
        assert(tree.find(val).has_value() == true);
    }

    // 随机删除一半元素
    std::shuffle(elements.begin(), elements.end(), rng);
    for (size_t i = 0; i < elements.size() / 2; ++i)
    {
        tree.erase(elements[i]);
    }

    assert(tree.size() == elements.size() - elements.size() / 2);
    assert(basic_rb_property_check(tree) == true);

    TIMING_END("大量分配测试")
        std::cout << "Test 14 passed.\n" << std::endl;
}

int main()
{
    try
    {
        test_constructor_and_basic_properties();
        test_insert_and_find();
        test_erase();
        test_iterator();
        test_copy_and_move();

        // 新增的随机插入测试
        test_random_insertion_small();
        test_random_insertion_large();
        test_random_insertion_with_duplicates();
        test_random_insertion_and_erase();

        // 新增的分配器测试
        test_allocator_basic();
        test_allocator_with_complex_type();
        test_allocator_stateful();
        test_allocator_copy_move();
        test_allocator_large_allocation();

        std::cout << "========================================" << std::endl;
        std::cout << "All unit tests passed successfully!" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Unit test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unit test failed with unknown exception." << std::endl;
        return 1;
    }
}