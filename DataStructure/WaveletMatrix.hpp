#include <cstdint>
#include <vector>
#include <bit>
#include <utility>
#include <cassert>

struct BitVector {
	using u64 = uint64_t;
	int64_t data_size;
	std::vector<u64> data;
	std::vector<u64> rank64;
	BitVector() {}
	BitVector(int64_t n) : data_size((n + 63) / 64), data(data_size + 1), rank64(data_size + 1) {}

	void set(int64_t i) { data[i / 64] |= 1ull << (i % 64); }
	void build() {
		for(int64_t i = 0; i < data_size; ++i) {
			rank64[i + 1] = rank64[i] + std::popcount(data[i]);
		}
	}
	int64_t rank(int64_t i) {
		u64 mask = (u64(1) << (u64(i) % 64)) - u64(1);
		return rank64[i / 64] + std::popcount<u64>(data[i / 64] & mask);
	}
};

class WaveletMatrix {
private:
	int64_t H, N;
	std::vector<BitVector> data;
public:
	WaveletMatrix(int64_t H, std::vector<uint64_t> A) : H(H), N((int64_t)A.size()), data(H) {
		for(int64_t h = H - 1; h >= 0; --h) {
			BitVector bv(N);
			std::vector<uint64_t> left, right;
			for(int64_t i = 0; i < N; ++i) {
				int64_t dir = A[i] >> h & 1;
				if(dir == 0) left.emplace_back(A[i]);
				else right.emplace_back(A[i]);
				
				if(dir) bv.set(i);
			}
			int64_t a = (int64_t)left.size(), b = (int64_t)right.size();
			for(int i = 0; i < a; ++i) A[i] = left[i];
			for(int i = 0; i < b; ++i) A[a + i] = right[i];
			bv.build();
			data[h] = bv;
		}
	}
	
	uint64_t kth_smallest(int64_t L, int64_t R, int64_t k) {
		assert(0 <= L && L < R && R <= N && 0 <= k && k < R - L);
		return kth_smallest_impl(H, L, R, k);
	}
	
private:
	std::tuple<int64_t, int64_t, int64_t, int64_t> get_subtree_range(int64_t h, int64_t L, int64_t R) {
		int64_t a0 = L - data[h].rank(L), a1 = data[h].rank(L);
		int64_t b0 = R - data[h].rank(R), b1 = data[h].rank(R);
		int64_t c0 = N - data[h].rank(N);
		return {a0, b0, c0 + a1, c0 + b1};
	}
	
	uint64_t kth_smallest_impl(int64_t h, int64_t l, int64_t r, int64_t k) {
		if(h == 0) return 0;
		auto [l0, r0, l1, r1] = get_subtree_range(h - 1, l, r);
		int64_t left_size = r0 - l0;
		if(k < left_size) {
			return kth_smallest_impl(h - 1, l0, r0, k);
		}
		else {
			return (1ull << (h - 1)) + kth_smallest_impl(h - 1, l1, r1, k - left_size);
		}
	}
};
