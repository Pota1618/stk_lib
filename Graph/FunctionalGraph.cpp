#include <vector>
#include <queue>
#include <string>
using namespace std;

class FunctionalGraph {
private:
	vector<int> G;
	vector<vector<int>> H;
	int N;
	vector<vector<int>> cycles;
	vector<int> dist_to_cycles;
	vector<int> closest_v_on_cycle; // returns id of a vertex on a cycle.
	vector<int> cluster;
	vector<vector<int>> dp;

public:
	FunctionalGraph(vector<int>& A) : G(A), N((int)A.size()) { Init(); }

	// obsolate
	int MoveSingle(int v, unsigned long long k) const {
		unsigned long long mn = min((unsigned long long)dist_to_cycles[v], k);
		for (int i = 0; i < (int)mn; ++i) v = G[v];
		k -= mn;
		if (k == 0) return v;

		unsigned long long cycle_size = cycles[cluster[v]].size();
		k %= cycle_size;
		for (int i = 0; i < (int)k; ++i) v = G[v];
		return v;
	}

	int Move(int v, unsigned long long k) const {
		for (int i = 0; i < 64; ++i) if (k >> i & 1) v = dp[i][v];
		return v;
	}

	// still O(log N), but for fairly large numbers (ABC 030 D)
	int Move(int v, const string& S) const {
		if (S.size() <= 15) {
			return Move(v, stoull(S));
		}
		else {
			// unsigned types has a higher opinion than signed in terms of implicit conversion
			// https://prepinsta.com/c-plus-plus/implicit-type-conversion-in-cpp/
			long long mod = cycles[cluster[v]].size();
			long long m = 0;
			for (char c : S) {
				m = (m * 10ll + (c - '0')) % mod;
			}
			m -= DistToCycle(v);
			m = (m % mod + mod) % mod;
			return Move(closest_v_on_cycle[v], m);
		}
	}

	int operator[](int v) { return G[v]; }

	int ClusterID(int v) const { return cluster[v]; }
	// returns the closest vertex id on the closest cycle
	int ClosestCycle(int v) const { return closest_v_on_cycle[v]; }
	bool OnCycle(int v) const { return DistToCycle(v) == 0; }
	int DistToCycle(int v) const { return dist_to_cycles[v]; }
	const vector<int>& DistToCycle() const { return dist_to_cycles; }
	const vector<vector<int>>& Cycles() const { return cycles; }

private:
	void Init() {
		H.resize(N);
		for (int i = 0; i < N; ++i) H[G[i]].emplace_back(i);
		CalcCycles();
		CalcCyclesInfo();
		CalcCluster();
		CalcDoubling();
	}

	void CalcCycles() {
		vector<int> seen(N, -1);
		vector<bool> pushed(N, false);

		for (int i = 0; i < N; ++i) {
			int v = i;
			while (true) {
				seen[v] = i;
				v = G[v];
				if (seen[v] != -1) break;
			}
			if (seen[v] != i || pushed[v]) continue;
			cycles.emplace_back();
			int s = v;
			while (true) {
				cycles.back().emplace_back(v);
				pushed[v] = true;
				v = G[v];
				if (v == s) break;
			}
		}
	}

	// calc dist_to_cycles and closest_cycle
	void CalcCyclesInfo() {
		dist_to_cycles.assign(N, -1);
		closest_v_on_cycle.assign(N, -1);
		queue<int> que;
		for (vector<int>& C : cycles) {
			for (int v : C) {
				dist_to_cycles[v] = 0;
				closest_v_on_cycle[v] = v;
				que.emplace(v);
			}
		}
		while (!que.empty()) {
			int v = que.front(); que.pop();
			for (int n : H[v]) {
				if (dist_to_cycles[n] != -1) continue;
				dist_to_cycles[n] = dist_to_cycles[v] + 1;
				closest_v_on_cycle[n] = closest_v_on_cycle[v];
				que.push(n);
			}
		}
	}

	void CalcCluster() {
		cluster.assign(N, -1);
		queue<int> que;
		for (int i = 0; i < (int)cycles.size(); ++i) {
			for (int v : cycles[i]) {
				cluster[v] = i;
				que.emplace(v);
			}
		}
		while (!que.empty()) {
			int v = que.front(); que.pop();
			for (int n : H[v]) {
				if (cluster[n] != -1) continue;
				cluster[n] = cluster[v];
				que.push(n);
			}
		}
	}
	void CalcDoubling() {
		dp.resize(64);
		dp[0] = G;
		for (int i = 0; i < 63; ++i) {
			dp[i + 1].resize(N);
			for (int j = 0; j < N; ++j) dp[i + 1][j] = dp[i][dp[i][j]];
		}
	}
};