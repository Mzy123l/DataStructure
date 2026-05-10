# 数据结构库 (Data Structures Library)

一个现代 C++ 的头文件库，实现了多种常用的数据结构。所有实现都是模板化的，支持自定义分配器，并遵循 RAII 原则。

## 特性

- 🚀 纯头文件实现，零依赖
- 📦 支持 C++20 标准
- 🧠 部分迭代器支持
- 🔧 可自定义比较器和分配器
- 🛡️ 异常安全保证
- 🧪 经过充分测试

## 数据结构列表

### 树结构 (Tree Structures)

| 数据结构 | 命名空间 | 头文件 | 描述 |
|---------|---------|--------|------|
| `AVLTree` | `Tree` | `AVLTree.hpp` | 自平衡二叉搜索树（AVL树），保证 O(log n) 的插入、删除和查找 |
| `RBTree` | `Tree` | `RBTree.hpp` | 自平衡二叉搜索树（红黑树），利用父指针低比特位存储颜色信息 |
| `BPlusTree` | `Tree` | `BPlusTree.hpp` | B+树，支持高效的范围查询，叶节点构成链表 |
| `Trie` | `Tree` | `Trie.hpp` | 前缀树/字典树，支持大小写字母和数字，用于字符串检索 |

### 链表 (Linked Lists)

| 数据结构 | 命名空间 | 头文件 | 描述 |
|---------|---------|--------|------|
| `singlyLinkedList` | `DateStructure` | `singlyLinkedList.hpp` | 单向链表，支持迭代器和通用算法 |
| `doublyLinkedList` | `DoublyLinkedList` | `doublyLinkedList.hpp` | 双向循环链表，使用哨兵节点，支持双向迭代 |

### 并查集 (Union-Find)

| 数据结构 | 命名空间 | 头文件 | 描述 |
|---------|---------|--------|------|
| `basicUnionFind` | `Forest` | `UnionFind.hpp` | 基础并查集，使用整数索引，支持按秩合并和路径压缩 |
| `UnionFind` | `Forest` | `UnionFind.hpp` | 广义并查集，支持任意可哈希类型，包装了基础版本 |

## 快速开始

### 安装

只需将头文件复制到你的项目中即可使用：

```bash
git clone <repository-url>
cp -r include/ /your/project/include/
```

### 基本使用

```cpp
#include "AVLTree.hpp"
#include "RBTree.hpp"
#include "singlyLinkedList.hpp"
#include "UnionFind.hpp"
#include <iostream>

int main() 
{
    // 1. 使用 AVL 树
    Tree::AVLTree<int> avl;
    avl.insert(5);
    avl.insert(3);
    avl.insert(7);
    
    auto result = avl.find(3);
    if (result) 
    {
        std::cout << "Found: " << *result << std::endl;
    }
    
    // 2. 使用红黑树
    Tree::RBTree<std::string> rb;
    rb.insert("hello");
    rb.insert("world");
    
    // 3. 使用单向链表
    DateStructure::singlyLinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_front(0);
    
    for (auto it = list.begin(); it != list.end(); ++it) 
    {
        std::cout << *it << " ";
    }
    
    // 4. 使用并查集
    Forest::UnionFind<std::string> uf;
    uf.add_element("Alice");
    uf.add_element("Bob");
    uf.add_element("Charlie");
    
    uf.unite("Alice", "Bob");
    std::cout << "Are Alice and Bob connected? " 
              << (uf.connected("Alice", "Bob") ? "Yes" : "No") << std::endl;
    
    return 0;
}
```

## 详细用法

### AVLTree 和 RBTree

```cpp
#include "AVLTree.hpp"
#include "RBTree.hpp"

// 自定义比较器
struct CaseInsensitiveCompare 
{
    bool operator()(const std::string& a, const std::string& b) const 
    {
        return std::lexicographical_compare(
            a.begin(), a.end(), b.begin(), b.end(),
            char c1, char c2 { return std::tolower(c1) < std::tolower(c2); }
        );
    }
};

int main() 
{
    // 默认构造
    Tree::AVLTree<int> avl;
    
    // 自定义比较器
    Tree::RBTree<std::string, CaseInsensitiveCompare> rb;
    
    // 插入元素
    for (int i = 0; i < 10; ++i) 
    {
        avl.insert(i);
        rb.insert("Key" + std::to_string(i));
    }
    
    // 范围遍历
    for (const auto& val : avl) 
    {
        std::cout << val << " ";
    }
    
    // 查找
    if (auto found = rb.find("key5")) 
    {
        std::cout << "Found: " << *found << std::endl;
    }
    
    // 删除
    avl.erase(5);
    rb.erase("Key5");
    
    // 清空
    avl.clear();
    
    return 0;
}
```

### BPlusTree

```cpp
#include "BPlusTree.hpp"

int main() 
{
    // 创建 B+树，指定阶数（默认为3）
    Tree::BPlusTree<int, std::string> bpt(4);
    
    // 插入键值对
    bpt.insert(1, "One");
    bpt.insert(2, "Two");
    bpt.insert(3, "Three");
    bpt.insert(4, "Four");
    
    // 查找
    if (auto* value = bpt.find(2)) 
    {
        std::cout << "Value: " << *value << std::endl;
    }
    
    // 删除
    bpt.erase(3);
    
    // 获取大小
    std::cout << "Size: " << bpt.size() << std::endl;
    
    return 0;
}
```

