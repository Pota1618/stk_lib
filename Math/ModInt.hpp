#include <cstdint>
#include <cassert>
#include <numeric>
#include <type_traits>

#include "stk/Math/PrimalityTest.hpp"

template<uint32_t MOD, enable_if_t<(1 <= MOD)>* = nullptr>
class ModInt {
private:
	using i64 = long long;
	using u32 = unsigned int;
	i64 value;
	static constexpr bool is_prime = K_IsPrime<MOD>;
public:
	ModInt() : value(0) {}
	
	template<typename T, typename enable_if<is_integral<T>::value, T>::type* = nullptr>
	explicit ModInt(T x) : value((x % MOD + MOD) % MOD) {}
	
	ModInt(const ModInt& other) : value(other.value) {}
	ModInt& operator=(const ModInt& other) { value = other.value; return *this; }
	
	template<typename T, typename enable_if<is_integral<T>::value, T>::type* = nullptr>
	ModInt& operator=(T x) { value = x; return *this; }
	
	friend ModInt operator+(const ModInt& lhs, const ModInt& rhs) { return ModInt(lhs) += rhs; }
	friend ModInt operator-(const ModInt& lhs, const ModInt& rhs) { return ModInt(lhs) -= rhs; }
	friend ModInt operator*(const ModInt& lhs, const ModInt& rhs) { return ModInt(lhs) *= rhs; }
	friend ModInt operator/(const ModInt& lhs, const ModInt& rhs) { return ModInt(lhs) /= rhs; }
	
	ModInt& operator+=(const ModInt& other) { value = (value + other.value) % MOD; return *this; }
	ModInt& operator-=(const ModInt& other) { value = (value - other.value + MOD) % MOD; return *this; }
	ModInt& operator*=(const ModInt& other) { value = (value * other.value) % MOD; return *this; }
	ModInt& operator/=(const ModInt& other) { return *this *= other.inv(); }
	
	ModInt operator+() const { return *this; }
    ModInt operator-() const { return ModInt() - *this; }
	
	ModInt pow(i64 k) {
		ModInt res(1), a(*this);
		while(k) {
			if(k & 1) res *= a;
			a *= a;
			k >>= 1;
		}
		return res;
	}
	
	ModInt inv() {
		assert(value != 0);
		if constexpr (is_prime) {
			return pow(mod() - 2);
		}
		else {
			assert(gcd((u32)value, mod()) == 1);
			i64 a = value, b = mod(), u = 1, v = 0;
			while (b) {
				i64 t = a / b;
				a -= t * b; swap(a, b);
				u -= t * v; swap(u, v);
			}
			u %= mod();
			if (u < 0) u += mod();
			return ModInt(u);
		}
	}
	
	ModInt& operator++() { 
		++value;
		if(value == mod()) value = 0;
		return *this;
	}
	ModInt& operator--() {
		if (value == 0) value = mod();
        --value;
        return *this;
	}
	ModInt operator++(int) {
		ModInt res = *this;
		++(*this);
		return res;
	}
	ModInt operator--(int) {
		ModInt res = *this;
		--(*this);
		return res;
	}
	
	constexpr static u32 mod() { return MOD; }
	static ModInt raw(u32 x) { ModInt res; res.value = x; return res; }
	u32 val() const { return value; }
};

using ModInt998244353 = ModInt<998244353>;
