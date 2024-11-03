// Dinic based max flow
// https://github.com/drken1215/algorithm/blob/master/GraphNetworkFlow/max_flow_dinic.cpp
//
// verify
// https://atcoder.jp/contests/abc259/tasks/abc259_g
// https://atcoder.jp/contests/typical90/tasks/typical90_by

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

template<class FLOWTYPE>
struct FlowEdge {
	int rev, from, to;
	FLOWTYPE cap, icap, flow;

	FlowEdge(int r, int f, int t, FLOWTYPE c) : rev(r), from(f), to(t), cap(c), icap(c), flow(0) {}
	void reset() { cap = icap, flow = 0; }

	friend ostream& operator<<(ostream& os, const FlowEdge& e) {
		return os << e.from << "->" << e.to << '(' << e.flow << '/' << e.icap << ')';
	}
};

template<class FLOWTYPE>
class FlowGraph {
protected:
	vector<vector<FlowEdge<FLOWTYPE>>> G;
	vector<pair<int, int>> pos;

	FlowGraph(int N = 0) : G(N) {}
	void init(int N = 0) {
		G.assign(N, FlowEdge<FLOWTYPE>());
		pos.clear();
	}

	FlowEdge<FLOWTYPE>& get_rev_edge(const FlowEdge<FLOWTYPE>& e) {
		if (e.from != e.to) return G[e.to][e.rev];
		else return G[e.to][e.rev + 1];
	}

public:
	vector<FlowEdge<FLOWTYPE>>& operator[](int i) { return G[i]; }
	const vector<FlowEdge<FLOWTYPE>>& operator[](int i) const { return G[i]; }
	size_t size() const { return G.size(); }

	const FlowEdge<FLOWTYPE>& get_edge(int i) const {
		return G[pos[i].first][pos[i].second];
	}
	vector<FlowEdge<FLOWTYPE>> get_edges() const {
		vector<FlowEdge<FLOWTYPE>> edges;
		for (int i = 0; i < (int)pos.size(); ++i) {
			edges.emplace_back(get_edge(i));
		}
		return edges;
	}
	void reset() {
		for (int i = 0; i < (int)G.size(); ++i) {
			for (FlowEdge<FLOWTYPE>& e : G[i]) e.reset();
		}
	}
	void change_edge(FlowEdge<FLOWTYPE>& e, FLOWTYPE new_cap, FLOWTYPE new_flow) {
		FlowEdge<FLOWTYPE>& re = get_rev_edge(e);
		e.cap = new_cap - new_flow, e.icap = new_cap, e.flow = new_flow;
		re.cap = new_flow;
	}

	void add_edge(int from, int to, FLOWTYPE cap) {
		pos.emplace_back(from, (int)G[from].size());
		G[from].emplace_back((int)G[to].size(), from, to, cap);
		G[to].emplace_back((int)G[from].size() - 1, to, from, 0);
	}

	friend ostream& operator<<(ostream& os, const FlowGraph& G) {
		const auto& edges = G.get_edges();
		for (const auto& e : edges) os << e << endl;
		return os;
	}
};

template<class FLOWTYPE>
class DinicGraph : public FlowGraph<FLOWTYPE> {
private:
	int s, t;
	vector<int> level, iter;
public:
	DinicGraph(int N, int s, int t) : FlowGraph<FLOWTYPE>(N), s(s), t(t), level(N, -1), iter(N, 0) {}
	FLOWTYPE flow(FLOWTYPE flow_limit = numeric_limits<FLOWTYPE>::max()) {
		FLOWTYPE current_flow = 0;

		while (current_flow < flow_limit) {
			bfs();
			if (level[t] < 0) break;
			iter.assign((int)iter.size(), 0);
			while (current_flow < flow_limit) {
				FLOWTYPE flow = dfs(s, flow_limit - current_flow);
				if (!flow) break;
				current_flow += flow;
			}
		}
		return current_flow;
	}

private:
	void bfs() {
		level.assign((int)this->G.size(), -1);
		level[s] = 0;
		queue<int> que; que.push(s);
		while (!que.empty()) {
			int v = que.front(); que.pop();
			for (const FlowEdge<FLOWTYPE>& e : this->G[v]) {
				if (level[e.to] < 0 && e.cap > 0) {
					level[e.to] = level[v] + 1;
					if (e.to == t) return;
					que.emplace(e.to);
				}
			}
		}
	}

	FLOWTYPE dfs(int v, FLOWTYPE up_flow) {
		if (v == t) return up_flow;
		FLOWTYPE res_flow = 0;
		for (int& i = iter[v]; i < (int)this->G[v].size(); ++i) {
			FlowEdge<FLOWTYPE>& e = this->G[v][i];
			FlowEdge<FLOWTYPE>& re = this->get_rev_edge(this->G[v][i]);
			if (level[v] >= level[e.to] || e.cap == 0) continue;
			FLOWTYPE flow = dfs(e.to, min(up_flow - res_flow, e.cap));
			if (flow <= 0) continue;
			res_flow += flow;
			e.cap -= flow, e.flow += flow;
			re.cap += flow, re.flow -= flow;
			if (res_flow == up_flow) break;
		}
		return res_flow;
	};
};
