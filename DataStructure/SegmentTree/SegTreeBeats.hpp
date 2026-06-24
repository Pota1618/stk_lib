#include <vector>
#include <bit>
#include <numeric>
#include <cstdint>
#include <cassert>

template <std::signed_integral T>
class SegTreeBeats {
private:
	using value_type = T;
	std::vector<value_type> data, lazy;
	std::vector<value_type> mx1, mx2, mx_cnt1;
	std::vector<value_type> mn1, mn2, mn_cnt1;
	int64_t n;
public:
	static constexpr value_type INF = std::numeric_limits<value_type>::max();
public:
	explicit SegTreeBeats(const std::vector<value_type>& vec) :
		data((1ull << (std::bit_width(vec.size()) + 1)), e()), lazy(data.size(), e()), 
		mx1(data.size(), -INF), mx2(data.size(), -INF), mx_cnt1(data.size(), 1),
		mn1(data.size(), INF), mn2(data.size(), INF), mn_cnt1(data.size(), 1), n((int64_t)vec.size()) 
		{ build_impl(vec, 1, 0, (int64_t)data.size()/2); }
	
	void apply_min(int64_t l, int64_t r, const value_type& f) {
		if(l >= r) return;
		assert(0 <= l && l < n && 0 < r && r <= n);
		apply_min_impl(1, l, r, 0, (int64_t)data.size() / 2, f);
	}
	
	void apply_max(int64_t l, int64_t r, const value_type& f) {
		if(l >= r) return;
		assert(0 <= l && l < n && 0 < r && r <= n);
		apply_max_impl(1, l, r, 0, (int64_t)data.size() / 2, f);
	}
	
	void apply(int64_t l, int64_t r, const value_type& f) {
		if(l >= r) return;
		assert(0 <= l && l < n && 0 < r && r <= n);
		apply_impl(1, l, r, 0, (int64_t)data.size() / 2, f);
	}
	
	value_type prod_min(int64_t l, int64_t r) {
		if(l >= r) return INF;
		assert(0 <= l && l < n && 0 < r && r <= n);
		return prod_min_impl(1, l, r, 0, (int64_t)data.size() / 2);
	}
	
	value_type prod_max(int64_t l, int64_t r) {
		if(l >= r) return -INF;
		assert(0 <= l && l < n && 0 < r && r <= n);
		return prod_max_impl(1, l, r, 0, (int64_t)data.size() / 2);
	}
	
	value_type prod(int64_t l, int64_t r) {
		if(l >= r) return e();
		assert(0 <= l && l < n && 0 < r && r <= n);
		return prod_impl(1, l, r, 0, (int64_t)data.size() / 2);
	}
	
private:
	static value_type op(const value_type& a, const value_type& b) { return a + b; }
	static value_type e() { return 0; }
	
	void build_impl(const std::vector<value_type>& vec, int64_t k, int64_t nl, int64_t nr) {
		if(nr - nl <= 1) {
			if(nl < (int64_t)vec.size()) {
				int64_t k = (int64_t)data.size() / 2 + nl;
				data[k] = vec[nl];
				mx1[k] = vec[nl];
				mn1[k] = vec[nl];
			}
			return;
		}
		int64_t mid = (nl + nr) / 2;
		build_impl(vec, 2*k, nl, mid);
		build_impl(vec, 2*k+1, mid, nr);
		update(k);
	}
	
	void apply_min_impl(int64_t id, int64_t l, int64_t r, int64_t nl, int64_t nr, const value_type& f) {
		if(r <= nl || nr <= l || mx1[id] <= f) return;
		if(l <= nl && nr <= r && mx2[id] < f) {
			map_node_max(id, f);
			return;
		}
		
		propagate(id, nr - nl);
		apply_min_impl(id*2, l, r, nl, (nl+nr)/2, f);
		apply_min_impl(id*2+1, l, r, (nl+nr)/2, nr, f);
		update(id);
	}
	
	void apply_max_impl(int64_t id, int64_t l, int64_t r, int64_t nl, int64_t nr, const value_type& f) {
		if(r <= nl || nr <= l || mn1[id] >= f) return;
		if(l <= nl && nr <= r && mn2[id] > f) {
			map_node_min(id, f);
			return;
		}
		
		propagate(id, nr - nl);
		apply_max_impl(id*2, l, r, nl, (nl+nr)/2, f);
		apply_max_impl(id*2 + 1, l, r, (nl+nr)/2, nr, f);
		update(id);
	}
	
	void apply_impl(int64_t id, int64_t l, int64_t r, int64_t nl, int64_t nr, const value_type& f) {
		if(r <= nl || nr <= l) return;
		if(l <= nl && nr <= r) {
			map_node_add(id, nr - nl, f);
			return;
		}
		
		propagate(id, nr - nl);
		apply_impl(id*2, l, r, nl, (nl+nr)/2, f);
		apply_impl(id*2+1, l, r, (nl+nr)/2, nr, f);
		update(id);
	}
	
