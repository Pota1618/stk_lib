#include <vector>
#include <unordered_map>

using namespace std;

class UnionFind {
private:
	// size をマイナスで管理することで、配列を使いまわす
	// サイズはリーダーしか参照しないので、そうでない頂点には親を設定する
	vector<int> m_data;
	int m_num_group;
public:
	UnionFind(size_t N) : m_data(N, -1), m_num_group((int)N) {}

	int leader(int u) {
		if (m_data[u] < 0) return u;
		else return m_data[u] = leader(m_data[u]);
	}

	bool same(int u, int v) { return leader(u) == leader(v); }

	bool merge(int u, int v) {
		u = leader(u); v = leader(v);
		if (u == v) return false;

		if (-m_data[u] < -m_data[v]) swap(u, v);
		m_data[u] += m_data[v]; // 上書きされる前にサイズを更新
		m_data[v] = u; // もう子のサイズは使わないので、親を設定

		--m_num_group;

		return true;
	}

	size_t size(int u) { return -m_data[leader(u)]; }
	size_t size() const { return m_data.size(); }

	int num_group() const { return m_num_group; }

	vector<vector<int>> groups() {
		unordered_map<int, int> leader_ids;
		for (int i = 0; i < (int)m_data.size(); ++i) {
			if (leader(i) == i) leader_ids[i] = (int)leader_ids.size();
		}
		vector<vector<int>> result(leader_ids.size());
		for (int i = 0; i < (int)m_data.size(); ++i) {
			result[leader_ids[leader(i)]].emplace_back(i);
		}
		return result;
	}
};