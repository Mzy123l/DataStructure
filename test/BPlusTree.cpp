// BPlusTree_UnitTest.cpp
// 此单元测试仅使用 BPlusTree 类的公有接口
#include "../include/BPlusTree.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main()
{

    std::cout << "开始 BPlusTree 单元测试...\n";

    // 测试 1: 构造与基本属性
    {
        Tree::BPlusTree<int, std::string> tree;
        assert(tree.empty());
        assert(tree.size() == 0);
        assert(tree.get_leftmost_leaf() == nullptr);
        assert(tree.get_rightmost_leaf() == nullptr);
        std::cout << "测试 1 通过: 默认构造与基本属性。\n";
    }

    // 测试 2: 插入与查找
    {
        Tree::BPlusTree<int, std::string> tree(/* allocator */{}, /* order */ 4);
        tree.insert(10, "ten");
        tree.insert(20, "twenty");
        tree.insert(5, "five");

        assert(tree.size() == 3);
        assert(!tree.empty());

        auto* val1 = tree.find(10);
        assert(val1 != nullptr && *val1 == "ten");

        auto* val2 = tree.find(20);
        assert(val2 != nullptr && *val2 == "twenty");

        auto* val3 = tree.find(5);
        assert(val3 != nullptr && *val3 == "five");

        auto* val4 = tree.find(99); // 不存在的键
        assert(val4 == nullptr);

        std::cout << "测试 2 通过: 插入与查找。\n";
    }

    // 测试 3: 重复插入更新值
    {
        Tree::BPlusTree<int, std::string> tree;
        tree.insert(1, "first");
        tree.insert(1, "updated");

        assert(tree.size() == 1); // 大小应为1
        auto* val = tree.find(1);
        assert(val != nullptr && *val == "updated");

        std::cout << "测试 3 通过: 重复插入更新值。\n";
    }

    // 测试 4: 删除
    {
        Tree::BPlusTree<int, double> tree;
        tree.insert(100, 3.14);
        tree.insert(200, 2.71);
        tree.insert(150, 1.62);
        tree.insert(900, 0);
        bool erased = tree.erase(200);
        assert(erased);
        assert(tree.size() == 3);
        assert(tree.find(200) == nullptr);
        assert(tree.find(100) != nullptr);
        assert(tree.find(150) != nullptr);

        bool erased_non_exist = tree.erase(999);
        assert(!erased_non_exist);
        assert(tree.size() == 3);

        std::cout << "测试 4 通过: 删除操作。\n";
    }

    // 测试 5: 清空树
    {
        Tree::BPlusTree<int, char> tree;
        tree.insert(1, 'a');
        tree.insert(2, 'b');
        assert(tree.size() == 2);

        tree.clear();
        assert(tree.empty());
        assert(tree.size() == 0);
        assert(tree.get_leftmost_leaf() == nullptr);
        assert(tree.get_rightmost_leaf() == nullptr);

        std::cout << "测试 5 通过: 清空树。\n";
    }

    // 测试 6: 叶节点链表遍历
    {
        // 顺序插入，使叶节点链表有序
        Tree::BPlusTree<int, std::string> tree(/* allocator */{}, /* order */ 3);
        std::vector<int> keys = { 5, 3, 8, 1, 4, 7, 9 };
        for (int k : keys)
        {
            tree.insert(k, "val" + std::to_string(k));
        }
        // 检查链表顺序
        auto* leaf = tree.get_leftmost_leaf();
        int prev_key = -1;
        int count = 0;
        while (leaf != nullptr)
        {
            for (const auto& [k, v] : leaf->leaf_data())
            {
                // 检查链表中的键是否按升序排列
                assert(k > prev_key);
                prev_key = k;
                count++;
            }
            leaf = leaf->leaf_next();
        }
        assert(count == keys.size());
        std::cout << "测试 6 通过: 叶节点链表遍历有序。\n";
    }

    // 测试 7: 内存使用统计 (可选测试，不检查具体值，只确保可调用)
    {
        Tree::BPlusTree<int, long> tree;
        auto usage = tree.memory_usage();
        // 可以打印，但不做断言，因为空树的内存使用取决于实现
        // std::cout << "空树内存使用: " << usage << " 字节\n";
        std::cout << "测试 7 通过: 内存使用统计接口可调用。\n";
    }

    // 测试 8: 移动语义
    {
        Tree::BPlusTree<int, std::string> tree1;
        tree1.insert(42, "answer");
        size_t original_size = tree1.size();

        Tree::BPlusTree<int, std::string> tree2 = std::move(tree1);
        assert(tree2.size() == original_size);
        assert(tree1.empty()); // 源对象应为空状态

        auto* val = tree2.find(42);
        assert(val != nullptr && *val == "answer");
        std::cout << "测试 8 通过: 移动语义。\n";
    }

    // 测试 9: 基于文档示例的复杂场景 (触发节点分裂与合并)
    {
        Tree::BPlusTree<int, int> tree(/* allocator */{}, /* order */ 4); // 小阶数，易于触发分裂
        // 插入数据使其发生分裂
        for (int i = 1; i <= 2000; ++i)
        {
            tree.insert(i, i * 10000);
        }
        assert(tree.size() == 2000);

        // 验证查找
        for (int i = 1; i <= 2000; ++i)
        {
            auto* val = tree.find(i);
            assert(val != nullptr && *val == i * 10000);
        }
        
        // 删除部分数据，可能触发合并
        for (int i = 1; i <= 1000; ++i)
        {
            bool erased = tree.erase(i);
            assert(erased);
        }
        assert(tree.size() == 1000);
        
        // 验证剩余数据
        for (int i = 1001; i <= 2000; ++i)
        {
            auto* val = tree.find(i);
            assert(val != nullptr && *val == i * 10000);
        }

        // 验证已删除的数据
        for (int i = 1; i <= 1000; ++i)
        {
            auto* val = tree.find(i);
            assert(val == nullptr);
        }
        std::cout << "测试 9 通过: 复杂场景(分裂与合并)。\n";
    }

    std::cout << "\n所有单元测试通过！\n";
    return 0;
}