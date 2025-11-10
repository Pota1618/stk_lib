#pragma once

#include <cmath>
#include <complex>
#include <vector>
#include <queue>
#include <numbers>
#include <algorithm>

#include "../Math/ModInt.hpp"

template<long long MOD, long long root>
class NTT {
private:
	using mint = ModInt<MOD>;
	std::vector<mint> roots, iroots;
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
	
	std::vector<mint> convolution(const std::vector<mint>& A, const std::vector<mint>& B) {
		size_t na = A.size(), nb = B.size();
		assert(na != 0 && nb != 0);
		size_t nc = na + nb - 1;
		size_t depth = bit_width(nc);
		size_t n2 = 1ull << depth;
		
		std::vector<mint> da, db;
		for(size_t i = 0; i < na; ++i) da.emplace_back(A[i]);
		for(size_t i = 0; i < nb; ++i) db.emplace_back(B[i]);
		while(da.size() < n2) da.emplace_back(0);
		while(db.size() < n2) db.emplace_back(0);
		
		da = ntt(da, depth - 1, false);
		db = ntt(db, depth - 1, false);
		
		std::vector<mint> dc(n2);
		for(size_t i = 0; i < n2; ++i) {
			dc[i] = da[i] * db[i];
		}
		
		dc = ntt(dc, depth - 1, true);
		
		std::vector<mint> res;
		res.reserve(nc);
		mint in2 = mint(n2).inv();
		for(size_t i = 0; i < nc; ++i) {
			res.emplace_back(dc[i] * in2);
		}
		return res;
	}
private:
	std::vector<mint> ntt(const std::vector<mint>& A, int depth, bool inv) {
		const int N = (int)A.size();
		if(N == 1) return A;
		
		std::vector<mint> even, odd;
		for(int i = 0; i < N; ++i) {
			(i&1 ? odd : even).emplace_back(A[i]);
		}
		
		even = ntt(even, depth - 1, inv);
		odd = ntt(odd, depth - 1, inv);
		
		mint r = (inv ? iroots : roots)[depth];
		
		mint now = mint::raw(1);
		std::vector<mint> res;
		for(int i = 0; i < N; ++i) {
			res.emplace_back(even[i % (N / 2)] + (now * odd[i % (N / 2)]));
			now *= r;
		}
		return res;
	}
};

std::vector<std::complex<double>> fft(const std::vector<std::complex<double>>& A, double inv) {
	const int N = (int)A.size();
	
	if(N == 1) {
		return A;
	}
	
	std::vector<std::complex<double>> even, odd;
	for(int i = 0; i < N; ++i) {
		(i&1 ? odd : even).emplace_back(A[i]);
	}
	
	even = fft(even, inv);
	odd = fft(odd, inv);
	std::complex<double> zeta(cos(2 * std::numbers::pi * inv / N), sin(2 * std::numbers::pi * inv / N));
	
	std::vector<std::complex<double>> res;
	res.reserve(N);
	std::complex<double> nzeta = 1;
	for(int i = 0; i < N; ++i) {
		res.emplace_back(even[i % (N / 2)] + nzeta * odd[i % (N / 2)]);
		nzeta *= zeta;
	}
	
	return res;
}

template<typename T, std::enable_if_t<std::is_integral_v<T>>* = nullptr>
std::vector<T> convolution(const std::vector<T>& A, const std::vector<T>& B) {
	size_t na = A.size(), nb = B.size();
	assert(na != 0 && nb != 0);
	size_t nc = na + nb - 1;
	size_t n2 = 1ull << bit_width(nc);
	
	std::vector<std::complex<double>> da, db;
	for(size_t i = 0; i < na; ++i) da.emplace_back(A[i]);
	for(size_t i = 0; i < nb; ++i) db.emplace_back(B[i]);
	while(da.size() < n2) da.emplace_back(0.0);
	while(db.size() < n2) db.emplace_back(0.0);
	
	da = fft(da, 1);
	db = fft(db, 1);
	
	std::vector<std::complex<double>> dc(n2);
	for(size_t i = 0; i < n2; ++i) {
		dc[i] = da[i] * db[i];
	}
	
	dc = fft(dc, -1);
	
	std::vector<T> res;
	res.reserve(nc);
	for(size_t i = 0; i < nc; ++i) {
		res.emplace_back(round(dc[i].real() / n2));
	}
	return res;
}
