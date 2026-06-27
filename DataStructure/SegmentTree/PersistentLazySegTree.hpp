#include <vector>
#include <memory>
#include <functional>
#include <cassert>

#include "stk/Math/Monoid.hpp"

// 抽象化永続遅延評価疎線段樹 (Persistent Sparse Lazy Segment Tree)
template <MapMonoid MM, size_t D = 30>
class SparsePersistentLazySegTree {
private:
	using data_type = MM::data_monoid::value_type;
	using lazy_type = MM::lazy_monoid::value_type;
	static constexpr auto op = MM::data_monoid::op;
	static constexpr auto composition = MM::lazy_monoid::op;
	static constexpr auto e = MM::data_monoid::e;
	static constexpr auto id = MM::lazy_monoid::e;
	static constexpr auto mapping = MM::mapping;
	static constexpr auto power = MM::power;
	
	struct Node;
	using node_ptr = std::shared_ptr<Node>;
	struct Node {
		node_ptr left, right;
		data_type data;
		lazy_type lazy;
		
		Node() : left(nullptr), right(nullptr), data(e()), lazy(id()) {}
		Node(const data_type& x) : left(nullptr), right(nullptr), data(x), lazy(id()) {}
	};
	
	node_ptr root;
	
public:
	SparsePersistentLazySegTree() : root(std::make_shared<Node>()) {}
	SparsePersistentLazySegTree(const std::vector<data_type>& vec) : root(std::make_shared<Node>()) {
		build(root, vec, 0, 1ull << D);
	}
	
	SparsePersistentLazySegTree apply(size_t l, size_t r, const lazy_type& f) {
		if(l >= r) return *this;
		node_ptr new_root = duplicate_node(root);
		apply_impl(new_root, l, r, 0, 1ull << D, f);
		return SparsePersistentLazySegTree(new_root);
	}
	
	data_type prod(size_t l, size_t r) {
		if(l >= r) return e();
		return prod_impl(root, l, r, 0, 1ull << D);
	}
	
	SparsePersistentLazySegTree copy(SparsePersistentLazySegTree source, size_t l, size_t r) {
		node_ptr new_root = duplicate_node(root);
		node_ptr src_new_root = duplicate_node(source.root);
		copy_impl(src_new_root, new_root, l, r, 0, 1ull << D);
		return SparsePersistentLazySegTree(new_root);
	}
private:
	SparsePersistentLazySegTree(node_ptr new_root) : root(new_root) {}

	void build(node_ptr node, const std::vector<data_type>& vec, size_t nl, size_t nr) {
		if(nl >= vec.size()) return;
		if(nr - nl == 1) {
			node->data = vec[nl];
			return;
		}
		size_t nm = (nl + nr) / 2;
		node->left = std::make_shared<Node>();
		node->right = std::make_shared<Node>();
		build(node->left, vec, nl, nm);
		build(node->right, vec, nm, nr);
		update(node);
	}

	void apply_impl(node_ptr node, size_t ql, size_t qr, size_t nl, size_t nr, const lazy_type& f) {
		if(qr <= nl || nr <= ql) return;
		else if(ql <= nl && nr <= qr) {
			map(node, f, nr - nl);
		}
		else {
			propagate(node, nr - nl);
			size_t nm = (nl + nr) / 2;
			apply_impl(node->left, ql, qr, nl, nm, f);
			apply_impl(node->right, ql, qr, nm, nr, f);
			update(node);
		}
	}
	
	data_type prod_impl(node_ptr node, size_t ql, size_t qr, size_t nl, size_t nr) {
		if(!node) return e();
		else if(qr <= nl || nr <= ql) return e();
		else if(ql <= nl && nr <= qr) {
			return node->data;
		}
		else {
			propagate(node, nr - nl);
			size_t nm = (nl + nr) / 2;
			data_type res_l = prod_impl(node->left, ql, qr, nl, nm);
			data_type res_r = prod_impl(node->right, ql, qr, nm, nr);
			return op(res_r, res_l);
		}
	}
	
