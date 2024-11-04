#include <vector>
#include <unordered_map>

using namespace std;

/*
普通の木構造の UnionFind
経路圧縮とマージテク両方を実装しているので
クエリ O(α(N))
*/
class UnionFind {
private:
	// size をマイナスで管理することで、配列を使いまわす
	// サイズはリーダーしか参照しないので、そうでない頂点には親を設定する
	vector<int> m_data;
	int m_num_group;
public:
	UnionFind(size_t N) : m_data(N, -1), m_num_group((int)N) {}

	int leader(int u) {
        assert(0 <= u && u < (int)m_data.size());
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


/*
型と合成方法を与えると勝手にマージしてくれる UnionFind
*/
template<typename Data>
class UnionFind_with_Data : public UnionFind {
private:
    vector<Data> data;
    const function<Data(Data, Data)> op;
public:
    UnionFind_with_Data(const vector<Data>& A, const function<Data(Data, Data)>& op)
    : UnionFind(A.size()), data(A), op(op) {}
    
    bool merge(int u, int v) {
        u = leader(u);
        v = leader(v);
        if(u == v) return false;
        
        Data val = op(data[u], data[v]);
        UnionFind::merge(u, v);
        data[leader(u)] = val;
        
        return true;
    }
    
    Data get(int v) { return data[leader(v)]; }
};


/*
頂点が属する集合の要素を列挙できる UnionFind
木構造ではなく配列をもってマージテクだけしているので
merge はならし O(log N), leader も O(log N)
*/
class UnionFindArray {
private:
    int n_groups;
    vector<vector<int>> A;
public:
    UnionFindArray(size_t N) : ngroups((int)N), A(N, vector<int>(1)) {
        for(int i = 0; i < N; ++i) A[i][0] = i;
    }
    
    int leader(int v) {
        assert(0 <= v && v < A.size());
        if(A[v].front() == v) return v;
        // マージする前に経路圧縮すると配列が変わってしまうのでしてない
        return leader(A[v].front());
    }
    
    bool same(int u, int v) {
        return leader(u) == leader(v);
    }
    
    bool merge(int u, int v) {
        u = leader(u); v = leader(v);
        if(u == v) return false;
        
        if(A[u].size() < A[v].size()) swap(u, v);
        
        // A[u] の後ろに A[v] を追加
        A[u].insert(A[u].end(), A[v].begin(), A[v].end());
        
        A[v].clear();
        A[v].emplace_back(u);
        
        --n_groups;
        return true;
    }
    
    vector<int> group(int v) {
        return A[leader(v)];
    }
    size_t size() const {
        return A.size();
    }
    size_t size(int v) {
        return A[leader(v)].size();
    }
};
