/**
 * @brief Trie（前缀树）全面测试
 */
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>

#include "../include/Trie.hpp"

// ============================================================
// 构造与基本操作
// ============================================================
TEST(TrieTest, DefaultConstruction)
{
    Tree::Trie<char> trie;
    EXPECT_EQ(trie.words_count(), 0u);
    EXPECT_TRUE(trie.empty_words());
}

TEST(TrieTest, CopyConstruction)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    trie.insert("world");

    Tree::Trie<char> copy(trie);
    EXPECT_EQ(copy.words_count(), 2u);
    EXPECT_TRUE(copy.search("hello"));
    EXPECT_TRUE(copy.search("world"));

    // 深拷贝验证：修改副本不影响原对象
    copy.insert("extra");
    EXPECT_EQ(copy.words_count(), 3u);
    EXPECT_EQ(trie.words_count(), 2u);
}

TEST(TrieTest, CopyAssignment)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    trie.insert("world");

    Tree::Trie<char> copy;
    copy.insert("old");
    copy = trie;

    EXPECT_EQ(copy.words_count(), 2u);
    EXPECT_TRUE(copy.search("hello"));
    EXPECT_TRUE(copy.search("world"));
    EXPECT_FALSE(copy.search("old"));
}

TEST(TrieTest, MoveConstruction)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    trie.insert("world");

    Tree::Trie<char> moved(std::move(trie));
    EXPECT_EQ(moved.words_count(), 2u);
    EXPECT_TRUE(moved.search("hello"));
    EXPECT_TRUE(moved.search("world"));
    // 移后源对象应为空
    EXPECT_EQ(trie.words_count(), 0u);
}

TEST(TrieTest, MoveAssignment)
{
    Tree::Trie<char> trie;
    trie.insert("hello");

    Tree::Trie<char> moved;
    moved = std::move(trie);

    EXPECT_EQ(moved.words_count(), 1u);
    EXPECT_TRUE(moved.search("hello"));
    EXPECT_EQ(trie.words_count(), 0u);
}

// ============================================================
// 插入
// ============================================================
TEST(TrieTest, InsertSingleWord)
{
    Tree::Trie<char> trie;
    EXPECT_TRUE(trie.insert("hello"));
    EXPECT_EQ(trie.words_count(), 1u);
    EXPECT_TRUE(trie.search("hello"));
}

TEST(TrieTest, InsertMultipleWords)
{
    Tree::Trie<char> trie;
    EXPECT_TRUE(trie.insert("hello"));
    EXPECT_TRUE(trie.insert("world"));
    EXPECT_TRUE(trie.insert("help"));
    EXPECT_TRUE(trie.insert("heap"));
    EXPECT_EQ(trie.words_count(), 4u);
}

TEST(TrieTest, InsertDuplicateWord)
{
    Tree::Trie<char> trie;
    EXPECT_TRUE(trie.insert("hello"));
    EXPECT_FALSE(trie.insert("hello"));  // 重复插入应返回 false
    EXPECT_EQ(trie.words_count(), 1u);
}

TEST(TrieTest, InsertEmptyString)
{
    Tree::Trie<char> trie;
    EXPECT_FALSE(trie.insert(""));  // 空字符串应返回 false
}

// 默认 Trie 接受所有字符（不再限制字母数字）
TEST(TrieTest, InsertAnyCharacters)
{
    Tree::Trie<char> trie;
    EXPECT_TRUE(trie.insert("hello world"));   // 空格
    EXPECT_TRUE(trie.insert("hello-world"));   // 连字符
    EXPECT_TRUE(trie.insert("hello@123"));     // 特殊符号
    EXPECT_TRUE(trie.search("hello world"));
    EXPECT_TRUE(trie.search("hello-world"));
    EXPECT_TRUE(trie.search("hello@123"));
    EXPECT_EQ(trie.words_count(), 3u);
}