	void copy_impl(node_ptr src, node_ptr dst, size_t ql, size_t qr, size_t nl, size_t nr) {
		if(qr <= nl || nr <= ql) return;
		else if(ql <= nl && nr <= qr) {
			dst->data = src->data;
			dst->lazy = src->lazy;
			dst->left = src->left;
			dst->right = src->right;
		}
		else {
			propagate(src, nr - nl);
			propagate(dst, nr - nl);
			size_t nm = (nl + nr) / 2;
			copy_impl(src->left, dst->left, ql, qr, nl, nm);
			copy_impl(src->right, dst->right, ql, qr, nm, nr);
			update(dst);
		}
	}

	void propagate(node_ptr node, int len) {
		node->left = duplicate_node(node->left);
		node->right = duplicate_node(node->right);
		map(node->left, node->lazy, len/2);
		map(node->right, node->lazy, len/2);
		node->lazy = id();
	}
	
	void map(node_ptr node, const lazy_type& f, int len) {
		node->data = mapping(power(f, len), node->data);
		node->lazy = composition(f, node->lazy);
	}
	
	inline void update(node_ptr node) {
		node->data = op(
			node->right->data,
			node->left->data
		);
	}
	
	inline node_ptr duplicate_node(node_ptr node) {
		return (node ? std::make_shared<Node>(*node) : std::make_shared<Node>());
	}
};

/*
https://judge.yosupo.jp/problem/persistent_range_affine_range_sum

template <class M>
concept Monoid = requires { 
	typename M::value_type;
    { M::e() } -> std::same_as<typename M::value_type>;
    { M::op(std::declval<typename M::value_type>(), std::declval<typename M::value_type>()) } 
		-> std::same_as<typename M::value_type>;
};

constexpr int64_t MOD = 998244353;
template <typename T = int64_t>
struct AddMonoid {
	using value_type = T;
	static constexpr bool commutative = true;
	static T e() { return 0; }
	static T op(T a, T b) { return (a + b) % MOD; }
};

template <typename T>
struct AffineMonoid {
	using value_type = std::pair<T, T>;
	static value_type e() { return std::make_pair(1, 0); }
	static value_type op(const value_type& g, const value_type& f) {
		return std::make_pair(
			(f.first * g.first) % MOD, 
			(g.first * f.second + g.second) % MOD
		);
	}
};

template <class MM>
concept MapMonoid = Monoid<typename MM::data_monoid> && Monoid<typename MM::lazy_monoid> && 
requires(typename MM::data_monoid::value_type x, typename MM::lazy_monoid::value_type f) {
	// The mapping function defines how the LazyMonoid acts on the DataMonoid
	{ MM::mapping(f, x) } -> std::same_as<decltype(x)>;
    { MM::power(f, std::declval<int>()) } -> std::same_as<decltype(f)>;
};

template <typename T>
struct RangeAffineRangeSum {
	using data_monoid = AddMonoid<T>;
	using lazy_monoid = AffineMonoid<T>;
	static data_monoid::value_type mapping(lazy_monoid::value_type f, data_monoid::value_type x) {
		return (f.first * x + f.second) % MOD;
	}
	static lazy_monoid::value_type power(lazy_monoid::value_type f, int p) {
		return std::make_pair(f.first, (f.second * p) % MOD);
	}
};

#include <unordered_map>
#include <set>
#include <iostream>
using namespace std;

int main() {
	int N, Q; cin >> N >> Q;
	vector<int64_t> A(N);
	for(int i = 0; i < N; ++i) cin >> A[i];
	using LazySegTreeType = SparsePersistentLazySegTree<RangeAffineRangeSum<int64_t>, 18>;
	LazySegTreeType seg(A);
	
	unordered_map<int, LazySegTreeType> segs;
	segs[-1] = seg;
	for(int i = 0; i < Q; ++i) {
		int q; cin >> q;
		if(q == 0) {
			int k, l, r, b, c;
			cin >> k >> l >> r >> b >> c;
			segs[i] = segs[k].apply(l, r, {b, c});
		}
		else if(q == 1) {
			int k, s, l, r; cin >> k >> s >> l >> r;
			segs[i] = segs[k].copy(segs[s], l, r);
		}
		else {
			int k, l, r; cin >> k >> l >> r;
			int64_t ans = segs[k].prod(l, r);
            cout << ans << '\n';
		}
	}
}
*/
