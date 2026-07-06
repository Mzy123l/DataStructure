/**
 * @brief 性能基准测试
 * 将自定义数据结构与 STL 标准库进行对比
 */
#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <random>
#include <set>
#include <map>
#include <unordered_map>
#include <forward_list>
#include <list>
#include <string>
#include <vector>

#include "../include/AVLTree.hpp"
#include "../include/RBTree.hpp"
#include "../include/BPlusTree.hpp"
#include "../include/Trie.hpp"
#include "../include/singlyLinkedList.hpp"
#include "../include/doublyLinkedList.hpp"
#include "../include/UnionFind.hpp"

using namespace std::chrono;

// 计时工具
struct Timer
{
    high_resolution_clock::time_point start;
    Timer() : start(high_resolution_clock::now()) {}
    double elapsed_ms() const
    {
        auto end = high_resolution_clock::now();
        return duration<double, std::milli>(end - start).count();
    }
};

// 测试规模
constexpr int SMALL = 1000;
constexpr int MEDIUM = 10000;
constexpr int LARGE = 100000;

// ============================================================
// AVLTree vs std::set
// ============================================================
TEST(Benchmark, AVLTree_vs_std_set_Insert)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, LARGE * 10);

    std::vector<int> data(LARGE);
    for (int& v : data) v = dist(rng);

    // AVLTree 插入
    {
        Tree::AVLTree<int> avl;
        Timer t;
        for (int v : data) avl.insert(v);
        double ms = t.elapsed_ms();
        std::cout << "  AVLTree insert " << avl.size() << " elements: " << ms << " ms" << std::endl;
    }

    // std::set 插入
    {
        std::set<int> s;
        Timer t;
        for (int v : data) s.insert(v);
        double ms = t.elapsed_ms();
        std::cout << "  std::set  insert " << s.size() << " elements: " << ms << " ms" << std::endl;
    }
}

TEST(Benchmark, AVLTree_vs_std_set_Find)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, MEDIUM * 10);

    Tree::AVLTree<int> avl;
    std::set<int> s;
    std::vector<int> data(MEDIUM);
    for (int& v : data) v = dist(rng);
    for (int v : data) { avl.insert(v); s.insert(v); }

    // 一半命中一半不命中
    std::vector<int> queries(MEDIUM);
    for (int i = 0; i < MEDIUM; ++i) queries[i] = dist(rng);

    {
        Timer t;
        int found = 0;
        for (int q : queries)
            if (avl.find(q)) ++found;
        double ms = t.elapsed_ms();
        std::cout << "  AVLTree find " << queries.size() << " queries (" << found << " hits): " << ms << " ms" << std::endl;
    }
    {
        Timer t;
        int found = 0;
        for (int q : queries)
            if (s.find(q) != s.end()) ++found;
        double ms = t.elapsed_ms();
        std::cout << "  std::set  find " << queries.size() << " queries (" << found << " hits): " << ms << " ms" << std::endl;
    }
}

TEST(Benchmark, AVLTree_vs_std_set_Erase)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, MEDIUM * 10);

    std::vector<int> data(MEDIUM);
    for (int& v : data) v = dist(rng);

    {
        Tree::AVLTree<int> avl;
        for (int v : data) avl.insert(v);
        Timer t;
        for (int v : data) avl.erase(v);
        double ms = t.elapsed_ms();
        EXPECT_TRUE(avl.empty());
        std::cout << "  AVLTree erase " << data.size() << " elements: " << ms << " ms" << std::endl;
    }
    {
        std::set<int> s;
        for (int v : data) s.insert(v);
        Timer t;
        for (int v : data) s.erase(v);
        double ms = t.elapsed_ms();
        EXPECT_TRUE(s.empty());
        std::cout << "  std::set  erase " << data.size() << " elements: " << ms << " ms" << std::endl;
    }
}

TEST(Benchmark, AVLTree_vs_std_set_Iteration)
{
    const int N = MEDIUM * 5;
    Tree::AVLTree<int> avl;
    std::set<int> s;
    for (int i = 0; i < N; ++i) { avl.insert(i); s.insert(i); }

    {
        Timer t;
        long long sum = 0;
        for (const auto& v : avl) sum += v;
        double ms = t.elapsed_ms();
        std::cout << "  AVLTree inorder  iteration " << N << " elements: " << ms << " ms (sum=" << sum << ")" << std::endl;
    }
    {
        Timer t;
        long long sum = 0;
        for (const auto& v : s) sum += v;
        double ms = t.elapsed_ms();
        std::cout << "  std::set  inorder iteration " << N << " elements: " << ms << " ms (sum=" << sum << ")" << std::endl;
    }
}

