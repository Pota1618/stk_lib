#include <iostream>
#include <unordered_map>
#include <cstdint>
#include <numeric>
#include <bit>
#include <utility>

using namespace std;
using ll = long long;

int64_t CeilSqrt(int64_t x) {
	int64_t left = 0, right = x;
	while(right - left > 1) {
		auto mid = (left + right) / 2;
		// mid*mid < x を判定している
		if(mid < (x + mid - 1) / mid) left = mid;
		else right = mid;
	}
	return right;
}

// https://qiita.com/sesame0224/items/f2ac77c367f588c0d29d
uint64_t ModInv(int64_t a, int64_t m) {
    int64_t b = m, u = 1, v = 0;
    while (b) {
        int64_t t = a / b;
        a -= t * b; swap(a, b);
        u -= t * v; swap(u, v);
    }
    u %= m; 
    if (u < 0) u += m;
    return u;
}

uint64_t ModPow(const uint64_t x, int64_t k, const uint64_t M) {
	if(k < 0) return ModPow(ModInv(x, M), -k, M);
	uint64_t res = 1, a = x;
	while(k) {
		if(k & 1) res = (res * a) % M;
		a = (a * a) % M;
		k >>= 1;
	}
	return res;
}

/**
 * @brief
 * gcd(X,M) == 1 のときだけ使える離散対数
 * 
 * @return 
 * X^K == Y (mod M) となるような最小の K
 * 
 * 存在しない場合は -1
 */
int64_t DescreteLogCoprimeMod(int64_t X, int64_t Y, const uint64_t M) {
	X = ((X % M) + M) % M;
	Y = ((Y % M) + M) % M;
	if(X == 0) {
		if(Y == 1) return 0;
		else if(Y == 0) return 1;
		else return -1;
	}
	uint64_t p = CeilSqrt(M);
	
	unordered_map<uint64_t, uint64_t> mp;
	uint64_t pow_x = 1;
	for(uint64_t j = 0; j < p; ++j) {
		// 存在しない場合にのみ構築されるので、pow_x が同じ場合に上書きはされない。
		mp.emplace(pow_x, j);
		pow_x = (pow_x * X) % M;
	}
	
	uint64_t ya = Y;
	uint64_t a = ModPow(X, -p, M);
	for(uint64_t i = 0; i < p; ++i) {
		if(mp.count(ya)) {
			auto j = mp[ya];
			return p * i + j;
		}
		ya = (ya * a) % M;
	}
	return -1;
}

/**
 * @brief
 * 任意 mod 離散対数
 * 
 * https://qiita.com/suisen_cp/items/d597c8ec576ae32ee2d7
 * @return 
 * X^K == Y (mod M) となるような最小の K
 * 
 * 存在しない場合は -1
 */
int64_t DescreteLog(int64_t X, int64_t Y, const uint64_t M) {
	if(M == 1) return 0;
	
	X = ((X % M) + M) % M;
	Y = ((Y % M) + M) % M;
	
	int d = bit_width(M);
	int64_t pow_x = 1;
	for(int i = 0; i < d; ++i) {
		if(pow_x == Y) return i;
		pow_x = (pow_x * X) % M;
	}
	uint64_t g = gcd(pow_x, M);
	
	if(Y % g != 0) return -1;
	
	uint64_t m = M / g;
	uint64_t z = Y * ModPow(pow_x, -1, m);
	int64_t t = DescreteLogCoprimeMod(X, z, m);
	return (t == -1 ? t : d + t);
}

void LibChecker_DescreteLog() {
	// cout << DescreteLog(5, 9, 11) << endl;
	int T; cin >> T;
	while(T--) {
		int64_t x, y, m;
		cin >> x >> y >> m;
		cout << DescreteLog(x, y, m) << endl;
	}
}
