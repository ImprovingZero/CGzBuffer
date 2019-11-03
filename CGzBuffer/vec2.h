#pragma once

#include<math.h>
#include<stdlib.h>
#include<iostream>
#include<vector>

class vec2
{
public:
	float e[2];

	vec2() { e[0] = e[1] = 0; }
	vec2(float x, float y) { e[0] = x; e[1] = y; }
	inline float x() const { return e[0]; }
	inline float y() const { return e[1]; }
	inline float u() const { return e[0]; }
	inline float v() const { return e[1]; }

	inline const vec2& operator+() const { return *this; }
	inline vec2 operator-() const { return vec2(-e[0], -e[1]); }
	inline float operator[](int i) const
	{
		if (i < 0 || i > 1) { std::cout << "vec2 error: invalid index" << std::endl; return 0; }
		else return e[i];
	}
	inline float& operator[](int i) { return e[i]; }

	inline vec2& operator+=(const vec2& v2);
	inline vec2& operator-=(const vec2& v2);
	inline vec2& operator*=(const vec2& v2);
	inline vec2& operator/=(const vec2& v2);
	inline vec2& operator*=(float t);
	inline vec2& operator/=(float t);
	inline float dot(const vec2& v);
	inline float cross(const vec2& v);

	inline float length() const
	{
		return sqrt(e[0] * e[0] + e[1] * e[1]);
	}
	inline float squared_length() const
	{
		return e[0] * e[0] + e[1] * e[1];
	}
	inline void makeUnit();
};

inline std::istream& operator>>(std::istream& is, vec2& t)
{
	is >> t.e[0] >> t.e[1];
	return is;
}
inline std::ostream& operator<<(std::ostream& os, vec2& t)
{
	os << t.e[0] << ' ' << t.e[1];
	return os;
}

inline float vec2::dot(const vec2& v)
{
	return e[0] * v.e[0] + e[1] * v.e[1];
}
inline float vec2::cross(const vec2& v)
{
	return e[0] * v.e[1] - e[1] * v.e[0];
}

inline vec2& vec2::operator+=(const vec2& v2)
{
	e[0] += v2.e[0];
	e[1] += v2.e[1];
	return *this;
}
inline vec2& vec2::operator-=(const vec2& v2)
{
	e[0] -= v2.e[0];
	e[1] -= v2.e[1];
	return *this;
}
inline vec2& vec2::operator*=(const vec2& v2)
{
	e[0] *= v2.e[0];
	e[1] *= v2.e[1];
	return *this;
}
inline vec2& vec2::operator/=(const vec2& v2)
{
	e[0] /= v2.e[0];
	e[1] /= v2.e[1];
	return *this;
}
inline vec2& vec2::operator*=(float t)
{
	e[0] *= t;
	e[1] *= t;
	return *this;
}
inline vec2& vec2::operator/=(float t)
{
	float k = 1.f / t;
	e[0] *= k;
	e[1] *= k;
	return *this;
}
inline void vec2::makeUnit()
{
	float len = this->length();
	e[0] /= len; e[1] /= len;
}

inline vec2 operator+(const vec2& v1, const vec2& v2)
{
	return vec2(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1]);
}
inline vec2 operator-(const vec2& v1, const vec2& v2)
{
	return vec2(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1]);
}
inline vec2 operator*(const vec2& v1, const vec2& v2)
{
	return vec2(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1]);
}
inline vec2 operator/(const vec2& v1, const vec2& v2)
{
	return vec2(v1.e[0] / v2.e[0], v1.e[1] / v2.e[1]);
}
inline vec2 operator*(const vec2& v, const float t)
{
	return vec2(t * v.e[0], t * v.e[1]);
}
inline vec2 operator*(const float t, const vec2& v)
{
	return vec2(t * v.e[0], t * v.e[1]);
}
inline vec2 operator/(const vec2& v, const float t)
{
	return vec2(v.e[0] / t, v.e[1] / t);
}
inline float Dot(const vec2& v1, const vec2& v2)
{
	return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1];
}
inline float Cross(const vec2& v1, const vec2& v2)
{
	return v1.e[0] * v2.e[1] - v1.e[1] * v2.e[0];
}
inline vec2 Unit(vec2 v)
{
	return v / v.length();
}
