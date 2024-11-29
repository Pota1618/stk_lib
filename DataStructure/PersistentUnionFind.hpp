// https://atcoder.jp/contests/code-thanks-festival-2017/tasks/code_thanks_festival_2017_h
// PersistentArray の枝分かれの数を増やすと、メモリを消費するかわりに速くなる

#include "stk/DataStructure/PersistentArray.hpp"

class PersistentUnionFind {
private:
	PersistentArray<int> m_data;
	size_t m_size;
	int m_num_group;
public:
	explicit PersistentUnionFind(size_t N) : m_data(N, -1), m_size(N), m_num_group((int)N) {}
	
	int leader(int u) const {
		assert(0 <= u && u < (int)m_size);
		if(m_data[u] < 0) return u;
		else return leader(m_data.get(u));
	}
	
	bool same(int u, int v) const { return leader(u) == leader(v); }
	
	// {新しい PUF, マージできたか}
	// 仮にマージできなくても、新しい UF が返される
	pair<PersistentUnionFind, bool> merge(int u, int v) const {
		u = leader(u); v = leader(v);
		if(u == v) return make_pair(PersistentUnionFind(m_data, size(), num_group()), false);
		
		if(-m_data.get(u) < -m_data.get(v)) swap(u, v);
		return make_pair(
			PersistentUnionFind(
				// サイズを足しこんで親を設定している
				m_data.set(u, m_data.get(u) + m_data.get(v)).set(v, u), 
				m_size, 
				m_num_group - 1
			), 
			true
		);
	}
	
	size_t size(int u) const { return -m_data.get(leader(u)); }
	size_t size() const { return m_size; }
	
	int num_group() const { return m_num_group; }
	
private:
	explicit PersistentUnionFind(const PersistentArray<int>& a, size_t size, int num_group)
		: m_data(a), m_size(size), m_num_group(num_group) {}
};
