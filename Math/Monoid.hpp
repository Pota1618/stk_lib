#include <concepts>
#include <utility>
#include <cmath>
#include <string>
#include <cstdint>
#include <numeric>

constexpr int64_t INF = 1ll << 60;

template <class M>
concept Monoid = requires { 
	typename M::value_type;
    { M::e() } -> std::same_as<typename M::value_type>;
    { M::op(std::declval<typename M::value_type>(), std::declval<typename M::value_type>()) } 
		-> std::same_as<typename M::value_type>;
};

template <class M>
concept Commutative = requires {
	{ M::commutative } -> std::convertible_to<bool>;
} && M::commutative;

template <class M>
concept CommutativeMonoid = Commutative<M> && Monoid<M>;

template <class M>
concept Group = Monoid<M> && requires {
	{ M::inv(std::declval<typename M::value_type>()) }
		-> std::same_as<typename M::value_type>;
};

template <class M>
concept CommutativeGroup = Commutative<M> && Group<M>;

template <typename T = int64_t>
struct AddMonoid {
	using value_type = T;
	static constexpr bool commutative = true;
	static T e() { return 0; }
	static T op(T a, T b) { return a + b; }
};

template<typename T = int64_t>
struct AddGroup : AddMonoid<T> {
	static T inv(T a) { return -a; }
};

template <typename T = int64_t>
struct MulMonoid {
	using value_type = T;
	static constexpr bool commutative = true;
	static T e() { return 1; }
	static T op(T a, T b) { return a * b; }
};

template <typename T = int64_t>
struct Min {
	using value_type = T;
	static constexpr bool commutative = true;
	static T e() { return INF; }
	static T op(T a, T b) { return std::min(a, b); }
};

template <typename T = int64_t>
struct MaxMonoid {
	using value_type = T;
	static constexpr bool commutative = true;
	static T e() { return -INF; }
	static T op(T a, T b) { return std::max(a, b); }
};

struct StringConcatMonoid {
	using value_type = std::string;
	static value_type e() { return ""; }
	static value_type op(const value_type& a, const value_type& b) { return a + b; }
};

template <typename T>
struct AffineMonoid {
	using value_type = std::pair<T, T>;
	static value_type e() { return std::make_pair(1, 0); }
	static value_type op(const value_type& g, const value_type& f) {
		return std::make_pair(
			f.first * g.first, 
			g.first * f.second + g.second
		);
	}
};

template<Monoid M1, Monoid M2>
struct ProductMonoid {
	using value_type = std::pair<
		typename M1::value_type,
		typename M2::value_type
	>;
	static constexpr bool commutative = CommutativeMonoid<M1> && CommutativeMonoid<M2>;
	
	using T = value_type;
	static T e() { return std::make_pair(M1::e(), M2::e()); }
	static T op(const T& a, const T& b) {
		return std::make_pair(
			M1::op(b.first, a.first),
			M2::op(b.second, a.second)
		);
	}
};

template <class MM>
concept MapMonoid = Monoid<typename MM::data_monoid> && Monoid<typename MM::lazy_monoid> && 
requires(typename MM::data_monoid::value_type x, typename MM::lazy_monoid::value_type f) {
	// The mapping function defines how the LazyMonoid acts on the DataMonoid
	{ MM::mapping(f, x) } -> std::same_as<decltype(x)>;
    { MM::power(f, std::declval<int>()) } -> std::same_as<decltype(f)>;
};
