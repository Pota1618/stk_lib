#include <vector>
#include <memory>
#include <functional>
#include <cassert>

template <class M>
concept Monoid = requires { 
	typename M::value_type;
    { M::e() } -> std::same_as<typename M::value_type>;
    { M::op(std::declval<typename M::value_type>(), std::declval<typename M::value_type>()) } 
		-> std::same_as<typename M::value_type>;
};

template <class MM>
concept MapMonoid = Monoid<typename MM::data_monoid> && Monoid<typename MM::lazy_monoid> && 
requires(typename MM::data_monoid::value_type x, typename MM::lazy_monoid::value_type f) {
	// The mapping function defines how the LazyMonoid acts on the DataMonoid
	{ MM::mapping(f, x) } -> std::same_as<decltype(x)>;
    { MM::power(f, std::declval<int>()) } -> std::same_as<decltype(f)>;
};
	
template <MapMonoid MM, size_t D = 18>
class PersistentLazySegTree {
	using i64 = int64_t;
	struct Node;
	using node_type = Node;
	using node_ptr_type = std::shared_ptr<node_type>;
	using data_type = MM::data_monoid::value_type;
	using lazy_type = MM::lazy_monoid::value_type;
	
	node_ptr_type root;
	
	inline static constexpr auto e = MM::data_monoid::e;
	inline static constexpr auto op = MM::data_monoid::op;
	inline static constexpr auto id = MM::lazy_monoid::e;
	inline static constexpr auto composition = MM::lazy_monoid::op;
	inline static constexpr auto mapping = MM::mapping;
	
	struct Node {
		data_type data;
		lazy_type lazy;
		node_ptr_type left, right;
		
		constexpr Node() : data(e()), lazy(id()), left(nullptr), right(nullptr) {}
		explicit Node(data_type x) : data(x), lazy(id()), left(nullptr), right(nullptr) {}
		
        void make_children() {
            left = duplicate(left);
            right = duplicate(right);
        }
		// distribute lazy value to children
		void propagate() {
            left->lazy = composition(this->lazy, left->lazy);
            right->lazy = composition(this->lazy, right->lazy);
		}
		// apply lazy to data
		void map(size_t p) {
            data = mapping(MM::power(lazy, p), data);
		}
		// recalclate prod
		void update() {
			data = op(
				right ? right->data : e(), 
				left ? left->data : e()
			);
		}
        
        inline static node_ptr_type duplicate(node_ptr_type node) {
            node_ptr_type res = node ? std::make_shared<Node>(*node) : std::make_shared<Node>();
            return res;
        }
	};
	
public:
	PersistentLazySegTree() : root(nullptr) {}
	explicit PersistentLazySegTree(node_ptr_type root) : root(root) {}
		
	PersistentLazySegTree set(size_t idx, data_type x) {
		node_ptr_type new_root = Node::duplicate(root);
        new_root = set_impl(new_root, idx, x, 0, 1ull << D);	
		return PersistentLazySegTree(new_root);
	}
	
	PersistentLazySegTree apply(size_t l, size_t r, lazy_type f) {
		assert(l <= r && r <= (1ull << D));
		node_ptr_type new_root = Node::duplicate(root);
        apply_impl(new_root, l, r, f, 0ull, 1ull<<D);
		return PersistentLazySegTree(new_root);
	}
	
	std::pair<PersistentLazySegTree, data_type> prod(size_t l, size_t r) {
		assert(l <= r && r <= (1ull << D));
        node_ptr_type new_root = Node::duplicate(root);
        data_type res = prod_impl(new_root, l, r, 0, 1ull << D);
        return std::make_pair(PersistentLazySegTree(new_root), res);
	}
	
	PersistentLazySegTree copy(PersistentLazySegTree source, size_t l, size_t r) {
		assert(l <= r && r <= (1ull << D));
		
        node_ptr_type new_root = Node::duplicate(root);
		// prod_impl(new_root, l, r, 0, 1ull << D);
		node_ptr_type src_new_root = Node::duplicate(source.root);
		// prod_impl(new_root, src_new_root, l, r, 0, 1ull<<D);
		copy_impl(src_new_root, new_root, l, r, 0, 1ull << D);
		return PersistentLazySegTree(new_root);
	}
	
private:
    node_ptr_type set_impl(node_ptr_type node, const size_t idx, data_type x, const size_t nl, const size_t nr) {
        if(nr - nl <= 1) {
            node->map(nr - nl);
            node->lazy = id();
            node->data = x;
            return node;
        }
        
        node->make_children();
        node->propagate();
        node->map(nr - nl);
        node->lazy = id();
        size_t m = (nl+nr)/2;
        if(idx < m) {
            node->left = Node::duplicate(node->left);
            set_impl(node->left, idx, x, nl, m);
        }
        else {
            node->right = Node::duplicate(node->right);
            set_impl(node->right, idx, x, m, nr);
        }
        node->update();
        return node;
    }

