#include "Qtree.h"

int numQtNode=0;

QtreeNode::QtreeNode(int w, int h, vec2i llc, QtreeNode* f)
	:_width(w),_height(h),_llc(llc),_fa(f),_z(-DBL_MAX)
{
	if (isLeaf())
	{
		for (int i = 0; i < 4; i++) _cld[i] = nullptr;
	}
	else
	{
		//cout << w << ' ' << h << endl;
		int l = _llc.x;
		int d = _llc.y;
		int midw = _width / 2;
		int midh = _height / 2;
		_cld[0] = new QtreeNode(midw, midh, vec2i(l, d), this);
		_cld[1] = new QtreeNode(w - midw, midh, vec2i(l + midw, d), this);
		_cld[2] = new QtreeNode(midw, h - midh, vec2i(l, d + midh), this);
		_cld[3] = new QtreeNode(w - midw, h - midh, vec2i(l + midw, d + midh), this);
	}
}

QtreeNode* QtreeNode::zTest(vec2i Min, vec2i Max, double z)
{
	if (z < _z) return nullptr;
	if (isLeaf())
	{
		if (z > _z) return this;
		else return nullptr;
	}
	int checkNum = -1;
	for (int i = 0; i < 4; i++)
		if (_cld[i]->inside(Min, Max)) checkNum = i;

	if (checkNum != -1)
	{
		return _cld[checkNum]->zTest(Min, Max, z);
	}

	return this;
}

QtreeNode* QtreeNode::zTest(vec2if& Min, vec2if& Max, double z)
{
	return zTest(vec2i(Min.x, Min.y), vec2i(Max.x, Max.y), z);
}

const double QtreeNode::update(std::vector<std::vector<double>>& depth)
{
	if (this->isLeaf())
	{
		_z = DBL_MAX;
		for (int i = getLeft(); i <= getRight(); i++)
		{
			for (int j = getDown(); j <= getUp(); j++)
				if (depth[j][i] < _z) _z = depth[j][i];
		}
		//std::cout << _z <<std::endl;
		return _z;
	}
	else
	{
		_z = DBL_MAX;
		for (int i = 0; i < 4; i++)
		{
			double temp = _cld[i]->update(depth);
			if (temp < _z) _z = temp;
		}
		return _z;
	}
}

void QtreeNode::popup() const
{
	QtreeNode* p = _fa;
	while (p != nullptr)
	{
		if (_z < p->_z)
		{
			p->_z = _z;
			p = p->_fa;
		}
		else break;
	}
}
int ddd = 0;
void QtreeNode::travelOutput(std::vector<std::vector<double>>& depth) const
{
	ddd++;
	//std::cout << _width << ' ' << _height << ' ' << isLeaf() << ' '<<_z<<std::endl;
	if (isLeaf())
	{
		for (int i = getDown(); i <= getUp(); i++)
		{
			for (int j = getLeft(); j <= getRight(); j++)
			{
				depth[i][j] = _z;
			}
		}
	}
	else
	{
		for (int i = 0; i < 4; i++)
			_cld[i]->travelOutput(depth);
	}
}
