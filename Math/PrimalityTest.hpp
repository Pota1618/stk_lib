#pragma once

#include <cstdint>
#include <array>

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
	constexpr std::array<uint64_t, 3> A = {2, 7, 61};
	
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

#if defined(__GNUC__)
typedef __int128_t int128_t;
typedef __uint128_t uint128_t;

static inline constexpr uint128_t mul128(uint64_t a, uint64_t b) { return (uint128_t)a * b; }
static inline constexpr uint64_t mod128(uint128_t x, uint64_t d) { return (uint64_t)(x % d); }

constexpr uint64_t K_PowMod_M64(uint64_t x, uint64_t k, uint64_t M) {
	uint64_t res = 1, a = x % M;
	while (k) {
		if (k & 1) res = mod128(mul128(res, a), M);
		a = mod128(mul128(a, a), M);
		k >>= 1;
	}
	return res;
}

constexpr bool K_MillerRabin_64(uint64_t n) {
	if (n <= 1) return false;
	if (n == 2) return true;
	if (n % 2 == 0) return false;

	uint64_t s = 0, d = n - 1;
	while ((d & 1) == 0) {
		++s;
		d >>= 1;
	}
	constexpr std::array<uint64_t, 8> A = { 2, 325, 9375, 28178, 450775, 9780504, 1795265022 };

	for (auto a : A) {
		if (a % n == 0) return true;
		uint64_t x = K_PowMod_M64(a, d, n);
		if (x != 1) {
			uint64_t t = 0;
			for (t = 0; t < s; ++t) {
				if (x == n - 1) break;
				x = mod128(mul128(x, x), n);
			}
			if (t == s) return false;
		}
	}
	return true;
}

template<uint64_t N> constexpr bool K_IsPrime64 = K_MillerRabin_64(N);

#elif defined(_MSC_VER)
// NOT TESTED YET
/*
#include <intrin.h>

typedef struct {
    uint64_t low;
    uint64_t high;
} uint128_t;

static inline constexpr uint128_t mul128(uint64_t a, uint64_t b) {
    uint128_t r;
    r.low = _umul128(a, b, &r.high);
    return r;
}

static inline constexpr uint64_t mod128(uint128_t x, uint64_t divisor) {
    uint64_t remainder;
    _udiv128(x.high, x.low, divisor, &remainder);
    return remainder;
}
*/

#else
#error "No 128-bit integer support on this compiler."
#endif


// テンプレートにしてコンパイル時計算であることをわかりやすくしただけ。
template<uint32_t N> constexpr bool K_IsPrime32 = K_MillerRabin_32(N);