### Trie (前缀树)

```cpp
#include "Trie.hpp"

int main() 
{
    Tree::Trie<char> trie;
    
    // 插入单词
    trie.insert("apple");
    trie.insert("app");
    trie.insert("banana");
    trie.insert("band");
    
    // 搜索
    std::cout << "Contains 'app': " << trie.search("app") << std::endl;      // true
    std::cout << "Contains 'appl': " << trie.search("appl") << std::endl;    // false
    
    // 前缀搜索
    std::cout << "Has prefix 'ban': " << trie.startsWith("ban") << std::endl; // true
    
    // 获取所有以特定前缀开头的单词
    auto words = trie.get_words_with_prefix("app");
    for (const auto& word : words) 
    {
        std::cout << word << " ";
    }
    // 输出: app apple
    
    // 删除单词
    trie.remove("app");
    
    // 单词计数
    std::cout << "Word count: " << trie.words_count() << std::endl;
    
    return 0;
}
```

### 链表

```cpp
#include "singlyLinkedList.hpp"
#include "doublyLinkedList.hpp"

int main() 
{
    // 单向链表
    DateStructure::singlyLinkedList<int> sll;
    sll.push_back(1);
    sll.push_back(2);
    sll.push_front(0);
    
    // 遍历
    for (auto it = sll.begin(); it != sll.end(); ++it) 
    {
        std::cout << *it << " ";
    }
    
    // 查找
    auto it = sll.find(2);
    if (it != sll.end()) 
    {
        std::cout << "Found: " << *it << std::endl;
    }
    
    // 双向链表
    DoublyLinkedList::doublyLinkedList<std::string> dll;
    dll.push_back("world");
    dll.push_front("hello");
    dll.emplace_back("!");
    
    // 双向遍历
    for (auto it = dll.begin(); it != dll.end(); ++it) 
    {
        std::cout << *it << " ";
    }
    
    return 0;
}
```

### 并查集

```cpp
#include "UnionFind.hpp"

int main() 
{
    // 基础并查集（整数索引）
    Forest::basicUnionFind uf(5);  // 5个元素：0,1,2,3,4
    
    uf.unite(0, 1);
    uf.unite(1, 2);
    uf.unite(3, 4);
    
    std::cout << "Connected components: " << uf.count() << std::endl;  // 2
    std::cout << "Are 0 and 2 connected? " << uf.connected(0, 2) << std::endl;  // true
    
    // 广义并查集（任意可哈希类型）
    Forest::UnionFind<std::string> uf2;
    uf2.add_element("Alice");
    uf2.add_element("Bob");
    uf2.add_element("Charlie");
    uf2.add_element("David");
    
    uf2.unite("Alice", "Bob");
    uf2.unite("Charlie", "David");
    
    // 查找代表元素
    auto& rep = uf2.find("Bob");  // 返回"Alice"或"Bob"中的一个
    
    // 批量添加
    uf.add_elements(10);  // 添加10个新元素
    
    return 0;
}
```

## 迭代器支持

部分容器提供迭代器支持， 建议仅用for范围循环和遍历迭代，暂不支持标准库算法：

```cpp
#include "AVLTree.hpp"
#include <algorithm>
#include <vector>

int main() 
{
    Tree::AVLTree<int> tree = {5, 3, 7, 1, 4, 6, 8};
    
    
    // 使用范围for循环
    for (const auto& val : tree) 
    {
        std::cout << val << " ";
    }
    
    return 0;
}
```

## 自定义分配器

所有容器都支持自定义分配器：

```cpp
#include "RBTree.hpp"
#include <memory_resource>  // C++17 多态分配器

int main() 
{
    // 使用多态分配器
    std::pmr::unsynchronized_pool_resource pool;
    Tree::RBTree<int, std::less<int>, std::pmr::polymorphic_allocator<int>> 
        rb(std::pmr::polymorphic_allocator<int>{&pool});
    
    rb.insert(1);
    rb.insert(2);
    rb.insert(3);
    
    // 使用自定义分配器
    MyCustomAllocator<int> alloc;
    Tree::AVLTree<int, std::less<int>, MyCustomAllocator<int>> avl(alloc);
    
    return 0;
}
```

## 编译要求

- **C++标准**: C++20 或更高版本
- **编译器**: 支持 C++20 的编译器（GCC 7+, Clang 5+, MSVC 2017+）
- **构建系统**: 任何支持 C++20 的构建系统

### 使用 CMake

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 将头文件目录添加到包含路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(my_app main.cpp)
```

### 直接编译

```bash
g++ -std=c++20 -I./include main.cpp -o my_app
```

## 性能特点

- **AVLTree**: 严格的平衡，查找性能最优，适合读多写少的场景
- **RBTree**: 相对宽松的平衡，插入删除性能较好，综合性能优秀
- **BPlusTree**: 适合磁盘存储和范围查询，数据库索引的经典结构
- **Trie**: 字符串检索专用，前缀匹配效率极高
- **链表**: O(1) 的插入删除（已知位置），适合频繁修改的场景
- **并查集**: 近乎 O(1) 的合并和查找操作，适合连通性问题

## 许可证

本项目采用 MIT 许可证。详见 LICENSE 文件。

## 贡献

欢迎提交 Issue 和 Pull Request。请确保代码符合项目代码风格，并添加相应的测试。

## 作者
// 正在学习数据结构中，这是一个很棒的项目！
由Sonder开发维护。
