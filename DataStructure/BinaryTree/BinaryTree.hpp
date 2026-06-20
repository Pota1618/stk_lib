#include <type_traits>
#include <concepts>
#include <memory>
#include <functional>

#include "stk/Math/Monoid.hpp"

// BST Concepts
template <class Node>
concept BinaryTreeNodeConcept = 
requires (Node node, typename Node::key_type key_type ) {
	requires std::same_as<decltype(node.key), decltype(key_type)>;
	requires std::same_as<decltype(node.left), std::shared_ptr<decltype(node)>>;
	requires std::same_as<decltype(node.right), std::shared_ptr<decltype(node)>>;
	requires std::same_as<decltype(node.subtree_size), int64_t>;
	{ node.update() } -> std::same_as<void>;
};

template <class Tree> 
concept BinaryTreeConcept = requires (
	typename Tree::key_type key_type,
	typename Tree::node_type node_type,
	typename Tree::node_ptr_type ptr_type
) {
	// node_ptr merge(ptr l, ptr r)
	{ Tree::merge(ptr_type, ptr_type) } -> std::same_as<decltype(ptr_type)>;
	
	// pair<node_ptr, node_ptr> split(node_ptr root, ll num_left);
	{ Tree::split(ptr_type, std::declval<int64_t>()) } 
		-> std::same_as<typename std::pair<decltype(ptr_type), decltype(ptr_type)>>;
		
	// node_ptr insert(root, idx, node)
	{ Tree::insert(ptr_type, std::declval<int64_t>(), ptr_type) } -> std::same_as<decltype(ptr_type)>;
	
	// node_ptr erase(root, idx)
	{ Tree::erase(ptr_type, std::declval<int64_t>()) } -> std::same_as<decltype(ptr_type)>;
	
	// node_ptr get(root, idx)
	{ Tree::get(ptr_type, std::declval<int64_t>()) } -> std::same_as<decltype(ptr_type)>;
};

// BST Nodes
template <typename T>
struct BinaryTreeNodeBase {
	using key_type = T;
	using node_ptr_type = std::shared_ptr<BinaryTreeNodeBase>;
	key_type key;
	node_ptr_type left, right;
	int64_t subtree_size;
	
	constexpr BinaryTreeNodeBase() : key(T()), left(nullptr), right(nullptr), subtree_size(0) {}
	explicit BinaryTreeNodeBase(const key_type& x) : key(x), left(nullptr), right(nullptr), subtree_size(1) {}
	~BinaryTreeNodeBase() = default;
	
	inline static int64_t size(node_ptr_type node) { return node ? node->subtree_size : 0; }
	inline void update() { subtree_size = 1 + size(left) + size(right); }
};

template <typename K, typename V>
struct BinaryTreeMapNode {
	using key_type = K;
	using val_type = V;
	using node_ptr_type = std::shared_ptr<BinaryTreeMapNode>;
	key_type key, val;
	node_ptr_type left, right;
	int64_t subtree_size;
	
	//constexpr BinaryTreeMapNode() : key(K()), val(V()), left(nullptr), right(nullptr), subtree_size(0) {}
	explicit BinaryTreeMapNode(const key_type& x) : key(x), val(V()), left(nullptr), right(nullptr), subtree_size(1) {}
	~BinaryTreeMapNode() = default;
	
	inline static int64_t size(node_ptr_type node) { return node ? node->subtree_size : 0; }
	inline void update() { subtree_size = 1 + size(left) + size(right); }
};

template <MapMonoid MM>
struct LazyReversibleBinaryTreeNode {
	using node_ptr_type = std::shared_ptr<LazyReversibleBinaryTreeNode>;
	using data_monoid = MM::data_monoid;
	using lazy_monoid = MM::lazy_monoid;
	using key_type = MM::data_monoid::value_type;
	using lazy_type = MM::lazy_monoid::value_type;
	key_type key, prod;
	lazy_type lazy;
	
	node_ptr_type left, right;
	int64_t subtree_size;
	bool rev;
	
	constexpr LazyReversibleBinaryTreeNode() : 
		key(MM::data_monoid::e()), prod(MM::data_monoid::e()), lazy(MM::lazy_monoid::e()), 
		left(nullptr), right(nullptr), subtree_size(0), rev(false) {}
	explicit LazyReversibleBinaryTreeNode(const key_type& x) : 
		key(x), prod(x), lazy(MM::lazy_monoid::e()), 
		left(nullptr), right(nullptr), subtree_size(1), rev(false) {}
	~LazyReversibleBinaryTreeNode() = default;
	
