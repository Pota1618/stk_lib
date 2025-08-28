#include <vector>

using namespace std;

// https://judge.yosupo.jp/problem/cartesian_tree
// https://drken1215.hatenablog.com/entry/2023/10/19/025800
template<typename T>
struct CartesianTree {
	int root;
	vector<int> par;

	CartesianTree(const vector<T>& A) : root(-1), par(A.size(), -1) {
		int N = (int)A.size();
		vector<int> st;
		st.reserve(N);

		for (int i = 0; i < N; ++i) {
			int prv = -1;
			while (!st.empty() && A[i] < A[st.back()]) {
				prv = st.back();
				st.pop_back();
			}

			if (prv != -1) par[prv] = i;
			if (!st.empty()) par[i] = st.back();

			st.push_back(i);
		}

		for (int i = 0; i < N; ++i) {
			if (par[i] == -1) {
				root = i;
				break;
			}
		}
	}
};