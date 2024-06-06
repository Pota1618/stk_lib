#include <vector>
#include <functional>
using namespace std;

template<typename T>
class SegTree {
private:
	const int N;
	int data_size; // data.size() / 2
	vector<T> data;
	// op(g, f) := g(f(x)) = (g・f)(x), 数学に合わせて左右逆なので注意
	const function<T(T, T)> op;
	const T e;

public:
	SegTree(int n, function<T(T, T)> op, T e) : N(n), op(op), e(e) {
		data_size = 1;
		while (n > data_size) data_size <<= 1;
		data.assign(data_size << 1, e);
	}

	SegTree(const vector<T>& A, function<T(T, T)> op, T e) : N((int)A.size()), op(op), e(e) {
		data_size = 1;
		while (A.size() > data_size) data_size <<= 1;
		data.assign(data_size << 1, e);

		copy(A.begin(), A.end(), data.begin() + data_size);
		for (int i = data_size - 1; i >= 1; --i) {
			data[i] = op(data[i << 1 | 1], data[i << 1]);
		}
	}

	void set(int i, T x) {
		assert(0 <= i && i < N);
		size_t idx = i + data_size;
		data[idx] = x;
		while (idx >> 1) {
			size_t p = idx >> 1;
			size_t l = p << 1, r = p << 1 | 1;
			data[p] = op(data[r], data[l]);
			idx >>= 1;
		}
	}
	void apply(int i, T x) { set(i, op(x, data[i + data_size])); }

	const T& operator[](int i) const { return data[i + data_size]; }
	T get(int i) const { return data[i + data_size]; }
	int size() const { return N; }

	T prod(int l, int r) const {
		assert(0 <= l && l <= N && 0 <= r && r <= N);
		T res_l = e, res_r = e;
		l += data_size, r += data_size;
		while (l < r) {
			if (l & 1) res_l = op(data[l++], res_l);
			if (r & 1) res_r = op(res_r, data[--r]);
			l >>= 1, r >>= 1;
		}
		return op(res_r, res_l);
	}
};