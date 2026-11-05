#pragma once
#include "Tree_BinaryRepresentation.hpp"
#include "Node_BinaryRepresentation.hpp"
#include <memory>
#include <concepts>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <cstdint>
#include <stack>
#include <cstdarg>


// 前向声明
namespace Tree
{
    namespace Inner_RBTree
    {
        // 定义比较器concept
        template<typename Comp, typename T>
        concept Comparator = requires(Comp comp, const T & a, const T & b)
        {
            { comp(a, b) } -> ::std::convertible_to<bool>;
            { comp(b, a) } -> ::std::convertible_to<bool>;
        };
    }

    template <typename T, Inner_RBTree::Comparator<T> Comp, typename Allocator>
    class RBTree;
}

namespace TreeNode
{
    /*
    ◦ 红黑树节点

    ◦ WARNING: 利用父指针存储颜色信息，依赖内存对齐!!!

    */
    enum class RBColor : std::uintptr_t
    {
        BLACK = 0,
        RED = 1
    };

    template <typename T>
    class RBNode : public Node_BinaryRepresentation<T>
    {
    public:
        template <typename T, ::Tree::Inner_RBTree::Comparator<T> Comp, typename Allocator>
        friend class ::Tree::RBTree;

        ~RBNode() = default;
    public:
        using value_type = T;
    private:
        using Base = Node_BinaryRepresentation<T>;
        using Node = RBNode<T>;
        using NodePtr = Node*;
        using Base::m_parent;

        // 直接获取值的辅助函数，零开销
        T& get_val() { return this->data; }
        const T& get_val() const { return this->data; }

    public:
        // 默认构造
        RBNode() : Base() {}

        // 拷贝 & 移动构造节点
        template <typename U>
            requires std::is_convertible_v<U, T>
        RBNode(U&& u, int8_t balance_factor = 0) : Base(std::forward<U>(u)) {}

        // 原地构造节点
        template <typename... Args>
        RBNode(std::in_place_t, Args&&... args) : Base(std::in_place, std::forward<Args>(args)...) {}

        // 获取父节点指针 (非const与const版本)
        NodePtr parent()
        {
            return reinterpret_cast<NodePtr>(reinterpret_cast<std::uintptr_t>(m_parent) & ~(std::uintptr_t{ 1 }));
        }
        NodePtr parent() const
        {
            return reinterpret_cast<NodePtr>(reinterpret_cast<std::uintptr_t>(m_parent) & ~(std::uintptr_t{ 1 }));
        }

        // 设置父节点指针(保留原有颜色)
        void set_parent(NodePtr ptr)
        {
            m_parent = reinterpret_cast<NodePtr>
                ((reinterpret_cast<std::uintptr_t>(ptr) & ~(std::uintptr_t{ 1 })) | (reinterpret_cast<std::uintptr_t>(m_parent) & 1));
        }

        // 获取颜色
        TreeNode::RBColor color() const
        {
            return static_cast<TreeNode::RBColor>(reinterpret_cast<std::uintptr_t>(m_parent) & 1);
        }

        // 设置颜色
        void set_color(TreeNode::RBColor color)
        {
            m_parent = reinterpret_cast<NodePtr>((reinterpret_cast<std::uintptr_t>(m_parent) & ~(std::uintptr_t{ 1 })) | static_cast<std::uintptr_t>(color));
        }

        // 同时设置父节点指针和颜色
        void set_parent_and_color(NodePtr parent, TreeNode::RBColor color)
        {
            m_parent = reinterpret_cast<NodePtr>((reinterpret_cast<std::uintptr_t>(parent) & ~(std::uintptr_t{ 1 })) | static_cast<std::uintptr_t>(color));
        }

        bool is_red() const { return color() == TreeNode::RBColor::RED; }
        bool is_black() const { return color() == TreeNode::RBColor::BLACK; }
    };
}

