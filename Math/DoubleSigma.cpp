#include <vector>
#include <functional>
#include <atcoder/fenwicktree.hpp>

using namespace std;

namespace DoubleSigma {
	template<class T = long long>
	T BruteForce(const vector<T>& A, const function<T(T, T)>& op) {
		T ans = 0;
		for (int j = 1; j < A.size(); ++j) for (int i = 0; i < j; ++i)
			ans += op(A[i], A[j]);
		return ans;
	}

	template<class T = long long>
	T BruteForceAllPairs(const vector<T>& A, const function<T(T, T)>& op) {
		T ans = 0;
		for (int j = 0; j < A.size(); ++j) for (int i = 0; i < A.size(); ++i)
			if (i != j) ans += op(A[i], A[j]);
		return ans;
	}

	template<class T = long long>
	T Sqrt(T K) {
		T left = 0, right = K + 10;
		while (right - left > 1) {
			T mid = (left + right) / 2;
			if (mid <= K / mid) left = mid;
			else right = mid;
		}
		return left;
	}

	// O(N * √maxA * log(maxA))
	template<class T = long long>
	T XmodY(const vector<T>& A) {
		T max_a = *max_element(all(A));
		T sq_a = Sqrt(max_a);

		vector<T> L(sq_a);
		atcoder::fenwick_tree<T> ft((int)max_a + 1);

		T ans = 0;
		for (const T& a : A) {
			if (a < sq_a) {
				ans -= a * L[a];
			}
			else {
				for (T k = 0; k < max_a + 1; k += a) {
					ans -= a * (k / a) * ft.sum((int)k, min<int>(int(k + a), int(max_a + 1)));
				}
			}

			ft.add((int)a, T(1));
			for (T j = 1; j < sq_a; ++j) {
				L[j] += a / j;
			}
		}
		for (int i = 0; i < A.size(); ++i)
			ans += (A.size() - 1 - i) * A[i];

		return ans;
	}

	// O(N + maxA * log(maxA))
	template<class T = long long>
	T FloorAllPairs(const vector<T>& A) {
		const T max_a = *max_element(A.begin(), A.end());
		vector<T> L(max_a + 1);
		for(int i = 0; i < A.size(); ++i) L[A[i]] += 1;
		for(int i = 0; i < max_a; ++i) L[i + 1] += L[i];
		T ans = 0;
		for (int a = 1; a <= max_a; ++a) {
			int c = L[a] - L[a - 1];
			if (c == 0) continue;
			for (int k = a; k <= max_a; k += a) {
				int l = min<int>(k + a - 1, max_a);
				ans += c * T(k / a) * (L[l] - L[k - 1]);
			}
		}
		ans -= A.size();
		return ans;
	}

	// O(N * √maxA * log(maxA))
	template<class T = long long>
	T FloorXoverY(const vector<T>& A) {
		T max_a = *max_element(A.begin(), A.end());
		T sq_a = Sqrt(max_a);

		vector<T> L(sq_a);
		atcoder::fenwick_tree<T> ft((int)max_a + 1);

		T ans = 0;
		for (const T& a : A) {
			if (a < sq_a) {
				ans += L[a];
			}
			else {
				for (T k = 0; k < max_a + 1; k += a) {
					ans += (k / a) * ft.sum((int)k, min<int>(int(k + a), (int)max_a + 1));
				}
			}

			ft.add((int)a, T(1));
			for (int j = 1; j < sq_a; ++j) {
				L[j] += a / j;
			}
		}
		return ans;
	}
} // namespace DoubleSigma

template<typename T>
class DoubleSigmaVerifier {
	int NT, N;
public:
	DoubleSigmaVerifier(int NT, int N)
		: N(N), NT(NT) {}

	bool verify(function<T(const vector<T>&)> bf, function<T(const vector<T>&)> f, string f_name = "The function") {
		for (int i = 0; i < NT; ++i) {
			vector<T> A(N);
			for (auto& a : A) {
				a = rand() % 1000 + 1;
			}
			if (f(A) != bf(A)) {
				for (auto a : A) cout << a << " ";
				cout << endl;
				return false;
			}
		}

		cout << f_name << " seems to be fine!\n";
		return true;
	}

	template<T(*op)(T, T)>
	static T BruteForce(const vector<T>& A) {
		T res = 0;
		for (int i = 0; i < A.size() - 1; ++i) {
			for (int j = i + 1; j < A.size(); ++j) {
				res += op(A[i], A[j]);
			}
		}
		return res;
	}

	template<T(*op)(T, T)>
	static T BruteForceAllPairs(const vector<T>& A) {
		T res = 0;
		for (int i = 0; i < A.size(); ++i) {
			for (int j = 0; j < A.size(); ++j) {
				if (i == j) continue;
				res += op(A[i], A[j]);
			}
		}
		return res;
	}
};

//ll FloorXoverY(ll x, ll y) { return x / y; }
//ll XmodY(ll x, ll y) { return x % y; }
//
//int main() {
//	DoubleSigmaVerifier<ll> dv(100, 1000);
//	dv.verify(dv.BruteForce<FloorXoverY>, DoubleSigma::FloorXoverY<ll>, "XoverY");
//	dv.verify(dv.BruteForceAllPairs<FloorXoverY>, DoubleSigma::FloorAllPairs<ll>, "XoverYAll");
//	dv.verify(dv.BruteForce<XmodY>, DoubleSigma::XmodY<ll>, "XmodY");
//}
