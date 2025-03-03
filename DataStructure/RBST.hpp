#include <memory>
using namespace std;

template<typename NodeType>
class BinarySearchTreeBase {
private:
	using T = NodeType::ValueType;
	using NodePtr = shared_ptr<NodeType>;
protected:
	virtual NodePtr get(size_t k) const = 0;
	virtual void insert(size_t k, const T& val) = 0;
	virtual void erase(size_t k) = 0;
	virtual NodePtr merge(NodePtr l, NodePtr r) = 0;
	virtual pair<NodePtr, NodePtr> split(NodePtr t, size_t k) = 0;
};

template<typename T>
struct RBSTNode {
	using ValueType = T;
	T val;
	shared_ptr<RBSTNode> left, right;
	static shared_ptr<RBSTNode> NIL;

	size_t subtree_size;
	RBSTNode() : val(T()), left(NIL), right(NIL), subtree_size(0) {}
	RBSTNode(T v) : val(v), left(NIL), right(NIL), subtree_size(1) {}

	inline size_t size() { return subtree_size; }

	void update() {
		this->subtree_size = 1 + this->left->subtree_size + this->right->subtree_size;
	}
};
template<typename T> shared_ptr<RBSTNode<T>> RBSTNode<T>::NIL(new RBSTNode<T>());

// Randomized Binary Search Tree
template<typename T>
class RBST : public BinarySearchTreeBase<RBSTNode<T>> {
protected:
	using Node = RBSTNode<T>;
	using NodePtr = shared_ptr<Node>;
	using NodePair = pair<NodePtr, NodePtr>;

	NodePtr root;
	const RBSTNode<T> NIL;

public:
	RBST() : root(Node::NIL) {}

	// k 番目のノードを返す
	NodePtr get(size_t k) const { return get(root, k); }
	// k 番目に val を挿入
	void insert(size_t k, const T& val) { root = insert(root, k, make_shared<Node>(val)); }
	// k 番目の値を削除
	void erase(size_t k) { root = erase(root, k); }

private:
	unsigned long long xor_shift() {
		static unsigned long long x = 123456789, y = 362436069, z = 521288629, w = 88675123;
		unsigned long long t = (x ^ (x << 11)); x = y; y = z; z = w;
		return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
	}

	NodePtr merge(NodePtr l, NodePtr r) {
		if (l == Node::NIL || r == Node::NIL) {
			if (l == Node::NIL) return r;
			else return l;
		}

		if (xor_shift() % (l->size() + r->size()) < l->size()) {
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

	// 左右のサイズが l, (size(t) - k) になるように分割
	NodePair split(NodePtr t, size_t k) {
		if (t == Node::NIL) return NodePair(Node::NIL, Node::NIL);

		if (k <= t->left->size()) {
			NodePair np = split(t->left, k);
			t->left = np.second;
			t->update();
			return NodePair(np.first, t);
		}
		else {
			NodePair np = split(t->right, k - (t->left->size()) - 1);
			t->right = np.first;
			t->update();
			return NodePair(t, np.second);
		}
	}

	// k 番目に node を挿入, ルートを返す
	NodePtr insert(NodePtr t, size_t k, NodePtr node) {
		NodePair np = split(t, k);
		return merge(merge(np.first, node), np.second);
	}

	// k 番目から値を削除, ルートを返す
	NodePtr erase(NodePtr t, size_t k) {
		NodePair right = split(t, k + 1); // [...][], [...]
		NodePair left = split(right.first, k); // [...], []
		return merge(left.first, right.second);
	}

	NodePtr get(NodePtr t, size_t k) const {
		if (t == Node::NIL) return t;
		size_t sz = t->left->size();

		if (sz > k) return get(t->left, k);
		else if (sz < k) return get(t->right, k - sz - 1);
		else return t;
	}
};

template<typename T>
class RBSTSet : RBST<T> {
private:
	using Node = RBST<T>::Node;
	using NodePtr = RBST<T>::NodePtr;
public:
	struct Iterator {
		size_t idx;
		const RBSTSet<T>* ptr;

		Iterator(size_t _idx, const RBSTSet<T>* _ptr) : idx(_idx), ptr(_ptr) {}
		bool operator==(const Iterator& other) const { return this->idx == other.idx && this->ptr == other.ptr; }
		bool operator!=(const Iterator& other) const { return !(*this == other); }
		void operator++() { assert(idx < ptr->size()); ++idx; }
		void operator--() { assert(0 < idx); --idx; }
		const T& operator*() const { return ptr->get_value(idx); }
	};

public:
	RBSTSet() {}

	size_t size() const { return this->root->size(); }
	Iterator begin() const { return Iterator(0, this); }
	Iterator end() const { return Iterator(size(), this); }

	Iterator lower_bound(const T& val) const {
		size_t idx = lower_bound(this->root, val);
		return idx == this->root->size() ? end() : Iterator(idx, this);
	}

	Iterator upper_bound(const T& val) const {
		Iterator it = lower_bound(val);
		if (it == end() || *it != val) return it;
		else {
			++it;
			return it;
		}
	}

	size_t count(const T& val) const {
		Iterator it = lower_bound(val);
		if (it == end()) return 0;

		return *it == val;
	}

	pair<Iterator, bool> insert(const T& val) {
		pair<Iterator, bool> res(this->lower_bound(val), false);

		if (res.first == end() || *(res.first) != val) {
			RBST<T>::insert(res.first.idx, val);
			res.second = true;
		}
		return res;
	}

	size_t erase(const T& val) {
		Iterator it = lower_bound(val);
		if (it == end() || *it != val) return 0;
		else {
			RBST<T>::erase(it.idx);
			return 1;
		}
	}

	const T& get_value(size_t k) const {
		assert(0 <= k && k < size());
		return RBST<T>::get(k)->val;
	}

private:
	size_t lower_bound(NodePtr t, const T& val) const {
		if (t == Node::NIL) return 0;

		if (t->val < val)
			return t->left->size() + lower_bound(t->right, val) + 1;
		else
			return lower_bound(t->left, val);
	}
};

/*
// https://kopricky.github.io/code/BinarySearchTree/rbst_set.html
*/