	void apply_impl(node_ptr_type node, const size_t l, const size_t r, lazy_type lazy, const size_t nl, const size_t nr) {
        if(nr - nl > 1) {
            node->make_children();
            node->propagate(); // 葉ではないなら伝播
        }
        node->map(nr - nl);
        node->lazy = id();
        
        if(l <= nl && nr <= r) { // 包含されている
			node->lazy = composition(lazy, node->lazy);
            if(nr - nl > 1) node->propagate();
            node->map(nr - nl);
            node->lazy = id();
		}
        else if(l < nr && nl < r) {
			apply_impl(node->left , l, r, lazy, nl, (nl+nr)/2);
			apply_impl(node->right, l, r, lazy, (nl+nr)/2, nr);
            node->update();
		}
	}
	
	data_type prod_impl(node_ptr_type node, const size_t l, const size_t r, const size_t nl, const size_t nr) {
		if(nr - nl > 1) {
            node->make_children();
            node->propagate(); // 葉ではないなら伝播
        }
        node->map(nr - nl);
		node->lazy = id();
		
        if(l <= nl && nr <= r) { // 包含されている
			return node->data;
		}
		else if(l < nr && nl < r) { // 重なっている
			size_t mid = (nl + nr) / 2;
			data_type res_l = prod_impl(node->left, l, r, nl, mid);
			data_type res_r = prod_impl(node->right, l, r, mid, nr);
			
			return op(res_r, res_l);
		} else {
			return e();
		}
	}
	
	node_ptr_type copy_impl(node_ptr_type src, node_ptr_type dst, const size_t l, const size_t r, const size_t nl, const size_t nr) {
		if(nr - nl > 1) {
            dst->make_children();
            dst->propagate();
            src->make_children();
            src->propagate();
        }
		dst->map(nr - nl);
        dst->lazy = id();
        src->map(nr - nl);
        src->lazy = id();
        
		if(l <= nl && nr <= r) { // 包含されている
			dst->data = src->data;
			dst->lazy = src->lazy;
			dst->left = src->left;
			dst->right = src->right;
		}
		else if(l < nr && nl < r) { // 重なっている
			size_t mid = (nl + nr) / 2;
			copy_impl(src->left, dst->left, l, r, nl, mid);
			copy_impl(src->right, dst->right, l, r, mid, nr);
            dst->update();
			src->update();
		}
		
		return dst;
	}
    
};

/*
https://judge.yosupo.jp/problem/persistent_range_affine_range_sum

using mint = long long;
constexpr mint MOD = 998244353;

struct RangeSumMonoid {
    using value_type = mint;
    static value_type e() { return 0; }
    static value_type op(const value_type& a, const value_type& b) { return (a + b) % MOD; }
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

struct RangeAffineRangeSumMonoid {
	using data_monoid = RangeSumMonoid;
	using lazy_monoid = AffineMonoid<mint>;
	
	static data_monoid::value_type mapping(lazy_monoid::value_type f, data_monoid::value_type x) {
        return (f.first * x + f.second) % MOD;
    }
	static lazy_monoid::value_type power(lazy_monoid::value_type f, size_t p) {
		return {f.first, (f.second * p) % MOD};
	}
};

#include <iostream>
using namespace std;

int main() {
	int N, Q; cin >> N >> Q;
	
	PersistentLazySegTree<RangeAffineRangeSumMonoid> seg;
	for(int i = 0; i < N; ++i) {
		int a; cin >> a;
		seg = seg.set(i, a);
	}
	
	unordered_map<int, PersistentLazySegTree<RangeAffineRangeSumMonoid>> segs;
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
			auto [tree, ans] = segs[k].prod(l, r);
            segs[i] = tree;
            cout << ans << '\n';
		}
	}
}
*/
