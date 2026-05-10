// Test_Trie.cpp
#include "../include/Trie.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

// 辅助函数：打印测试结果
void print_test_result(const std::string& test_name, bool passed)
{
    std::cout << (passed ? "[PASS] " : "[FAIL] ") << test_name << std::endl;
}

int main()
{
    int total_tests = 0;
    int passed_tests = 0;

    // 测试1: 默认构造函数和基本属性
    {
        ++total_tests;
        Tree::Trie<char> trie;
        bool passed = trie.words_count() == 0 && trie.empty_words();
        print_test_result("默认构造函数和基本属性", passed);
        if (passed) ++passed_tests;
    }

    // 测试2: 插入和搜索单个单词
    {
        ++total_tests;
        Tree::Trie<char> trie;
        bool insert_result = trie.insert("hello");
        bool search_result = trie.search("hello");
        bool passed = insert_result && search_result && trie.words_count() == 1;
        print_test_result("插入和搜索单个单词", passed);
        if (passed) ++passed_tests;
    }

    // 测试3: 插入重复单词
    {
        ++total_tests;
        Tree::Trie<char> trie;
        trie.insert("test");
        bool second_insert = trie.insert("test"); // 应该返回false
        bool passed = !second_insert && trie.words_count() == 1;
        print_test_result("插入重复单词", passed);
        if (passed) ++passed_tests;
    }

    // 测试4: 搜索不存在的单词
    {
        ++total_tests;
        Tree::Trie<char> trie;
        trie.insert("world");
        bool search_existing = trie.search("world");
        bool search_nonexistent = trie.search("word"); // 相似但不同
        bool passed = search_existing && !search_nonexistent;
        print_test_result("搜索不存在的单词", passed);
        if (passed) ++passed_tests;
    }

    // 测试5: startsWith前缀检查
    {
        ++total_tests;
        Tree::Trie<char> trie;
        trie.insert("application");
        trie.insert("apple");
        trie.insert("app");

        bool prefix1 = trie.startsWith("app");
        bool prefix2 = trie.startsWith("applic");
        bool prefix3 = trie.startsWith("b"); // 不存在的前缀

        bool passed = prefix1 && prefix2 && !prefix3;
        print_test_result("startsWith前缀检查", passed);
        if (passed) ++passed_tests;
    }

    // 测试6: 删除单词
    {
        ++total_tests;
        Tree::Trie<char> trie;
        trie.insert("delete");
        trie.insert("deletion");
        trie.insert("del");

        size_t before_count = trie.words_count();
        bool delete1 = trie.remove("delete");    // 应该成功
        bool delete2 = trie.remove("nonexist");  // 应该失败
        bool search_after = trie.search("delete"); // 应该失败
        bool search_other = trie.search("deletion"); // 应该成功

        bool passed = delete1 && !delete2 && !search_after && search_other
            && trie.words_count() == before_count - 1;
        print_test_result("删除单词", passed);
        if (passed) ++passed_tests;
    }

    // 测试7: 获取前缀匹配的所有单词
    {
        ++total_tests;
        Tree::Trie<char> trie;
        trie.insert("cat");
        trie.insert("category");
        trie.insert("caterpillar");
        trie.insert("dog"); // 不应出现在结果中

        auto words = trie.get_words_with_prefix("cat");
        bool size_ok = words.size() == 3;
        bool contains_cat = false, contains_category = false, contains_caterpillar = false;

        for (const auto& w : words)
        {
            if (w == "cat") contains_cat = true;
            if (w == "category") contains_category = true;
            if (w == "caterpillar") contains_caterpillar = true;
        }

        bool passed = size_ok && contains_cat && contains_category && contains_caterpillar;
        print_test_result("获取前缀匹配的所有单词", passed);
        if (passed) ++passed_tests;
    }

    // 测试8: 清空操作
    {
        ++total_tests;
        Tree::Trie<char> trie;
        trie.insert("one");
        trie.insert("two");
        trie.insert("three");

        bool before_clear = trie.words_count() == 3;
        trie.clear();
        bool after_clear = trie.words_count() == 0 && trie.empty_words();
        bool search_after = !trie.search("one") && !trie.search("two");

        bool passed = before_clear && after_clear && search_after;
        print_test_result("清空操作", passed);
        if (passed) ++passed_tests;
    }

    // 测试9: 拷贝构造函数
    {
        ++total_tests;
        Tree::Trie<char> trie1;
        trie1.insert("original");
        trie1.insert("copy");

        Tree::Trie<char> trie2(trie1); // 拷贝构造

        // 验证两个trie都有相同的内容
        bool same_content = trie2.search("original") && trie2.search("copy")
            && trie2.words_count() == 2;
        std::cout << trie1.search("original")<< trie1.search("copy")<< (trie1.words_count() == 2);
        std::cout << trie2.search("original") << trie2.search("copy") << (trie2.words_count() == 2);
        // 修改原始trie，验证拷贝是独立的
        trie1.remove("original");
        bool copy_unchanged = trie2.search("original") && trie2.words_count() == 2;
        std::cout << trie1.search("original") << (trie1.words_count() == 2);
        std::cout << trie2.search("original") << (trie2.words_count() == 2);
        bool passed = same_content && copy_unchanged;
        print_test_result("拷贝构造函数", passed);
        if (passed) ++passed_tests;
    }

    // 测试10: 拷贝赋值运算符
    {
        ++total_tests;
        Tree::Trie<char> trie1;
        trie1.insert("source");

        Tree::Trie<char> trie2;
        trie2.insert("old");

        trie2 = trie1; // 拷贝赋值

        bool has_source = trie2.search("source");
        bool no_old = !trie2.search("old");
        bool correct_count = trie2.words_count() == 1;
        std::cout << has_source << no_old << correct_count;
        // 自赋值测试
        trie2 = trie2;
        bool after_self_assign = trie2.search("source") && trie2.words_count() == 1;
        std::cout << after_self_assign;
        bool passed = has_source && no_old && correct_count && after_self_assign;
        print_test_result("拷贝赋值运算符", passed);
        if (passed) ++passed_tests;
    }

    // 测试11: 移动构造函数
    {
        ++total_tests;
        Tree::Trie<char> trie1;
        trie1.insert("moveme");

        size_t original_count = trie1.words_count();
        Tree::Trie<char> trie2(std::move(trie1)); // 移动构造

        bool moved_content = trie2.search("moveme") && trie2.words_count() == original_count;
        bool source_emptied = trie1.empty_words() && trie1.words_count() == 0;

        bool passed = moved_content && source_emptied;
        print_test_result("移动构造函数", passed);
        if (passed) ++passed_tests;
    }

    // 测试12: 移动赋值运算符
    {
        ++total_tests;
        Tree::Trie<char> trie1;
        trie1.insert("moving");

        Tree::Trie<char> trie2;
        trie2.insert("tobereplaced");

        size_t original_count = trie1.words_count();
        trie2 = std::move(trie1); // 移动赋值

        bool moved_content = trie2.search("moving") && trie2.words_count() == original_count;
        bool source_emptied = trie1.empty_words();
        bool old_content_gone = !trie2.search("tobereplaced");

        bool passed = moved_content && source_emptied && old_content_gone;
        print_test_result("移动赋值运算符", passed);
        if (passed) ++passed_tests;
    }

    // 测试13: 字符集测试（数字+大小写字母）
    {
        ++total_tests;
        Tree::Trie<char> trie;
        // 测试各种有效字符
        bool insert1 = trie.insert("Test123");
        bool insert2 = trie.insert("abcDEF");
        bool insert3 = trie.insert("987XYZ");

        bool search1 = trie.search("Test123");
        bool search2 = trie.search("abcDEF");
        bool search3 = trie.search("987XYZ");

        bool passed = insert1 && insert2 && insert3 &&
            search1 && search2 && search3 &&
            trie.words_count() == 3;
        print_test_result("字符集测试（数字+大小写字母）", passed);
        if (passed) ++passed_tests;
    }

    // 测试14: 空字符串处理
    {
        ++total_tests;
        Tree::Trie<char> trie;
        bool insert_empty = trie.insert(""); // 根据文档，应该返回false
        bool passed = !insert_empty && trie.words_count() == 0;
        print_test_result("空字符串处理", passed);
        if (passed) ++passed_tests;
    }

    // 测试15: 复杂删除场景（删除后节点清理）
    {
        ++total_tests;
        Tree::Trie<char> trie;
        trie.insert("app");
        trie.insert("apple");
        trie.insert("application");

        // 删除"apple"，但"app"和"application"还存在
        bool delete_apple = trie.remove("apple");
        bool app_exists = trie.search("app");
        bool apple_gone = !trie.search("apple");
        bool application_exists = trie.search("application");

        bool passed = delete_apple && app_exists && apple_gone && application_exists
            && trie.words_count() == 2;
        print_test_result("复杂删除场景", passed);
        if (passed) ++passed_tests;
    }

    // 汇总结果
    std::cout << "\n========================================" << std::endl;
    std::cout << "测试汇总: " << passed_tests << "/" << total_tests << " 通过" << std::endl;

    if (passed_tests == total_tests)
    {
        std::cout << "所有测试通过！" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "有 " << (total_tests - passed_tests) << " 个测试失败" << std::endl;
        return 1;
    }
}