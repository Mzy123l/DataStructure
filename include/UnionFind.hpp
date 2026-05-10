#pragma once
#include <unordered_map>
#include <vector>
#include <functional>
#include <stdexcept>

namespace Forest
{
	/// <summary> 并查集
	/// 按秩合并 + 路径优化
	/// </summary> 狭义并查集
	template <typename Allocator = std::allocator<std::size_t>>
	class basicUnionFind
	{
	private:
		std::vector<std::size_t, Allocator> parent;  // 父节点数组
		std::vector<std::size_t, Allocator> rank;    // 秩数组（树的深度/大小）
		std::size_t tree_count;                // 连通分量数量，即树的数量
	public:
		// 默认构造函数
		basicUnionFind(std::size_t n = 0) :tree_count(n),  rank(n ,0)
		{
			parent.reserve(n);
			for (std::size_t i = 0; i < n; ++i)
			{
				parent.push_back(i);
			}
		}
		// 禁止拷贝
		basicUnionFind(const basicUnionFind&) = delete;
		basicUnionFind& operator= (const basicUnionFind&) = delete;

		// 移动构造
		basicUnionFind(basicUnionFind&& other) noexcept:parent(std::move(other.parent)), rank(std::move(other.rank)), tree_count(other.tree_count)
		{
			other.tree_count = 0;
		}

		// 移动赋值
		basicUnionFind& operator=(basicUnionFind&& other) noexcept
		{
			if (this != &other)
			{
				parent = std::move(other.parent);
				rank = std::move(other.rank);
				tree_count = other.tree_count;
				other.tree_count = 0;
			}
			return *this;
		}

		~basicUnionFind() = default;
		
		// 查找根节点，并进行路径压缩
		std::size_t find(std::size_t target)
		{
			/* 此处可递归，仅遍历一次，但存在爆栈风险
			// 路径压缩优化
			if (parent[target] != target)
			{
				// 递归压缩
				parent[target] = find(parent[target]);
			}
			return parent[target];
			*/

			// 寻找根节点
			std::size_t root = find_root(target);
			if(root == parent.size())
				throw std::out_of_range("Element not found in UnionFind");
			// 路径压缩, 将路径上所有节点直接指向根节点
			path_compression(target, root);

			return root;
		}

		// 合并操作（按秩合并）
		// 如果已经在同一集合，返回false
		bool unite(std::size_t x, std::size_t y)
		{
			std::size_t rootX = find(x);
			std::size_t rootY = find(y);

			if (rootX == rootY)
				return false;
			merge_by_rank(rootX, rootY);
			return true;
		}

	
		// 判断两个元素是否在同一集合，会自动进行路径压缩
		bool connected(std::size_t x, std::size_t y) 
		{
			return find(x) == find(y);
		}

		// 获取集合数量
		std::size_t count() const 
		{
			return tree_count;
		}
		// 元素数量
		std::size_t elementCount() const
		{
			return parent.size();
		}
		// 添加新元素,返回添加的元素下标
		std::size_t add_element()
		{
			std::size_t size = parent.size();
			parent.push_back(size);
			rank.push_back(0);
			++tree_count;
			return size;
		}
		// 批量增加
		void add_elements(std::size_t count)
		{
			std::size_t start = parent.size();
			parent.resize(start + count);
			rank.resize(start + count, 0);
			for (std::size_t i = start; i < start + count; ++i)
			{
				parent[i] = i;
			}
			tree_count += count;
		}
		// 获取根节点集合
		std::vector<std::size_t> get_roots() const
		{
			std::vector<std::size_t> roots;
			roots.reserve(tree_count);
			for (std::size_t i = 0; i < parent.size(); ++i)
			{
				if (parent[i] == i)
					roots.push_back(i);
			}
			return roots;
		}
		// 容量管理
		void reserve(std::size_t capacity)
		{
			parent.reserve(capacity);
			rank.reserve(capacity);
		}
		// 自适应大小
		void shrink_to_fit()
		{
			parent.shrink_to_fit();
			rank.shrink_to_fit();
		}

		// 清空并查集
		void clear()
		{
			parent.clear();
			rank.clear();
			tree_count = 0;
		}
	private:
		// 查找根节点
		std::size_t find_root(std::size_t target)
		{
			if (target >= parent.size())
				return parent.size();
			std::size_t root = target;
			while (parent[root] != root)
			{
				root = parent[root];
			}
			return root;
		}

		// 路径压缩, 将路径上所有节点直接指向根节点
		void path_compression(std::size_t target, std::size_t root)
		{
			while (target != root)
			{
				std::size_t next = parent[target];
				parent[target] = root;
				target = next;
			}
		}

