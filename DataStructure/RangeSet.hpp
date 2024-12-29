#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <numeric>
#include <cassert>

using namespace std;

/**
 * 開区間 [l, r) で値の有無を管理する集合。
 * 全ての区間が互いに重ならないようマージしながら管理している。
 */
template<typename T = long long>
class RangeSet {
private:
    map<T, T> mp; // 区間 [l, r) の集合。l で二分探索したいので map でもつ。
    using map_iterator = map<T, T>::iterator;
    constexpr static T INF = numeric_limits<T>::max() / 2;
public:
    RangeSet() : mp() {
        mp[INF] = INF;
        mp[-INF] = -INF;
    }
    
    /**
     * @brief 区間 [l, r) を挿入。必要なら区間同士が重ならないようマージする。
     * @returns 新たに集合に追加された区間の長さの合計。増加量。
     */
    T emplace(T l, T r) {
        assert(l < r);
        map_iterator it = prev(mp.upper_bound(l));
        
        // すでに区間が含まれている場合
        if(it->first <= l && r <= it->second) {
            return T(0);
        }
        if(it->second < l) {
            ++it;
            if(r < it->first) {
                mp[l] = r;
                return r - l;
            }
            --it;
        }
        
        T erased_length(0);
        
        // 左が既存の区間に重なっている場合
        if(it->first <= l && l <= it->second) {
            l = it->first;
            erased_length += it->second - it->first;
            it = mp.erase(it);
        }
        else {
            ++it;
        }
        
        // マージ。実際は先にいらない部分を消してから最後に長いひとつを挿入。
        // 各区間は 1 回ずつしかマージされないので、while は合計で O(区間の個数) 回で抑えられるはず。
        while(it->second < r) {
            erased_length += it->second - it->first;
            it = mp.erase(it);
        }
        if(it->first <= r && r <= it->second) {
            r = it->second;
            erased_length += it->second - it->first;
            mp.erase(it);
        }
        mp[l] = r;
        
        return (r - l) - erased_length;
    }
    
    /**
     * @brief 区間 [l, r) を削除
     * @returns 消した区間の長さの合計。減少量。
     */
    T erase(T l, T r) {
        assert(l < r);
        map_iterator it = prev(mp.upper_bound(l));
        
        if(r <= it->second) {
            if(it->first == l && r == it->second) {
                mp.erase(it);
            }
            else if(it->second == r) {
                it->second = l;
            }
            else if(it->first == l) {
                mp[r] = it->second;
                mp.erase(it);
            }
            else {
                mp[r] = it->second;
                mp[it->first] = l;
            }
            
            return r - l;
        }
        
        T erased_length(0);
        
        if(it->first <= l && l < it->second) {
            erased_length += it->second - l;
            it->second = l;
        }
        ++it;
        
        while(it->second < r) {
            erased_length += it->second - it->first;
            it = mp.erase(it);
        }
        
        if(it->second == r) {
            erased_length += it->second - it->first;
            it = mp.erase(it);
        }
        else if(it->first < r) {
            erased_length += r - it->first;
            mp[r] = it->second;
            mp.erase(it);
        }
        
        return erased_length;
    }
    
    /**
     * @brief 値 x が集合に含まれるか判定する。
     */
    bool contains(T x) {
        auto it = prev(mp.upper_bound(x));
        return (it->first <= x && x < it->second);
    }
    
    T mex(T x) {
        if(!contains(x)) return x;
        else {
            auto it = prev(mp.upper_bound(x));
            return it->second;
        }
    }
    
    void print() {
        for(map_iterator it = ++mp.begin(); it != --mp.end(); ++it) {
            cout << "[" << it->first << ", " << it->second << ") ";
        }
        cout << endl;
    }
};

/*
// https://codeforces.com/contest/915/problem/E
int main() {
    ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int N, Q; cin >> N >> Q;
    int ans = 0;
    RangeSet<int> st;
    ans += st.emplace(1, N + 1);
    while(Q--) {
        int l, r, k; cin >> l >> r >> k;
        if(k == 1) {
            ans -= st.erase(l, r + 1);
        }
        else {
            ans += st.emplace(l, r + 1);
        }
        // st.print();
        cout << ans << endl;
    }
}
*/
