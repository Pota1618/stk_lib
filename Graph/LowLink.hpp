#include <vector>
#include <set>

using namespace std;

class LowLink {
public:
	struct Edge {
		int to, id;
		Edge(int to, int id = -1) : to(to), id(id) {}
	};
protected:
	vector<int> ord, low, par;
	vector<vector<Edge>> G;
	vector<pair<int, int>> E;
	int N, M;
	vector<bool> is_articulation;
	vector<bool> is_bridge;
	set<pair<int, int>> bridges;

	int num_connected_components;

public:
	LowLink(int N) : N(N), M(0), ord(N, -1), low(N), par(N, -1), G(N), is_articulation(N), is_bridge(), E(), num_connected_components(0) {}

	// 無向グラフしか想定してないので、一方向入れればよいです。
	void AddEdge(int u, int v) {
		G[u].emplace_back(v, M);
		G[v].emplace_back(u, M++);
		E.emplace_back(min(u, v), max(u, v));
	}

	virtual void Build() {
		is_bridge.resize(M);
		int n = 0;
		for (int i = 0; i < N; ++i) {
			if (ord[i] == -1) {
				++num_connected_components;
				dfs(i, -1, -1, n);
			}
		}
	}

	bool IsArticulationPoint(int v) const { return is_articulation[v]; }
	bool IsBridgeByID(int id) const { return is_bridge[id]; }
	bool IsBridge(const pair<int, int>& e) { return bridges.count(e) != 0; }

public:
	pair<int, int> GetEdge(int id) const { return E[id]; }
	const int NumComps() const { return num_connected_components; }

public:
	const vector<Edge>& operator[](int v) { return G[v]; }

private:
	// 多重辺対応のため最後に見たエッジを追加
	int dfs(int v, int p, int last_edge_id, int& num) {
		low[v] = ord[v] = num++;
		par[v] = p;
		int n_children = 0;
		for (auto e : G[v]) {
			int n = e.to, id = e.id;
			if (ord[n] == -1) {
				low[v] = min(low[v], dfs(n, v, e.id, num));

				n_children += 1;
				if (par[v] != -1 && ord[v] <= low[n]) {
					is_articulation[v] = true;
				}

				if (ord[v] < low[n]) {
					is_bridge[id] = true;
					bridges.emplace(min(v, n), max(v, n));
				}
			}
			else if (e.id != last_edge_id) { // 頂点番号の代わりに辺を見ることで多重辺対応
				low[v] = min(low[v], ord[n]);
			}
		}
		if (par[v] == -1 && n_children >= 2) is_articulation[v] = true;
		return low[v];
	}
};