		// 按秩合并
		void merge_by_rank(std::size_t rootX, std::size_t rootY)
		{
			if (rank[rootX] < rank[rootY])
			{
				// 将秩较小的树合并到秩较大的树上
				parent[rootX] = rootY;
			}
			else if (rank[rootX] > rank[rootY]) 
			{
				parent[rootY] = rootX;
			}
			else 
			{
				// 秩相等时，任意选择，但秩需要加1
				parent[rootY] = rootX;
				++rank[rootX];
			}
			--tree_count;
		}
	};

	// 定义可哈希概念
	template<typename T>
	concept Hashable = requires(T t) 
	{
		{ std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
	};

	/*
	* <summary> 广义并查集
	* @brief 带有数据域的并查集，要求数据类型T必须可哈希
	* 必须特化std::hash<T>
	*/
	template<Hashable T, typename Allocator_size = std::allocator<std::size_t>, typename Allocator = std::allocator<std::pair<const T, std::size_t>>>
	class UnionFind  
	{
	private:
		basicUnionFind<Allocator_size> uf; // 基础并查集
		std::vector<T, typename std::allocator_traits<Allocator>::template rebind_alloc<T>> idToValue; // id到值的转换 idToValue[id]->val
		std::unordered_map<T, std::size_t, std::hash<T>, std::equal_to<T>, Allocator> valueToId; // 值到id的转换  valueToId[val]->id

	public:
		// 默认构造函数
		UnionFind() :uf(0) ,idToValue(), valueToId(){}

		// 禁用拷贝构造函数
		UnionFind(const UnionFind&) = delete;
		UnionFind& operator=(const UnionFind&) = delete;
		
		// 移动构造函数
		UnionFind(UnionFind&& other) noexcept :uf(std::move(other.uf)), idToValue(std::move(other.idToValue)), valueToId(std::move(other.valueToId)) {}

		// 移动赋值
		UnionFind& operator=(UnionFind&& other)	 noexcept
		{
			if (this != &other)
			{
				uf = std::move(other.uf);
				idToValue = std::move(other.idToValue);
				valueToId = std::move(other.valueToId);
			}
			return *this;
		}

		~UnionFind() = default;

		// 添加新元素, 若已存在，返回false
		template<typename... Args>
		bool add_element(Args... args)
		{
			T new_element(std::forward<Args>(args)...);
			auto it = valueToId.find(new_element);
			if (it == valueToId.end())
			{
				std::size_t id = uf.add_element();
				idToValue.push_back(std::move(new_element));
				// 使用 emplace 避免额外的默认构造
				valueToId.emplace(idToValue.back(), id);
				return true;
			}
			return false;
		}

		// 为字符串类型添加重载（特化）
		bool add_element(const char* str) requires (std::is_same_v<T, std::string>)
		{
			return add_element(std::string(str));
		}

		// 查找根节点
		const T& find(const T& val)
		{
			auto it = valueToId.find(val);
			if (it == valueToId.end()) 
			{
				throw std::out_of_range("Element not found in UnionFind");
			}
			std::size_t root_id = uf.find(it->second);
			return idToValue[root_id];
		}

		// 合并操作（按秩合并）
		// 如果已经在同一集合，返回false
		bool unite(const T& x, const T& y)
		{
			return uf.unite(valueToId[x], valueToId[y]);
		}


		// 判断两个元素是否在同一集合，会自动进行路径压缩
		bool connected(const T& x, const T& y)
		{
			return uf.find(valueToId[x]) == uf.find(valueToId[y]);
		}

		// 获取集合数量
		std::size_t count() const
		{
			return uf.count();
		}

		// 元素数量
		std::size_t elementCount() const
		{
			return uf.elementCount();
		}

		
		// 获取根节点集合
		std::vector<T> get_roots() const
		{
			std::vector<std::size_t> roots_id = uf.get_roots();//此处返回值优化，无需使用移动构造
			std::vector<T> roots_value;
			std::size_t size = roots_id.size();
			roots_value.reserve(size);
			for (std::size_t i = 0; i < size; ++i)
			{
				roots_value.push_back(idToValue[roots_id[i]]);
			}
			return roots_value;
		}

		// 容量管理
		void reserve(std::size_t capacity)
		{
			uf.reserve(capacity);
			idToValue.reserve(capacity);
			valueToId.reserve(capacity);
		}
		// 自适应大小
		void shrink_to_fit()
		{
			uf.shrink_to_fit();
			idToValue.shrink_to_fit();
			
		}

	
		// 清空并查集
		void clear()
		{
			uf.clear();
			valueToId.clear();
			idToValue.clear();
		}
	};
}
