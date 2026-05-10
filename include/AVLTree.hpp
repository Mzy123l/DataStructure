#pragma once
#include "Tree_BinaryRepresentation.hpp"
#include "Node_BinaryRepresentation.hpp"
#include <memory>
#include <concepts>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <cstdint>


namespace Tree
{
    namespace Inner_AVLTree
    {


        // 定义比较器concept
        template<typename Comp, typename T>
        concept Comparator = requires(Comp comp, const T& a, const T& b)
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
        template <typename T, Tree::Inner_AVLTree::Comparator<T> Comp, typename Allocator>
        friend class ::Tree::AVLTree;

        ~AVLNode() = default;

        // 默认构造
        AVLNode() :Base() {}

        // 拷贝& 移动构造节点
        template <typename U>
            requires std::is_convertible_v<U, T>
        AVLNode(U&& u, int8_t balance_factor = 0) : Base(std::forward<U>(u)), m_balance_factor(balance_factor) {}

        // 原地构造节点
        template <typename... Args>
        AVLNode(std::in_place_t, Args&&... args) : Base(std::in_place, std::forward<Args>(args)...) {}
    public:
        using value_type = T;

        

    private:
        int8_t m_balance_factor = 0;

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
        
        /*
        * Comp(a, b) == true;->a在b前面
        * Comp(a, b) == false;->a不能在b前面
        */
    private:
        // 从基类引入访问节点结构的辅助函数
        using Base::left_child;
        using Base::right_child;
        using Base::set_lchild;
        using Base::set_rchild;
        using Base::set_parent;
        using Base::parent;

        // 从基类引入旋转操作
        using Base::left_rotate;
        using Base::right_rotate;
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
        AVLTree(const Allocator& alloc = Allocator()) :Base(0, NodeAllocator(alloc)) {}

        // 拷贝构造
        AVLTree(const AVLTree& other) :Base(other.m_size, other.m_allocator)
        {
            deepCopy(other);
        }

        // 移动构造
        AVLTree(AVLTree&& other) noexcept :Base(std::move(other)) {}

        // 拷贝赋值运算符
        AVLTree& operator=(const AVLTree& other)
        {
            if (this != &other)
            {
                clear();
                m_size = other.m_size;
                m_allocator = other.m_allocator;
                deepCopy(other);
                
            }
            return *this;
        }

        // 移动赋值运算符
        AVLTree& operator=(AVLTree&& other) noexcept
        {
            if (this != &other)
            {
                Base::operator=(std::move(other));
            }
            return *this;
        }

        // 析构函数
        ~AVLTree() = default;


