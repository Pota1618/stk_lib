#include <cstdint>
#include <array>

using namespace std;

// uint32 に収まる範囲での MOD で x ^ k を求める。
constexpr uint32_t K_PowMod_M32(uint64_t x, uint64_t k, uint32_t M) {
	uint64_t res = 1, a = x % M;
	while(k) {
		if(k & 1) res = (res * a) % M;
		a = (a * a) % M;
		k >>= 1;
	}
	return res;
}

// uint32 に収まる範囲の整数での素数判定。
constexpr bool K_MillerRabin_32(uint32_t n) {
	if(n <= 1) return false;
	if(n == 2) return true;
	if(n % 2 == 0) return false;
	
	uint64_t s = 0, d = n - 1;
	while((d & 1) == 0) {
		++s;
		d >>= 1;
	}
	constexpr array<uint64_t, 3> A = {2, 7, 61};
	
	for(auto a : A) {
		if(n <= a) return true;
		uint64_t x = K_PowMod_M32(a, d, n);
		if(x != 1) {
			uint64_t t = 0;
			for(t = 0; t < s; ++t) {
				if(x == n - 1) break;
				x = (x * x) % n;
			}
			if(t == s) return false;
		}
	}
	return true;
}

// テンプレートにしてコンパイル時計算であることをわかりやすくしただけ。
template<uint32_t n> constexpr bool K_IsPrime = K_MillerRabin_32(n);
