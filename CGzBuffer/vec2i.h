#pragma once

class vec2i
{
public:
	union
	{
		struct {
			int x, y;
		};
		struct {
			int v[2];
		};
	};
	vec2i() :x(0), y(0) {};
	vec2i(const vec2i& v) :x(v.x), y(v.y) {};
	vec2i(const int a, const int b) :x(a), y(b) {};
};

class vec2if
{
public:
	union {
		struct {
			int x, y;
			float z;
		};
		struct {
			int v[2];
			float f;
		};
	};
	vec2if() :x(0), y(0), z(0.f) {};
	vec2if(const int a, const int b, const float c)
		:x(a), y(b), z(c) {};
};