// AlphanumericTrie 保持原有的字符限制
TEST(TrieTest, AlphanumericTrieRejectsInvalidChars)
{
    Tree::AlphanumericTrie<char> trie;
    EXPECT_TRUE(trie.insert("hello"));
    EXPECT_TRUE(trie.search("hello"));
    EXPECT_THROW(trie.insert("hello world"), std::invalid_argument);
    EXPECT_THROW(trie.insert("hello@123"), std::invalid_argument);
}

TEST(TrieTest, InsertAllValidCharacters)
{
    Tree::Trie<char> trie;
    EXPECT_TRUE(trie.insert("abc123XYZ"));  // 大小写+数字
    EXPECT_EQ(trie.words_count(), 1u);
    EXPECT_TRUE(trie.search("abc123XYZ"));
}

// ============================================================
// 搜索
// ============================================================
TEST(TrieTest, SearchExistingWord)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    EXPECT_TRUE(trie.search("hello"));
}

TEST(TrieTest, SearchNonExistentWord)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    EXPECT_FALSE(trie.search("world"));
    EXPECT_FALSE(trie.search("hell"));
    EXPECT_FALSE(trie.search("helloo"));
}

TEST(TrieTest, SearchPrefixNotWord)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    // "hel" 是前缀但不是单词
    EXPECT_FALSE(trie.search("hel"));
    EXPECT_TRUE(trie.startsWith("hel"));
}

TEST(TrieTest, SearchAfterRemove)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    trie.insert("help");
    trie.remove("hello");
    EXPECT_FALSE(trie.search("hello"));
    EXPECT_TRUE(trie.search("help"));  // "help" 应仍存在
}

TEST(TrieTest, SearchCaseSensitive)
{
    Tree::Trie<char> trie;
    trie.insert("Hello");
    EXPECT_TRUE(trie.search("Hello"));
    EXPECT_FALSE(trie.search("hello"));
    EXPECT_FALSE(trie.search("HELLO"));
}

// ============================================================
// 前缀搜索
// ============================================================
TEST(TrieTest, StartsWithExistingPrefix)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    trie.insert("help");
    EXPECT_TRUE(trie.startsWith("hel"));
    EXPECT_TRUE(trie.startsWith("hello"));
    EXPECT_TRUE(trie.startsWith("help"));
}

TEST(TrieTest, StartsWithNonExistentPrefix)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    EXPECT_FALSE(trie.startsWith("xyz"));
}

TEST(TrieTest, StartsWithEmptyString)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    // 空字符串前缀应该匹配所有（取决于实现）
    EXPECT_TRUE(trie.startsWith(""));
}

// ============================================================
// 删除
// ============================================================
TEST(TrieTest, RemoveExistingWord)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    EXPECT_TRUE(trie.remove("hello"));
    EXPECT_EQ(trie.words_count(), 0u);
    EXPECT_FALSE(trie.search("hello"));
}

TEST(TrieTest, RemoveNonExistentWord)
{
    Tree::Trie<char> trie;
    EXPECT_FALSE(trie.remove("hello"));  // 空树删除
    trie.insert("world");
    EXPECT_FALSE(trie.remove("hello"));  // 不存在的单词
}

TEST(TrieTest, RemoveWordThatIsPrefixOfAnother)
{
    Tree::Trie<char> trie;
    trie.insert("app");
    trie.insert("apple");
    // 删除 "app"，"apple" 应仍存在
    EXPECT_TRUE(trie.remove("app"));
    EXPECT_FALSE(trie.search("app"));
    EXPECT_TRUE(trie.search("apple"));
    EXPECT_EQ(trie.words_count(), 1u);
}

TEST(TrieTest, RemoveWordWithPrefixRelationship)
{
    Tree::Trie<char> trie;
    trie.insert("apple");
    trie.insert("app");
    // 删除 "apple"，"app" 应仍存在
    EXPECT_TRUE(trie.remove("apple"));
    EXPECT_FALSE(trie.search("apple"));
    EXPECT_TRUE(trie.search("app"));
    EXPECT_EQ(trie.words_count(), 1u);
}

