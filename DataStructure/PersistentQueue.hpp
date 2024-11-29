// https://judge.yosupo.jp/problem/persistent_queue

#include "stk/DataStructure/PersistentArray.hpp"

/*
完全永続キュー。
変更を加えたとき、新しい PersistentQueue を返す。
また、どの時点からでも変更が可能。
内部に完全永続配列を使っている。ただ横にスライドしているだけ。
*/
template<typename T>
class PersistentQueue {
private:
    PersistentArray<T> pa;
    size_t left, right;
public:
    PersistentQueue() : pa(), left(0), right(0) {}
    
    size_t size() const { return right - left; }
    bool empty() const { return size() == 0; }
    
    const T& front() const {
        assert(!empty());
        return pa.get(left);
    }
    const T& back() const {
        assert(!empty());
        return pa.get(right - 1);
    }
    const T& get(size_t i) const {
        return pa.get(left + i);
    }
    
    PersistentQueue push(const T& val) const {
        return PersistentQueue(pa.set(right, val), left, right + 1);
    }
    PersistentQueue pop() const {
        assert(!empty());
        return PersistentQueue(pa, left + 1, right);
    }
    
private:
    PersistentQueue(const PersistentArray<T>& pa, size_t left, size_t right) 
        : pa(pa), left(left), right(right) {}
};
