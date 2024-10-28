#include <algorithm>
#include <vector>

using namespace std;

class CentroidDecomposition {
private:
    int N;
    vector<vector<int>> G;
    vector<int> sz, par;
    vector<bool> used;
    
    void calc_size(int u, int p) {
        sz[u] = 1; // 何度も同じ頂点を計算しなおすので初期化しないとダメ
        for(int v : G[u]) if(v != p) {
            if(used[v]) continue;
            
            calc_size(v, u);
            sz[u] += sz[v];
        }
    }
    
    void cd_build(int u, int p) {
        calc_size(u, -1);
        int tot = sz[u];
        bool ok = false;
        int pp = -1;
        
        // find centroid
        while(!ok) {
            ok = true;
            for(int v : G[u]) {
                if(!used[v] && v != pp && 2 * sz[v] > tot) {
                    pp = u; u = v;
                    ok = false;
                    break;
                }
            }
        }
        
        par[u] = p;
        used[u] = true;
        for(int v : G[u]) {
            if(!used[v]) cd_build(v, u);
        }
    }
    
public:
    CentroidDecomposition(int n) : N(n), G(n), sz(n), par(n, -1), used(n) {}
    void add_edge(int u, int v) { G[u].emplace_back(v); G[v].emplace_back(u); }
    
    void build() { cd_build(0, -1); }
    
    int get_par(int v) { return par[v]; }
};