	inline static int64_t size(node_ptr_type node) { return node ? node->subtree_size : 0; }
	void update() {
		subtree_size = size(right) + 1 + size(left);
		prod = key;
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
		key = MM::mapping(lazy, key);
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

// BST Data Structures
template <BinaryTreeConcept Tree>
class BinaryTreeSet {
public:
	using tree_type = Tree;
	using key_type = Tree::key_type;
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
		const key_type& operator*() const { return ptr->get(idx); }
	};

	BinaryTreeSet() : root(nullptr) {}
	
	int64_t size() const { return node_type::size(root); }
	Iterator begin() const { return Iterator(0, this); }
	Iterator end() const { return Iterator(size(), this); }
	
	Iterator insert(const key_type& key) {
		Iterator res = lower_bound(key);
		if(res == end() || *res != key) {
			root = tree_type::insert(root, res.idx, std::make_shared<node_type>(key));
		}
		return res;
	}
	
	void erase(const key_type& key) {
		Iterator it = lower_bound(key);
		if(it != end() && *it == key) {
			root = tree_type::erase(root, it.idx);
		}
	}
	
	const key_type& get(int64_t k) const {
		assert(0 <= k && k < size());
		return tree_type::get(root, k)->key;
	}
	
	Iterator lower_bound(const key_type& key) const {
		int64_t idx = lower_bound(this->root, key);
		return idx == node_type::size(this->root) ? end() : Iterator(idx, this);
	}
	
	Iterator upper_bound(const key_type& key) const {
		Iterator it = lower_bound(key);
		if(it == end() || *it != key) return it;
		else { ++it; return it; }
	}
	
	int64_t count(const key_type& key) const {
		Iterator it = lower_bound(key);
		return it != end() && *it == key;
	}
	
private:
	int64_t lower_bound(node_ptr_type tree, const key_type& key) const {
		if(tree == nullptr) return 0;
		
		if(tree->key < key) 
			return node_type::size(tree->left) + lower_bound(tree->right, key) + 1;
		else
			return lower_bound(tree->left, key);
	}
};

template <BinaryTreeConcept Tree>
class BinaryTreeMap {
private:
	using key_type = BinaryTreeSet<Tree>::node_type::key_type;
	using val_type = BinaryTreeSet<Tree>::node_type::val_type;
	BinaryTreeSet<Tree> st;
public:
	struct Iterator {
		int64_t idx;
		const BinaryTreeMap* ptr;
		
		Iterator(int64_t _idx, const BinaryTreeMap* _ptr) : idx(_idx), ptr(_ptr) {}
		bool operator==(const Iterator& other) const { return idx == other.idx && ptr == other.ptr; }
		bool operator!=(const Iterator& other) const { return !(*this==other); }
		void operator++() { assert(idx < ptr->st.size()); ++idx; }
		void operator--() { assert(0 < idx); --idx; }
		auto operator*() const { 
			auto node = Tree::get(ptr->st.root, idx);
			return std::make_pair(std::cref(node->key), std::ref(node->val));
		}
	};
	
	BinaryTreeMap() : st() {}
	
	int64_t size() { return st.size(); }
	Iterator begin() { return Iterator(0, this); }
	Iterator end() { return Iterator(st.size(), this); }
	
	int64_t count(const key_type& key) { return st.count(key); }
	
	val_type& operator[](const key_type& key) {
		auto it = st.lower_bound(key);
		if(it == st.end() || *it != key) {
			auto it = st.insert(key);
			return Tree::get(st.root, it.idx)->val;
		}
		else {
			return Tree::get(st.root, it.idx)->val;
		}
	}
};

template <BinaryTreeConcept Tree>
class BinaryTreeArray {
private:
	using key_type = Tree::key_type;
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
		key_type operator*() const { return ptr->get(idx); }
	};

	BinaryTreeArray() : root(nullptr) {}
	
	int64_t size() const { return node_type::size(root); }
	Iterator begin() const { return Iterator(0, this); }
	Iterator end() const { return Iterator(size(), this); }
	
	key_type get(int64_t idx) const {
		assert(0 <= idx && idx < size());
		auto node = Tree::get(root, idx);
		return node->key;
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
	
	key_type prod(int64_t l, int64_t r) {
		if(l == r) return node_type::data_monoid::e();
		assert(l < r && 0 <= l && l < size() && 0 < r && r <= size());
		auto [nl, nm] = Tree::split(root, l);
		auto [nt, nr] = Tree::split(nm, r-l);
		auto res = nt->prod;
		root = Tree::merge(Tree::merge(nl, nt), nr);
		return res;
	}
	
	Iterator insert(int64_t idx, const key_type& x) {
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
