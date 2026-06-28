#include <concepts>
#include <utility>
#include <memory>
#include <cstdint>
#include <cassert>
#include <type_traits>

#include "stk/DataStructure/BinaryTree/BinaryTree.hpp"

template <BinaryTreeNodeConcept Node>
class RandomizedBinarySearchTree {
public:
	using key_type = Node::key_type;
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

template <LazyBinaryTreeNodeConcept Node>
class LazyRandomizedBinarySearchTree {
public:
	using key_type = Node::key_type;
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
			l->right = merge(l->right, r);
			l->update();
			return l;
		}
		else {
			r->propagate();
			r->left = merge(l, r->left);
			r->update();
			return r;
		}
	}
	
	// split tree into [k elements] and [(size(tree) - k) elements]
	static std::pair<node_ptr_type, node_ptr_type> split(node_ptr_type tree, int64_t k) {
		if(tree == nullptr) return std::make_pair(nullptr, nullptr);
		tree->propagate();
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
		int64_t sz = Node::size(t->left);
		
		if(sz > k) return get(t->left, k);
		else if(sz < k) return get(t->right, k - sz - 1);
		else return t;
	}
};

template <typename T> using RBSTSet = BinaryTreeSet<RandomizedBinarySearchTree<BinaryTreeNodeBase<T>>>;
template <typename K, typename V> using RBSTMap = BinaryTreeMap<RandomizedBinarySearchTree<BinaryTreeMapNode<K, V>>>;
template <MapMonoid MM> using RBSTArray = BinaryTreeArray<LazyRandomizedBinarySearchTree<LazyBinaryTreeNode<MM>>>;
