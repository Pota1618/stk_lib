#include "stk/Math/Monoid.hpp"

template <MapMonoid MM, size_t D = 30>
class SparseLazySegTree {
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
	SparseLazySegTree() : root(std::make_shared<Node>()) {}
	explicit SparseLazySegTree(const std::vector<data_type>& vec) : root(std::make_shared<Node>()) {
		build(root, vec, 0, 1ull << D);
	}
	
	void apply(size_t l, size_t r, const lazy_type& f) {
		if(l >= r) return;
		apply_impl(root, l, r, 0, 1ull << D, f);
	}
	
	data_type prod(size_t l, size_t r) {
		if(l >= r) return e();
		return prod_impl(root, l, r, 0, 1ull << D);
	}
private:
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

	void propagate(node_ptr node, int len) {
		if(!node->left) node->left = std::make_shared<Node>();
		if(!node->right) node->right = std::make_shared<Node>();
		map(node->left, node->lazy, len/2);
		map(node->right, node->lazy, len/2);
		node->lazy = id();
	}
	
	void map(node_ptr node, const lazy_type& f, int len) {
		node->data = mapping(power(f, len), node->data);
		node->lazy = composition(f, node->lazy);
	}
	
	void update(node_ptr node) {
		node->data = op(
			node->right->data,
			node->left->data
		);
	}
};