namespace Tree
{
    /*
     ◦ 红黑树实现

     */
    template <typename T, Inner_RBTree::Comparator<T> Comp = ::std::less<T>, typename Allocator = ::std::allocator<TreeNode::RBNode<T>>>
    class RBTree : public Tree_BinaryRepresentation<T, RBTree<T, Comp, Allocator>, TreeNode::RBNode<T>,
        typename std::allocator_traits<Allocator>::template rebind_alloc<TreeNode::RBNode<T>>>
    {
    private:
        using NodeAllocator =
            typename std::allocator_traits<Allocator>::template rebind_alloc<TreeNode::RBNode<T>>;
        using NodeTraits = std::allocator_traits<NodeAllocator>;
        using Base = Tree_BinaryRepresentation<T, RBTree<T, Comp, Allocator>, TreeNode::RBNode<T>, NodeAllocator>;
        using Node = TreeNode::RBNode<T>;
        using NodePtr = Node*;
        using Base::m_size;
        using Base::m_allocator;
        using iterator = Base::InIterator;
        using const_iterator = Base::const_InIterator;
        using Base::m_root;

        using Base::left_child;
        using Base::right_child;
        using Base::set_lchild;
        using Base::set_rchild;
        using Base::parent;
        using Base::set_parent;
        using Base::right_rotate;
        using Base::left_rotate;
        using Base::left_right_rotate;
        using Base::right_left_rotate;

    private:
        // 将比较器作为成员变量，如果 Comp 是无状态的空类， [[no_unique_address]] 可以防止其占用额外内存空间
        [[no_unique_address]] Comp m_comp;

    public:
        // 获取分配器
        Allocator get_allocator() const noexcept { return Allocator(m_allocator); }
        // 获取比较器
        Comp key_comp() const { return m_comp; }

        // 构造函数：支持传入自定义比较器实例
        RBTree(const Comp& comp = Comp(), const Allocator& alloc = Allocator())
            : Base(0, NodeAllocator(alloc)), m_comp(comp)
        {}
        RBTree(const Allocator& alloc)
            : Base(0, NodeAllocator(alloc)), m_comp(Comp())
        {}
        RBTree(const RBTree& other) : Base(other), m_comp(other.m_comp) {}

        RBTree& operator=(const RBTree& other)
        {
            if (this != &other)
            {
                Base::operator=(other);
                m_comp = other.m_comp;
            }
            return *this;
        }

        RBTree(RBTree&& other) noexcept : Base(std::move(other)), m_comp(std::move(other.m_comp)) {}

        RBTree& operator=(RBTree&& other) noexcept
        {
            if (this != &other)
            {
                Base::operator=(std::move(other));
                m_comp = std::move(other.m_comp);
            }
            return *this;
        }

        ~RBTree() = default;

        iterator begin() { return Base::begin_inorder(); }
        iterator end() { return Base::end_inorder(); }
        const_iterator begin() const { return Base::begin_inorder(); }
        const_iterator end() const { return Base::end_inorder(); }

        std::size_t size() const { return m_size; }
        bool empty() const { return m_size == 0; }
        using Base::clear;

    public:
        // 插入值（完美转发构造）
        template <typename... Args>
        bool insert(Args&&... args)
        {
            NodePtr new_node = this->create_node(std::forward<Args>(args)...);
            new_node->set_color(TreeNode::RBColor::RED);
            new_node->set_parent(nullptr);
            new_node->set_child(0, nullptr);
            new_node->set_child(1, nullptr);

            NodePtr p = nullptr;
            NodePtr current = m_root;
            const T& new_val = new_node->get_val();

            while (current)
            {
                p = current;
                const T& cur_val = current->get_val();
                if (m_comp(new_val, cur_val)) // 使用成员变量 m_comp
                {
                    current = left_child(current);
                }
                else if (m_comp(cur_val, new_val))
                {
                    current = right_child(current);
                }
                else
                {
                    // 遇到相同值时不插入，销毁已申请的节点，防止内存泄漏
                    this->destroy_node(new_node);
                    return false;
                }
            }

            new_node->set_parent(p);
            if (!p)
            {
                m_root = new_node;
            }
            else if (m_comp(new_val, p->get_val()))
            {
                set_lchild(p, new_node);
            }
            else
            {
                set_rchild(p, new_node);
            }

            ++m_size;
            insert_fixup(new_node);
            return true;
        }

        // 按值删除
        bool erase(const T& val)
        {
            NodePtr z = m_root;
            NodePtr z_parent = nullptr;

            while (z)
            {
                const T& z_val = z->get_val();
                if (m_comp(val, z_val))
                {
                    z_parent = z;
                    z = left_child(z);
                }
                else if (m_comp(z_val, val))
                {
                    z_parent = z;
                    z = right_child(z);
                }
                else
                {
                    break;
                }
            }

            if (!z) return false;

            TreeNode::RBColor original_color = z->color();
            NodePtr x = nullptr;
            NodePtr x_parent = nullptr;
            NodePtr y = nullptr;
            NodePtr y_original_parent = z_parent;

            if (!left_child(z))
            {
                y = z;
                x = right_child(z);
                x_parent = y_original_parent;
                transplant(z, right_child(z));
                // BUG FIX: 删除原代码中的 if (!x) x = y_original_parent;
            }
            else if (!right_child(z))
            {
                y = z;
                x = left_child(z);
                x_parent = y_original_parent;
                transplant(z, left_child(z));
                // BUG FIX: 删除原代码中的 if (!x) x = y_original_parent;
            }
            else
            {
                y = minimum(right_child(z));
                original_color = y->color();
                x = right_child(y);
                y_original_parent = parent(y);

                if (parent(y) != z)
                {
                    transplant(y, right_child(y));
                    set_rchild(y, right_child(z));
                    if (right_child(z)) set_parent(right_child(z), y);
                    x_parent = y_original_parent;
                }
                else
                {
                    if (x) set_parent(x, y);
                    x_parent = y;
                }

                transplant(z, y);
                set_lchild(y, left_child(z));
                if (left_child(z)) set_parent(left_child(z), y);
                y->set_color(z->color());
            }

            if (original_color == TreeNode::RBColor::BLACK)
            {
                NodePtr w = nullptr;
                while (x != m_root && (!x || x->is_black()))
                {
                    NodePtr xp = x ? parent(x) : x_parent;
                    if (!xp) break;

                    if (x == left_child(xp))
                    {
                        w = right_child(xp);
                        if (w && w->is_red())
                        {
                            w->set_color(TreeNode::RBColor::BLACK);
                            xp->set_color(TreeNode::RBColor::RED);
                            left_rotate(m_root, xp);
                            w = right_child(xp);
                        }

                        if (!w)
                        {
                            x = xp;
                            x_parent = parent(xp);
                            continue;
                        }

                        bool left_black = !left_child(w) || left_child(w)->is_black();
                        bool right_black = !right_child(w) || right_child(w)->is_black();

                        if (left_black && right_black)
                        {
                            w->set_color(TreeNode::RBColor::RED);
                            x = xp;
                            x_parent = parent(xp);
                        }
                        else
                        {
                            if (right_black)
                            {
                                if (left_child(w)) left_child(w)->set_color(TreeNode::RBColor::BLACK);
                                w->set_color(TreeNode::RBColor::RED);
                                right_rotate(m_root, w);
                                w = right_child(xp);
                            }

                            if (w) w->set_color(xp->color());
                            xp->set_color(TreeNode::RBColor::BLACK);
                            if (right_child(w)) right_child(w)->set_color(TreeNode::RBColor::BLACK);
                            left_rotate(m_root, xp);
                            x = m_root;
                            break;
                        }
                    }
                    else
                    {
                        w = left_child(xp);
                        if (w && w->is_red())
                        {
                            w->set_color(TreeNode::RBColor::BLACK);
                            xp->set_color(TreeNode::RBColor::RED);
                            right_rotate(m_root, xp);
                            w = left_child(xp);
                        }

                        if (!w)
                        {
                            x = xp;
                            x_parent = parent(xp);
                            continue;
                        }

                        bool left_black = !left_child(w) || left_child(w)->is_black();
                        bool right_black = !right_child(w) || right_child(w)->is_black();

                        if (left_black && right_black)
                        {
                            w->set_color(TreeNode::RBColor::RED);
                            x = xp;
                            x_parent = parent(xp);
                        }
                        else
                        {
                            if (left_black)
                            {
                                if (right_child(w)) right_child(w)->set_color(TreeNode::RBColor::BLACK);
                                w->set_color(TreeNode::RBColor::RED);
                                left_rotate(m_root, w);
                                w = left_child(xp);
                            }

                            if (w) w->set_color(xp->color());
                            xp->set_color(TreeNode::RBColor::BLACK);
                            if (left_child(w)) left_child(w)->set_color(TreeNode::RBColor::BLACK);
                            right_rotate(m_root, xp);
                            x = m_root;
                            break;
                        }
                    }
                }
                if (x) x->set_color(TreeNode::RBColor::BLACK);
            }

            this->destroy_node(z);
            --m_size;
            return true;
        }
        // 查找
        std::optional<std::reference_wrapper<T>> find(const T& val)
        {
            return do_find(val);
        }

        const std::optional<std::reference_wrapper<T>> find(const T& val) const
        {
            return do_find(val);
        }

    private:
        NodePtr minimum(NodePtr node) const
        {
            while (left_child(node)) node = left_child(node);
            return node;
        }

        void transplant(NodePtr u, NodePtr v)
        {
            if (!parent(u))
            {
                m_root = v;
            }
            else if (u == left_child(parent(u)))
            {
                set_lchild(parent(u), v);
            }
            else
            {
                set_rchild(parent(u), v);
            }
            if (v) set_parent(v, parent(u));
        }

        // 插入修复
        void insert_fixup(NodePtr z)
        {
            NodePtr p = nullptr;
            while ((p = z->parent()) != nullptr && p->is_red())
            {
                NodePtr g = p->parent();
                if (!g) break;

                if (p == left_child(g))
                {
                    NodePtr uncle = right_child(g);
                    if (uncle && uncle->is_red())
                    {
                        p->set_color(TreeNode::RBColor::BLACK);
                        uncle->set_color(TreeNode::RBColor::BLACK);
                        g->set_color(TreeNode::RBColor::RED);
                        z = g;
                    }
                    else
                    {
                        if (z == right_child(p))
                        {
                            z = p;
                            left_rotate(m_root, z);
                            p = z->parent();
                            g = p ? p->parent() : nullptr;
                        }
                        if (p) p->set_color(TreeNode::RBColor::BLACK);
                        if (g)
                        {
                            g->set_color(TreeNode::RBColor::RED);
                            right_rotate(m_root, g);
                        }
                    }
                }
                else
                {
                    NodePtr uncle = left_child(g);
                    if (uncle && uncle->is_red())
                    {
                        p->set_color(TreeNode::RBColor::BLACK);
                        uncle->set_color(TreeNode::RBColor::BLACK);
                        g->set_color(TreeNode::RBColor::RED);
                        z = g;
                    }
                    else
                    {
                        if (z == left_child(p))
                        {
                            z = p;
                            right_rotate(m_root, z);
                            p = z->parent();
                            g = p ? p->parent() : nullptr;
                        }
                        if (p) p->set_color(TreeNode::RBColor::BLACK);
                        if (g)
                        {
                            g->set_color(TreeNode::RBColor::RED);
                            left_rotate(m_root, g);
                        }
                    }
                }
            }
            if (m_root) m_root->set_color(TreeNode::RBColor::BLACK);
        }

        // 查找辅助函数
        std::optional<std::reference_wrapper<T>> do_find(const T& value) const
        {
            NodePtr current = m_root;
            while (current)
            {
                const T& cur_val = current->get_val();
                if (m_comp(value, cur_val))
                {
                    current = left_child(current);
                }
                else if (m_comp(cur_val, value))
                {
                    current = right_child(current);
                }
                else
                {
                    return current->value();
                }
            }
            return std::nullopt;
        }
    };
}