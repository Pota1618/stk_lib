// https://atcoder.jp/contests/abc330/tasks/abc330_e
// https://atcoder.jp/contests/abc272/tasks/abc272_e

#include <vector>
#include <set>

using namespace std;

class Mex {
	int N;
	set<int> st;
	vector<int> cnt;
public:
	Mex(int max_n) : N(max_n), cnt(N + 1) {
		for (int i = 0; i <= N; ++i) st.emplace(i);
	}

	void insert(int x) {
		if (x < 0 || N < x) return;
		cnt[x]++;
		st.erase(x);
	}

	void erase(int x) {
		if (x < 0 || N < x) return;
		if (cnt[x] == 0) return;
		cnt[x]--;
		if (cnt[x] == 0) st.insert(x);
	}

	int operator()() const { return *st.begin(); }
};
