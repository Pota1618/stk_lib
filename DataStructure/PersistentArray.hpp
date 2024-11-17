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
template<typename T, size_t B = 2>
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
            root = push(root, i, v[i]);
        }
    }
    
    const T& get(size_t k) const { return get(root, k); }
    
    template<typename... Ts>
    PersistentArray emplace(size_t k, Ts&&... args) const {
        return PersistentArray(emplace(root, k, forward<Ts>(args)...));
    }
    PersistentArray push(size_t k, const T& x) const {
        return PersistentArray(push(root, k, x));
    }
    
private:
    explicit PersistentArray(const NodePtr& root) : root(root) {}
    
    const T& get(const NodePtr& t, size_t k) const {
        if(k == 0) return t->val;
        return get(t->ch[k % B], k / B);
    }
    
    template<typename... Ts>
    NodePtr emplace(const NodePtr& t, size_t k, Ts&&... args) const {
        NodePtr res = t ? make_shared<Node>(*t) : make_shared<Node>();
        
        if(k == 0) {
            res->val = move(T(forward<Ts>(args)...));
        }
        else {
            res->ch[k % B] = emplace(res->ch[k % B], k / B, forward<Ts>(args)...);
        }
        return res;
    }
    
    NodePtr push(const NodePtr& t, size_t k, const T& x) const {
        NodePtr res = t ? make_shared<Node>(*t) : make_shared<Node>();
        
        if(k == 0) {
            res->val = x;
        }
        else {
            res->ch[k % B] = push(res->ch[k % B], k / B, x);
        }
        return res;
    }
};