        // 插入值
        template <typename... Args>
        bool insert(Args&&... args)
        {
            // 1. 创建新节点
            NodePtr new_node = this->create_node(std::forward<Args>(args)...);
            new_node->m_balance_factor = 0;

            // 2. 空树情况
            if (!m_root)
            {
                m_root = new_node;
                ++m_size;
                return true;
            }

            // 3. 查找插入位置并记录路径
            NodePtr current = m_root;
            std::vector<NodePtr> path;  // 记录从根节点到插入位置父节点的路径
            NodePtr parent = nullptr;
            bool inserted_on_left = false;

            Comp comp{};

            while (current)
            {
                path.push_back(current);

                if (comp(new_node->value().value().get(), current->value().value().get()))
                {
                    parent = current;
                    current = left_child(current);
                    inserted_on_left = true;
                }
                else if (comp(current->value().value().get(), new_node->value().value().get()))
                {
                    parent = current;
                    current = right_child(current);
                    inserted_on_left = false;
                }
                else
                {
                    // 值已存在，不插入
                    this->destroy_node(new_node);
                    return false;
                }
            }

            // 4. 链接新节点到父节点
            if (inserted_on_left)
            {
                set_lchild(parent, new_node);
            }
            else
            {
                set_rchild(parent, new_node);
            }
            set_parent(new_node, parent);

            // 5. 更新平衡因子并修复平衡
            // 从父节点开始向上回溯调整
            bool is_left_child = inserted_on_left;

            // 反向遍历路径（从插入点的父节点向上到根节点）
            for (auto it = path.rbegin(); it != path.rend(); ++it)
            {
                NodePtr node = *it;

                // 更新当前节点的平衡因子
                if (is_left_child)
                {
                    node->m_balance_factor -= 1;  // 左子树高度增加
                }
                else
                {
                    node->m_balance_factor += 1;  // 右子树高度增加
                }

                // 检查平衡因子
                if (node->m_balance_factor == 0)
                {
                    // 高度恢复平衡，停止回溯
                    break;
                }
                else if (node->m_balance_factor == 1 || node->m_balance_factor == -1)
                {
                    // 高度增加，继续向上回溯
                    // 确定当前节点是其父节点的左孩子还是右孩子
                    if (it + 1 != path.rend())
                    {
                        NodePtr parent_of_node = *(it + 1);
                        is_left_child = (left_child(parent_of_node) == node);
                    }
                    // 否则已经到达根节点，循环结束
                }
                else if (node->m_balance_factor == 2)
                {
                    // 右子树过高，需要旋转
                    NodePtr y = right_child(node);
                    // 根据AVL树定义，当平衡因子为2时，y不可能为空

                    if (y->m_balance_factor == 1)
                    {
                        // RR 型：左旋
                        left_rotate(m_root, node);
                        node->m_balance_factor = 0;
                        y->m_balance_factor = 0;
                    }
                    else if (y->m_balance_factor == -1)
                    {
                        // RL 型：先右旋再左旋
                        NodePtr z = left_child(y);
                        int8_t bf_z = z->m_balance_factor;
                        right_left_rotate(m_root, node);

                        // 根据z的原平衡因子更新平衡因子
                        if (bf_z == 0)
                        {
                            node->m_balance_factor = 0;
                            y->m_balance_factor = 0;
                        }
                        else if (bf_z == 1)
                        {
                            node->m_balance_factor = -1;
                            y->m_balance_factor = 0;
                        }
                        else if (bf_z == -1)
                        {
                            node->m_balance_factor = 0;
                            y->m_balance_factor = 1;
                        }
                    }
                    break; // 旋转后子树高度恢复，停止回溯
                }
                else if (node->m_balance_factor == -2)
                {
                    // 左子树过高，需要旋转
                    NodePtr y = left_child(node);
                    // 根据AVL树定义，当平衡因子为-2时，y不可能为空

                    if (y->m_balance_factor == -1)
                    {
                        // LL 型：右旋
                        right_rotate(m_root, node);
                        node->m_balance_factor = 0;
                        y->m_balance_factor = 0;
                    }
                    else if (y->m_balance_factor == 1)
                    {
                        // LR 型：先左旋再右旋
                        NodePtr z = right_child(y);
                        int8_t bf_z = z->m_balance_factor;
                        left_right_rotate(m_root, node);

                        // 根据z的原平衡因子更新平衡因子
                        if (bf_z == 0)
                        {
                            node->m_balance_factor = 0;
                            y->m_balance_factor = 0;
                        }
                        else if (bf_z == 1)
                        {
                            node->m_balance_factor = 0;
                            y->m_balance_factor = -1;
                        }
                        else if (bf_z == -1)
                        {
                            node->m_balance_factor = 1;
                            y->m_balance_factor = 0;
                        }
                    }
                    break; // 旋转后子树高度恢复，停止回溯
                }
            }

            ++m_size;
            return true;
        }

