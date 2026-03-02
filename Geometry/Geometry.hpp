#include <iostream>
#include <cmath>
#include <vector>

namespace Geometry {
	using Real = double;
	constexpr Real EPS = 1e-9;	
	
	inline int sign(Real x) { return (x < -EPS ? -1 : (x > EPS ? +1 : 0)); };
	inline bool eq(Real a, Real b) { return std::fabs(a-b) < EPS; }
	
	struct Point2;
	struct Vec2;
	struct Line;
	struct Segment;
	
	bool operator< (const Point2& a, const Point2& b);
	bool operator==(const Point2& a, const Point2& b);
	bool operator!=(const Point2& a, const Point2& b);
	bool operator>=(const Point2& a, const Point2& b);
	bool operator> (const Point2& a, const Point2& b);
	bool operator<=(const Point2& a, const Point2& b);
	Vec2 operator+(const Vec2& v0, const Vec2& v1);
	Vec2 operator-(const Vec2& v0, const Vec2& v1);
	Vec2 operator*(const Vec2& v, Real d);
	Vec2 operator*(Real d, const Vec2& v);
	Vec2 operator/(const Vec2& v, Real d);
	
	struct Point2 {
		Real x, y;
		Point2() : x(0), y(0) {}
		Point2(Real _x, Real _y) : x(_x), y(_y) {}
		
		friend std::ostream& operator<<(std::ostream& os, const Point2& p) { os << p.x << " " << p.y; return os; }
	};
	
	struct Vec2 : public Point2 {
		enum CCW {
			COUNTER_CLOCKWISE,
			CLOCKWISE,
			ONLINE_BACK,
			ONLINE_FRONT,
			ON_SEGMENT
		};
		
		Vec2() : Point2() {}
		Vec2(Real x, Real y) : Point2(x, y) {}
		Vec2(const Point2& origin, const Point2& tip) : Point2(tip.x - origin.x, tip.y - origin.y) {}
		
		Real length() const { return std::sqrt(x*x + y*y); }
		Real length2() const { return x*x + y*y; }
		Vec2& normalize() { 
			Real l = length();
			x /= l, y /= l;
			return *this;
		}
		Vec2 normalized() const {
			Real l = length();
			return Vec2(x/l, y/l);
		}
		
		/*
		v0, v1 についての位置関係を返す。
		ゼロベクトルについては未定義。
		COUNTER_CLOCKWISE ... v0, v1 が反時計回りに並ぶ
		CLOCKWISE         ... v0, v1 が時計回りに並ぶ
		ONLINE_BACK       ... v0, v1 が逆を向いている
		ONLINE_FRONT      ... v0, v1 が同じ向きで、|v0|<|v1|
		ON_SEGMENT        ... v0, v1 が同じ向きで、|v0|>=|v1|
		*/
		int ccw(const Vec2& other) {
			int sign_s = sign(cross(other));
			if(sign_s == 0) {
				int sign_d = sign(dot(other));
				if(sign_d == -1) { return CCW::ONLINE_BACK; }
				if(sign(this->length2() - other.length2()) == -1) { return CCW::ONLINE_FRONT; }
				else { return CCW::ON_SEGMENT; }
			}
			else if(sign_s == 1) { return CCW::COUNTER_CLOCKWISE; }
			else { return CCW::CLOCKWISE; }
		}
		
		Vec2& operator+=(const Vec2& other) { x += other.x, y += other.y; return *this; }
		Vec2& operator-=(const Vec2& other) { x -= other.x, y -= other.y; return *this; }
		Vec2& operator*=(Real d) { x *= d, y *= d; return *this; }
		Vec2& operator/=(Real d) { x /= d, y /= d; return *this; }
		Real dot(const Vec2& other) const { return x*other.x + y*other.y; }
		Real cross(const Vec2& other) const { return x*other.y - other.x*y; }
		bool parallel(const Vec2& other) const { return eq(this->cross(other), 0); }
		bool perpendicular(const Vec2& other) const { return eq(this->dot(other), 0); }
	};
	
	struct Line {
		Vec2 p0, p1;
		Line() : p0(Vec2()), p1(1, 0) {}
		Line(const Vec2& s, const Vec2& t) : p0(s), p1(t) {}
		
		virtual Real distance(const Vec2& p) const {
			return (project(p) - p).length();
		}
		Vec2 project(const Vec2& p) const {
			auto v = vec();
			return p0 + v.dot(Vec2(p0, p)) / v.length2() * v;
		}
		Vec2 reflection(const Vec2& p) const {
			return p + 2 * (project(p) - p);
		}
		bool parallel(const Line& other) const {
			return vec().parallel(other.vec());
		}
		bool perpendicular(const Line& other) const {
			return vec().perpendicular(other.vec());
		}
		