// ============================================================
// RBTree vs std::set
// ============================================================
TEST(Benchmark, RBTree_vs_std_set_Insert)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, LARGE * 10);
    std::vector<int> data(LARGE);
    for (int& v : data) v = dist(rng);

    {
        Tree::RBTree<int> rb;
        Timer t;
        for (int v : data) rb.insert(v);
        double ms = t.elapsed_ms();
        std::cout << "  RBTree   insert " << rb.size() << " elements: " << ms << " ms" << std::endl;
    }
    {
        std::set<int> s;
        Timer t;
        for (int v : data) s.insert(v);
        double ms = t.elapsed_ms();
        std::cout << "  std::set  insert " << s.size() << " elements: " << ms << " ms" << std::endl;
    }
}

TEST(Benchmark, RBTree_vs_std_set_Find)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, MEDIUM * 10);
    Tree::RBTree<int> rb;
    std::set<int> s;
    std::vector<int> data(MEDIUM);
    for (int& v : data) v = dist(rng);
    for (int v : data) { rb.insert(v); s.insert(v); }

    std::vector<int> queries(MEDIUM);
    for (int& q : queries) q = dist(rng);

    {
        Timer t;
        int found = 0;
        for (int q : queries)
            if (rb.find(q)) ++found;
        double ms = t.elapsed_ms();
        std::cout << "  RBTree   find " << queries.size() << " queries: " << ms << " ms" << std::endl;
    }
    {
        Timer t;
        int found = 0;
        for (int q : queries)
            if (s.find(q) != s.end()) ++found;
        double ms = t.elapsed_ms();
        std::cout << "  std::set  find " << queries.size() << " queries: " << ms << " ms" << std::endl;
    }
}

TEST(Benchmark, RBTree_vs_std_set_Erase)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, MEDIUM * 10);
    std::vector<int> data(MEDIUM);
    for (int& v : data) v = dist(rng);

    {
        Tree::RBTree<int> rb;
        for (int v : data) rb.insert(v);
        Timer t;
        for (int v : data) rb.erase(v);
        double ms = t.elapsed_ms();
        std::cout << "  RBTree   erase " << data.size() << " elements: " << ms << " ms" << std::endl;
    }
    {
        std::set<int> s;
        for (int v : data) s.insert(v);
        Timer t;
        for (int v : data) s.erase(v);
        double ms = t.elapsed_ms();
        std::cout << "  std::set  erase " << data.size() << " elements: " << ms << " ms" << std::endl;
    }
}

// ============================================================
// BPlusTree vs std::map
// ============================================================
TEST(Benchmark, BPlusTree_vs_std_map)
{
    const int N = MEDIUM * 5;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> key_dist(0, N * 10);
    std::uniform_int_distribution<int> val_dist(0, 99999);

    std::vector<std::pair<int, int>> data(N);
    for (auto& [k, v] : data) { k = key_dist(rng); v = val_dist(rng); }

    std::cout << "  B+Tree (order=64):" << std::endl;
    {
        Tree::BPlusTree<int, int> bpt({}, 64);
        Timer t;
        for (auto& [k, v] : data) bpt.insert(k, v);
        double ms = t.elapsed_ms();
        std::cout << "    insert " << bpt.size() << " kv: " << ms << " ms" << std::endl;
    }
    {
        Tree::BPlusTree<int, int> bpt({}, 64);
        for (auto& [k, v] : data) bpt.insert(k, v);
        std::vector<int> queries(N);
        for (int& q : queries) q = key_dist(rng);
        Timer t;
        int found = 0;
        for (int q : queries)
            if (bpt.find(q)) ++found;
        double ms = t.elapsed_ms();
        std::cout << "    find  " << queries.size() << " queries: " << ms << " ms" << std::endl;
    }
    {
        Tree::BPlusTree<int, int> bpt({}, 64);
        for (auto& [k, v] : data) bpt.insert(k, v);
        Timer t;
        for (auto& [k, v] : data) bpt.erase(k);
        double ms = t.elapsed_ms();
        std::cout << "    erase " << data.size() << " kv: " << ms << " ms" << std::endl;
    }

    std::cout << "  std::map:" << std::endl;
    {
        std::map<int, int> m;
        Timer t;
        for (auto& [k, v] : data) m[k] = v;
        double ms = t.elapsed_ms();
        std::cout << "    insert " << m.size() << " kv: " << ms << " ms" << std::endl;
    }
    {
        std::map<int, int> m;
        for (auto& [k, v] : data) m[k] = v;
        std::vector<int> queries(N);
        for (int& q : queries) q = key_dist(rng);
        Timer t;
        int found = 0;
        for (int q : queries)
            if (m.find(q) != m.end()) ++found;
        double ms = t.elapsed_ms();
        std::cout << "    find  " << queries.size() << " queries: " << ms << " ms" << std::endl;
    }
    {
        std::map<int, int> m;
        for (auto& [k, v] : data) m[k] = v;
        Timer t;
        for (auto& [k, v] : data) m.erase(k);
        double ms = t.elapsed_ms();
        std::cout << "    erase " << data.size() << " kv: " << ms << " ms" << std::endl;
    }
}

