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
        concept Comparator = requires(Comp comp, const T& a, const T& b)
        {
            { comp(a, b) } -> std::convertible_to<bool>;
            { comp(b, a) } -> std::convertible_to<bool>;
           
        };
    }

    template <typename T, Inner_RBTree::Comparator<T> Comp, typename Allocator>
    class RBTree;
}


namespace TreeNode
{
    
    /*
    * 红黑树节点
    * WARNING: 利用父指针存储颜色信息，依赖内存对齐!!!
    */
    enum class RBColor :std::uintptr_t
    {
        BLACK = 0,
        RED = 1
    };
    template <typename T>
    class RBNode :public Node_BinaryRepresentation<T>
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
    public:
        // 默认构造
        RBNode() :Base() {}

        // 拷贝& 移动构造节点
        template <typename U>
            requires std::is_convertible_v<U, T>
        RBNode(U&& u, int8_t balance_factor = 0) : Base(std::forward<U>(u)) {}

        // 原地构造节点
        template <typename... Args>
        RBNode(std::in_place_t, Args&&... args) : Base(std::in_place, std::forward<Args>(args)...) {}

        // 父指针存储颜色
        // 获取父节点指针
        NodePtr parent() 
        { 
            return reinterpret_cast<NodePtr>(reinterpret_cast<std::uintptr_t>(m_parent) &~(std::uintptr_t{ 1 }));
        }

        // 设置父节点指针(保留原有颜色)
        void set_parent(NodePtr ptr)
        {
            m_parent = reinterpret_cast<NodePtr>
                ((reinterpret_cast<std::uintptr_t>(ptr) &~(std::uintptr_t{ 1 })) | (reinterpret_cast<std::uintptr_t>(m_parent) & 1));
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

        bool is_red()const { return color() == TreeNode::RBColor::RED; }
        bool is_black()const { return color() == TreeNode::RBColor::BLACK; }


    };
}
namespace Tree
{
 
    /*
     * 红黑树节点
     * WARNING: 利用父指针存储颜色信息，依赖内存对齐!!!
     */
    template <typename T, Inner_RBTree::Comparator<T> Comp = std::less<T>, typename Allocator = std::allocator<TreeNode::RBNode<T>>>
    class RBTree :public Tree_BinaryRepresentation<T, RBTree<T, Comp, Allocator>, TreeNode::RBNode<T>, 
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

    public:
        // 获取分配器 - 返回用户传入的类型
        Allocator get_allocator() const noexcept
        {
            // 从节点分配器转换回用户传入的类型
            return Allocator(m_allocator);
        }

        // 默认构造
        RBTree(const Allocator& alloc = Allocator()) :Base(0, NodeAllocator(alloc)) {}

        // 拷贝
        // 使用基类深拷贝函数，直接拷贝带颜色的父指针信息
        // 拷贝构造函数
        RBTree(const RBTree& other) : Base(other) {}

        // 拷贝赋值运算符
        RBTree& operator=(const RBTree& other)
        {
            return static_cast<RBTree&>(Base::operator=(other));
        }

        // 移动构造函数
        RBTree(RBTree&& other) noexcept :Base(other) {}

        // 移动赋值运算符
        RBTree& operator=(RBTree&& other) noexcept
        {
            return static_cast<RBTree&>(Base::operator=(std::move(other)));
        }

        // 析构函数
        ~RBTree() = default;

        // 迭代器
        iterator begin() { return Base::begin_inorder(); }
        iterator end() { return Base::end_inorder(); }
        const_iterator begin() const { return Base::begin_inorder(); }
        const_iterator end() const { return Base::end_inorder(); }

        // 大小
        std::size_t size() const { return m_size; }

        //是否为空
        bool empty() const { return m_size == 0; }

        //清空
        using Base::clear;


       

    public:
        // 插入值（完美转发构造）
        template <typename... Args>
        bool insert(Args&&... args)
        {
            // 1. 创建新节点（初始为红色）
            NodePtr new_node = this->create_node(std::forward<Args>(args)...);
            new_node->set_color(TreeNode::RBColor::RED);
            new_node->set_parent(nullptr);
            new_node->set_child(0, nullptr); // 左孩子
            new_node->set_child(1, nullptr); // 右孩子

            // 2. 二叉搜索树插入
            NodePtr parent = nullptr;
            NodePtr current = m_root;
            while (current)
            {
                parent = current;
                if (Comp()(new_node->value().value().get(), current->value().value().get()))
                {
                    current = left_child(current);
                }
                else if(Comp()(current->value().value().get(), new_node->value().value().get()))
                {
                    current = right_child(current);
                }
                else
                {
                    return false;
                }
            }

            new_node->set_parent(parent);
            if (!parent)
            {
                m_root = new_node;
            }
            else if (Comp()(new_node->value().value().get(), parent->value().value().get()))
            {
                set_lchild(parent, new_node);
            }
            else
            {
                set_rchild(parent, new_node);
            }

            ++m_size;

            // 3. 修复红黑树性质
            insert_fixup(new_node);

            return true;
        }

