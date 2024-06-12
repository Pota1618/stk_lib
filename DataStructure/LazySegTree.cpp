#include <vector>
#include <functional>
#include <cassert>
using namespace std;

template<typename S, typename F>
class LazySegTree {
private:
	function<S(S, S)> op;
	const S e;
	function<S(F, S)> mapping;
	function<F(F, F)> composition;
	const F id;
	function<F(F, int)> power;

	int N, sz, log;

	vector<S> data;
	vector<F> lazy;
	vector<int> length;

public:
	LazySegTree(int n, function<S(S, S)> op, S e, function<S(F, S)> mapping, function<F(F, F)> composition, F id)
		: LazySegTree(vector<S>(n, e), op, e, mapping, composition, [](F f, int k) { return f; }, id) {}
	LazySegTree(int n, function<S(S, S)> op, S e, function<S(F, S)> mapping, function<F(F, F)> composition, function<F(F, int)> power, F id)
		: LazySegTree(vector<S>(n, e), op, e, mapping, composition, power, id) {}
	LazySegTree(const vector<S>& A, function<S(S, S)> op, S e, function<S(F, S)> mapping, function<F(F, F)> composition, F id)
		: LazySegTree(A, op, e, mapping, composition, [](F f, int k) { return f; }, id) {}
	LazySegTree(const vector<S>& A, function<S(S, S)> op, S e, function<S(F, S)> mapping, function<F(F, F)> composition, function<F(F, int)> power, F id)
		: N((int)A.size()), op(op), e(e), mapping(mapping), composition(composition), power(power), id(id), log(0)
	{
        while (N > (1 << log)) ++log;
		sz = 1 << log;
		data.assign(sz << 1, e);
		lazy.assign(sz, id);
		length.assign(sz << 1, 1);

		copy(A.begin(), A.end(), data.begin() + sz);
		for (int i = sz - 1; i >= 1; --i) Update(i);
		for (int i = sz - 1; i >= 1; --i) length[i] = length[i << 1] + length[i << 1 | 1];
	}

	void set(int idx, S x) {
		assert(0 <= idx && idx < N);
		idx += sz;
		for (int i = log; i >= 1; --i) Propagate(idx >> i);
		data[idx] = x;
		for (int i = log; i >= 1; --i) Update(idx >> i);
	}

	S operator[](int i) { return get(i); }
	S get(int idx) {
		assert(0 <= idx && idx < N);
		idx += sz;
		for (int i = log; i >= 1; --i) Propagate(idx >> i);
		return data[idx];
	}

	S prod(int l, int r) {
		assert(0 <= l && l <= N && 0 <= r && r <= N);
		if (l == r) return e;

        l += sz; r += sz;

		for (int i = log; i >= 1; --i) {
			if (((l >> i) << i) != l) Propagate(l >> i);
			if (((r >> i) << i) != r) Propagate((r - 1) >> i);
		}

		S res_l = e, res_r = e;
		while (l < r) {
			if (l & 1) res_l = op(data[l++], res_l);
			if (r & 1) res_r = op(res_r, data[--r]);
            l >>= 1; r >>= 1;
		}

		return op(res_r, res_l);
	}

	S all_prod() { return data[1]; }

	void apply(int p, F f) {
		assert(0 <= p && p < N);
		p += sz;
		for (int i = log; i >= 1; --i) Propagate(p >> i);
		data[p] = mapping(f, data[p]);
		for (int i = 1; i <= log; ++i) Update(p >> i);
	}

	void apply(int l, int r, F f) {
		assert(0 <= l && l <= N && 0 <= r && r <= N);
		if (l == r) return;

        l += sz; r += sz;

		for (int i = log; i >= 1; --i) {
			if (((l >> i) << i) != l) Propagate(l >> i);
			if (((r >> i) << i) != r) Propagate((r - 1) >> i);
		}

		{
			int l2 = l, r2 = r;
			while (l < r) {
				if (l & 1) MappingToCell(l++, f);
				if (r & 1) MappingToCell(--r, f);
                l >>= 1; r >>= 1;
			}
            l = l2; r = r2;
		}

		for (int i = 1; i <= log; ++i) {
			if (((l >> i) << i) != l) Update(l >> i);
			if (((r >> i) << i) != r) Update((r - 1) >> i);
		}
	}

	int size() { return N; }

private:
	void Update(int i) { data[i] = op(data[i << 1 | 1], data[i << 1]); }
	void MappingToCell(int k, F f) {
		data[k] = mapping(power(f, length[k]), data[k]); // 区間 sum とかで power(f,x) := f*x にする
		if (k < sz) lazy[k] = composition(f, lazy[k]);
	}
	void Propagate(int k) {
		MappingToCell(k << 1, lazy[k]);
		MappingToCell(k << 1 | 1, lazy[k]);
		lazy[k] = id;
	}
};
