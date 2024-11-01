#include <vector>
#include <cassert>
#include <iostream>
#include <iomanip>

using namespace std;

/*
実行時にサイズを指定できるビットセット
*/
class BitSet {
private:
	using u64 = unsigned long long;
	vector<u64> A;
	size_t N;
public:
	struct Bit {
		u64* bits;
		size_t pos;

		Bit(u64& bit_block, size_t i) : bits(&bit_block), pos(i % 64) {}

		operator bool() const { return bool((*bits) >> pos & 1); }
		bool operator~() const { return (*bits >> pos & 1) == 0; }
		Bit& operator=(bool x) {
			if (x) *bits |= mask(pos);
			else *bits &= ~mask(pos);
			return *this;
		}
		Bit& operator=(const Bit& other) { return *this = bool(other); }
	};
public:
	BitSet(size_t n) : A((n + 63) / 64), N(n) {}

	Bit operator[](size_t i) {
		assert(i < N);
		return Bit(A[i / 64], i);
	}
	bool operator[](size_t i) const {
		assert(i < N);
		return bool(A[i / 64] >> (i % 64) & 1);
	}

	BitSet& operator~() {
		for (u64& a : A) a = ~a;
		return *this;
	}
	BitSet& operator&=(const BitSet& other) {
		assert(size() == other.size());
		for (size_t i = 0; i < A.size(); ++i)
			A[i] &= other.A[i];
		return *this;
	}
	BitSet& operator|=(const BitSet& other) {
		assert(size() == other.size());
		for (size_t i = 0; i < A.size(); ++i)
			A[i] |= other.A[i];
		return *this;
	}
	BitSet& operator^=(const BitSet& other) {
		assert(size() == other.size());
		for (size_t i = 0; i < A.size(); ++i)
			A[i] ^= other.A[i];
		return *this;
	}

	// すべてを 1 にする
	BitSet& set() { for (u64& a : A) a = ~0ull; return *this; }
	// pos 番目の bit を val にする
	BitSet& set(size_t pos, bool val) { operator[](pos) = val; return *this; }
	// 全 bit を 0 にする
	BitSet& reset() { for (u64& a : A) a = 0; return *this; }
	// pos 番目の bit を 0 にする
	BitSet& reset(size_t pos) { operator[](pos) = false; return *this; }
	// 全 bit を反転する
	BitSet& flip() { for (u64& a : A) a ^= ~0; return *this; }
	// pos 番目の bit を反転する
	BitSet& flip(size_t pos) { operator[](pos) = operator[](pos) != true; return *this; }
	// pos 番目が 1 かどうか判定する
	bool test(size_t pos) const { return operator[](pos); }
	// bit の個数
	size_t size() const { return N; }

	const u64* data() const { return A.data(); }

	u64 get_block(size_t pos) const { return A[pos]; }
	void set_block(size_t pos, u64 val) { A[pos] = val; }

private:
	static inline u64 mask(size_t pos) { return 1ull << pos; }
};

BitSet operator&(const BitSet& a, const BitSet& b) {
	assert(a.size() == b.size());
	size_t sz = (a.size() + 63) / 64;
	BitSet res(a.size());
	for (size_t i = 0; i < sz; ++i)
		res.set_block(i, a.get_block(i) & b.get_block(i));
	return res;
}

BitSet operator|(const BitSet& a, const BitSet& b) {
	assert(a.size() == b.size());
	size_t sz = (a.size() + 63) / 64;
	BitSet res(a.size());
	for (size_t i = 0; i < sz; ++i)
		res.set_block(i, a.get_block(i) | b.get_block(i));
	return res;
}

BitSet operator^(const BitSet& a, const BitSet& b) {
	assert(a.size() == b.size());
	size_t sz = (a.size() + 63) / 64;
	BitSet res(a.size());
	for (size_t i = 0; i < sz; ++i)
		res.set_block(i, a.get_block(i) ^ b.get_block(i));
	return res;
}

ostream& operator<<(ostream& os, const BitSet& bs) {
	ios_base::fmtflags flags = os.flags();
	os << noboolalpha;
	for (size_t i = 0; i < bs.size(); ++i)
		os << bs[i];
	os << setiosflags(flags);
	return os;
}
