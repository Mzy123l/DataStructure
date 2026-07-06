#pragma once
#include "Tree_BinaryRepresentation.hpp"
#include "Node_BinaryRepresentation.hpp"
#include <memory>
#include <concepts>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <cstdint>
#include <functional>


namespace Tree
{
    namespace Inner_AVLTree
    {
        // 定义比较器concept
        template<typename Comp, typename T>
        concept Comparator = requires(Comp comp, const T & a, const T & b)
        {
            { comp(a, b) } -> std::convertible_to<bool>;
            { comp(b, a) } -> std::convertible_to<bool>;
        };
    }
    // 前向声明
    template <typename T, Inner_AVLTree::Comparator<T> Comp, typename Allocator>
    class AVLTree;
}

namespace TreeNode
{
    // AVL树节点
    template <typename T>
    class AVLNode :public Node_BinaryRepresentation<T>
    {
    public:
        template <typename U, Tree::Inner_AVLTree::Comparator<U> Comp, typename Allocator>
        friend class ::Tree::AVLTree;

        ~AVLNode() = default;

        // 默认构造
        AVLNode() :Base() {}

        // 拷贝& 移动构造节点
        template <typename U>
            requires std::is_convertible_v<U, T>
        AVLNode(U&& u, std::size_t height = 1) : Base(std::forward<U>(u)), m_height(height) {}

        // 原地构造节点
        template <typename... Args>
        AVLNode(std::in_place_t, Args&&... args) : Base(std::in_place, std::forward<Args>(args)...) {}

    public:
        using value_type = T;

        // 直接获取值的辅助函数，绕开 std::optional 的拆包开销
        [[gnu::always_inline]] T& get_val() { return *this->data; }
        [[gnu::always_inline]] const T& get_val() const { return *this->data; }

        // 重写基类的递归 height()，变为 O(1) 的缓存读取
        std::size_t height() const { return m_height; }

        // 更新高度接口
        void set_height(std::size_t h) { m_height = h; }

    private:
        // 直接缓存树高
        std::size_t m_height = 1;

        using Base = Node_BinaryRepresentation<T>;
    };
}