        // 按值删除
        bool erase(const T& val)
        {
            // 1. 查找要删除的节点
            NodePtr z = m_root;
            NodePtr z_parent = nullptr;

            while (z)
            {
                if (Comp()(val, z->value().value().get()))
                {
                    z_parent = z;
                    z = left_child(z);
                }
                else if (Comp()(z->value().value().get(), val))
                {
                    z_parent = z;
                    z = right_child(z);
                }
                else
                {
                    break; // 找到要删除的节点
                }
            }

            if (!z)
            {
                return false; // 未找到要删除的节点
            }

            // 2. 记录原始颜色和要修复的节点
            TreeNode::RBColor original_color = z->color();
            NodePtr x = nullptr; // 用于修复的节点
            NodePtr x_parent = nullptr; // x 的父节点
            NodePtr y = nullptr; // 实际被删除或移动的节点
            NodePtr y_original_parent = z_parent; // y 的原始父节点

            // 3. 根据子节点数量处理删除
            if (!left_child(z))
            {
                // 情况1：没有左子节点
                y = z;
                x = right_child(z);
                x_parent = y_original_parent;

                transplant(z, right_child(z));

                if (!x)
                {
                    x = y_original_parent; // 如果 x 是 nullptr，用 y 的父节点作为修复起点
                }
            }
            else if (!right_child(z))
            {
                // 情况2：没有右子节点
                y = z;
                x = left_child(z);
                x_parent = y_original_parent;

                transplant(z, left_child(z));

                if (!x)
                {
                    x = y_original_parent;
                }
            }
            else
            {
                // 情况3：有两个子节点
                y = minimum(right_child(z)); // 找到后继节点
                original_color = y->color();
                x = right_child(y);
                y_original_parent = parent(y);

                if (parent(y) != z)
                {
                    // y 不是 z 的直接右子节点
                    transplant(y, right_child(y));
                    set_rchild(y, right_child(z));
                    if (right_child(z))
                    {
                        set_parent(right_child(z), y);
                    }
                    x_parent = y_original_parent; // x 的父节点是 y 的原始父节点
                }
                else
                {
                    // y 是 z 的直接右子节点
                    if (x)
                    {
                        set_parent(x, y);
                    }
                    x_parent = y; // x 的父节点是 y
                }

                // 用 y 替换 z
                transplant(z, y);
                set_lchild(y, left_child(z));
                if (left_child(z))
                {
                    set_parent(left_child(z), y);
                }

                // 复制 z 的颜色到 y（保持 y 在原来位置的颜色）
                y->set_color(z->color());
            }

            // 4. 如果删除的是黑色节点，需要修复红黑树性质
            if (original_color == TreeNode::RBColor::BLACK)
            {
                // 内联的删除修复逻辑
                NodePtr w = nullptr; // 兄弟节点

                // 修复循环
                while (x != m_root && (!x || x->is_black()))
                {
                    // 获取 x 的父节点
                    NodePtr xp = x ? parent(x) : x_parent;
                    if (!xp)
                    {
                        // 如果 x 的父节点也为空，跳出循环
                        break;
                    }

                    if (x == left_child(xp))
                    {
                        // x 是左子节点的情况
                        w = right_child(xp);

                        if (w && w->is_red())
                        {
                            // 情况1：兄弟节点为红色
                            w->set_color(TreeNode::RBColor::BLACK);
                            xp->set_color(TreeNode::RBColor::RED);
                            left_rotate(m_root, xp);
                            w = right_child(xp);
                        }

                        if (!w)
                        {
                            // 如果没有兄弟节点，向上回溯
                            x = xp;
                            x_parent = parent(xp);
                            continue;
                        }

                        bool left_black = !left_child(w) || left_child(w)->is_black();
                        bool right_black = !right_child(w) || right_child(w)->is_black();

                        if (left_black && right_black)
                        {
                            // 情况2：兄弟节点及其子节点均为黑色
                            w->set_color(TreeNode::RBColor::RED);
                            x = xp;
                            x_parent = parent(xp);
                        }
                        else
                        {
                            if (right_black)
                            {
                                // 情况3：兄弟节点左红右黑
                                if (left_child(w))
                                {
                                    left_child(w)->set_color(TreeNode::RBColor::BLACK);
                                }
                                w->set_color(TreeNode::RBColor::RED);
                                right_rotate(m_root, w);
                                w = right_child(xp);
                            }

                            // 情况4：兄弟节点右红
                            if (w)
                            {
                                w->set_color(xp->color());
                            }
                            xp->set_color(TreeNode::RBColor::BLACK);
                            if (right_child(w))
                            {
                                right_child(w)->set_color(TreeNode::RBColor::BLACK);
                            }
                            left_rotate(m_root, xp);
                            x = m_root;
                            break;
                        }
                    }
                    else
                    {
                        // 对称情况：x 是右子节点
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
                                if (right_child(w))
                                {
                                    right_child(w)->set_color(TreeNode::RBColor::BLACK);
                                }
                                w->set_color(TreeNode::RBColor::RED);
                                left_rotate(m_root, w);
                                w = left_child(xp);
                            }

                            if (w)
                            {
                                w->set_color(xp->color());
                            }
                            xp->set_color(TreeNode::RBColor::BLACK);
                            if (left_child(w))
                            {
                                left_child(w)->set_color(TreeNode::RBColor::BLACK);
                            }
                            right_rotate(m_root, xp);
                            x = m_root;
                            break;
                        }
                    }
                }

