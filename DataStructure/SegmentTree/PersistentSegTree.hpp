#include <memory>
#include <functional>
#include <bit>

template <typename T, size_t D = 30>
class PersistentSegTree {
private:
	struct Node;
	using NodePtr = std::shared_ptr<Node>;
	NodePtr root;
	const T e;
	const std::function<T(T,T)> op;
	struct Node {
		T val;
		NodePtr left, right;
		explicit Node(const T& val) : val(val), left(nullptr), right(nullptr) {}
	};
	
public:
	explicit PersistentSegTree() = default;
	explicit PersistentSegTree(const T& e, const std::function<T(T,T)>& op) 
		: root(nullptr), e(e), op(op) {}
	explicit PersistentSegTree(const NodePtr& root, const T& e, const std::function<T(T, T)>& op)
		: root(root), e(e), op(op) {}
		
	const PersistentSegTree set(size_t i, const T& x) const {
		NodePtr cur = root ? make_shared<Node>(*root) : make_shared<Node>(e);
		PersistentSegTree res(cur, e, op);
		
		size_t l = 0, r = 1ull << D;
		std::vector<NodePtr> nodes = {cur};
		while(r - l > 1) {
			nodes.emplace_back(cur);
			size_t mid = (l + r) / 2;
			if(i < mid) {
				r = mid;
				if(cur->left) cur = make_shared<Node>(*(cur->left));
				else cur = make_shared<Node>(e);
				nodes.back()->left = cur;
			}
			else {
				l = mid;
				if(cur->right) cur = make_shared<Node>(*(cur->right));
				else cur = make_shared<Node>(e);
				nodes.back()->right = cur;
			}
		}
		
		cur->val = x;
		
		while(!nodes.empty()) {
			NodePtr cur = nodes.back();
			nodes.pop_back();
			
			// nullptr exeption
			cur->val = op(
				cur->right ? cur->right->val : e, 
				cur->left ? cur->left->val : e
			);
		}
		
		return res;
	}
	
	T prod(size_t l, size_t r) const {
		std::vector<pair<size_t, size_t>> st = {{0ull, 1ull<<D}};
		std::vector<NodePtr> nodes = {root};
		T res = e;
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
