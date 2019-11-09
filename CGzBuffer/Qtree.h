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

	const bool isLeaf() const
	{
		return (_width <= 1 || _height <= 1);
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

