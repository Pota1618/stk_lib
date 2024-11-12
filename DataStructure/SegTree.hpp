#include <vector>
#include <functional>
#include <cassert>
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
	
	T all_prod() const {
		return data[1];
	}
	
	// prod[l, l+1, ... r) が ok となるような最大の r を返す
	// 条件を満たす最大の r を返す二分探索
	// l == N の場合は、区間の長さが 0 なので、N を返す。単位元が条件を満たすことを要求する。
	int binary_search(int l, const function<bool(T)>& ok) {
		assert(0 <= l && l <= N);
		assert(ok(e) == true);
		if(l == N) return N;
		
		l += data_size;
		T product = e;
		
		do {
			// 左端が変わらないようにできるだけ親に上る。
			while(l % 2 == 0) l >>= 1;
			
			// 右に伸ばしすぎのパターン
			// 伸ばしすぎということはその間に答えがあるはず
			if(!ok(op(data[l], product))) {
				// 子が存在する限り
				while(l < data_size) {
					// 左の子に降りて
					l <<= 1;
					// もしその区間を追加しても良いなら
					if(ok(op(data[l], product))) {
						// 追加して
						product = op(data[l], product);
						// 次は右隣の区間に移る
						++l;
					}
				}
				// 一番下のノードまで降りたらその区間が答え
				return l - data_size;
			}
			// 右に伸ばせるなら伸ばす
			product = op(data[l], product);
			// 右隣のノードへ
			++l;
		} while((l & -l) != l); // 2^n または 0 でない間？　右端まで辿り着いたら終了？
		
		return N;
	}
};