                // 确保 x 为黑色
                if (x)
                {
                    x->set_color(TreeNode::RBColor::BLACK);
                }
            }

            // 5. 释放被删除节点的内存
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
        // 辅助函数：获取最小节点
        NodePtr minimum(NodePtr node) const
        {
            while (left_child(node))
            {
                node = left_child(node);
            }
            return node;
        }

        // 辅助函数：替换子树
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
            if (v)
            {
                set_parent(v, parent(u));
            }
        }

        // 插入修复
        void insert_fixup(NodePtr z)
        {
            while (z->parent() && z->parent()->is_red())
            {
                if (z->parent() == left_child(parent(parent(z))))
                {
                    NodePtr uncle = right_child(parent(parent(z)));
                    if (uncle && uncle->is_red())
                    {
                        // 情况1：叔叔节点为红色
                        z->parent()->set_color(TreeNode::RBColor::BLACK);
                        uncle->set_color(TreeNode::RBColor::BLACK);
                        parent(parent(z))->set_color(TreeNode::RBColor::RED);
                        z = parent(parent(z));
                    }
                    else
                    {
                        if (z == right_child(parent(z)))
                        {
                            // 情况2：叔叔节点为黑色，且当前节点是右孩子
                            z = parent(z);
                            left_rotate(m_root, z);
                        }
                        // 情况3：叔叔节点为黑色，且当前节点是左孩子
                        z->parent()->set_color(TreeNode::RBColor::BLACK);
                        parent(parent(z))->set_color(TreeNode::RBColor::RED);
                        right_rotate(m_root, parent(parent(z)));
                    }
                }
                else
                {
                    // 对称情况（父节点是祖父节点的右孩子）
                    NodePtr uncle = left_child(parent(parent(z)));
                    if (uncle && uncle->is_red())
                    {
                        z->parent()->set_color(TreeNode::RBColor::BLACK);
                        uncle->set_color(TreeNode::RBColor::BLACK);
                        parent(parent(z))->set_color(TreeNode::RBColor::RED);
                        z = parent(parent(z));
                    }
                    else
                    {
                        if (z == left_child(parent(z)))
                        {
                            z = parent(z);
                            right_rotate(m_root, z);
                        }
                        z->parent()->set_color(TreeNode::RBColor::BLACK);
                        parent(parent(z))->set_color(TreeNode::RBColor::RED);
                        left_rotate(m_root, parent(parent(z)));
                    }
                }
            }
            // 确保根为黑色
            if (m_root) m_root->set_color(TreeNode::RBColor::BLACK);
        }

       
 
        // 查找辅助函数
        std::optional<std::reference_wrapper<T>> do_find(const T& value) const
        {
            NodePtr current = m_root;
            while (current)
            {
                if (Comp()(value, current->value().value().get()))
                {
                    current = left_child(current);
                }
                else if (Comp()(current->value().value().get(), value))
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