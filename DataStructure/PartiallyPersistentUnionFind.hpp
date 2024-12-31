// https://atcoder.jp/contests/agc002/tasks/agc002_d

#include <iostream>
#include <vector>

using namespace std;

// https://qiita.com/alumite14/items/f4c355720f2a6da88ca5
class PartiallyPersistentUnionFind {
private:
    static constexpr int INF = 1 << 29;
    int cur_time;
    vector<int> par;
    vector<int> merged_time;
    vector<vector<pair<int,int>>> size_history;
public:
    PartiallyPersistentUnionFind(size_t N) : 
        cur_time(0), par(N, -1), merged_time(N, INF), size_history(N, vector<pair<int,int>>(1, make_pair(0, 1))) {}
    
    // O(log N)
    int leader(int x, int t) {
        while(merged_time[x] <= t) {
            x = par[x];
        }
        return x;
    }
    
    // O(1)
    bool merge(int x, int y) {
        cur_time += 1;
        
        int rx = leader(x, cur_time), ry = leader(y, cur_time);
        
        if(rx == ry) {
            return false;
        }
        
        // rx 側が大きくなるようにスワップ
        // サイズをマイナスで管理しているので不等号が逆になっている
        if(par[rx] > par[ry]) {
            swap(rx, ry);
        }
        
        // rx に ry をマージ
        par[rx] += par[ry];
        par[ry] = rx;
        merged_time[ry] = cur_time;
        size_history[rx].emplace_back(cur_time, -par[rx]);
        
        return true;
    }
    
    // O(log N)
    bool same(int x, int y, int t) {
        return leader(x, t) == leader(y, t);
    }
    
    // O(log N)
    int size(int x, int t) {
        int rx = leader(x, t);
        auto it = prev(lower_bound(size_history[rx].begin(), size_history[rx].end(), make_pair(t, INF)));
        return it->second;
    }
};