	value_type prod_min_impl(int64_t id, int64_t l, int64_t r, int64_t nl, int64_t nr) {
		if(r <= nl || nr <= l) return INF;
		if(l <= nl && nr <= r) return mn1[id];
		
		propagate(id, nr - nl);
		int64_t mid = (nl + nr) / 2;
		value_type prod_l = prod_min_impl(2*id, l, r, nl, mid);
		value_type prod_r = prod_min_impl(2*id+1, l, r, mid, nr);
		return std::min(prod_l, prod_r);
	}
	
	value_type prod_max_impl(int64_t id, int64_t l, int64_t r, int64_t nl, int64_t nr) {
		if(r <= nl || nr <= l) return -INF;
		if(l <= nl && nr <= r) return mx1[id];
		
		propagate(id, nr - nl);
		int64_t mid = (nl + nr) / 2;
		value_type prod_l = prod_max_impl(2*id, l, r, nl, mid);
		value_type prod_r = prod_max_impl(2*id+1, l, r, mid, nr);
		return std::max(prod_l, prod_r);
	}
	
	value_type prod_impl(int64_t id, int64_t l, int64_t r, int64_t nl, int64_t nr) {
		if(r <= nl || nr <= l) return e();
		if(l <= nl && nr <= r) return data[id];
		
		propagate(id, nr - nl);
		int64_t mid = (nl + nr) / 2;
		int64_t prod_l = prod_impl(2*id, l, r, nl, mid);
		int64_t prod_r = prod_impl(2*id+1, l, r, mid, nr);
		return op(prod_r, prod_l);
	}
	
	void map_node_min(int64_t k, const value_type& f) {
		data[k] -= (mn1[k] - f) * mn_cnt1[k];
		
		if(mx1[k] == mn1[k]) {
			mx1[k] = mn1[k] = f;
		}
		else if(mx2[k] == mn1[k]) {
			mn1[k] = mx2[k] = f;
		}
		else {
			mn1[k] = f;
		}
	}
	
	void map_node_max(int64_t k, const value_type& f) {
		data[k] -= (mx1[k] - f) * mx_cnt1[k];
		
		if(mx1[k] == mn1[k]) {
			mx1[k] = mn1[k] = f;
		}
		else if(mx1[k] == mn2[k]) {
			mx1[k] = mn2[k] = f;
		}
		else {
			mx1[k] = f;
		}
	}
	
	void map_node_add(int64_t k, int64_t len, const value_type& f) {
		mx1[k] += f;
		if(mx2[k] != -INF) mx2[k] += f;
		mn1[k] += f;
		if(mn2[k] != INF) mn2[k] += f;
		
		data[k] += f * len;
		lazy[k] += f;
	}
	
	void propagate(int64_t k, int64_t len) {
		map_node_add(2*k, len/2, lazy[k]);
		map_node_add(2*k+1, len/2, lazy[k]);
		lazy[k] = 0;
		
		if(mx1[k] < mx1[2*k]) map_node_max(2*k, mx1[k]);
		if(mn1[k] > mn1[2*k]) map_node_min(2*k, mn1[k]);
		if(mx1[k] < mx1[2*k+1]) map_node_max(2*k+1, mx1[k]);
		if(mn1[k] > mn1[2*k+1]) map_node_min(2*k+1, mn1[k]);
	}
		
	void update(int64_t k) {
		data[k] = op(data[2*k], data[2*k+1]);
		
		// 右の子に最大がある
		if(mx1[2*k] < mx1[2*k+1]) {
			mx1[k] = mx1[2*k+1];
			mx_cnt1[k] = mx_cnt1[2*k+1];
			mx2[k] = std::max(mx1[2*k], mx2[2*k+1]);
		}
		// 左の子に最大がある
		else if(mx1[2*k] > mx1[2*k+1]) {
			mx1[k] = mx1[2*k];
			mx_cnt1[k] = mx_cnt1[2*k];
			mx2[k] = std::max(mx2[2*k], mx1[2*k+1]);
		}
		// 両方の最大が同じ。最大値が両方に存在する。
		else {
			mx1[k] = mx1[2*k];
			mx_cnt1[k] = mx_cnt1[2*k] + mx_cnt1[2*k+1];
			mx2[k] = std::max(mx2[2*k], mx2[2*k+1]);
		}
		
		// 右の子に最小がある
		if(mn1[2*k] > mn1[2*k+1]) {
			mn1[k] = mn1[2*k+1];
			mn_cnt1[k] = mn_cnt1[2*k+1];
			mn2[k] = std::min(mn1[2*k], mn2[2*k+1]);
		}
		// 左の子に最小がある
		else if(mn1[2*k] < mn1[2*k+1]) {
			mn1[k] = mn1[2*k];
			mn_cnt1[k] = mn_cnt1[2*k];
			mn2[k] = std::min(mn2[2*k], mn1[2*k+1]);
		}
		// 最小が両方にある
		else {
			mn1[k] = mn1[2*k];
			mn_cnt1[k] = mn_cnt1[2*k] + mn_cnt1[2*k+1];
			mn2[k] = std::min(mn2[2*k], mn2[2*k+1]);
		}
	}
};

// https://smijake3.hatenablog.com/entry/2019/04/28/021457
// https://tjkendev.github.io/procon-library/cpp/range_query/segment_tree_beats_2.html
