#include <vector>
#include <cassert>
#include <iostream>
#include <iomanip>

using namespace std;

/*
実行時にサイズを指定できるビットセット
*/
class BitSet {
public:
	static const size_t BLOCK_SIZE = 64;
private:
	using u64 = unsigned long long;
	size_t N;
	u64* A;
public:
	struct Bit {
		u64* bits;
		size_t pos;

		Bit(u64* bit_block, size_t i) : bits(bit_block), pos(i% BLOCK_SIZE) {}

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
	BitSet(size_t n) : N(n) {
		size_t asize = (n + BLOCK_SIZE - 1) / BLOCK_SIZE;
		A = (u64*)malloc(sizeof(u64) * asize);
		fill(A, A + asize, u64(0));
	}
	BitSet(const BitSet& other) : N(other.N) {
		size_t asize = other.array_size();
		A = (u64*)malloc(sizeof(u64) * asize);
		for (size_t i = 0; i < asize; ++i)
			A[i] = other.A[i];
	}
	BitSet(BitSet&& other) noexcept : N(other.N) {
		A = other.A;
		other.A = nullptr;
	}
	BitSet& operator=(const BitSet& other) {
		N = other.N;
		if (A) free(A);
		A = (u64*)malloc(sizeof(u64) * other.array_size());
		for (size_t i = 0; i < other.array_size(); ++i)
			A[i] = other.A[i];
		return *this;
	}
	BitSet& operator=(BitSet&& other) noexcept {
		N = other.N;
		if (A) free(A);
		A = other.A;
		other.A = nullptr;
		return *this;
	}

	~BitSet() { if (A) free(A); }

	Bit operator[](size_t i) {
		assert(i < N);
		return Bit(&A[i / BLOCK_SIZE], i);
	}
	bool operator[](size_t i) const {
		assert(i < N);
		return bool(A[i / BLOCK_SIZE] >> (i % BLOCK_SIZE) & 1);
	}
	BitSet& operator~() {
		for (size_t i = 0; i < array_size(); ++i)
			A[i] = ~A[i];
		return *this;
	}
	BitSet& operator&=(const BitSet& other) {
		assert(size() == other.size());
		for (size_t i = 0; i < array_size(); ++i)
			A[i] &= other.A[i];
		return *this;
	}
	BitSet& operator|=(const BitSet& other) {
		assert(size() == other.size());
		for (size_t i = 0; i < array_size(); ++i)
			A[i] |= other.A[i];
		return *this;
	}
	BitSet& operator^=(const BitSet& other) {
		assert(size() == other.size());
		for (size_t i = 0; i < array_size(); ++i)
			A[i] ^= other.A[i];
		return *this;
	}
	bool operator==(const BitSet& other) const {
		if (size() != other.size()) return false;

		for (size_t i = 0; i < array_size() - 1; ++i)
			if (A[i] != other.A[i])
				return false;

		u64 m = (N % BLOCK_SIZE == 0 ? ~0 : mask(N % BLOCK_SIZE) - 1);
		if ((A[array_size() - 1] & m) != (other.A[array_size() - 1] & m))
			return false;

		return true;
	}
	bool operator!=(const BitSet& other) const {
		return !(*this == other);
	}

	// すべてを 1 にする
	BitSet& set() { fill(A, A + array_size(), ~u64(0)); return *this; }
	// pos 番目の bit を val にする
	BitSet& set(size_t pos, bool val) { operator[](pos) = val; return *this; }
	// 全 bit を 0 にする
	BitSet& reset() { fill(A, A + array_size(), u64(0)); return *this; }
	// pos 番目の bit を 0 にする
	BitSet& reset(size_t pos) { operator[](pos) = false; return *this; }
	// 全 bit を反転する
	BitSet& flip() { for (size_t i = 0; i < array_size(); ++i) A[i] ^= ~u64(0); return *this; }
	// pos 番目の bit を反転する
	BitSet& flip(size_t pos) { operator[](pos) = operator[](pos) != true; return *this; }
	// pos 番目が 1 かどうか判定する
	bool test(size_t pos) const { return operator[](pos); }
	// 1 の個数
	size_t count() const {
		size_t res = 0;
		for (size_t i = 0; i < array_size() - 1; ++i) {
			res += popcount(A[i]);
		}
		u64 m = (N % BLOCK_SIZE == 0 ? ~u64(0) : mask(N % BLOCK_SIZE) - 1);
		res += popcount(A[array_size() - 1] & m);

		return res;
	}

	// bit の個数
	size_t size() const { return N; }
	inline size_t array_size() const { return (N + BLOCK_SIZE - 1) / BLOCK_SIZE; }

	const u64* data() const { return A; }

	u64 get_block(size_t pos) const { return A[pos]; }
	void set_block(size_t pos, u64 val) { A[pos] = val; }

private:
	static inline u64 mask(size_t pos) { return 1ull << pos; }
};

BitSet operator&(const BitSet& a, const BitSet& b) {
	assert(a.size() == b.size());
	size_t sz = (a.size() + BitSet::BLOCK_SIZE - 1) / BitSet::BLOCK_SIZE;
	BitSet res(a.size());
	for (size_t i = 0; i < sz; ++i)
		res.set_block(i, a.get_block(i) & b.get_block(i));
	return res;
}
BitSet operator|(const BitSet& a, const BitSet& b) {
	assert(a.size() == b.size());
	size_t sz = (a.size() + BitSet::BLOCK_SIZE - 1) / BitSet::BLOCK_SIZE;
	BitSet res(a.size());
	for (size_t i = 0; i < sz; ++i)
		res.set_block(i, a.get_block(i) | b.get_block(i));
	return res;
}
BitSet operator^(const BitSet& a, const BitSet& b) {
	assert(a.size() == b.size());
	size_t sz = (a.size() + BitSet::BLOCK_SIZE - 1) / BitSet::BLOCK_SIZE;
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
