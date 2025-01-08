// https://atcoder.jp/contests/abc376/tasks/abc376_b

#include <iostream>
#include <cassert>

using namespace std;
using ll = long long;

/**
 * @brief 長さ N, 0-indexed の円環。
 */
template<typename T = long long>
class CircularGraph {
private:
    T N;
public:
    CircularGraph(T n) : N(n) {}
    
    /**
     * @brief x, y の最短距離。
     */
    T dist(T x, T y) {
        return min(directional_dist<false>(x, y), directional_dist<true>(x, y));
    }
    
    /**
     * @param ccw true ならば、反時計回り（インデックスが減る方向）での距離を計算する。counter clockwise
     */
    template<bool ccw = false>
    T directional_dist(T x, T y) {
        y = (y + N - x) % N;
        x = 0;
        if constexpr(ccw == false) {
            return y;
        }
        else {
            return N - y;
        }
    }
    
    /**
     * @brief `obs` を通れない場合の x, y の距離。
     */
    T dist_obstacle(T x, T y, T obs) {
        assert(x != obs && y != obs);
        if(x == y) return 0;
        
        if(is_ordered_triple<false>(x, obs, y)) {
            return directional_dist<true>(x, y);
        }
        else {
            return directional_dist<false>(x, y);
        }
    }
    
    /**
     * @brief `x`, `m`, `y` が `ccw` で指定された方向に順に並んでいるか。 
     * 
     * @returns
     * x から ccw で指定された向きにスタートして、x, m, y を順に通り、距離の合計が N 未満ならば true
     * 
     * @param ccw true ならば、反時計回り（インデックスが減る方向）での距離を計算する。
     */
    template<bool ccw = false>
    bool is_ordered_triple(T x, T m, T y) {
        return directional_dist<ccw>(x, m) + directional_dist<ccw>(m, y) < N;
    }
};
