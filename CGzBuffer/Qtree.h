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
	QtreeNode(int w, int h, vec2i llc, QtreeNode* f,
		std::vector<std::vector<QtreeNode*>>& ptr);

	const int getLeft() const { return _llc.x; }
	const int getRight() const { return _llc.x + _width - 1; }
	const int getDown() const { return _llc.y; }
	const int getUp() const { return _llc.y + _height - 1; }

	inline const bool inside(vec2i& Min, vec2i& Max) const
	{
		if (Min.x >= getLeft() && Max.x <= getRight() &&
			Min.y >= getDown() && Max.y <= getUp())
			return true;
		else return false;
	}

	QtreeNode* zTest(vec2i Min, vec2i Max, double z);
	QtreeNode* zTest(vec2if& Min, vec2if& Max, double z);
	const double update(std::vector<std::vector<double>>& depth);
	void popup() const;

	void travelOutput(std::vector<std::vector<double>>& depth) const;

	inline const bool isLeaf() const
	{
		return (_width < 2 || _height < 2);
	}
};