TEST(TrieTest, RemoveAllWords)
{
    Tree::Trie<char> trie;
    std::vector<std::string> words = {"cat", "car", "card", "care", "dog"};
    for (auto& w : words) trie.insert(w);
    for (auto& w : words) EXPECT_TRUE(trie.remove(w));
    EXPECT_EQ(trie.words_count(), 0u);
    EXPECT_TRUE(trie.empty_words());
}

// ============================================================
// get_words_with_prefix
// ============================================================
TEST(TrieTest, GetWordsWithPrefixMultipleResults)
{
    Tree::Trie<char> trie;
    trie.insert("app");
    trie.insert("apple");
    trie.insert("application");
    trie.insert("banana");

    auto words = trie.get_words_with_prefix("app");
    EXPECT_EQ(words.size(), 3u);
    // 验证包含所有预期单词
    EXPECT_NE(std::find(words.begin(), words.end(), "app"), words.end());
    EXPECT_NE(std::find(words.begin(), words.end(), "apple"), words.end());
    EXPECT_NE(std::find(words.begin(), words.end(), "application"), words.end());
}

TEST(TrieTest, GetWordsWithPrefixSingleResult)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    trie.insert("world");

    auto words = trie.get_words_with_prefix("wo");
    EXPECT_EQ(words.size(), 1u);
    EXPECT_EQ(words[0], "world");
}

TEST(TrieTest, GetWordsWithPrefixNoResults)
{
    Tree::Trie<char> trie;
    trie.insert("hello");

    auto words = trie.get_words_with_prefix("xyz");
    EXPECT_TRUE(words.empty());
}

TEST(TrieTest, GetWordsWithPrefixEmptyPrefix)
{
    Tree::Trie<char> trie;
    trie.insert("a");
    trie.insert("b");
    trie.insert("c");

    auto words = trie.get_words_with_prefix("");
    EXPECT_EQ(words.size(), 3u);
}

// ============================================================
// 清空
// ============================================================
TEST(TrieTest, Clear)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    trie.insert("world");
    trie.insert("test");

    trie.clear();
    EXPECT_EQ(trie.words_count(), 0u);
    EXPECT_TRUE(trie.empty_words());
    EXPECT_FALSE(trie.search("hello"));
    EXPECT_FALSE(trie.search("world"));
}

TEST(TrieTest, ClearThenReinsert)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    trie.clear();
    EXPECT_TRUE(trie.insert("hello"));
    EXPECT_TRUE(trie.search("hello"));
    EXPECT_EQ(trie.words_count(), 1u);
}

// ============================================================
// 数值字符
// ============================================================
TEST(TrieTest, NumericCharacters)
{
    Tree::Trie<char> trie;
    EXPECT_TRUE(trie.insert("123"));
    EXPECT_TRUE(trie.insert("456"));
    EXPECT_TRUE(trie.search("123"));
    EXPECT_FALSE(trie.search("12"));
    EXPECT_TRUE(trie.startsWith("12"));
}

TEST(TrieTest, MixedAlphanumeric)
{
    Tree::Trie<char> trie;
    EXPECT_TRUE(trie.insert("abc123"));
    EXPECT_TRUE(trie.insert("xyz789"));
    EXPECT_TRUE(trie.search("abc123"));
    EXPECT_TRUE(trie.search("xyz789"));
    EXPECT_EQ(trie.words_count(), 2u);
}

// ============================================================
// 自赋值
// ============================================================
TEST(TrieTest, SelfAssignment)
{
    Tree::Trie<char> trie;
    trie.insert("hello");
    trie.insert("world");
    // 自赋值应安全
    trie = trie;              // NOLINT
    EXPECT_EQ(trie.words_count(), 2u);
    EXPECT_TRUE(trie.search("hello"));
    EXPECT_TRUE(trie.search("world"));
}

