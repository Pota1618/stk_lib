#include <vector>
#include <functional>
using namespace std;

template<typename S>
class Doubling {
private:
	int N;
	vector<int> G;
	vector<S> value;
	function<S(S, S)> op;
	vector<vector<S>> dp;
	vector<vector<int>> nx;

public:
	Doubling(int N, const vector<int>& G, const vector<S>& value, function<S(S, S)> op)
		: N(N), G(G), value(value), op(op)
	{
		nx.resize(64, vector<int>(N));
		dp.resize(64, vector<S>(N));
		for (int i = 0; i < N; ++i) {
			nx[0][i] = G[i];
			dp[0][i] = value[G[i]];
		}
		for (int p = 1; p < 64; ++p) {
			for (int i = 0; i < N; ++i) {
				nx[p][i] = nx[p - 1][nx[p - 1][i]];
				dp[p][i] = op(dp[p - 1][i], dp[p - 1][nx[p - 1][i]]); // 指数が小さい方が左
			}
		}
	}

	S prod(int s, unsigned long long k) const {
		S res = value[s];
		int v = s;
		for (int p = 0; p < 64; ++p) {
			if (k >> p & 1) {
				res = op(res, dp[p][v]); // 指数が小さい方が左
				v = nx[p][v];
			}
		}
		return res;
	}
};