        // 删除值
        bool erase(const T& val)
        {
            // 查找要删除的节点
            NodePtr target = m_root;
            NodePtr parent_of_target = nullptr;
            bool target_is_left_child = false;
            Comp comp{};

            while (target)
            {
                if (comp(val, target->value().value().get()))
                {
                    parent_of_target = target;
                    target_is_left_child = true;
                    target = left_child(target);
                }
                else if (comp(target->value().value().get(), val))
                {
                    parent_of_target = target;
                    target_is_left_child = false;
                    target = right_child(target);
                }
                else
                {
                    break; // 找到目标节点
                }
            }

            // 如果没找到，返回false
            if (!target)
            {
                return false;
            }

            // 删除节点
            NodePtr replacement = nullptr;
            NodePtr balance_start = nullptr; // 开始平衡调整的节点

            // 情况1：目标节点没有左孩子
            if (!left_child(target))
            {
                replacement = right_child(target);
                balance_start = parent_of_target;

                // 连接替换节点到父节点
                if (parent_of_target)
                {
                    if (target_is_left_child)
                    {
                        set_lchild(parent_of_target, replacement);
                    }
                    else
                    {
                        set_rchild(parent_of_target, replacement);
                    }
                }
                else
                {
                    // 删除的是根节点
                    m_root = replacement;
                }

                if (replacement)
                {
                    set_parent(replacement, parent_of_target);
                }
            }
            // 情况2：目标节点没有右孩子
            else if (!right_child(target))
            {
                replacement = left_child(target);
                balance_start = parent_of_target;

                // 连接替换节点到父节点
                if (parent_of_target)
                {
                    if (target_is_left_child)
                    {
                        set_lchild(parent_of_target, replacement);
                    }
                    else
                    {
                        set_rchild(parent_of_target, replacement);
                    }
                }
                else
                {
                    // 删除的是根节点
                    m_root = replacement;
                }

                if (replacement)
                {
                    set_parent(replacement, parent_of_target);
                }
            }
            // 情况3：目标节点有两个孩子
            else
            {
                // 找到中序后继（右子树的最左节点）
                NodePtr successor = right_child(target);
                NodePtr successor_parent = target;
                bool successor_is_left_child = false;

                while (left_child(successor))
                {
                    successor_parent = successor;
                    successor = left_child(successor);
                    successor_is_left_child = true;
                }

                // 保存后继的平衡因子
                int8_t successor_bf = successor->m_balance_factor;

                // 获取后继的右孩子（可能为空）
                NodePtr successor_right_child = right_child(successor);

                // 处理后继节点的父节点链接
                if (successor_parent == target)
                {
                    // 后继是目标的直接右孩子
                    set_rchild(target, successor_right_child);
                    if (successor_right_child)
                    {
                        set_parent(successor_right_child, target);
                    }
                    balance_start = successor;
                }
                else
                {
                    // 后继不是目标的直接右孩子
                    set_lchild(successor_parent, successor_right_child);
                    if (successor_right_child)
                    {
                        set_parent(successor_right_child, successor_parent);
                    }
                    balance_start = successor_parent;
                }

                // 用后继替换目标节点
                // 1. 复制目标节点的左右子节点到后继
                set_lchild(successor, left_child(target));
                if (left_child(target))
                {
                    set_parent(left_child(target), successor);
                }

                set_rchild(successor, right_child(target));
                if (right_child(target) && right_child(target) != successor)
                {
                    set_parent(right_child(target), successor);
                }

                // 2. 更新后继的父节点
                set_parent(successor, parent_of_target);

                // 3. 更新父节点对后继的引用
                if (parent_of_target)
                {
                    if (target_is_left_child)
                    {
                        set_lchild(parent_of_target, successor);
                    }
                    else
                    {
                        set_rchild(parent_of_target, successor);
                    }
                }
                else
                {
                    // 目标节点是根节点
                    m_root = successor;
                }

                // 4. 更新后继的平衡因子为目标节点的平衡因子
                successor->m_balance_factor = target->m_balance_factor;

                // 5. 将后继的平衡因子设置为保存的值
                // （因为successor现在在target的位置，但实际的删除发生在successor的原位置）
                // 我们将从successor的原父节点开始调整
            }

            // 销毁目标节点
            this->destroy_node(target);
            --m_size;

            // 从balance_start开始向上调整平衡因子
            // 使用标准的AVL删除平衡调整算法
            NodePtr node = balance_start;
            while (node)
            {
                // 重新计算当前节点的平衡因子
                int left_height = left_child(node) ? left_child(node)->height() : 0;
                int right_height = right_child(node) ? right_child(node)->height() : 0;
                node->m_balance_factor = static_cast<int8_t>(right_height - left_height);

                // 检查是否需要旋转
                if (node->m_balance_factor == 2)
                {
                    // 右子树过高
                    NodePtr y = right_child(node);
                    if (!y) break; // 安全保护，理论上不应该发生

                    int8_t y_bf = y->m_balance_factor;

                    if (y_bf >= 0)
                    {
                        // RR型（y_bf == 0 或 1）
                        left_rotate(m_root, node);

                        if (y_bf == 0)
                        {
                            // 特殊情况：旋转后树高不变
                            node->m_balance_factor = 1;
                            y->m_balance_factor = -1;
                            break; // 不需要继续向上调整
                        }
                        else
                        {
                            // y_bf == 1
                            node->m_balance_factor = 0;
                            y->m_balance_factor = 0;
                            node = parent(y); // 继续向上调整
                        }
                    }
                    else
                    {
                        // RL型（y_bf == -1）
                        NodePtr z = left_child(y);
                        if (!z) break; // 安全保护

                        int8_t z_bf = z->m_balance_factor;
                        right_left_rotate(m_root, node);

                        // 根据z的平衡因子设置新的平衡因子
                        if (z_bf == 0)
                        {
                            node->m_balance_factor = 0;
                            y->m_balance_factor = 0;
                        }
                        else if (z_bf == 1)
                        {
                            node->m_balance_factor = -1;
                            y->m_balance_factor = 0;
                        }
                        else // z_bf == -1
                        {
                            node->m_balance_factor = 0;
                            y->m_balance_factor = 1;
                        }

                        if (z) z->m_balance_factor = 0;
                        node = parent(z); // 从z的父节点继续调整
                    }
                }
                else if (node->m_balance_factor == -2)
                {
                    // 左子树过高
                    NodePtr y = left_child(node);
                    if (!y) break; // 安全保护，理论上不应该发生

                    int8_t y_bf = y->m_balance_factor;

                    if (y_bf <= 0)
                    {
                        // LL型（y_bf == 0 或 -1）
                        right_rotate(m_root, node);

                        if (y_bf == 0)
                        {
                            // 特殊情况：旋转后树高不变
                            node->m_balance_factor = -1;
                            y->m_balance_factor = 1;
                            break; // 不需要继续向上调整
                        }
                        else
                        {
                            // y_bf == -1
                            node->m_balance_factor = 0;
                            y->m_balance_factor = 0;
                            node = parent(y); // 继续向上调整
                        }
                    }
                    else
                    {
                        // LR型（y_bf == 1）
                        NodePtr z = right_child(y);
                        if (!z) break; // 安全保护

                        int8_t z_bf = z->m_balance_factor;
                        left_right_rotate(m_root, node);

                        // 根据z的平衡因子设置新的平衡因子
                        if (z_bf == 0)
                        {
                            node->m_balance_factor = 0;
                            y->m_balance_factor = 0;
                        }
                        else if (z_bf == 1)
                        {
                            node->m_balance_factor = 0;
                            y->m_balance_factor = -1;
                        }
                        else // z_bf == -1
                        {
                            node->m_balance_factor = 1;
                            y->m_balance_factor = 0;
                        }

                        if (z) z->m_balance_factor = 0;
                        node = parent(z); // 从z的父节点继续调整
                    }
                }
                else if (node->m_balance_factor == 0)
                {
                    // 高度减少了，需要继续向上调整
                    node = parent(node);
                }
                else
                {
                    // 平衡因子为1或-1，高度未变，停止调整
                    break;
                }
            }

            return true;
        }
        
