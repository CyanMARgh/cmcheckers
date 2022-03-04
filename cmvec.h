#pragma once

#include <cmath>
struct ivec2 {
	int16_t x, y;
	ivec2(int16_t x_ = 0, int16_t y_ = 0) : x(x_), y(y_) { }
	friend bool operator==(const ivec2 &a, const ivec2 b) {
		return a.x == b.x && a.y == b.y;
	}
	friend bool operator!=(const ivec2 a, const ivec2 b) {
		return !(a == b);
	}
	ivec2 &operator+=(const ivec2 &b) {
		x += b.x;
		y += b.y;
		return *this;
	}
	ivec2 &operator-=(const ivec2 &b) {
		x -= b.x;
		y -= b.y;
		return *this;
	}
	ivec2 &operator*=(const ivec2 &b) {
		x *= b.x;
		y *= b.y;
		return *this;
	}
	ivec2 &operator*=(const uint8_t &b) {
		x *= b;
		y *= b;
		return *this;
	}
	ivec2 &operator/=(const ivec2 &b) {
		x /= b.x;
		y /= b.y;
		return *this;
	}
	ivec2 &operator/=(const uint8_t &b) {
		x /= b;
		y /= b;
		return *this;
	}
	ivec2 operator+(const ivec2 &b) const {
		ivec2 t = *this;
		return t += b;
	}
	ivec2 operator-(const ivec2 &b) const {
		ivec2 t = *this;
		return t -= b;
	}
	ivec2 operator*(const ivec2 &b) const {
		ivec2 t = *this;
		return t *= b;
	}
	ivec2 operator*(const uint8_t &b) const {
		ivec2 t = *this;
		return t *= b;
	}
	ivec2 operator/(const ivec2 &b) const {
		ivec2 t = *this;
		return t /= b;
	}
	ivec2 operator/(const uint8_t &b) const {
		ivec2 t = *this;
		return t /= b;
	}
};
ivec2 abs(const ivec2 &a) {
	return {(int16_t) abs(a.x), (int16_t) abs(a.y)};
}
int16_t sum(const ivec2 &a) {
	return a.x + a.y;
}
int16_t proj(const ivec2 &d, const ivec2 &n) {
	ivec2 d_ = d * abs(n);
	if (d_ != d) return -1;
	return sum(d) / sum(n);
}
bool inZone(const ivec2 p, const ivec2 z) {
	return p.x >= 0 && p.x < z.x && p.y >= 0 && p.y <= z.y;
}