#include <memory>
#include <functional>
#include <bit>
#include <vector>

#include "stk/Math/Monoid.hpp"

template <Monoid M, size_t D = 30>
class PersistentSegTree {
private:
	struct Node;
	using value_type = M::value_type;
	using node_ptr_type = std::shared_ptr<Node>;
	node_ptr_type root;
	inline static constexpr auto e = M::e;
	inline static constexpr auto op = M::op;
	struct Node {
		value_type val;
		node_ptr_type left, right;
		explicit Node(const value_type& val) : val(val), left(nullptr), right(nullptr) {}
	};
	
public:
	explicit PersistentSegTree() : root(nullptr) {}
	explicit PersistentSegTree(const node_ptr_type& root) : root(root) {}
		
	const PersistentSegTree set(size_t i, const value_type& x) const {
		node_ptr_type cur = root ? std::make_shared<Node>(*root) : std::make_shared<Node>(e());
		PersistentSegTree res(cur);
		
		size_t l = 0, r = 1ull << D;
		std::vector<node_ptr_type> nodes = {cur};
		while(r - l > 1) {
			nodes.emplace_back(cur);
			size_t mid = (l + r) / 2;
			if(i < mid) {
				r = mid;
				if(cur->left) cur = std::make_shared<Node>(*(cur->left));
				else cur = std::make_shared<Node>(e());
				nodes.back()->left = cur;
			}
			else {
				l = mid;
				if(cur->right) cur = std::make_shared<Node>(*(cur->right));
				else cur = std::make_shared<Node>(e());
				nodes.back()->right = cur;
			}
		}
		
		cur->val = x;
		
		while(!nodes.empty()) {
			node_ptr_type cur = nodes.back();
			nodes.pop_back();
			
			// nullptr exeption
			cur->val = op(
				cur->right ? cur->right->val : e(), 
				cur->left ? cur->left->val : e()
			);
		}
		
		return res;
	}
	
	value_type prod(size_t l, size_t r) const {
		std::vector<std::pair<size_t, size_t>> st = {{0ull, 1ull<<D}};
		std::vector<node_ptr_type> nodes = {root};
		value_type res = e();
		while(!st.empty()) {
			auto [nl, nr] = st.back(); st.pop_back();
			auto node = nodes.back(); nodes.pop_back();
			
			if(!node) continue;
			
			if(l <= nl && nr <= r) {
				res = op(node->val, res);
			}
			else if(nr <= l || r <= nl) {
				continue;
			}
			else {
				size_t nm = (nl + nr) / 2;
				st.emplace_back(nm, nr);
				nodes.emplace_back(node->right);
				st.emplace_back(nl, nm);
				nodes.emplace_back(node->left);
			}
		}
		return res;
	}
};
