#include <vector>
#include <numeric>
#include <algorithm>

// https://judge.yosupo.jp/problem/unionfind_with_potential_non_commutative_group
/*
非可換対応ポテンシャル UnionFind
x * u == v となるように辺を張る。
*/
template<typename Group>
class Potentialized_UnionFind {
private:
    int n_groups;
    vector<Group> vals;
    vector<int> par;
    vector<size_t> sz;
    const Group e;
    const function<Group(Group, Group)> op;
    const function<Group(Group)> inv;
public:
    Potentialized_UnionFind(size_t N, const function<Group(Group, Group)>& op, const function<Group(Group)>& inv, const Group& e)
        : n_groups((int)N), vals(N, e), par(N), sz(N, 1), op(op), inv(inv), e(e)
    {
        iota(par.begin(), par.end(), 0);
    }
    
    // (leader, [leader から v までの合成結果])
    pair<int, Group> leader(int v) {
        Group res = e;
        while(v != par[v]) {
            res = op(vals[v], res);
            res = op(vals[par[v]], res);
            vals[v] = op(vals[par[v]], vals[v]);
            v = par[v] = par[par[v]];
        }
        return make_pair(v, res);
    }
    
    // (issame, u->v パスの合成結果)
    // issame == false の場合は単位元が返る
    pair<bool, Group> same(int u, int v) {
        auto [lu, xu] = leader(u);
        auto [lv, xv] = leader(v);
        
        // 別の連結成分の場合はパスなし
        if(lu != lv) return make_pair(false, e);
        
        // 子から親に向かって、順番に、左から作用させた値を保存している
        // u->v に向かうときは、u が右からの順になってほしい
        // leader は間で相殺されるので、結局パスの総積になっている
        return make_pair(true, op(inv(xv), xu));
    }
    
    // x * from == to となるように辺を張る
    bool merge(int from, int to, Group x) {
        auto [v1, x1] = leader(from);
        auto [v2, x2] = leader(to);
        // すでに正しくマージされているので操作の必要なし
        if(v1 == v2) return false;
        
        if(sz[v1] < sz[v2]) {
            swap(v1, v2);
            swap(x1, x2);
            x = inv(x);
        }
        
        // 元々の v1 の高さを追加
        x = op(x1, x);
        // 元々の v2 の高さを引くと、v2->v1 間の差分がわかる
        x = op(x, inv(x2));
        
        vals[v2] = x;
        par[v2] = v1;
        sz[v1] += sz[v2];
        
        --n_groups;
        
        return true;
    }
};
