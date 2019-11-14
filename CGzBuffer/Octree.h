#pragma once
#include"common.h"

class AABB2if
{
public:
	vec2if _min;
	vec2if _max;

	AABB2if() :
		_min(vec2if(INT_MAX, INT_MAX, DBL_MAX)),
		_max(vec2if(-INT_MAX, -INT_MAX, -DBL_MAX)) {}
	
	inline void add(int x, int y, double z)
	{
		if (x < _min.x) _min.x = x;
		if (x > _max.x) _max.x = x;
		if (y < _min.y) _min.y = y;
		if (y > _max.y) _max.y = y;
		if (z < _min.z) _min.z = z;
		if (z > _max.z) _max.z = z;
	}
	inline void add(AABB2if& a)
	{
		add(a._max.x, a._max.y, a._max.z);
		add(a._min.x, a._min.y, a._min.z);
	}
	inline void setBox(int x, int y, double z, int w, int h, double d)
	{
		_min.x = x; _min.y = y; _min.z = z;
		_max.x = x + w - 1; _max.y = y + h - 1; _max.z = z + d;
	}
	inline const int width() const { return _max.x - _min.x + 1; }
	inline const int height() const { return _max.y - _min.y + 1; }
	inline const double depth() const { return _max.z - _min.z; }

};

class OctreeNode
{
public:
	vec2if _corner;
	int _width;
	int _height;
	double _depth;
	int _fa;
	int _cld[8];
	std::vector<int> _inc;

	OctreeNode(AABB2if& a, int fa = -1) :_fa(fa)
	{
		_inc.clear();
		_corner = a._min;
		_width = a.width();
		_height = a.height();
		_depth = a.depth();
		for (auto& i : _cld) i = -1;
	}
	
	inline const int getLeft() const { return _corner.x; }
	inline const int getRight() const { return _corner.x + _width - 1; }
	inline const int getDown() const { return _corner.y; }
	inline const int getUp() const { return _corner.y + _height - 1; }
	inline const double getNear() const { return _corner.z + _depth; }
	inline const double getFar() const { return _corner.z ; }
	inline const vec2i get2dMin() const
	{
		return vec2i(getLeft(), getDown());
	}
	inline const vec2i get2dMax() const
	{
		return vec2i(getRight(), getUp());
	}
	void divide(std::vector<AABB2if>& aabb) const;

	inline const bool inside(AABB2if& aabb) const
	{
		if (aabb._min.x >= getLeft() && aabb._max.x<=getRight() &&
			aabb._min.y>=getDown() && aabb._max.y <= getUp() &&
			aabb._min.z<=getNear() && aabb._max.z >= getFar())
			return true;
		else return false;
	}
	inline const bool isLeaf() const
	{
		return (_width < 32 || _height < 32);
	}
};

class Octree
{
public:
	std::vector<OctreeNode*> _octNode;
	Octree(AABB2if &a);
	void putin(AABB2if& a, int id);

};