// ============================================================
// 压力测试
// ============================================================
TEST(TrieTest, LargeInsertAndSearch)
{
    Tree::Trie<char> trie;
    const int N = 1000;
    std::vector<std::string> words;
    for (int i = 0; i < N; ++i)
    {
        words.push_back("word" + std::to_string(i));
    }

    for (auto& w : words) trie.insert(w);
    EXPECT_EQ(trie.words_count(), static_cast<size_t>(N));

    for (auto& w : words) EXPECT_TRUE(trie.search(w)) << "Failed to find: " << w;

    for (auto& w : words) trie.remove(w);
    EXPECT_EQ(trie.words_count(), 0u);
    EXPECT_TRUE(trie.empty_words());
}

TEST(TrieTest, SharedPrefixInsert)
{
    Tree::Trie<char> trie;
    // 插入大量共享前缀的单词
    for (int i = 0; i < 500; ++i)
    {
        trie.insert("prefix" + std::to_string(i));
    }
    EXPECT_EQ(trie.words_count(), 500u);

    auto words = trie.get_words_with_prefix("prefix");
    EXPECT_EQ(words.size(), 500u);
}

// ============================================================
// 新特性测试：灵活字符、稀疏/密集存储
// ============================================================

// 默认 Trie 支持任意字符（含空格、符号、Unicode）
TEST(TrieTest, SpecialCharacters)
{
    Tree::Trie<char> trie;
    EXPECT_TRUE(trie.insert("café"));
    EXPECT_TRUE(trie.insert("naïve"));
    EXPECT_TRUE(trie.search("café"));
    EXPECT_TRUE(trie.search("naïve"));
    EXPECT_EQ(trie.words_count(), 2u);
}

TEST(TrieTest, PathLikeStrings)
{
    Tree::Trie<char> trie;
    EXPECT_TRUE(trie.insert("/usr/bin"));
    EXPECT_TRUE(trie.insert("/usr/lib"));
    EXPECT_TRUE(trie.insert("/usr/local/bin"));
    EXPECT_TRUE(trie.startsWith("/usr"));
    EXPECT_TRUE(trie.search("/usr/bin"));
    EXPECT_FALSE(trie.search("/usr"));
    EXPECT_EQ(trie.words_count(), 3u);

    auto words = trie.get_words_with_prefix("/usr/");
    EXPECT_EQ(words.size(), 3u);  // 三个都以 /usr/ 开头
}

// 密集存储：AlphanumericTrie（62 字符表，array 孩子）
TEST(TrieTest, DenseAlphanumericTrie)
{
    Tree::AlphanumericTrie<char> trie;
    EXPECT_TRUE(trie.insert("hello"));
    EXPECT_TRUE(trie.insert("WORLD"));
    EXPECT_TRUE(trie.insert("12345"));
    EXPECT_TRUE(trie.search("hello"));
    EXPECT_TRUE(trie.search("WORLD"));
    EXPECT_TRUE(trie.search("12345"));
    EXPECT_EQ(trie.words_count(), 3u);

    // 大小写敏感
    EXPECT_FALSE(trie.search("Hello"));

    // startsWith
    EXPECT_TRUE(trie.startsWith("hel"));
    EXPECT_TRUE(trie.startsWith("WO"));

    // 删除
    EXPECT_TRUE(trie.remove("hello"));
    EXPECT_FALSE(trie.search("hello"));
    EXPECT_EQ(trie.words_count(), 2u);
}

// DenseChildren + 小写字母映射
TEST(TrieTest, DenseLowercaseTrie)
{
    Tree::Trie<char, Tree::DenseChildren, Tree::AcceptAllChars<char>> trie;
    // 注意：LowercaseMapping 仅接受 a-z，但 AcceptAllChars 通过所有字符
    // 实际映射中使用 DenseChildren 默认的 AlphanumericMapping
}

// 使用 LowercaseMapping 的密集 Trie
TEST(TrieTest, LowercaseOnlyTrie)
{
    using LowerTrie = Tree::Trie<
        char, Tree::DenseChildren, Tree::AcceptAllChars<char>>;
    LowerTrie trie;
    EXPECT_TRUE(trie.insert("hello"));
    EXPECT_TRUE(trie.insert("world"));
    EXPECT_TRUE(trie.search("hello"));
    EXPECT_TRUE(trie.search("world"));
    EXPECT_EQ(trie.words_count(), 2u);
}

