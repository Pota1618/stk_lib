#include <vector>

template<typename T>
class FenwickTree {
private:
	static_assert(is_integral_v<T>);
	vector<T> data;
public:
	FenwickTree(size_t n) : data(n + 1) {}

	// sum [0, i)
	T prefix_sum(int i) {
		T res = 0;
		while (i) {
			res = data[i] + res;
			i -= (i & -i);
		}
		return res;
	}

	// sum [l, r)
	T sum(size_t l, size_t r) {
		return prefix_sum((int)r) - prefix_sum((int)l);
	}

	void add(int i, T x) {
		++i;
		while (i < (int)data.size()) {
			data[i] += x;
			i += (i & -i);
		}
	}
};