namespace Tree
{
    // AVL树
    template <typename T, Inner_AVLTree::Comparator<T> Comp = std::less<T>, typename Allocator = std::allocator<TreeNode::AVLNode<T>>>
    class AVLTree :public Tree_BinaryRepresentation<T, AVLTree<T, Comp, Allocator>, TreeNode::AVLNode<T>,
        typename std::allocator_traits<Allocator>::template rebind_alloc<TreeNode::AVLNode<T>> >
    {
    private:
        using NodeAllocator =
            typename std::allocator_traits<Allocator>::template rebind_alloc<TreeNode::AVLNode<T>>;
        using NodeTraits = std::allocator_traits<NodeAllocator>;

        using Base = Tree_BinaryRepresentation<T, AVLTree<T, Comp, Allocator>, TreeNode::AVLNode<T>, NodeAllocator>;
        using Node = TreeNode::AVLNode<T>;
        using NodePtr = Node*;
        using Base::m_size;
        using Base::m_allocator;
        using iterator = Base::InIterator;
        using const_iterator = Base::const_InIterator;
        using Base::m_root;

    private:
        // 从基类引入访问节点结构的辅助函数
        using Base::left_child;
        using Base::right_child;
        using Base::set_lchild;
        using Base::set_rchild;
        using Base::set_parent;
        using Base::parent;

        // 存储比较器作为成员。使用 [[no_unique_address]] (EBCO)
        [[no_unique_address]] Comp m_comp;

        // ===== 核心优化：O(1) 高度与平衡因子计算工具 =====
        static int get_height(NodePtr node)
        {
            return node ? static_cast<int>(node->height()) : 0;
        }

        static int get_balance(NodePtr node)
        {
            return node ? get_height(right_child(node)) - get_height(left_child(node)) : 0;
        }

        static void update_height(NodePtr node)
        {
            if (node)
            {
                node->set_height(1 + std::max(get_height(left_child(node)), get_height(right_child(node))));
            }
        }

        // ===== 高度感知旋转 =====
        void left_rotate_avl(NodePtr& root, NodePtr x)
        {
            NodePtr y = right_child(x);
            Base::left_rotate(root, x);
            // 旋转后先更新靠下的节点 x，再更新靠上的节点 y
            update_height(x);
            if (y) update_height(y);
        }

        void right_rotate_avl(NodePtr& root, NodePtr x)
        {
            NodePtr y = left_child(x);
            Base::right_rotate(root, x);
            // 旋转后先更新靠下的节点 x，再更新靠上的节点 y
            update_height(x);
            if (y) update_height(y);
        }

        // ===== 统一的高度回溯与平衡修复机制 =====
        // is_insert: true=插入（高度只增不减，可提前终止）
        //            false=删除（高度可能波动，需完整回溯）
        void balance_upwards(NodePtr curr, bool is_insert = false)
        {
            while (curr)
            {
                NodePtr lc = left_child(curr);
                NodePtr rc = right_child(curr);
                int lh = get_height(lc);
                int rh = get_height(rc);
                int old_h = static_cast<int>(curr->height());
                int new_h = 1 + std::max(lh, rh);
                int balance = rh - lh;

                if (balance > 1) // 右子树过高
                {
                    if (get_balance(rc) < 0) // RL 型
                    {
                        right_rotate_avl(m_root, rc);
                    }
                    left_rotate_avl(m_root, curr);
                    // 旋转后子树高度恢复到插入前的值，父节点不受影响
                    if (is_insert) return;
                    curr = parent(parent(curr));
                }
                else if (balance < -1) // 左子树过高
                {
                    if (get_balance(lc) > 0) // LR 型
                    {
                        left_rotate_avl(m_root, lc);
                    }
                    right_rotate_avl(m_root, curr);
                    if (is_insert) return;
                    curr = parent(parent(curr));
                }
                else
                {
                    curr->set_height(new_h);
                    // 插入优化：高度不变则提前终止
                    if (is_insert && new_h == old_h) return;
                    curr = parent(curr);
                }
            }
        }

    public:
        // 获取分配器
        Allocator get_allocator() const noexcept
        {
            return Allocator(m_allocator);
        }

        // 默认构造
        explicit AVLTree(const Comp& comp = Comp(), const Allocator& alloc = Allocator())
            : Base(0, NodeAllocator(alloc)), m_comp(comp)
        {}
        explicit AVLTree(const Allocator& alloc)
            : Base(0, NodeAllocator(alloc)), m_comp(Comp())
        {}
        // 拷贝构造
        AVLTree(const AVLTree& other)
            : Base(other.m_size, other.m_allocator), m_comp(other.m_comp)
        {
            deepCopy(other);
        }

        // 移动构造
        AVLTree(AVLTree&& other) noexcept
            : Base(std::move(other)), m_comp(std::move(other.m_comp))
        {}

        // 拷贝赋值
        AVLTree& operator=(const AVLTree& other)
        {
            if (this != &other)
            {
                clear();
                m_size = other.m_size;
                m_allocator = other.m_allocator;
                m_comp = other.m_comp;
                deepCopy(other);
            }
            return *this;
        }

        // 移动赋值
        AVLTree& operator=(AVLTree&& other) noexcept
        {
            if (this != &other)
            {
                Base::operator=(std::move(other));
                m_comp = std::move(other.m_comp);
            }
            return *this;
        }

        ~AVLTree() = default;

        // 插入值
        template <typename... Args>
        bool insert(Args&&... args)
        {
            NodePtr new_node = this->create_node(std::forward<Args>(args)...);

            if (!m_root)
            {
                m_root = new_node;
                ++m_size;
                return true;
            }

            NodePtr curr = m_root;
            NodePtr p = nullptr;
            bool is_left = false;
            const T& new_val = new_node->get_val();

            while (curr)
            {
                p = curr;
                const T& cur_val = curr->get_val();
                if (m_comp(new_val, cur_val))
                {
                    curr = left_child(curr);
                    is_left = true;
                }
                else if (m_comp(cur_val, new_val))
                {
                    curr = right_child(curr);
                    is_left = false;
                }
                else
                {
                    this->destroy_node(new_node);
                    return false;
                }
            }

            if (is_left) set_lchild(p, new_node);
            else set_rchild(p, new_node);
            set_parent(new_node, p);
            ++m_size;

            // 自底向上维护高度与平衡
            balance_upwards(p, true);  // 插入路径：可提前终止
            return true;
        }

        // 删除值
        bool erase(const T& val)
        {
            NodePtr target = m_root;
            NodePtr parent_of_target = nullptr;
            bool target_is_left_child = false;

            while (target)
            {
                const T& cur_val = target->get_val();
                if (m_comp(val, cur_val))
                {
                    parent_of_target = target;
                    target_is_left_child = true;
                    target = left_child(target);
                }
                else if (m_comp(cur_val, val))
                {
                    parent_of_target = target;
                    target_is_left_child = false;
                    target = right_child(target);
                }
                else break;
            }

            if (!target) return false;

            NodePtr balance_start = nullptr;

            // 情况1&2：目标节点无左或右孩子
            if (!left_child(target))
            {
                NodePtr replacement = right_child(target);
                balance_start = parent_of_target; // 从父节点开始回溯平衡

                if (parent_of_target)
                {
                    if (target_is_left_child) set_lchild(parent_of_target, replacement);
                    else set_rchild(parent_of_target, replacement);
                }
                else m_root = replacement;

                if (replacement) set_parent(replacement, parent_of_target);
            }
            else if (!right_child(target))
            {
                NodePtr replacement = left_child(target);
                balance_start = parent_of_target; // 从父节点开始回溯平衡

                if (parent_of_target)
                {
                    if (target_is_left_child) set_lchild(parent_of_target, replacement);
                    else set_rchild(parent_of_target, replacement);
                }
                else m_root = replacement;

                if (replacement) set_parent(replacement, parent_of_target);
            }
            // 情况3：目标节点有两个孩子
            else
            {
                // 找后继（右子树的最左节点）
                NodePtr successor = right_child(target);
                NodePtr successor_parent = target;
                while (left_child(successor))
                {
                    successor_parent = successor;
                    successor = left_child(successor);
                }

                NodePtr successor_right = right_child(successor);

                if (successor_parent == target)
                {
                    balance_start = successor; // 直接右孩子作为后继，从后继开始回溯
                }
                else
                {
                    balance_start = successor_parent; // 否则从后继的父节点回溯
                    set_lchild(successor_parent, successor_right);
                    if (successor_right) set_parent(successor_right, successor_parent);

                    set_rchild(successor, right_child(target));
                    if (right_child(target)) set_parent(right_child(target), successor);
                }

                set_lchild(successor, left_child(target));
                if (left_child(target)) set_parent(left_child(target), successor);

                set_parent(successor, parent_of_target);
                if (parent_of_target)
                {
                    if (target_is_left_child) set_lchild(parent_of_target, successor);
                    else set_rchild(parent_of_target, successor);
                }
                else m_root = successor;
            }

            this->destroy_node(target);
            --m_size;

            // O(log N) 重平衡与高度更新
            balance_upwards(balance_start);
            return true;
        }

        // 迭代器
        iterator begin() { return Base::begin_inorder(); }
        iterator end() { return Base::end_inorder(); }
        const_iterator begin() const { return Base::begin_inorder(); }
        const_iterator end() const { return Base::end_inorder(); }

        // 大小
        std::size_t size() const { return m_size; }

        // 是否为空
        bool empty() const { return m_size == 0; }

        // 清空
        using Base::clear;

        // 查找
        std::optional<std::reference_wrapper<T>> find(const T& value)
        {
            return do_find(value);
        }

        std::optional<std::reference_wrapper<T>> find(const T& value) const
        {
            return do_find(value);
        }

    private:
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

        // 深拷贝辅助函数 
        void deepCopy(const AVLTree& other)
        {
            if (other.m_size == 0)
            {
                m_root = nullptr;
                return;
            }
            NodePtr other_root = other.m_root;
            m_root = this->create_node(other_root->get_val(), static_cast<NodePtr>(other_root)->height());
            std::stack<std::pair<NodePtr, NodePtr>> stack; // other->this
            stack.push({ static_cast<NodePtr>(other.m_root), m_root });
            while (!stack.empty())
            {
                std::pair<NodePtr, NodePtr> top = stack.top();
                NodePtr orig = top.first;
                NodePtr copy = top.second;
                stack.pop();
                if (orig->child(1))
                {
                    NodePtr rc = this->create_node(static_cast<NodePtr>(orig->child(1))->get_val(), static_cast<NodePtr>(orig->child(1))->height());
                    copy->set_child(1, rc);
                    rc->set_parent(copy);
                    stack.push({ static_cast<NodePtr>(orig->child(1)), rc });
                }
                if (orig->child(0))
                {
                    NodePtr lc = this->create_node(static_cast<NodePtr>(orig->child(0))->get_val(), static_cast<NodePtr>(orig->child(0))->height());
                    copy->set_child(0, lc);
                    lc->set_parent(copy);
                    stack.push({ static_cast<NodePtr>(orig->child(0)), lc });
                }
            }
        }
    };
}
