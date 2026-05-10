#include "../include/AVLTree.hpp"
#include <cassert>
#include <iostream>
#include <vector>
#include <random>
#include <set>
#include <optional>
#include <functional> // 用于 std::reference_wrapper

int main()
{
    std::cout << "开始 AVLTree 单元测试 (更新版)...\n";

    // ----------------- 测试 1: 基本插入与大小 -----------------
    {
        Tree::AVLTree<int> tree;
        assert(tree.empty());
        assert(tree.size() == 0);

        // 插入新元素
        assert(tree.insert(5) == true);
        assert(tree.insert(3) == true);
        assert(tree.insert(7) == true);
        assert(!tree.empty());
        assert(tree.size() == 3);

        // 插入重复元素应失败
        assert(tree.insert(5) == false);
        assert(tree.size() == 3); // 大小不应改变
        std::cout << "测试 1 (基本插入与大小) 通过。\n";
    }

    // ----------------- 测试 2: 查找功能 -----------------
    {
        Tree::AVLTree<int> tree;
        tree.insert(10);
        tree.insert(5);
        tree.insert(15);
        tree.insert(3);
        tree.insert(7);

        // 查找存在的元素
        std::optional<std::reference_wrapper<int>> result;

        result = tree.find(10);
        assert(result.has_value());
        assert(result.value().get() == 10);

        result = tree.find(3);
        assert(result.has_value());
        assert(result.value().get() == 3);

        result = tree.find(7);
        assert(result.has_value());
        assert(result.value().get() == 7);

        // 查找不存在的元素
        result = tree.find(99);
        assert(!result.has_value());

        result = tree.find(0);
        assert(!result.has_value());

        // 常量版本的查找
        const Tree::AVLTree<int>& const_tree = tree;
        auto const_result = const_tree.find(5);
        assert(const_result.has_value());
        assert(const_result.value().get() == 5);

        std::cout << "测试 2 (查找功能) 通过。\n";
    }

    // ----------------- 测试 3: 中序遍历顺序 -----------------
    {
        Tree::AVLTree<int> tree;
        std::vector<int> values = { 8, 3, 10, 1, 6, 14, 4, 7, 13 };
        for (int v : values)
        {
            tree.insert(v);
        }
        // AVL 树应保持中序遍历有序
        std::vector<int> inOrderResult;
        for (auto it = tree.begin(); it != tree.end(); ++it)
        {
            inOrderResult.push_back(*it);
        }
        for (const auto& ele : tree)
        {
            std::cout<<ele<<" ";
        }
        // 验证有序
        assert(std::is_sorted(inOrderResult.begin(), inOrderResult.end()));
        // 验证包含所有插入元素
        std::sort(values.begin(), values.end());
        assert(inOrderResult == values);
        std::cout << "测试 3 (中序遍历顺序) 通过。\n";
    }

    // ----------------- 测试 4: 删除功能 -----------------
    {
        Tree::AVLTree<int> tree;
        tree.insert(9);
        tree.insert(5);
        tree.insert(12);
        tree.insert(2);
        tree.insert(7);
        tree.insert(10);
        tree.insert(15);
        assert(tree.size() == 7);

        // 删除叶子节点
        assert(tree.erase(2) == true);
        assert(!tree.find(2).has_value());
        assert(tree.size() == 6);
        // 验证顺序依然正确
        std::vector<int> afterFirstDel;
        for (int val : tree)
        { // 使用 range-based for loop (依赖 begin/end)
            afterFirstDel.push_back(val);
        }
        assert(std::is_sorted(afterFirstDel.begin(), afterFirstDel.end()));

        // 删除有一个子节点的节点
        assert(tree.erase(12) == true);
        assert(!tree.find(12).has_value());
        assert(tree.size() == 5);

        // 删除有两个子节点的节点
        assert(tree.erase(9) == true);
        assert(!tree.find(9).has_value());
        assert(tree.size() == 4);

        // 删除不存在的元素
        assert(tree.erase(999) == false);
        assert(tree.size() == 4);

        // 清空树
        tree.clear();
        assert(tree.empty());
        assert(tree.size() == 0);
        std::cout << "测试 4 (删除功能) 通过。\n";
    }

    // ----------------- 测试 5: 查找结果的可修改性 -----------------
    {
        Tree::AVLTree<int> tree;
        tree.insert(100);
        tree.insert(200);
        tree.insert(300);

        // 获取查找结果的引用
        auto result = tree.find(200);
        assert(result.has_value());

        // 通过引用修改值
        int& ref = result.value().get();
        assert(ref == 200);

        // 修改值
        ref = 250;

        // 验证值已被修改
        auto updated_result = tree.find(200);
        assert(!updated_result.has_value()); // 200 不应该存在了

        auto new_result = tree.find(250);
        assert(new_result.has_value()); // 250 应该存在

        // 验证树中只有一个 250
        int count = 0;
        for (int val : tree)
        {
            if (val == 250) count++;
        }
        assert(count == 1);

        std::cout << "测试 5 (查找结果的可修改性) 通过。\n";
    }

    // ----------------- 测试 6: 拷贝构造与拷贝赋值 -----------------
    {
        Tree::AVLTree<int> tree1;
        tree1.insert(5);
        tree1.insert(1);
        tree1.insert(8);

        // 拷贝构造
        Tree::AVLTree<int> tree2 = tree1; // 调用拷贝构造函数
        for (const auto& val : tree2)
        {
            std::cout << val << " ";
        }
        assert(tree2.size() == 3);
        assert(tree2.find(5).has_value() &&
            tree2.find(1).has_value() &&
            tree2.find(8).has_value());
        // 验证是深拷贝：修改原树不影响副本
        tree1.insert(10);
        assert(tree1.find(10).has_value());
        assert(!tree2.find(10).has_value()); // tree2 不应有 10
        assert(tree2.size() == 3);

        // 拷贝赋值
        Tree::AVLTree<int> tree3;
        tree3 = tree2; // 调用拷贝赋值运算符
        assert(tree3.size() == 3);
        std::vector<int> v3;
        for (int val : tree3) v3.push_back(val);
        assert(v3 == std::vector<int>({ 1, 5, 8 }));
        std::cout << "测试 6 (拷贝构造与拷贝赋值) 通过。\n";
    }

    // ----------------- 测试 7: 移动构造与移动赋值 -----------------
    {
        Tree::AVLTree<int> tree1;
        tree1.insert(100);
        tree1.insert(50);
        std::size_t originalSize = tree1.size();

        // 移动构造
        Tree::AVLTree<int> tree2 = std::move(tree1);
        assert(tree2.size() == originalSize);
        assert(tree1.empty()); // 移动后源对象应为空

        // 移动赋值
        Tree::AVLTree<int> tree3;
        tree3 = std::move(tree2);
        assert(tree3.size() == originalSize);
        assert(tree2.empty()); // 移动后源对象应为空
        std::cout << "测试 7 (移动构造与移动赋值) 通过。\n";
    }

    // ----------------- 测试 8: 自定义比较器与分配器 -----------------
    {
        // 测试自定义比较器（从大到小排序）
        struct Greater
        {
            bool operator()(int a, int b) const { return a > b; }
        };
        Tree::AVLTree<int, Greater> treeDesc;
        treeDesc.insert(1);
        treeDesc.insert(3);
        treeDesc.insert(2);
        std::vector<int> descResult;
        for (int val : treeDesc) descResult.push_back(val);
        // 应降序排列
        assert(descResult == std::vector<int>({ 3, 2, 1 }));

        // 测试自定义分配器（使用默认分配器，此处为演示接口）
        using CustomAlloc = std::allocator<TreeNode::AVLNode<int>>;
        Tree::AVLTree<int, std::less<int>, CustomAlloc> treeWithAlloc;
        treeWithAlloc.insert(42);
        assert(treeWithAlloc.find(42).has_value());
        // 可调用 get_allocator
        auto alloc = treeWithAlloc.get_allocator();
        static_assert(std::is_same_v<decltype(alloc), CustomAlloc>);
        std::cout << "测试 8 (自定义比较器与分配器) 通过。\n";
    }

    // ----------------- 测试 9: 随机插入与删除压力测试 -----------------
    {
        std::mt19937 rng(42); // 固定种子以保证可重复性
        std::uniform_int_distribution<int> dist(-10000, 10000);
        std::set<int> refSet;  // 使用 std::set 作为参考
        Tree::AVLTree<int> tree;

        // 随机插入
        for (int i = 0; i < 1000; ++i)
        {
            int val = dist(rng);
            bool insertedSet = refSet.insert(val).second;
            bool insertedTree = tree.insert(val);
            assert(insertedSet == insertedTree);
        }

        assert(tree.size() == refSet.size());

        // 验证顺序和内容
        std::vector<int> treeVec, setVec;
        for (int v : tree) treeVec.push_back(v);
        for (int v : refSet) setVec.push_back(v);
        assert(treeVec == setVec);

        // 验证查找结果
        for (int v : setVec)
        {
            auto result = tree.find(v);
            assert(result.has_value());
            assert(result.value().get() == v);
        }

        // 随机删除
        for (int i = 0; i < 500; ++i)
        {
            int val = dist(rng);
            size_t erasedSet = refSet.erase(val);
            bool erasedTree = tree.erase(val);
            assert(erasedSet == static_cast<size_t>(erasedTree));
        }
        assert(tree.size() == refSet.size());

        // 再次验证
        treeVec.clear(); setVec.clear();
        for (int v : tree) treeVec.push_back(v);
        for (int v : refSet) setVec.push_back(v);
        assert(treeVec == setVec);

        // 再次验证查找结果
        for (int v : setVec)
        {
            auto result = tree.find(v);
            assert(result.has_value());
            assert(result.value().get() == v);
        }
        std::cout << "测试 9 (随机插入删除压力测试) 通过。\n";
    }

    // ----------------- 测试 10: 清空与空树状态 -----------------
    {
        Tree::AVLTree<int> tree;
        assert(tree.empty());
        tree.insert(1);
        assert(!tree.empty());
        tree.clear();
        assert(tree.empty());
        assert(tree.begin() == tree.end()); // 空树迭代器相等

        // 空树查找应返回 nullopt
        auto result = tree.find(1);
        assert(!result.has_value());
        std::cout << "测试 10 (清空与空树状态) 通过。\n";
    }

    // ----------------- 测试 11: 复杂类型的查找 -----------------
    {
        struct Person
        {
            int id;
            std::string name;

            bool operator<(const Person& other) const
            {
                return id < other.id;
            }
        };

        Tree::AVLTree<Person> tree;
        tree.insert(Person{ 1, "Alice" });
        tree.insert(Person{ 2, "Bob" });
        tree.insert(Person{ 3, "Charlie" });

        // 查找存在的元素
        Person searchKey{ 2, "" }; // 只需要id匹配
        auto result = tree.find(searchKey);
        assert(result.has_value());
        assert(result.value().get().id == 2);
        assert(result.value().get().name == "Bob");

        // 修改查找到的结果
        Person& personRef = result.value().get();
        personRef.name = "Robert";

        // 验证修改生效
        auto updatedResult = tree.find(Person{ 2, "" });
        assert(updatedResult.has_value());
        assert(updatedResult.value().get().name == "Robert");

        std::cout << "测试 11 (复杂类型的查找) 通过。\n";
    }

    std::cout << "\n所有测试通过！\n";
    return 0;
}