// ============================================================
// Trie 性能测试
// ============================================================
TEST(Benchmark, Trie_Performance)
{
    const int N = MEDIUM;

    // 生成随机单词
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> len_dist(3, 8);
    std::uniform_int_distribution<int> char_dist(0, 25);
    std::vector<std::string> words;
    for (int i = 0; i < N; ++i)
    {
        int len = len_dist(rng);
        std::string w(len, 'a');
        for (int j = 0; j < len; ++j) w[j] = 'a' + char_dist(rng);
        words.push_back(w);
    }

    std::cout << "  Trie (sparse, default):" << std::endl;
    {
        Tree::Trie<char> trie;
        Timer t;
        for (auto& w : words) trie.insert(w);
        double ms = t.elapsed_ms();
        std::cout << "    insert " << trie.words_count() << " words: " << ms << " ms" << std::endl;
    }
    {
        Tree::Trie<char> trie;
        for (auto& w : words) trie.insert(w);
        Timer t;
        int found = 0;
        for (auto& w : words)
            if (trie.search(w)) ++found;
        double ms = t.elapsed_ms();
        std::cout << "    search " << words.size() << " words (" << found << " hits): " << ms << " ms" << std::endl;
    }

    std::cout << "  Trie (dense, alphanumeric):" << std::endl;
    {
        Tree::AlphanumericTrie<char> trie;
        Timer t;
        for (auto& w : words) trie.insert(w);
        double ms = t.elapsed_ms();
        std::cout << "    insert " << trie.words_count() << " words: " << ms << " ms" << std::endl;
    }
    {
        Tree::AlphanumericTrie<char> trie;
        for (auto& w : words) trie.insert(w);
        Timer t;
        int found = 0;
        for (auto& w : words)
            if (trie.search(w)) ++found;
        double ms = t.elapsed_ms();
        std::cout << "    search " << words.size() << " words (" << found << " hits): " << ms << " ms" << std::endl;
    }
}

// ============================================================
// singlyLinkedList vs std::forward_list
// ============================================================
TEST(Benchmark, singlyLinkedList_vs_forward_list)
{
    const int N = MEDIUM * 5;

    {
        DateStructure::singlyLinkedList<int> sll;
        Timer t;
        for (int i = 0; i < N; ++i) sll.push_front(i);
        double ms = t.elapsed_ms();
        std::cout << "  singlyLinkedList push_front " << N << ": " << ms << " ms" << std::endl;
    }
    {
        std::forward_list<int> fl;
        Timer t;
        for (int i = 0; i < N; ++i) fl.push_front(i);
        double ms = t.elapsed_ms();
        std::cout << "  std::forward_list push_front " << N << ": " << ms << " ms" << std::endl;
    }
}

// ============================================================
// doublyLinkedList vs std::list
// ============================================================
TEST(Benchmark, doublyLinkedList_vs_list)
{
    const int N = MEDIUM * 5;

    {
        DoublyLinkedList::doublyLinkedList<int> dll;
        Timer t;
        for (int i = 0; i < N; ++i) dll.push_back(i);
        double ms = t.elapsed_ms();
        std::cout << "  doublyLinkedList push_back " << N << ": " << ms << " ms" << std::endl;
    }
    {
        std::list<int> l;
        Timer t;
        for (int i = 0; i < N; ++i) l.push_back(i);
        double ms = t.elapsed_ms();
        std::cout << "  std::list       push_back " << N << ": " << ms << " ms" << std::endl;
    }

    {
        DoublyLinkedList::doublyLinkedList<int> dll;
        for (int i = 0; i < N; ++i) dll.push_back(i);
        Timer t;
        long long sum = 0;
        for (const auto& v : dll) sum += v;
        double ms = t.elapsed_ms();
        std::cout << "  doublyLinkedList iterate " << N << ": " << ms << " ms" << std::endl;
    }
    {
        std::list<int> l;
        for (int i = 0; i < N; ++i) l.push_back(i);
        Timer t;
        long long sum = 0;
        for (const auto& v : l) sum += v;
        double ms = t.elapsed_ms();
        std::cout << "  std::list       iterate " << N << ": " << ms << " ms" << std::endl;
    }
}

