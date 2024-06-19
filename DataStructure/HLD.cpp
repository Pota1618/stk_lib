#include <vector>
#include <functional>

using namespace std;

template<typename Edge>
class HLD {
private:
	vector<vector<Edge>> G;
	vector<int> sz, in, out, head, ord, par;
	const function<int(const Edge&)> get_next;
	const function<int(const Edge&)> get_edge_id;
	vector<int> eid2vid; // 辺 id -> その辺の子側の頂点
public:
	HLD(const vector<vector<Edge>>& graph, const function<int(const Edge&)>& get_next_vertex = [](const Edge& e) { return e; }, const function<int(const Edge&)>& get_edge_index = nullptr) :
		G(graph), sz(graph.size()), in(graph.size()), out(graph.size()), head(graph.size()), ord(graph.size()), par(graph.size()), get_next(get_next_vertex), get_edge_id(get_edge_index), eid2vid()
	{
		// 根の heavy-path の head をちゃんと設定すれば任意の根でも構築できるけどあまり意味がないので、とりあえず 0 にしている。
		// int r = abs(rand()) % G.size();
		// head[r] = r;
		build(0);
	}

	int lca(int u, int v) const {
		while (true) {
			if (in[u] > in[v]) swap(u, v); // 同じ heavy-path ならば u の方が浅いことを担保
			if (head[u] == head[v]) return u; // 同じ heavy-path なら浅い方が lca
			v = par[head[v]]; // 深い方をそれがが属する heavy-path の一つ上に更新して次へ
		}
	}

	int kth_ancestor(int v, int k) {
		while (true) {
			int u = head[v];
			if (in[u] + k <= in[v]) return ord[in[v] - k];
			k -= (in[v] - in[u]) + 1;
			v = par[u];
		}
	}
	
	template<typename T>
	T path_prod(int u, int v, const T& te, const function<T(int, int)>& prod_u, const function<T(int, int)>& prod_v, const function<T(T, T)>& op, bool edge = false) const {
		T l = te, r = te;
		bool swapped = false;
		while (true) {
			if (in[u] > in[v]) swap(u, v), swapped = !swapped;
			if (head[u] == head[v]) break;

			if (swapped) r = op(prod_v(in[head[v]], in[v] + 1), r);
			else l = op(l, prod_u(in[head[v]], in[v] + 1));

			v = par[head[v]];
		}
		if (swapped) return op(op(l, prod_v(in[u] + edge, in[v] + 1)), r);
		else return op(op(l, prod_u(in[u] + edge, in[v] + 1)), r);
	}

	template<typename T>
	T path_sum(int u, int v, const T& te, const function<T(int, int)>& sum, const function<T(T, T)>& op, bool edge = false) const {
		T res = te;
		while (true) {
			if (in[u] > in[v]) swap(u, v);
			if (head[u] == head[v]) break;
			res = op(sum(in[head[v]], in[v] + 1), res);
			v = par[head[v]];
		}
		return op(sum(in[u] + edge, in[v] + 1), res);
	}

	void path_update(int u, int v, const function<void(int, int)>& func, bool edge = false) {
		while (true) {
			if (in[u] > in[v]) swap(u, v);
			if (head[u] == head[v]) break;
			func(in[head[v]], in[v] + 1);
			v = par[head[v]];
		}
		func(in[u] + edge, in[v] + 1);
	}

	template<typename T>
	T subtree_sum(int v, const function<T(int, int)>& func) const { return func(in[v], out[v]); }
	void subtree_update(int v, const function<void(int, int)>& func) { func(in[v], out[v]); }

	inline int index(int v) const { return in[v]; }
	inline int edge_index(int eid) const { assert(get_edge_id != nullptr); return eid2vid[eid]; }
	inline int subtree_size(int v) const { return sz[v]; }
	inline size_t size() const { return G.size(); }

private:
	void build(int root) {
		dfs_sz(root, -1);
		int t = 0;
		dfs_hld(root, -1, t);
		if (get_edge_id != nullptr) {
			eid2vid.resize(G.size());
			dfs_edge_id(root, -1);
		}
	}
	void dfs_sz(int u, int p) {
		par[u] = p, sz[u] = 1;
		// 親方向の辺は heavy-path になれないので、先頭にある場合は後ろに持っていく
		if (G[u].size() && get_next(G[u][0]) == p) swap(G[u][0], G[u].back());
		for (Edge& e : G[u]) {
			int v = get_next(e);
			if (v == p) continue;
			dfs_sz(v, u);
			sz[u] += sz[v];
			// heavy-path 候補なので先頭へ持っていく
			if (sz[get_next(G[u][0])] < sz[v]) swap(G[u][0], e);
		}
	}
	void dfs_hld(int u, int p, int& t) {
		in[u] = t;
		ord[t] = u; // dfs の行きがけ順で頂点をならべたもの。
		t += 1;
		int u0 = (G[u].size() ? get_next(G[u][0]) : -1); // 頂点が一つしかないときの対策
		for (const Edge& e : G[u]) {
			int v = get_next(e);
			if (v == p) continue;
			// heavy-path を通ってきたなら head は親と同じ
			// そうでなければ新たな heavy-path の先頭になる
			head[v] = (u0 == v ? head[u] : v);
			dfs_hld(v, u, t);
		}
		out[u] = t;
	}
	void dfs_edge_id(int u, int p) {
		for (const Edge& e : G[u]) {
			int v = get_next(e);
			if (v == p) eid2vid[get_edge_id(e)] = in[u]; // 辺のデータを持つべき頂点は、子側の頂点。
			else dfs_edge_id(v, u);
		}
	}
};
