#pragma once
#include"common.h"
//#include"ZBuffer.h"

class QtreeNode
{
public:
	double _z;
	QtreeNode* _cld[4];
	vec2i _llc;
	int _width;
	int _height;
	QtreeNode* _fa;

	QtreeNode(int w, int h, vec2i llc, QtreeNode* f);

	const int getLeft() const { return _llc.x; }
	const int getRight() const { return _llc.x + _width - 1; }
	const int getDown() const { return _llc.y; }
	const int getUp() const { return _llc.y + _height - 1; }

	inline const bool inside(vec2i& Min, vec2i& Max) const
	{
		if (Min.x > getLeft() && Max.x<getRight() &&
			Min.y>getDown() && Max.y < getUp())
			return true;
		else return false;
	}

	QtreeNode* zTest(vec2i& Min, vec2i& Max, double z);
	const double update(std::vector<std::vector<double>>& depth);
	void popup() const;

	const bool isLeaf() const
	{
		return (_width < 32 || _height < 32);
	}
};

class Qtree
{
public:
	//ZBuffer* _zbuffer;
	QtreeNode* _root;
	int _width;
	int _height;
};

