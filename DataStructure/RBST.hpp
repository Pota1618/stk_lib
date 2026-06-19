#include <concepts>
#include <utility>
#include <memory>
#include <cstdint>
#include <cassert>
#include <type_traits>

#include "stk/Math/Monoid.hpp"

template <class Node>
concept BinaryTreeNodeConcept = 
requires (Node node, typename Node::value_type value_type ) {
	requires std::same_as<decltype(node.val), decltype(value_type)>;
	requires std::same_as<decltype(node.left), std::shared_ptr<decltype(node)>>;
	requires std::same_as<decltype(node.right), std::shared_ptr<decltype(node)>>;
	requires std::same_as<decltype(node.subtree_size), int64_t>;
	{ node.update() } -> std::same_as<void>;
};

template <typename T>
struct BinaryTreeNodeBase {
	using value_type = T;
	using node_ptr_type = std::shared_ptr<BinaryTreeNodeBase>;
	value_type val;
	node_ptr_type left, right;
	int64_t subtree_size;
	
	constexpr BinaryTreeNodeBase() : val(T()), left(nullptr), right(nullptr), subtree_size(0) {}
	explicit BinaryTreeNodeBase(const value_type& x) : val(x), left(nullptr), right(nullptr), subtree_size(1) {}
	~BinaryTreeNodeBase() = default;
	
	inline static int64_t size(node_ptr_type node) { return node ? node->subtree_size : 0; }
	inline void update() { subtree_size = 1 + size(left) + size(right); }
};

template <MapMonoid MM>
struct LazyReversibleBinaryTreeNode {
	using node_ptr_type = std::shared_ptr<LazyReversibleBinaryTreeNode>;
	using data_monoid = MM::data_monoid;
	using lazy_monoid = MM::lazy_monoid;
	using value_type = MM::data_monoid::value_type;
	using lazy_type = MM::lazy_monoid::value_type;
	value_type val, prod;
	lazy_type lazy;
	
	node_ptr_type left, right;
	int64_t subtree_size;
	bool rev;
	
	constexpr LazyReversibleBinaryTreeNode() : 
		val(MM::data_monoid::e()), prod(MM::data_monoid::e()), lazy(MM::lazy_monoid::e()), 
		left(nullptr), right(nullptr), subtree_size(0), rev(false) {}
	explicit LazyReversibleBinaryTreeNode(const value_type& x) : 
		val(x), prod(x), lazy(MM::lazy_monoid::e()), 
		left(nullptr), right(nullptr), subtree_size(1), rev(false) {}
	~LazyReversibleBinaryTreeNode() = default;
	
	inline static int64_t size(node_ptr_type node) { return node ? node->subtree_size : 0; }
	void update() {
		subtree_size = size(right) + 1 + size(left);
		prod = val;
		// ensure both children updated before accessing their data
		if(left) {
			left->propagate();
			left->map();
			prod = data_monoid::op(prod, left->prod);
		}
		if(right) {
			right->propagate();
			right->map();
			prod = data_monoid::op(right->prod, prod);
		}
	}
	void map() {
		prod = MM::mapping(MM::power(lazy, subtree_size), prod);
		val = MM::mapping(lazy, val);
		if(rev) std::swap(left, right);
		
		lazy = MM::lazy_monoid::e();
		rev = false;
	}
	void propagate() {
		if(left) {
			left->lazy = lazy_monoid::op(lazy, left->lazy);
			left->rev = (rev != left->rev); // xor
		}
		if(right) {
			right->lazy = lazy_monoid::op(lazy, right->lazy);
			right->rev = (rev != right->rev); // xor
		}
	}
};

