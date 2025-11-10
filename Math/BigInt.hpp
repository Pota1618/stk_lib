#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <cassert>
#include <algorithm>

class BigInt {
private:
	int sign;
	std::vector<int> digits;
public:
	BigInt() : digits(1, 0), sign(1) {}
	BigInt(const std::string& n) : sign(1) {
		assert(0 < n.size());
		digits.resize(n.size());
		for (size_t i = 0; i < n.size(); ++i) {
			digits[n.size() - i - 1] = n[i] - '0';
		}
		if (n[0] == '-') {
			assert(1 < n.size());
			digits.pop_back();
			sign = -1;
		}
	}

	std::string to_string() const {
		std::string res;
		if (sign == -1) res += '-';
		for (size_t i = 0; i < digits.size(); ++i) {
			res += digits[digits.size() - i - 1] + '0';
		}
		return res;
	}

	bool operator<(const BigInt& other) const {
		if (this->sign != other.sign) 
			return this->sign == -1;
		
		size_t N = this->len(), M = other.len();
		if (this->sign == 1) {
			if (N != M)
				return N < M;
			
			for (size_t i = 0; i < N; ++i) {
				if (this->digits[i] != other.digits[i]) {
					return this->digits[i] < other.digits[i];
				}
			}
		}
		else {
			if (N != M)
				return N > M;

			for (size_t i = 0; i < N; ++i) {
				if (this->digits[i] != other.digits[i]) {
					return this->digits[i] > other.digits[i];
				}
			}
		}

		return false;
	}

	bool operator==(const BigInt& other) const {
		return this->sign == other.sign && this->digits == other.digits;
	}
	bool operator!=(const BigInt& other) const { return !(*this == other); }
	bool operator<=(const BigInt& other) const { return *this < other || *this == other; }
	bool operator>(const BigInt& other) const { return !(*this <= other); }
	bool operator>=(const BigInt& other) const { return !(*this < other); }

	size_t len() const { return digits.size(); }

	BigInt operator+(const BigInt& other) const {
		size_t N = std::max(digits.size(), other.digits.size());
		std::vector<int> ds(N);
		for (size_t i = 0; i < N; ++i) {
			ds[i] =
				+ (i < digits.size() ? digits[i] : 0) * sign
				+ (i < other.digits.size() ? other.digits[i] : 0) * other.sign;
		}
		BigInt res(1, ds);
		res.fix_carry();
		return res;
	}

	BigInt operator-(const BigInt& other) const {
		size_t N = std::max(digits.size(), other.digits.size());
		std::vector<int> ds(N);
		for (size_t i = 0; i < N; ++i) {
			ds[i] =
				+ (i < digits.size() ? digits[i] : 0) * sign
				- (i < other.digits.size() ? other.digits[i] : 0) * other.sign;
		}
		BigInt res(1, ds);
		res.fix_carry();
		return res;
	}

private:
	BigInt(int s, const std::vector<int>& v) : sign(s), digits(v) {}

	void fix_carry() {
		size_t N = digits.size();

		// 一番上の桁以外の処理
		for (size_t i = 0; i + 1 < N; ++i) {
			// 繰り上がり
			if (digits[i] >= 10) {
				int k = digits[i] / 10;
				digits[i] -= k * 10;
				digits[i + 1] += k;
			}
			// 繰り下がり
			if (digits[i] < 0) {
				int k = (-digits[i] - 1) / 10 + 1;
				digits[i] += k * 10;
				digits[i + 1] -= k;
			}
		}

		// 一番上の桁が大きすぎる場合の処理
		while (digits.back() >= 10) {
			int k = digits.back() / 10;
			digits.back() -= k * 10;
			digits.emplace_back(k);
		}

		// リーディングゼロの削除
		while (digits.size() >= 2 && digits.back() == 0) {
			digits.pop_back();
		}

		// 絶対値が負になっている場合
		if (digits.back() < 0) {
			sign *= -1;
			N = digits.size();

			if(N == 1) {
				digits.front() *= -1;
			}
			else {
				digits.back() = -digits.back() - 1;
				digits.front() = 10 - digits.front();
				for (size_t i = 1; i + 1 < N; ++i) {
					digits[i] = 10 - digits[i] - 1;
				}
			}

			fix_carry();
		}

		// 0 は + にしておく。念のため。
		if (digits.size() == 1 && digits.front() == 0) {
			sign = 1;
		}
	}
};

inline std::string to_string(const BigInt& val) {
	return val.to_string();
}
