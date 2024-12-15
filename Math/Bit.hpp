// https://cpprefjp.github.io/lang/cpp17/generalizing_the_range-based_for_loop.html
// https://noshi91.hatenablog.com/entry/2021/04/01/213231

#include <type_traits>

/*
for(T bit : Subset(0b0011001, 8))
みたいに書くと全体集合を 8 個のビットとして
大きい方から部分集合を走査できる。
自身と空集合も含む。
終了判定を -1 で行っているので、符号あり整数型を要求する。
*/
template<typename T = int>
class Subsets {
private:
	static_assert(is_signed<T>::value, "typename T must be a signed integer");
	T s;
public:
	explicit Subsets(T bits) : s(bits) {}
	
	struct Iterator {
		T s, t;
		bool operator!=(monostate) { return t != -1; }
		void operator++() { --t; }
		T operator*() { return t &= s; }
	};
	
	Iterator begin() { return {s, s}; }
	monostate end() { return {}; }
};

/*
for(T bit : Subset(0b1011001))
みたいに書くと大きい方から補集合の部分集合を走査できる。
補集合自身と空集合も含む。
*/
template<typename T = int>
class Complement {
private:
	Subsets<T> ss;
public:
	explicit Complement(T bits, size_t n) : ss(((T(1) << n) - 1) ^ bits) {}
	using Iterator = Subsets<T>::Iterator;
	Iterator begin() { return ss.begin(); }
	monostate end() { return {}; }
};