        // 迭代器
        iterator begin() { return Base::begin_inorder(); }
        iterator end() {return Base::end_inorder();}
        const_iterator begin() const { return Base::begin_inorder(); }
        const_iterator end() const { return Base::end_inorder(); }

        // 大小
        std::size_t size() const { return m_size; }

        //是否为空
        bool empty() const { return m_size == 0; }
        
        //清空
        using Base::clear;


        // 查找
        std::optional<std::reference_wrapper<T>> find(const T& value)
        {
            return do_find(value);
        }

        const std::optional<std::reference_wrapper<T>> find(const T& value) const
        {
            return do_find(value);
        }
       

    private:
        // 查找辅助函数
        std::optional<std::reference_wrapper<T>> do_find(const T& value) const
        {
            NodePtr current = m_root;
            Comp comp{};
            while (current)
            {
                if (comp(value, current->value().value().get()))
                {
                    current = left_child(current);
                }
                else if (comp(current->value().value().get(), value))
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
            m_root = this->create_node(other_root->value().value().get(), static_cast<NodePtr>(other_root)->m_balance_factor);
            std::stack<std::pair<NodePtr, NodePtr>> stack;// other->this
            stack.push({ static_cast<NodePtr>(other.m_root), m_root });
            while (!stack.empty())
            {
                std::pair<NodePtr, NodePtr> top = stack.top();
                NodePtr orig = top.first;
                NodePtr copy = top.second;
                stack.pop();
                if (orig->child(1))
                {
                    NodePtr rc = this->create_node(orig->child(1)->value().value().get(), static_cast<NodePtr>(orig->child(1))->m_balance_factor);
                    copy->set_child(1, rc);
                    rc->set_parent(copy);
                    stack.push({ static_cast<NodePtr>(orig->child(1)), rc });
                }
                if (orig->child(0))
                {
                    NodePtr lc = this->create_node(orig->child(0)->value().value().get(), static_cast<NodePtr>(orig->child(0))->m_balance_factor);
                    copy->set_child(0, lc);
                    lc->set_parent(copy);
                    stack.push({ static_cast<NodePtr>(orig->child(0)), lc });
                }
            }
        }
    };
}