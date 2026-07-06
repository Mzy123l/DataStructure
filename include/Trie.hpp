#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <queue>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Tree
{

// ============================================================
// 跨平台强制内联宏（若未定义）
// ============================================================
#ifndef ALWAYS_INLINE
#if defined(__GNUC__) || defined(__clang__)
#define ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE
#endif
#endif

// ============================================================
// 字母表映射（供 DenseChildren 使用）
// ============================================================

// 默认：数字 + 大小写字母 (0-9, A-Z, a-z)
template <typename charT>
struct AlphanumericMapping
{
    static constexpr size_t alphabet_size = 62;

    ALWAYS_INLINE static constexpr size_t to_index(charT ch) noexcept
    {
        if (ch >= '0' && ch <= '9') return ch - '0';
        if (ch >= 'A' && ch <= 'Z') return 10 + (ch - 'A');
        return 36 + (ch - 'a');
    }

    ALWAYS_INLINE static constexpr bool is_valid(charT ch) noexcept
    {
        return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') ||
               (ch >= 'a' && ch <= 'z');
    }

    ALWAYS_INLINE static constexpr charT to_char(size_t idx) noexcept
    {
        if (idx < 10) return static_cast<charT>('0' + idx);
        if (idx < 36) return static_cast<charT>('A' + (idx - 10));
        return static_cast<charT>('a' + (idx - 36));
    }
};

// 小写字母映射
template <typename charT>
struct LowercaseMapping
{
    static constexpr size_t alphabet_size = 26;

    ALWAYS_INLINE static constexpr size_t to_index(charT ch) noexcept
    {
        return ch - 'a';
    }

    ALWAYS_INLINE static constexpr bool is_valid(charT ch) noexcept
    {
        return ch >= 'a' && ch <= 'z';
    }

    ALWAYS_INLINE static constexpr charT to_char(size_t idx) noexcept
    {
        return static_cast<charT>('a' + idx);
    }
};

// DNA 碱基映射 (A/C/G/T)
template <typename charT>
struct DNAMapping
{
    static constexpr size_t alphabet_size = 4;

    ALWAYS_INLINE static constexpr size_t to_index(charT ch) noexcept
    {
        switch (ch)
        {
        case 'A': case 'a': return 0;
        case 'C': case 'c': return 1;
        case 'G': case 'g': return 2;
        case 'T': case 't': return 3;
        default: return 0;
        }
    }

    ALWAYS_INLINE static constexpr bool is_valid(charT ch) noexcept
    {
        return ch == 'A' || ch == 'a' || ch == 'C' || ch == 'c' ||
               ch == 'G' || ch == 'g' || ch == 'T' || ch == 't';
    }

    ALWAYS_INLINE static constexpr charT to_char(size_t idx) noexcept
    {
        constexpr charT chars[] = {'A', 'C', 'G', 'T'};
        return chars[idx];
    }
};

// ============================================================
// 字符过滤器
// ============================================================

template <typename charT>
struct AcceptAllChars
{
    ALWAYS_INLINE static constexpr bool is_valid(charT) noexcept
    {
        return true;
    }
};

template <typename charT>
struct AlphanumericFilter
{
    ALWAYS_INLINE static constexpr bool is_valid(charT ch) noexcept
    {
        return AlphanumericMapping<charT>::is_valid(ch);
    }
};

// ============================================================
// 存储策略：稀疏（hash map）与密集（array）
// ============================================================

// 稀疏存储 — 内存友好，支持任意字符集
template <typename charT, typename NodePtr>
class SparseChildren
{
public:
    ALWAYS_INLINE NodePtr get(charT ch) const
    {
        auto it = map_.find(ch);
        return it != map_.end() ? it->second : nullptr;
    }

    ALWAYS_INLINE void set(charT ch, NodePtr node) { map_[ch] = node; }

    ALWAYS_INLINE void erase(charT ch) { map_.erase(ch); }

    ALWAYS_INLINE bool is_leaf() const noexcept { return map_.empty(); }

    template <typename Func>
    void for_each(Func&& f) const
    {
        for (const auto& [ch, child] : map_)
            f(ch, child);
    }

private:
    std::unordered_map<charT, NodePtr> map_;
};

// 密集存储 — 无哈希开销，O(1)，需指定字母表映射
template <typename charT, typename NodePtr,
          typename Mapping = AlphanumericMapping<charT>>
class DenseChildren
{
public:
    static constexpr size_t N = Mapping::alphabet_size;

    DenseChildren() { arr_.fill(nullptr); }

    ALWAYS_INLINE NodePtr get(charT ch) const
    {
        if (!Mapping::is_valid(ch)) return nullptr;
        return arr_[Mapping::to_index(ch)];
    }

    ALWAYS_INLINE void set(charT ch, NodePtr node)
    {
        arr_[Mapping::to_index(ch)] = node;
    }

    ALWAYS_INLINE void erase(charT ch)
    {
        arr_[Mapping::to_index(ch)] = nullptr;
    }

    ALWAYS_INLINE bool is_leaf() const noexcept
    {
        for (size_t i = 0; i < N; ++i)
            if (arr_[i]) return false;
        return true;
    }

    // 回调接收 (charT, NodePtr)
    template <typename Func>
    void for_each(Func&& f) const
    {
        for (size_t i = 0; i < N; ++i)
            if (arr_[i]) f(Mapping::to_char(i), arr_[i]);
    }

private:
    std::array<NodePtr, N> arr_;
};

// ============================================================
// Trie 前缀树（优化版）
// ============================================================
// 模板参数:
//   charT        - 字符类型（默认 char）
//   StoragePolicy - 子节点存储策略模板（SparseChildren 或 DenseChildren）
//   CharFilter   - 字符验证器
//   Allocator    - 底层分配器
// ============================================================
template <typename charT = char,
          template <typename, typename> class StoragePolicy = SparseChildren,
          typename CharFilter = AcceptAllChars<charT>,
          typename Allocator = std::allocator<charT>>
class Trie
{
private:
    // 前向声明节点类型
    struct TrieNode;
    using NodePtr = TrieNode*;

    // 具体存储类型
    using ChildrenStorage = StoragePolicy<charT, NodePtr>;

    struct TrieNode
    {
        ChildrenStorage children;
        bool is_word_end = false;
    };

    // 节点分配器
    using NodeAlloc = typename std::allocator_traits<
        Allocator>::template rebind_alloc<TrieNode>;
    using AllocTraits = std::allocator_traits<NodeAlloc>;

public:
    // ============================================================
    // 构造 / 析构 / 拷贝 / 移动
    // ============================================================
    Trie() : node_alloc_(Allocator()) {}

    explicit Trie(const Allocator& alloc) : node_alloc_(alloc) {}

    Trie(const CharFilter&, const Allocator& alloc = Allocator())
        : node_alloc_(alloc)
    {}

    Trie(const Trie& other)
        : node_alloc_(AllocTraits::select_on_container_copy_construction(
              other.node_alloc_))
    {
        deep_copy(other);
    }

    Trie& operator=(const Trie& other)
    {
        if (this != &other)
        {
            clear();
            node_alloc_ = AllocTraits::select_on_container_copy_construction(
                other.node_alloc_);
            deep_copy(other);
        }
        return *this;
    }

    Trie(Trie&& other) noexcept
        : root_(other.root_)
        , word_count_(other.word_count_)
        , node_count_(other.node_count_)
        , node_alloc_(std::move(other.node_alloc_))
    {
        other.root_ = nullptr;
        other.word_count_ = 0;
        other.node_count_ = 0;
    }

    Trie& operator=(Trie&& other) noexcept
    {
        if (this != &other)
        {
            clear();
            root_ = other.root_;
            word_count_ = other.word_count_;
            node_count_ = other.node_count_;
            node_alloc_ = std::move(other.node_alloc_);
            other.root_ = nullptr;
            other.word_count_ = 0;
            other.node_count_ = 0;
        }
        return *this;
    }

    ~Trie() { clear(); }

    // ============================================================
    // 插入单词
    // ============================================================
    bool insert(const std::basic_string<charT>& word)
    {
        if (word.empty()) return false;

        if (!root_) root_ = create_node();

        NodePtr current = root_;
        for (charT ch : word)
        {
            if (!CharFilter::is_valid(ch))
            {
                throw std::invalid_argument(
                    "Trie::insert: invalid character");
            }
            NodePtr child = current->children.get(ch);
            if (!child)
            {
                child = create_node();
                current->children.set(ch, child);
            }
            current = child;
        }

        if (current->is_word_end) return false;
        current->is_word_end = true;
        ++word_count_;
        return true;
    }

    // ============================================================
    // 搜索单词
    // ============================================================
    bool search(const std::basic_string<charT>& word) const
    {
        NodePtr node = traverse(word);
        return node && node->is_word_end;
    }

    // ============================================================
    // 前缀匹配
    // ============================================================
    bool startsWith(const std::basic_string<charT>& prefix) const
    {
        return traverse(prefix) != nullptr;
    }

    // ============================================================
    // 删除单词
    // ============================================================
    bool remove(const std::basic_string<charT>& word)
    {
        if (word.empty() || !root_) return false;

        std::stack<std::pair<NodePtr, charT>> path;
        NodePtr current = root_;

        for (charT ch : word)
        {
            if (!CharFilter::is_valid(ch)) return false;
            NodePtr child = current->children.get(ch);
            if (!child) return false;
            path.push({current, ch});
            current = child;
        }

        if (!current->is_word_end) return false;

        current->is_word_end = false;
        --word_count_;

        // 回溯清理不再需要的节点
        if (current->children.is_leaf())
        {
            while (!path.empty())
            {
                auto [parent, ch] = path.top();
                path.pop();

                parent->children.erase(ch);
                destroy_node(current);
                current = parent;

                if (current->is_word_end || !current->children.is_leaf())
                    break;
            }
        }

        return true;
    }

    // ============================================================
    // 获取所有以给定前缀开头的单词
    // ============================================================
    std::vector<std::basic_string<charT>> get_words_with_prefix(
        const std::basic_string<charT>& prefix) const
    {
        std::vector<std::basic_string<charT>> result;
        NodePtr start = traverse(prefix);
        if (start)
        {
            auto current(prefix);
            collect_words(start, current, result);
        }
        return result;
    }

    // ============================================================
    // 基本信息
    // ============================================================
    std::size_t words_count() const noexcept { return word_count_; }
    std::size_t node_count() const noexcept { return node_count_; }
    bool empty_words() const noexcept { return word_count_ == 0; }
    bool empty() const noexcept { return node_count_ == 0; }
    std::size_t size() const noexcept { return node_count_; }

    // ============================================================
    // 清空
    // ============================================================
    void clear()
    {
        if (root_)
        {
            clear_recursive(root_);
            root_ = nullptr;
        }
        word_count_ = 0;
        node_count_ = 0;
    }

    // ============================================================
    // 分配器
    // ============================================================
    Allocator get_allocator() const noexcept
    {
        return Allocator(node_alloc_);
    }

private:
    // ============================================================
    // 内部方法
    // ============================================================

    NodePtr create_node()
    {
        NodePtr p = AllocTraits::allocate(node_alloc_, 1);
        try
        {
            AllocTraits::construct(node_alloc_, p);
        }
        catch (...)
        {
            AllocTraits::deallocate(node_alloc_, p, 1);
            throw;
        }
        ++node_count_;
        return p;
    }

    void destroy_node(NodePtr p) noexcept
    {
        if (p)
        {
            AllocTraits::destroy(node_alloc_, p);
            AllocTraits::deallocate(node_alloc_, p, 1);
            --node_count_;
        }
    }

    NodePtr traverse(const std::basic_string<charT>& str) const
    {
        if (!root_) return nullptr;
        NodePtr current = root_;
        for (charT ch : str)
        {
            if (!CharFilter::is_valid(ch)) return nullptr;
            current = current->children.get(ch);
            if (!current) return nullptr;
        }
        return current;
    }

    void collect_words(
        NodePtr node,
        std::basic_string<charT>& prefix,
        std::vector<std::basic_string<charT>>& result) const
    {
        if (node->is_word_end) result.push_back(prefix);

        node->children.for_each(
            [&](charT ch, NodePtr child)
            {
                prefix.push_back(ch);
                collect_words(child, prefix, result);
                prefix.pop_back();
            });
    }

    void clear_recursive(NodePtr node) noexcept
    {
        node->children.for_each(
            [this](charT, NodePtr child) { clear_recursive(child); });
        destroy_node(node);
    }

    void deep_copy(const Trie& other)
    {
        if (!other.root_) return;

        std::queue<NodePtr> src_q, dst_q;

        root_ = create_node();
        root_->is_word_end = other.root_->is_word_end;

        src_q.push(other.root_);
        dst_q.push(root_);

        while (!src_q.empty())
        {
            NodePtr src = src_q.front();
            src_q.pop();
            NodePtr dst = dst_q.front();
            dst_q.pop();

            src->children.for_each(
                [&](charT ch, NodePtr src_child)
                {
                    NodePtr dst_child = create_node();
                    dst_child->is_word_end = src_child->is_word_end;
                    dst->children.set(ch, dst_child);
                    src_q.push(src_child);
                    dst_q.push(dst_child);
                });
        }

        word_count_ = other.word_count_;
    }

    // ============================================================
    // 成员变量
    // ============================================================
    NodePtr root_ = nullptr;
    std::size_t word_count_ = 0;
    std::size_t node_count_ = 0;
    [[no_unique_address]] NodeAlloc node_alloc_;
};

// ============================================================
// 便捷别名
// ============================================================

// 默认 Trie（稀疏存储，接受所有字符）
template <typename charT = char, typename Allocator = std::allocator<charT>>
using BasicTrie = Trie<charT, SparseChildren, AcceptAllChars<charT>, Allocator>;

// 字母数字 Trie（密集存储，62 字符表）
template <typename charT = char, typename Allocator = std::allocator<charT>>
using AlphanumericTrie = Trie<
    charT, DenseChildren, AlphanumericFilter<charT>, Allocator>;

// 小写字母 Trie（密集存储，26 字母表）
template <typename charT = char, typename Allocator = std::allocator<charT>>
using LowercaseTrie = Trie<
    charT, DenseChildren, AcceptAllChars<charT>, Allocator>;

}  // namespace Tree