template <class Tree> 
concept BinaryTreeConcept = requires (
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

template <BinaryTreeConcept Tree>
class BinaryTreeSet {
public:
	using tree_type = Tree;
	using value_type = Tree::value_type;
	using node_type = Tree::node_type;
	using node_ptr_type = Tree::node_ptr_type;
	node_ptr_type root;
public:
	struct Iterator {
		int64_t idx;
		const BinaryTreeSet* ptr;
		
		Iterator(int64_t _idx, const BinaryTreeSet* _ptr) : idx(_idx), ptr(_ptr) {}
		bool operator==(const Iterator& other) const { return idx == other.idx && ptr == other.ptr; }
		bool operator!=(const Iterator& other) const { return !(*this==other); }
		void operator++() { assert(idx < node_type::size(ptr->root)); ++idx; }
		void operator--() { assert(0 < idx); --idx; }
		const value_type& operator*() const { return ptr->get(idx); }
	};

	BinaryTreeSet() : root(nullptr) {}
	
	int64_t size() const { return node_type::size(root); }
	Iterator begin() const { return Iterator(0, this); }
	Iterator end() const { return Iterator(size(), this); }
	
	Iterator insert(const value_type& val) {
		Iterator res = lower_bound(val);
		if(res == end() || *res != val) {
			root = tree_type::insert(root, res.idx, std::make_shared<node_type>(val));
		}
		return res;
	}
	
	void erase(const value_type& val) {
		Iterator it = lower_bound(val);
		if(it != end() && *it == val) {
			root = tree_type::erase(root, it.idx);
		}
	}
	
	const value_type& get(int64_t k) const {
		assert(0 <= k && k < size());
		return tree_type::get(root, k)->val;
	}
	
	Iterator lower_bound(const value_type& val) const {
		int64_t idx = lower_bound(this->root, val);
		return idx == node_type::size(this->root) ? end() : Iterator(idx, this);
	}
	
	Iterator upper_bound(const value_type& val) const {
		Iterator it = lower_bound(val);
		if(it == end() || *it != val) return it;
		else { ++it; return it; }
	}
	
	int64_t count(const value_type& val) const {
		Iterator it = lower_bound(val);
		return it != end() && *it == val;
	}
	
private:
	int64_t lower_bound(node_ptr_type tree, const value_type& val) const {
		if(tree == nullptr) return 0;
		
		if(tree->val < val) 
			return node_type::size(tree->left) + lower_bound(tree->right, val) + 1;
		else
			return lower_bound(tree->left, val);
	}
};

template <BinaryTreeNodeConcept Node>
class RandomizedBinarySearchTree {
public:
	using value_type = Node::value_type;
	using node_type = Node;
	using node_ptr_type = std::shared_ptr<node_type>;
	
public:
	static uint64_t xor_shift() {
		static uint64_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
		uint64_t t = (x ^ (x << 11)); x = y; y = z; z = w;
		return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
	}
	
	static node_ptr_type merge(node_ptr_type l, node_ptr_type r) {
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
	static std::pair<node_ptr_type, node_ptr_type> split(node_ptr_type tree, int64_t k) {
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
	static node_ptr_type insert(node_ptr_type t, int64_t k, node_ptr_type node) {
		std::pair<node_ptr_type, node_ptr_type> p = split(t, k);
		return merge(merge(p.first, node), p.second);
	}
	
	// k 番目の要素を削除, ルートを返す
	static node_ptr_type erase(node_ptr_type t, int64_t k) {
		std::pair<node_ptr_type, node_ptr_type> right = split(t, k + 1); // [k][1],[n-k-1]
		std::pair<node_ptr_type, node_ptr_type> left = split(right.first, k); // [k][1]
		return merge(left.first, right.second); // [k][n-k-1]
	}
	
	static node_ptr_type get(node_ptr_type t, int64_t k) {
		if(t == nullptr) return t;
		int64_t sz = Node::size(t->left);
		
		if(sz > k) return get(t->left, k);
		else if(sz < k) return get(t->right, k - sz - 1);
		else return t;
	}
};

template <BinaryTreeNodeConcept Node>
class LazyReversibleRandomizedBinarySearchTree {
public:
	using value_type = Node::value_type;
	using lazy_type = Node::lazy_type;
	using node_type = Node;
	using node_ptr_type = std::shared_ptr<node_type>;
public:
	static uint64_t xor_shift() {
		static uint64_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
		uint64_t t = (x ^ (x << 11)); x = y; y = z; z = w;
		return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
	}
	
	static node_ptr_type merge(node_ptr_type l, node_ptr_type r) {
		if(l == nullptr) return r;
		if(r == nullptr) return l;
		
		if(xor_shift() % (uint64_t)(Node::size(l) + Node::size(r)) < (uint64_t)Node::size(l)) {
			l->propagate();
			l->map();
			l->right = merge(l->right, r);
			l->update();
			return l;
		}
		else {
			r->propagate();
			r->map();
			r->left = merge(l, r->left);
			r->update();
			return r;
		}
	}
	
	static std::pair<node_ptr_type, node_ptr_type> split(node_ptr_type tree, int64_t k) {
		if(tree == nullptr) return std::make_pair(nullptr, nullptr);
		
		tree->propagate();
		tree->map();
		
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
	static node_ptr_type insert(node_ptr_type t, int64_t k, node_ptr_type node) {
		std::pair<node_ptr_type, node_ptr_type> p = split(t, k);
		return merge(merge(p.first, node), p.second);
	}
	
	// k 番目の要素を削除, ルートを返す
	static node_ptr_type erase(node_ptr_type t, int64_t k) {
		std::pair<node_ptr_type, node_ptr_type> right = split(t, k + 1); // [k][1],[n-k-1]
		std::pair<node_ptr_type, node_ptr_type> left = split(right.first, k); // [k][1]
		return merge(left.first, right.second); // [k][n-k-1]
	}
	
	static node_ptr_type get(node_ptr_type t, int64_t k) {
		if(t == nullptr) return t;
		t->propagate();
		t->map();
		int64_t sz = Node::size(t->left);
		
		node_ptr_type res = nullptr;
		if(sz > k) res = get(t->left, k);
		else if(sz < k) res = get(t->right, k - sz - 1);
		else res = t;
		
		t->update();
		return res;
	}
};

template <BinaryTreeConcept Tree>
class BinaryTreeArray {
private:
	using value_type = Tree::value_type;
	using lazy_type = Tree::lazy_type;
	using node_type = Tree::node_type;
	using node_ptr_type = std::shared_ptr<typename Tree::node_type>;
	node_ptr_type root;
public:
	struct Iterator {
		int64_t idx;
		const BinaryTreeArray* ptr;
		
		Iterator(int64_t _idx, const BinaryTreeArray* _ptr) : idx(_idx), ptr(_ptr) {}
		bool operator==(const Iterator& other) const { return idx == other.idx && ptr == other.ptr; }
		bool operator!=(const Iterator& other) const { return !(*this==other); }
		void operator++() { assert(idx < node_type::size(ptr->root)); ++idx; }
		void operator--() { assert(0 < idx); --idx; }
		value_type operator*() const { return ptr->get(idx); }
	};

	BinaryTreeArray() : root(nullptr) {}
	
	int64_t size() const { return node_type::size(root); }
	Iterator begin() const { return Iterator(0, this); }
	Iterator end() const { return Iterator(size(), this); }
	
	value_type get(int64_t idx) const {
		assert(0 <= idx && idx < size());
		auto node = Tree::get(root, idx);
		return node->val;
	}
	
	void reverse(int64_t l, int64_t r) {
		if(l == r) return;
		assert(l < r && 0 <= l && l < size() && 0 < r && r <= size());
		auto [nl, nm] = Tree::split(root, l);
		auto [nt, nr] = Tree::split(nm, r-l);
		nt->rev ^= true;
		root = Tree::merge(Tree::merge(nl, nt), nr);
	}
	
	void apply(int64_t l, int64_t r, const lazy_type& f) {
		if(l == r) return;
		assert(l < r && 0 <= l && l < size() && 0 < r && r <= size());
		
		auto [nl, nm] = Tree::split(root, l);
		auto [nt, nr] = Tree::split(nm, r-l);
		nt->lazy = node_type::lazy_monoid::op(f, nt->lazy);
		root = Tree::merge(Tree::merge(nl, nt), nr);
	}
	
	value_type prod(int64_t l, int64_t r) {
		if(l == r) return node_type::data_monoid::e();
		assert(l < r && 0 <= l && l < size() && 0 < r && r <= size());
		auto [nl, nm] = Tree::split(root, l);
		auto [nt, nr] = Tree::split(nm, r-l);
		auto res = nt->prod;
		root = Tree::merge(Tree::merge(nl, nt), nr);
		return res;
	}
	
	Iterator insert(int64_t idx, const value_type& x) {
		assert(0 <= idx && idx <= size());
		auto [l, r] = Tree::split(root, idx);
		l = Tree::merge(l, std::make_shared<node_type>(x));
		root = Tree::merge(l, r);
		return Iterator(idx, this);
	}
	
	void erase(int64_t idx) {
		assert(0 <= idx && idx < size());
		auto [l, m] = Tree::split(root, idx);
		auto [t, r] = Tree::split(m, 1);
		root = Tree::merge(l, r);
	}
};

template <typename T> using RBSTSet = BinaryTreeSet<RandomizedBinarySearchTree<BinaryTreeNodeBase<T>>>;
template <typename T> using RBSTArray = BinaryTreeArray<LazyReversibleRandomizedBinarySearchTree<LazyReversibleBinaryTreeNode<T>>>;
