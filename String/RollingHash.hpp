#include <vector>
#include <string>

using namespace std;

// verified:
// https://atcoder.jp/contests/abc284/tasks/abc284_f
// https://atcoder.jp/contests/tenka1-2014-qualb/tasks/tenka1_2014_qualB_b
// https://atcoder.jp/contests/abc141/tasks/abc141_e
// https://qiita.com/keymoon/items/11fac5627672a6d6a9f6

// 2023/09/17 Add a static Hash function for one time hashing.
// 2024/02/22 Add Asull() and AsPair functions for HashData. remove rev from HashData.
// 2024/10/28 Make RollingHash independent from atcoder::modint

class RollingHash {
private:
	using mint1 = uint64_t;
	using mint2 = uint64_t;

	static vector<mint1> P;
	static vector<mint2> Q;
	constexpr static uint32_t MOD1 = 998244353;
	constexpr static uint32_t MOD2 = 1000000007;
	constexpr static uint32_t BASE = 314159; // seed

	string S;
	vector<mint1> HS1, HT1;
	vector<mint2> HS2, HT2;

public:
	struct HashData {
		size_t length;
		mint1 h1;
		mint2 h2;

		HashData(size_t len, mint1 hash1, mint2 hash2) : length(len), h1(hash1% MOD1), h2(hash2% MOD2) {}

		HashData operator+(const HashData& other) {
			return HashData(length + other.length, h1 * P[other.length] + other.h1, h2 * Q[other.length] + other.h2);
		}

		uint64_t Asull() const { return (h1 << 32) + h2; }
		pair<int, int> AsPair() const { return make_pair((int)h1, (int)h2); }

		bool operator==(const HashData& other) const { return length == other.length && h1 == other.h1 && h2 == other.h2; }
		bool operator!=(const HashData& other) const { return !((*this) == other); }
	};

public:
	RollingHash(const string& s) : S(s) {
		HS1.reserve(s.size());
		HS2.reserve(s.size());
		HT1.reserve(s.size());
		HT2.reserve(s.size());
		HS1.emplace_back(0);
		HS2.emplace_back(0);
		HT1.emplace_back(0);
		HT2.emplace_back(0);
		for (size_t i = 0; i < S.size(); ++i) HS1.emplace_back((HS1.back() * BASE + S[i]) % MOD1);
		for (size_t i = 0; i < S.size(); ++i) HS2.emplace_back((HS2.back() * BASE + S[i]) % MOD2);
		for (size_t i = 0; i < S.size(); ++i) HT1.emplace_back((HT1.back() * BASE + S[S.size() - 1 - i]) % MOD1);
		for (size_t i = 0; i < S.size(); ++i) HT2.emplace_back((HT2.back() * BASE + S[S.size() - 1 - i]) % MOD2);
		ResizeF(s.size());
	}

	// [l, r)
	// if l > r then returns reversed hash, which means (r, l].
	HashData Hash(size_t l, size_t r) {
		bool rev = false;
		if (l > r) {
			rev = true;
			// swap(l, r);
			l = S.size() - l;
			r = S.size() - r;
		}
		if (!rev) return HashData(r - l, (HS1[r] + MOD1) - (HS1[l] * P[r - l]) % MOD1, (HS2[r] + MOD2) - (HS2[l] * Q[r - l]) % MOD2);
		else return HashData(r - l, (HT1[r] + MOD1) - (HT1[l] * P[r - l]) % MOD1, (HT2[r] + MOD2) - (HT2[l] * Q[r - l]) % MOD2);
	}

	static HashData Hash(const string& s) {
		mint1 h1 = 0; mint2 h2 = 0;
		for (char c : s) {
			h1 = (h1 * BASE + c) % MOD1;
			h2 = (h2 * BASE + c) % MOD2;
		}
		return HashData(s.size(), h1, h2);
	}

private:
	void ResizeF(size_t N) {
		while (P.size() <= N + 1) P.emplace_back((P.back() * BASE) % MOD1);
		while (Q.size() <= N + 1) Q.emplace_back((Q.back() * BASE) % MOD2);
	}
};

vector<uint64_t> RollingHash::P(1, 1);
vector<uint64_t> RollingHash::Q(1, 1);

/*
int main() {
	string S = "ABCD";
	RollingHash H(S);
	cout << H.Hash(0, 3).h1.val() << endl; // "ABC"
	cout << H.Hash(3, 0).h1.val() << endl; // "CBA"
	cout << H.Hash(3, 3).h1.val() << endl; // ""
}
*/