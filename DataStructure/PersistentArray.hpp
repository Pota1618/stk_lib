// https://judge.yosupo.jp/problem/persistent_queue

#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <cassert>

using namespace std;

/*
完全永続配列。
変更を加えたとき、新しい PersistentArray を返す。
また、どの時点からでも変更が可能。
B 分木によって実現されているので、クエリあたりの時間計算量は O(log_baseB(N))
*/
template<typename T, size_t B = 8>
class PersistentArray {
private:
    struct Node;
    using NodePtr = shared_ptr<Node>;
    struct Node {
        T val;
        array<NodePtr, B> ch;
    };
    
    NodePtr root = nullptr;
public:
    PersistentArray() = default;
    explicit PersistentArray(const vector<T>& v) {
        for(size_t i = 0; i < v.size(); ++i) {
            destructive_set(i, v[i]);
        }
    }
	explicit PersistentArray(size_t n) {
		for(size_t i = 0; i < n; ++i) {
			destructive_set(i, T());
		}
	}
	explicit PersistentArray(size_t n, T val) {
		for(size_t i = 0; i < n; ++i) {
			destructive_set(i, val);
		}
	}
    
    const T& get(size_t k) const { 
		NodePtr cur = root;
		while(k) {
			cur = cur->ch[k % B];
			k /= B;
		}
		return cur->val; 
	}
	
	const T& operator[](size_t k) const { return get(k); }
	
    PersistentArray set(size_t k, const T& x) const {
        NodePtr cur = root ? make_shared<Node>(*root) : make_shared<Node>();
        PersistentArray res(cur);
        while(k) {
            size_t i = k % B;
            cur->ch[i] = cur->ch[i] ? make_shared<Node>(*(cur->ch[i])) : make_shared<Node>();
			cur = cur->ch[i];
			k /= B;
		}
        cur->val = x;
        return res;
    }
    
private:
    void destructive_set(size_t k, const T& x) {
		NodePtr cur = root ? root : make_shared<Node>();
		root = cur;
        while(k) {
			size_t i = k % B;
            if(!cur->ch[i]) cur->ch[i] = make_shared<Node>();
            cur = cur->ch[i];
			k /= B;
        }
        cur->val = x;
    }
    
    explicit PersistentArray(const NodePtr& root) : root(root) {}
};
