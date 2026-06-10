#include <concepts>
#include <utility>
#include <memory>
#include <cstdint>
#include <cassert>

template <class Tree> 
concept BinarySearchTreeBase = requires {
	typename Tree::value_type;
	typename Tree::node_type;
	typename Tree::node_ptr_type;
	
	// void insert(ll idx, T val)
	{ Tree::insert(std::declval<int64_t>(), std::declval<Tree::value_type>()) } -> std::same_as<void>;
	// void erase(ll idx)
	{ Tree::erase(std::declval<int64_t>()) } -> std::same_as<void>;
	// ptr merge(ptr l, ptr r)
	{ Tree::merge(std::declval<Tree::node_ptr_type>(), std::declval<Tree::node_ptr_type>()) }
		-> std::same_as<typename Tree::node_ptr_type>;
	// pair<ptr, ptr> split(ptr root, ll num);
	{ Tree::split(std::declval<Tree::node_ptr_type>(), std::declval<int64_t>()) }
		-> std::same_as<typename std::pair<typename Tree::node_ptr_type, typename Tree::node_ptr_type>>;
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
		inline static const node_ptr_type NIL = std::make_shared<node_type>();
		
		constexpr Node() : val(T()), left(NIL), right(NIL), subtree_size(0) {}
		explicit Node(value_type x) : val(x), left(NIL), right(NIL), subtree_size(1) {}
		~Node() = default;
		
		inline int64_t size() { return subtree_size; }
		void update() { subtree_size = 1 + left->subtree_size + right->subtree_size; }
	};
	
public:
	static uint64_t xor_shift() {
		static uint64_t x = 123456789, y = 362436069, z = 521288629, w = 88675123;
		uint64_t t = (x ^ (x << 11)); x = y; y = z; z = w;
		return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
	}
	
	node_ptr_type merge(node_ptr_type l, node_ptr_type r) {
		if(l == Node::NIL) return r;
		if(r == Node::NIL) return l;
		
		if(xor_shift() % (uint64_t)(l->size() + r->size()) < (uint64_t)l->size()) {
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
		if(tree == Node::NIL) return std::make_pair(Node::NIL, Node::NIL);
		
		if(k <= tree->left->size()) {
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
			auto p = split(tree->right, k - tree->left->size() - 1);
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
		if(t == Node::NIL) return t;
		int64_t sz = t->left->size();
		
		if(sz > k) return get(t->left, k);
		else if(sz < k) return get(t->right, k - sz - 1);
		else return t;
	}
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
		void operator++() { assert(idx < ptr->size()); ++idx; }
		void operator--() { assert(0 < idx); --idx; }
		const T& operator*() const { return ptr->get(idx); }
	};
	
	RandomizedBinarySearchTreeSet() : root(node_type::NIL) {}
	
	int64_t size() const { return this->root->size(); }
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
		return idx == this->root->size() ? end() : Iterator(idx, this);
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
		if(tree == node_type::NIL) return 0;
		
		if(tree->val < val) 
			return tree->left->size() + lower_bound(tree->right, val) + 1;
		else
			return lower_bound(tree->left, val);
	}
};
