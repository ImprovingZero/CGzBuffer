#pragma once

#include<math.h>
#include<stdlib.h>
#include<iostream>
#include<vector>
#include<array>

class vec3
{
public:
	union
	{
		struct {
			double x, y, z;
		};
		struct {
			double e[3];
		};
	};

	vec3() { e[0] = e[1] = e[2] = 0; }
	vec3(double x, double y, double z) { e[0] = x; e[1] = y; e[2] = z; }
	//inline double x() const { return e[0]; }
	//inline double y() const { return e[1]; }
	//inline double z() const { return e[2]; }
	inline double r() const { return e[0]; }
	inline double g() const { return e[1]; }
	inline double b() const { return e[2]; }

	inline const vec3& operator+() const { return *this; }
	inline vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
	inline double operator[](int i) const 
	{ 
		if (i < 0 || i>2) { std::cout << "vec3 error: invalid index" << std::endl; return 0; }
		else return e[i];
	}
	inline double& operator[](int i) { return e[i]; }

	inline vec3& operator+=(const vec3 &v2);
	inline vec3& operator-=(const vec3 &v2);
	inline vec3& operator*=(const vec3 &v2);
	inline vec3& operator/=(const vec3 &v2);
	inline vec3& operator*=(double t);
	inline vec3& operator/=(double t);
	inline double dot(const vec3 &v);
	inline vec3 cross(const vec3 &v);
	inline bool operator==(const vec3 & v2) const
	{
		return (e[0] == v2.e[0] && e[1] == v2.e[1] && e[2] == v2.e[2]);
	}

	inline double length() const
	{
		return sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
	}
	inline double squared_length() const
	{
		return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
	}
	inline void makeUnit();
};

inline std::istream& operator>>(std::istream& is, vec3& t)
{
	is >> t.e[0] >> t.e[1] >> t.e[2];
	return is;
}
inline std::ostream& operator<<(std::ostream& os, vec3& t)
{
	os << t.e[0] << ' ' << t.e[1] << ' ' << t.e[2];
	return os;
}

inline double vec3::dot(const vec3& v)
{
	return e[0] * v.e[0] + e[1] * v.e[1] + e[2] * v.e[2];
}
inline vec3 vec3::cross(const vec3& v)
{
	return vec3(e[1] * v.e[2] - e[2] * v.e[1],
		e[2] * v.e[0] - e[0] * v.e[2],
		e[0] * v.e[1] - e[1] * v.e[0]);
}

inline vec3& vec3::operator+=(const vec3& v2)
{
	e[0] += v2.e[0];
	e[1] += v2.e[1];
	e[2] += v2.e[2];
	return *this;
}
inline vec3& vec3::operator-=(const vec3& v2)
{
	e[0] -= v2.e[0];
	e[1] -= v2.e[1];
	e[2] -= v2.e[2];
	return *this;
}
inline vec3& vec3::operator*=(const vec3& v2)
{
	e[0] *= v2.e[0];
	e[1] *= v2.e[1];
	e[2] *= v2.e[2];
	return *this;
}
inline vec3& vec3::operator/=(const vec3& v2)
{
	e[0] /= v2.e[0];
	e[1] /= v2.e[1];
	e[2] /= v2.e[2];
	return *this;
}
inline vec3& vec3::operator*=(double t)
{
	e[0] *= t;
	e[1] *= t;
	e[2] *= t;
	return *this;
}
inline vec3& vec3::operator/=(double t)
{
	double k = 1.f / t;
	e[0] *= k;
	e[1] *= k;
	e[2] *= k;
	return *this;
}
inline void vec3::makeUnit()
{
	double len = this->length();
	e[0] /= len; e[1] /= len; e[2] /= len;
}

inline vec3 operator+(const vec3& v1, const vec3& v2)
{
	return vec3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
}
inline vec3 operator-(const vec3& v1, const vec3& v2)
{
	return vec3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
}
inline vec3 operator*(const vec3& v1, const vec3& v2)
{
	return vec3(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1], v1.e[2] * v2.e[2]);
}
inline vec3 operator/(const vec3& v1, const vec3& v2)
{
	return vec3(v1.e[0] / v2.e[0], v1.e[1] / v2.e[1], v1.e[2] / v2.e[2]);
}
inline vec3 operator*(const vec3& v, const double t)
{
	return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}
inline vec3 operator*(const double t, const vec3& v)
{
	return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}
inline vec3 operator/(const vec3& v, const double t)
{
	return vec3(v.e[0] / t, v.e[1] / t, v.e[2] / t);
}
inline double Dot(const vec3& v1, const vec3& v2)
{
	return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2];
}
inline vec3 Cross(const vec3& v1, const vec3& v2)
{
	return vec3((v1.e[1] * v2.e[2] - v1.e[2] * v2.e[1]),
		(v1.e[2] * v2.e[0] - v1.e[0] * v2.e[2]),
		(v1.e[0] * v2.e[1] - v1.e[1] * v2.e[0]));
}
inline vec3 Unit(vec3 v)
{
	return v / v.length();
}
inline const vec3 Reflect(const vec3& v, const vec3& n)
{
	return v - 2.f * Dot(v, n) * n;
}