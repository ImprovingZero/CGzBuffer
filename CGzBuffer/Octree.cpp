#include "Octree.h"

Octree::Octree(AABB2if& a)
{
	//std::cout << "Octree::Octree---\n" << a._min.x << ' ' << a._max.x << ' '
	//		<< a._min.y << ' ' << a._max.y << std::endl;
	_octNode.clear();
	_octNode.push_back(new OctreeNode(a));
	int h = -1;
	while (h + 1 < _octNode.size())
	{
		h++;
		auto& p = _octNode[h];
		if (p->isLeaf()) continue;
		std::vector<AABB2if> aabb(0);
		p->divide(aabb);
		for (int i = 0; i < 8; i++)
		{
			p->_cld[i] = _octNode.size();
			_octNode.push_back(new OctreeNode(aabb[i], h));
		}
	}
}

void Octree::putin(AABB2if& a, int id)
{
	OctreeNode* p = _octNode[0];
	//std::cout << "PUTIN::---\n" << a._min.x << ' ' << a._max.x << ' '
	//	<< a._min.y << ' ' << a._max.y << ' ' << a._min.z << ' '<<a._max.z << std::endl;
	//system("pause");
	while (!p->isLeaf())
	{
		int findOut = -1;
		for (int i = 0; i < 8; i++)
		{
			if (_octNode[p->_cld[i]]->inside(a))
			{
				findOut = i;
				break;
			}
		}
		//std::cout << findOut << std::endl;
		if (findOut == -1)
		{
			p->_inc.push_back(id);
			return;
		}
		p = _octNode[p->_cld[findOut]];
	}
	p->_inc.push_back(id);
	
}

void OctreeNode::divide(std::vector<AABB2if>& aabb) const
{
	int midw = _width / 2;
	int midh = _height / 2;
	double midd = _depth / 2;
	for (int i = 0; i < 8; i++) aabb.push_back(AABB2if());
	aabb[0].setBox(_corner.x, _corner.y, _corner.z, midw, midh, midd);
	aabb[1].setBox(_corner.x + midw, _corner.y, _corner.z, _width - midw, midh, midd);
	aabb[2].setBox(_corner.x, _corner.y + midh, _corner.z, midw, _height - midh, midd);
	aabb[3].setBox(_corner.x + midw, _corner.y + midh, _corner.z, _width - midw, _height - midh, midd);
	aabb[4].setBox(_corner.x, _corner.y, _corner.z + midd, midw, midh, _depth-midd);
	aabb[5].setBox(_corner.x + midw, _corner.y, _corner.z + midd, _width - midw, midh, _height - midd);
	aabb[6].setBox(_corner.x, _corner.y + midh, _corner.z + midd, midw, _height - midh, _depth - midd);
	aabb[7].setBox(_corner.x + midw, _corner.y + midh, _corner.z + midd, _width - midw, _height - midh, _depth - midd);
	//Thankfully we live in 3d world where no need for hex-tree
}