// ============================================================
// UnionFind 性能测试
// ============================================================
TEST(Benchmark, UnionFind_Performance)
{
    const int N = LARGE;

    {
        Forest::basicUnionFind uf(N);
        std::mt19937 rng(42);
        std::uniform_int_distribution<size_t> dist(0, N - 1);

        Timer t;
        for (int i = 0; i < N; ++i)
        {
            size_t a = dist(rng);
            size_t b = dist(rng);
            uf.unite(a, b);
        }
        double ms = t.elapsed_ms();
        std::cout << "  basicUnionFind " << N << " elements, " << N << " unites: " << ms << " ms";
        std::cout << " (components: " << uf.count() << ")" << std::endl;
    }
    {
        Forest::basicUnionFind uf(N);
        std::mt19937 rng(42);
        std::uniform_int_distribution<size_t> dist(0, N - 1);

        // 先做一批合并
        for (int i = 0; i < N / 2; ++i)
        {
            size_t a = dist(rng);
            size_t b = dist(rng);
            uf.unite(a, b);
        }

        Timer t;
        for (int i = 0; i < N; ++i)
        {
            size_t x = dist(rng);
            uf.find(x);
        }
        double ms = t.elapsed_ms();
        std::cout << "  basicUnionFind find " << N << " queries (path compressed): " << ms << " ms" << std::endl;
    }
}

// ============================================================
// 顺序 vs 随机插入对比
// ============================================================
TEST(Benchmark, Sequential_vs_Random)
{
    const int N = MEDIUM;

    std::cout << "=== Sequential Insert ===" << std::endl;
    {
        Tree::AVLTree<int> avl;
        Timer t;
        for (int i = 0; i < N; ++i) avl.insert(i);
        std::cout << "  AVLTree sequential: " << t.elapsed_ms() << " ms" << std::endl;
    }
    {
        Tree::RBTree<int> rb;
        Timer t;
        for (int i = 0; i < N; ++i) rb.insert(i);
        std::cout << "  RBTree  sequential: " << t.elapsed_ms() << " ms" << std::endl;
    }
    {
        std::set<int> s;
        Timer t;
        for (int i = 0; i < N; ++i) s.insert(i);
        std::cout << "  std::set sequential: " << t.elapsed_ms() << " ms" << std::endl;
    }

    std::cout << "=== Reverse Order Insert ===" << std::endl;
    {
        Tree::AVLTree<int> avl;
        Timer t;
        for (int i = N - 1; i >= 0; --i) avl.insert(i);
        std::cout << "  AVLTree reverse: " << t.elapsed_ms() << " ms" << std::endl;
    }
    {
        Tree::RBTree<int> rb;
        Timer t;
        for (int i = N - 1; i >= 0; --i) rb.insert(i);
        std::cout << "  RBTree  reverse: " << t.elapsed_ms() << " ms" << std::endl;
    }
    {
        std::set<int> s;
        Timer t;
        for (int i = N - 1; i >= 0; --i) s.insert(i);
        std::cout << "  std::set reverse: " << t.elapsed_ms() << " ms" << std::endl;
    }

    std::cout << "=== Random Insert ===" << std::endl;
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, N * 10);
    std::vector<int> data(N);
    for (int& v : data) v = dist(rng);

    {
        Tree::AVLTree<int> avl;
        Timer t;
        for (int v : data) avl.insert(v);
        std::cout << "  AVLTree random: " << t.elapsed_ms() << " ms" << std::endl;
    }
    {
        Tree::RBTree<int> rb;
        Timer t;
        for (int v : data) rb.insert(v);
        std::cout << "  RBTree  random: " << t.elapsed_ms() << " ms" << std::endl;
    }
    {
        std::set<int> s;
        Timer t;
        for (int v : data) s.insert(v);
        std::cout << "  std::set random: " << t.elapsed_ms() << " ms" << std::endl;
    }
}
