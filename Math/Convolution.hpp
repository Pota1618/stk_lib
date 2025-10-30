#include <cmath>
#include <complex>
#include <vector>
#include <queue>
#include <numbers>
#include <algorithm>

#include "stk/Math/ModInt.hpp"

template<long long MOD, long long root>
class NTT {
private:
	using mint = ModInt<MOD>;
	vector<mint> roots, iroots;
public:
	NTT() {
		long long p = MOD - 1;
		int cnt = 0;
		while(p % 2 == 0) {
			p /= 2;
			cnt += 1;
		}
		mint x = mint(root).pow(p);
		for(int i = 0; i < cnt; ++i) {
			roots.emplace_back(x);
			iroots.emplace_back(x.inv());
			x = x * x;
		}
		reverse(roots.begin(), roots.end());
		reverse(iroots.begin(), iroots.end());
	}
	
	vector<mint> convolution(const vector<mint>& A, const vector<mint>& B) {
		size_t na = A.size(), nb = B.size();
		assert(na != 0 && nb != 0);
		size_t nc = na + nb - 1;
		size_t depth = bit_width(nc);
		size_t n2 = 1ull << depth;
		
		vector<mint> da, db;
		for(size_t i = 0; i < na; ++i) da.emplace_back(A[i]);
		for(size_t i = 0; i < nb; ++i) db.emplace_back(B[i]);
		while(da.size() < n2) da.emplace_back(0);
		while(db.size() < n2) db.emplace_back(0);
		
		da = ntt(da, depth - 1, false);
		db = ntt(db, depth - 1, false);
		
		vector<mint> dc(n2);
		for(size_t i = 0; i < n2; ++i) {
			dc[i] = da[i] * db[i];
		}
		
		dc = ntt(dc, depth - 1, true);
		
		vector<mint> res;
		res.reserve(nc);
		mint in2 = mint(n2).inv();
		for(size_t i = 0; i < nc; ++i) {
			res.emplace_back(dc[i] * in2);
		}
		return res;
	}
private:
	vector<mint> ntt(const vector<mint>& A, int depth, bool inv) {
		const int N = (int)A.size();
		if(N == 1) return A;
		
		vector<mint> even, odd;
		for(int i = 0; i < N; ++i) {
			(i&1 ? odd : even).emplace_back(A[i]);
		}
		
		even = ntt(even, depth - 1, inv);
		odd = ntt(odd, depth - 1, inv);
		
		mint r = (inv ? iroots : roots)[depth];
		
		mint now = mint::raw(1);
		vector<mint> res;
		for(int i = 0; i < N; ++i) {
			res.emplace_back(even[i % (N / 2)] + (now * odd[i % (N / 2)]));
			now *= r;
		}
		return res;
	}
};

vector<complex<double>> fft(const vector<complex<double>>& A, double inv) {
	const int N = (int)A.size();
	
	if(N == 1) {
		return A;
	}
	
	vector<complex<double>> even, odd;
	for(int i = 0; i < N; ++i) {
		(i&1 ? odd : even).emplace_back(A[i]);
	}
	
	even = fft(even, inv);
	odd = fft(odd, inv);
	complex<double> zeta(cos(2 * numbers::pi * inv / N), sin(2 * numbers::pi * inv / N));
	
	vector<complex<double>> res;
	res.reserve(N);
	complex<double> nzeta = 1;
	for(int i = 0; i < N; ++i) {
		res.emplace_back(even[i % (N / 2)] + nzeta * odd[i % (N / 2)]);
		nzeta *= zeta;
	}
	
	return res;
}

template<typename T, enable_if_t<is_integral_v<T>>* = nullptr>
vector<T> convolution(const vector<T>& A, const vector<T>& B) {
	size_t na = A.size(), nb = B.size();
	assert(na != 0 && nb != 0);
	size_t nc = na + nb - 1;
	size_t n2 = 1ull << bit_width(nc);
	
	vector<complex<double>> da, db;
	for(size_t i = 0; i < na; ++i) da.emplace_back(A[i]);
	for(size_t i = 0; i < nb; ++i) db.emplace_back(B[i]);
	while(da.size() < n2) da.emplace_back(0.0);
	while(db.size() < n2) db.emplace_back(0.0);
	
	da = fft(da, 1);
	db = fft(db, 1);
	
	vector<complex<double>> dc(n2);
	for(size_t i = 0; i < n2; ++i) {
		dc[i] = da[i] * db[i];
	}
	
	dc = fft(dc, -1);
	
	vector<T> res;
	res.reserve(nc);
	for(size_t i = 0; i < nc; ++i) {
		res.emplace_back(round(dc[i].real() / n2));
	}
	return res;
}
