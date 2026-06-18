#include <concepts>
#include <utility>
#include <memory>
#include <cstdint>
#include <cassert>
#include <type_traits>

#include "stk/Math/Monoid.hpp"

template <class Tree> 
concept BinarySearchTree = requires (
	Tree tree,
	typename Tree::value_type value_type,
	typename Tree::node_type node_type,
	typename Tree::node_ptr_type ptr_type
) {
	// node_ptr merge(ptr l, ptr r)
	{ tree.merge(ptr_type, ptr_type) } -> std::same_as<decltype(ptr_type)>;
	
	// pair<node_ptr, node_ptr> split(node_ptr root, ll num_left);
	{ tree.split(ptr_type, std::declval<int64_t>()) } 
		-> std::same_as<typename std::pair<decltype(ptr_type), decltype(ptr_type)>>;
		
	// node_ptr insert(root, idx, node)
	{ tree.insert(ptr_type, value_type, ptr_type) } -> std::same_as<decltype(ptr_type)>;
	
	// node_ptr erase(root, idx)
	{ tree.erase(ptr_type, std::declval<int64_t>()) } -> std::same_as<decltype(ptr_type)>;
};

template <typename T>
class RandomizedBinarySearchTree {
public:
	struct Node;
	using value_type = T;
	using node_type = Node;
	using node_ptr_type = std::shared_ptr<node_type>;
	
	struct Node {
		value_type val;
		node_ptr_type left, right;
		int64_t subtree_size;
		
		constexpr Node() : val(T()), left(nullptr), right(nullptr), subtree_size(0) {}
		explicit Node(value_type x) : val(x), left(nullptr), right(nullptr), subtree_size(1) {}
		~Node() = default;
		
		inline static int64_t size(node_ptr_type node) { return node ? node->subtree_size : 0; }
		inline void update() { subtree_size = 1 + size(left) + size(right); }
	};
	
public:
	static uint64_t xor_shift() {
		static uint64_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
		uint64_t t = (x ^ (x << 11)); x = y; y = z; z = w;
		return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
	}
	
	node_ptr_type merge(node_ptr_type l, node_ptr_type r) {
		if(l == nullptr) return r;
		if(r == nullptr) return l;
		
		if(xor_shift() % (uint64_t)(Node::size(l) + Node::size(r)) < (uint64_t)Node::size(l)) {
			l->right = merge(l->right, r);
			l->update();
			return l;
		}
		else {
			r->left = merge(l, r->left);
			r->update();
			return r;
		}
	}
	
	// split tree into [k elements] and [(size(tree) - k) elements]
	std::pair<node_ptr_type, node_ptr_type> split(node_ptr_type tree, int64_t k) {
		if(tree == nullptr) return std::make_pair(nullptr, nullptr);
		
		if(k <= Node::size(tree->left)) {
			// 左のノードからから k 個切り離す
			auto p = split(tree->left, k);
			// あまりを元の木につなげなおす
			tree->left = p.second;
			// つなぎなおされたノードをアップデート
			tree->update();
			// (切り離した k 個, あまりをつなぎなおした元の木)
			return std::make_pair(p.first, tree);
		}
		else {
			// left 側の個数 + t (一個)だけでは足りないため、right から k-left.size 個必要
			// 右側のノードから k - left.size - 1 個切り離す
			// -1 は、t を最後にプラスするため
			auto p = split(tree->right, k - Node::size(tree->left) - 1);
			// t.left 側を全部使うので、right に切り離した分をつなげる
			tree->right = p.first;
			// つなぎなおされたノードをアップデート
			tree->update();
			// ([left & 今切り離した right 側のうち、k-left.size-1 個], あまり)
			return std::make_pair(tree, p.second);
		}
	}
	
	// k 番目に値を挿入して, ルートを返す
	node_ptr_type insert(node_ptr_type t, int64_t k, node_ptr_type node) {
		std::pair<node_ptr_type, node_ptr_type> p = split(t, k);
		return merge(merge(p.first, node), p.second);
	}
	
	// k 番目の要素を削除, ルートを返す
	node_ptr_type erase(node_ptr_type t, int64_t k) {
		std::pair<node_ptr_type, node_ptr_type> right = split(t, k + 1); // [k][1],[n-k-1]
		std::pair<node_ptr_type, node_ptr_type> left = split(right.first, k); // [k][1]
		return merge(left.first, right.second); // [k][n-k-1]
	}
	
	node_ptr_type get(node_ptr_type t, int64_t k) const {
		if(t == nullptr) return t;
		int64_t sz = Node::size(t->left);
		
		if(sz > k) return get(t->left, k);
		else if(sz < k) return get(t->right, k - sz - 1);
		else return t;
	}
	
	static_assert(BinarySearchTree<RandomizedBinarySearchTree<T>>);
};

template <typename T>
class RandomizedBinarySearchTreeSet : RandomizedBinarySearchTree<T> {
	using node_type = RandomizedBinarySearchTree<T>::Node;
	using node_ptr_type = std::shared_ptr<node_type>;
	node_ptr_type root;
	
public:
	struct Iterator {
		int64_t idx;
		//using container_ptr_type = RandomizedBinarySearchTreeSet<T>*;
		const RandomizedBinarySearchTreeSet<T>* ptr;
		
		Iterator(int64_t _idx, const RandomizedBinarySearchTreeSet<T>* _ptr) : idx(_idx), ptr(_ptr) {}
		bool operator==(const Iterator& other) const { return idx == other.idx && ptr == other.ptr; }
		bool operator!=(const Iterator& other) const { return !(*this==other); }
		void operator++() { assert(idx < node_type::size(ptr->root)); ++idx; }
		void operator--() { assert(0 < idx); --idx; }
		const T& operator*() const { return ptr->get(idx); }
	};
	
	RandomizedBinarySearchTreeSet() : root(nullptr) {}
	
	int64_t size() const { return node_type::size(this->root); }
	Iterator begin() const { return Iterator(0, this); }
	Iterator end() const { return Iterator(size(), this); }
	
	Iterator insert(const T& val) {
		Iterator res = lower_bound(val);
		if(res == end() || *res != val) {
			root = RandomizedBinarySearchTree<T>::insert(root, res.idx, std::make_shared<node_type>(val));
		}
		return res;
	}
	
	void erase(const T& val) {
		Iterator it = lower_bound(val);
		if(it != end() && *it == val) {
			root = RandomizedBinarySearchTree<T>::erase(root, it.idx);
		}
	}
	
	const T& get(int64_t k) const {
		assert(0 <= k && k < size());
		return RandomizedBinarySearchTree<T>::get(root, k)->val;
	}
	
	Iterator lower_bound(const T& val) const {
		int64_t idx = lower_bound(this->root, val);
		return idx == node_type::size(this->root) ? end() : Iterator(idx, this);
	}
	
	Iterator upper_bound(const T& val) const {
		Iterator it = lower_bound(val);
		if(it == end() || *it != val) return it;
		else { ++it; return it; }
	}
	
	int64_t count(const T& val) const {
		Iterator it = lower_bound(val);
		return it != end() && *it == val;
	}
	
private:
	int64_t lower_bound(node_ptr_type tree, const T& val) const {
		if(tree == nullptr) return 0;
		
		if(tree->val < val) 
			return node_type::size(tree->left) + lower_bound(tree->right, val) + 1;
		else
			return lower_bound(tree->left, val);
	}
};