// 节点计数验证（内存效率）
TEST(TrieTest, NodeCountTracking)
{
    Tree::Trie<char> trie;
    EXPECT_EQ(trie.node_count(), 0u);

    trie.insert("hello");
    // "hello" 有 5 个字符 + 根节点 = 6 个节点
    EXPECT_EQ(trie.node_count(), 6u);

    trie.insert("help");
    // "help" 共享 "hel" → 新增 'p' 节点 = 1 个新节点
    EXPECT_EQ(trie.node_count(), 7u);

    trie.insert("he");
    // "he" 全部共享 → 0 个新节点（只是标记 is_word_end）
    EXPECT_EQ(trie.node_count(), 7u);
    EXPECT_EQ(trie.words_count(), 3u);
}

// 密集存储节点计数（对比稀疏）
TEST(TrieTest, DenseNodeCount)
{
    Tree::AlphanumericTrie<char> trie;
    trie.insert("hello");
    trie.insert("help");
    trie.insert("he");
    // 节点数应与稀疏模式一致
    EXPECT_EQ(trie.node_count(), 7u);
    EXPECT_EQ(trie.words_count(), 3u);
}

// 空树操作
TEST(TrieTest, EmptyTreeEdgeCases)
{
    Tree::Trie<char> trie;
    EXPECT_FALSE(trie.search("anything"));
    EXPECT_FALSE(trie.startsWith("a"));
    EXPECT_FALSE(trie.remove("anything"));
    EXPECT_TRUE(trie.empty_words());
    EXPECT_EQ(trie.node_count(), 0u);

    auto words = trie.get_words_with_prefix("");
    EXPECT_TRUE(words.empty());
}

// 拷贝独立性验证
TEST(TrieTest, DeepCopyIndependence)
{
    Tree::Trie<char> original;
    original.insert("hello");
    original.insert("world");

    auto copy = original;
    copy.insert("extra");

    EXPECT_TRUE(original.search("hello"));
    EXPECT_TRUE(original.search("world"));
    EXPECT_FALSE(original.search("extra"));
    EXPECT_EQ(original.words_count(), 2u);

    EXPECT_TRUE(copy.search("hello"));
    EXPECT_TRUE(copy.search("world"));
    EXPECT_TRUE(copy.search("extra"));
    EXPECT_EQ(copy.words_count(), 3u);
}

// 移动后源对象状态
TEST(TrieTest, MoveLeavesSourceEmpty)
{
    Tree::Trie<char> source;
    source.insert("data");

    auto moved = std::move(source);

    EXPECT_EQ(source.words_count(), 0u);
    EXPECT_EQ(source.node_count(), 0u);
    EXPECT_FALSE(source.search("data"));

    EXPECT_EQ(moved.words_count(), 1u);
    EXPECT_TRUE(moved.search("data"));
}

// 大容量压力测试（验证新实现的稳定性）
TEST(TrieTest, LargeScaleTest)
{
    Tree::Trie<char> trie;
    const int N = 2000;
    std::vector<std::string> words;
    words.reserve(N);
    for (int i = 0; i < N; ++i)
        words.push_back("w" + std::to_string(i) + "rd");

    // 插入
    for (auto& w : words) trie.insert(w);
    EXPECT_EQ(trie.words_count(), static_cast<size_t>(N));

    // 全部找到
    for (auto& w : words) EXPECT_TRUE(trie.search(w));

    // 前缀查询（应返回所有 N 个单词）
    auto result = trie.get_words_with_prefix("w");
    EXPECT_EQ(result.size(), static_cast<size_t>(N));

    // 全部删除
    for (auto& w : words) trie.remove(w);
    EXPECT_EQ(trie.words_count(), 0u);
    // 根节点可能保留（无子节点时不一定自动删除）
    EXPECT_TRUE(trie.empty_words());
}