		Vec2 vec() const { return Vec2(p0, p1); }
		Vec2 counter_vec() const { return Vec2(p1, p0); }
		
		friend std::ostream& operator<<(std::ostream& os, const Line& l) { os << l.p0 << " - " << l.p1; return os; }
	};
	
	struct Segment : public Line {
		Segment() : Line() {}
		Segment(const Vec2& s, const Vec2& t) : Line(s, t) {}
		
		// 二つの線分の交点を返す。接している場合も true
		// 長さ 0 の線分は未定義
		// 無限にある場合もどれを返すか未定義
		std::pair<bool, Vec2> intersect(const Segment& other) const {
			// 互いの両端を共有している場合
			if(this->p0 == other.p0 || this->p0 == other.p1) {
				return std::make_pair(true, p0);
			}
			if(this->p1 == other.p0 || this->p1 == other.p1) {
				return std::make_pair(true, p1);
			}
			
			// p0 を原点とする座標系で考える
			Vec2 v0(this->vec()), v1(this->p0, other.p0), v2(this->p0, other.p1);
			
			if(v1.parallel(v2)) { // 正則でない場合
				if(v0.parallel(v1)) {
					// 区間の交差判定
					Real l0 = 0, r0 = v0.length();
					Real l1 = v1.dot(v0) / r0, r1 = v2.dot(v0) / r0;
					if(l1 > r1) std::swap(l1, r1);
					Real L = std::max(l0, l1), R = std::min(r0, r1);
					if(sign(R - L) >= 0) {
						// 重なりの中点を返すことにしておく
						Real t = (R + L) / 2;
						return std::make_pair(true, this->p0 + v0.normalized() * t);
					}
					else {
						return std::make_pair(false, Vec2());
					}
				}
				else {
					if(sign(v1.dot(v2)) == 1) {
						return std::make_pair(false, Vec2());
					}
					else {
						return std::make_pair(true, this->p0);
					}
				}
			}
			else {
				// 一度 [v1, v2] を基底として座標変換すると、y=x+1 (0<=x<=1)と交差するかの問題になる。
				// v0 を適切な範囲でのみスケールして、v1-v2 の上でぴったり止まれればよい。
				// スケールはうまく面積が一致するようなところと考えるとわかりやすい
				Vec2 v(v0.x*v2.y - v0.y*v2.x, -v0.x*v1.y + v1.x*v0.y);
				Real det = v1.cross(v2);
				v /= det; // v := invM * v0
				if(sign(v.x) != -1 && sign(v.y) != -1 && sign(v.x+v.y - 1) != -1) {
					return std::make_pair(true, this->p0 + v0 * (det / (v1.cross(v0) + v0.cross(v2))));
				}
				else {
					return std::make_pair(false, Vec2());
				}
			}
		}
		
		Real distance(const Vec2& p) const override {
			const auto v = vec();
			Real t = v.dot(p - p0) / v.length2();
			if(t < 0) return (p0 - p).length();
			if(t > 1) return (p1 - p).length();
			return ((p0 + v * t) - p).length();
		}
		
		Real distance(const Segment& s) const {
			if(this->intersect(s).first) 
				return 0;
			Real res = this->distance(s.p0);
			res = std::min(res, this->distance(s.p1));
			res = std::min(res, s.distance(this->p0));
			res = std::min(res, s.distance(this->p1));
			return res;
		}
	};
	
	Vec2 operator+(const Vec2& v0, const Vec2& v1) { return Vec2(v0.x + v1.x, v0.y + v1.y); }
	Vec2 operator-(const Vec2& v0, const Vec2& v1) { return Vec2(v0.x - v1.x, v0.y - v1.y); }
	Vec2 operator*(const Vec2& v, Real d) { return Vec2(v.x*d, v.y*d); }
	Vec2 operator*(Real d, const Vec2& v) { return Vec2(v.x*d, v.y*d); }
	Vec2 operator/(const Vec2& v, Real d) { return Vec2(v.x/d, v.y/d); }
	
	bool operator<(const Point2& a, const Point2& b) { return a.x != b.x ? a.x < b.x : a.y < b.y; }
	bool operator==(const Point2& a, const Point2& b) { return eq(a.x, b.x) && eq(a.y, b.y); }
	bool operator!=(const Point2& a, const Point2& b) { return !(a == b); }
	bool operator>=(const Point2& a, const Point2& b) { return !(a < b); }
	bool operator>(const Point2& a, const Point2& b) { return a.x != b.x ? a.x > b.x : a.y > b.y; }
	bool operator<=(const Point2& a, const Point2& b) { return !(a > b); }
}
