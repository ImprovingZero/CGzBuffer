#include "PolyList.h"
int delActivePolyNum = 0;
int ActiveEdgeNum = 0;

struct dzxy {
	double dzx, dzy;
	dzxy(vec3 nml, double sc, vec3 u, vec3 v, vec3 w)
	{
		double a = nml.dot(u);
		double b = nml.dot(v);
		double c = nml.dot(-w);
		dzx = -a / c * sc;
		dzy = b / c * sc;
	}
	dzxy() :dzx(0), dzy(0) {};
};
std::vector<dzxy> Dz(0);
std::vector<bool> in(0);
std::vector<double> TriClosest(0);
std::vector<vec2i> AABBmin(0);
std::vector<vec2i> AABBmax(0);
std::vector<vec2i> update(0);
std::vector<AABB2if> AABB(0);
std::vector<EdgeListNode*>edges(0);
std::vector<vec2> projects(0);
std::vector<double> polyZ(0);
std::vector<int> octId(0);

inline const int minInd(const double a, const double b, const double c)
{
	if (a < b && a < c) return 0;
	else if (b < c) return 1;
	else return 2;
}
inline void sortTriVec2(std::vector<vec2if>& p)
{
	if (p[0].y < p[1].y) std::swap(p[0], p[1]);
	if (p[0].y < p[2].y) std::swap(p[0], p[2]);
	if (p[1].y < p[2].y) std::swap(p[1], p[2]);
}

void clearGlobal()
{
	AABBmin.clear();
	AABBmax.clear();
	AABB.clear();
	edges.clear();
	TriClosest.clear();
	Dz.clear();
	in.clear();
	octId.clear();
	projects.clear();
	polyZ.clear();
}

vec2if PolyList::projectVertex(vec3 pos)
{
	
	vec3 vecP = pos - _cam->getLLC();
	vec3 p = pos - _cam->_w * Dot(vecP, _cam->_w);
	double x = (p.dot(_cam->_u) / _cam->getHorizontal().length()) * U_PIX_NUM;
	double y = (p.dot(_cam->_v) / _cam->getVertical().length()) * V_PIX_NUM;
	double z = p.dot(_cam->_v);
	/*
	std::cout << "PROJECT VERTEX: \n";
	std::cout << pos.x() << ' ' << pos.y() << ' ' << pos.z() << std::endl;
	std::cout << "LLC pos: " << _cam->getLLC().x() << ' ' << _cam->getLLC().y() << ' '
		<< _cam->getLLC().z() << std::endl;
	std::cout << _cam->getHorizontal().length() << ' ' << _cam->getVertical().length() << std::endl;
	std::cout << "Project at: " << x << ' ' << y << std::endl;
	system("pause");
	*/
	return vec2if(int(x), int(y), z);
}

vec2if PolyList::projectTEMP(Vertex& v)
{
	double x = _model->_pos[v._pos].x;
	double y = _model->_pos[v._pos].y;
	double z = _model->_pos[v._pos].z;
	int x0, y0;
	x0 = int((x - _ltemp) / (_rtemp - _ltemp) * U_PIX_NUM + 0.5f);
	y0 = int((y - _dtemp) / (_utemp - _dtemp) * V_PIX_NUM + 0.5f);
	x0 = int(double(x0) * 0.9 + double(U_PIX_NUM) * 0.05);
	y0 = int(double(y0) * 0.9 + double(V_PIX_NUM) * 0.05);
	return vec2if(x0, y0, z);
}

void PolyList::calcRangeTEMP(Model* model)
{
	_ltemp = DBL_MAX;//FLT_MAX;
	_rtemp = -DBL_MAX;//-FLT_MAX;
	_utemp = -DBL_MAX;// FLT_MAX;
	_dtemp = DBL_MAX;// FLT_MAX;
	for (auto &b : model->_face)
	{
		for (int i = 0; i < 3; i++)
		{
			vec3 a = model->_pos[b._vtx[i]._pos];
			_rtemp = (std::max)(_rtemp, a.x);
			_ltemp = (std::min)(_ltemp, a.x);
			_utemp = (std::max)(_utemp, a.y);
			_dtemp = (std::min)(_dtemp, a.y);
		}
	}
	
	double u = _rtemp - _ltemp;
	double v = _utemp - _dtemp;
	double c_u = (_rtemp + _ltemp) / 2;
	double c_v = (_utemp + _dtemp) / 2;
	
	const double ratio = double(U_PIX_NUM)/double(V_PIX_NUM);
	if (u / v > ratio)
	{
		_utemp = c_v + u / ratio / 2;
		_dtemp = c_v - u / ratio / 2;
	}
	else
	{
		_ltemp = c_u - v * ratio / 2;
		_rtemp = c_u + v * ratio / 2;
	}
	_scaleZ = (_rtemp - _ltemp) / double(U_PIX_NUM);
}

void PolyList::calcCut(vec2if p1, vec2if p2, std::vector<vec2if>& v)
{
	v.clear();
	if (p1.y < p2.y)
	{
		v.push_back(p2); v.push_back(p1);
	}
	else if (p1.y > p2.y)
	{
		v.push_back(p1); v.push_back(p2);
	}
}

//std::vector<std::vector<int>> tempoutput(0);

void PolyList::initScan()
{
	std::ofstream fout("debug_initScan.txt");
	calcRangeTEMP(_model);

	clearGlobal();
	_actList = new ActiveList(this);
	_poly.clear();
	_edge.clear();
	edges.clear();
	for (int i = 0; i < V_PIX_NUM; i++)
	{
		_poly.push_back(std::vector<PolyListNode*>(0));
		_edge.push_back(std::vector<EdgeListNode*>(0));
	}
	
	int paraCull = 0;
	std::cout << "Totol num of faces: " << _model->_face.size() << std::endl;
	for (int id = 0; id < _model->_face.size(); id++)
	{
		fout << id << "-----"<< std::endl;
		if (id % 1000 == 0)
		{
			std::cout << "Initializing PolyList: "
				<< float(id) / float(_model->_face.size())*100 << " %\n";
		}
		auto face = _model->_face[id];
		Dz.push_back(dzxy(face._nml, _scaleZ, _cam->_u, _cam->_v, _cam->_w));
		in.push_back(0);
		for (int k = 0; k < 3; k++) edges.push_back(nullptr);
		if (abs(face._nml.dot(_cam->_w)) < 0.01)
		{
			paraCull++;
			fout << "Continue" << std::endl;
			continue;
		}
		std::vector<vec2if> p(0);
		fout << "P1" << ' ';
		for (int i = 0; i < 3; i++)
		{
			p.push_back(projectTEMP(face._vtx[i]));
			//p.push_back(projectVertex(face._vtx[i]));
		}
		sortTriVec2(p);
		/*
		bool cutOff = 1; //if Triangle is out of screen
		for (int i = 0; i < 3; i++)
		{
			if ((p[i].x() < U_PIX_NUM && p[i].x() >= 0) &&
				(p[i].y() < V_PIX_NUM && p[i].y() >= 0))
			{
				cutOff = 0;
				break;
			}
		}
		if (cutOff) continue;
		*/
		//Polygon List:
		_poly[p[0].y].push_back(
			new PolyListNode(face._nml, id, p[0].y - p[2].y, p[0].z)
		);
		fout << "P2" << ' ';

		std::vector<int> tempY(0);
		//Edge List: MANY COMPLEX SITUATIONS CANNOT HANDLE
		for (int i = 0; i < 3; i++)
		{
			std::vector<vec2if> v(0);
			calcCut(p[i], p[(i + 1) % 3], v);
			
			if (!v.empty())
			{
				_edge[v[0].y].push_back(
					new EdgeListNode(v[0].x, v[0].x - v[1].x, v[0].y - v[1].y, id, v[0].z)
				);
				edges[id * 3 + i] = _edge[v[0].y].back();
				tempY.push_back(v[0].y);
			}
			else
			{
				tempY.push_back(0);
			}
		}
		fout << "P3" << std::endl;
		if (edges[id * 3 + 0] == nullptr || tempY[0] < tempY[1])
			std::swap(edges[id * 3 + 0], edges[id * 3 + 1]);
		if (edges[id * 3 + 0] == nullptr || tempY[0] < tempY[2])
			std::swap(edges[id * 3 + 0], edges[id * 3 + 2]);
		if (edges[id * 3 + 1] == nullptr || tempY[1] < tempY[2])
			std::swap(edges[id * 3 + 1], edges[id * 3 + 2]);
		fout << "finish" << std::endl;
	}
	std::cout << "There are " << paraCull << " triangles parallel to Cam direction been culled" << std::endl;
}

void PolyList::initNaive()
{
	calcRangeTEMP(_model);
	clearGlobal();

	_poly.clear();
	_edge.clear();
	_poly.push_back(std::vector<PolyListNode*>(0));
	_edge.push_back(std::vector<EdgeListNode*>(0));
	int paraCull = 0;
	std::cout << "Totol num of faces: " << _model->_face.size() << std::endl;

	for (int id = 0; id < _model->_face.size(); id++)
	{
		if (id % 1000 == 0)
		{
			std::cout << "Initializing: "
				<< float(id) / float(_model->_face.size()) * 100 << " %\n";
		}

		auto face = _model->_face[id];
		Dz.push_back(dzxy(face._nml, _scaleZ, _cam->_u, _cam->_v, _cam->_w));
		TriClosest.push_back(-DBL_MAX);
		AABBmin.push_back(vec2i(INT_MAX, INT_MAX));
		AABBmax.push_back(vec2i(-INT_MAX, -INT_MAX));

		if (abs(face._nml.dot(_cam->_w)) < 0.01)
		{
			paraCull++;
			continue;
		}
		std::vector<vec2if> p(0);
		for (int i = 0; i < 3; i++)
		{
			p.push_back(projectTEMP(face._vtx[i]));
			//p.push_back(projectVertex(face._vtx[i]));
			if (p[i].z > TriClosest[id]) TriClosest[id] = p[i].z;
			if (p[i].x < AABBmin[id].x) AABBmin[id].x = p[i].x;
			if (p[i].x > AABBmax[id].x) AABBmax[id].x = p[i].x;
			if (p[i].y < AABBmin[id].y) AABBmin[id].y = p[i].y;
			if (p[i].y > AABBmax[id].y) AABBmax[id].y = p[i].y;
			projects.push_back(vec2(p[i].x, p[i].y));
		}
		polyZ.push_back(p[0].z);
		sortTriVec2(p);
		//Polygon List:
		_poly[0].push_back(
			new PolyListNode(face._nml, id, p[0].y - p[2].y, p[0].z)
		);
		_polyY.push_back(p[0].y);

		//Edge List: MANY COMPLEX SITUATIONS CANNOT HANDLE
		std::vector<int> tempY(0);
		for (int i = 0; i < 3; i++)
		{
			std::vector<vec2if> v(0);
			calcCut(p[i], p[(i + 1) % 3], v);

			if (!v.empty())
			{
				_edge[0].push_back(
					new EdgeListNode(v[0].x, v[0].x - v[1].x, v[0].y - v[1].y, id, v[0].z)
				);
				tempY.push_back(v[0].y);
			}
			else
			{
				tempY.push_back(0);
				_edge[0].push_back(nullptr);
			}
		}
		
		int l = _edge[0].size();

		if (_edge[0][l - 3] == nullptr || tempY[0] < tempY[1])
			std::swap(_edge[0][l - 3], _edge[0][l - 2]);
		if (_edge[0][l - 3] == nullptr || tempY[0] < tempY[2]) 
			std::swap(_edge[0][l - 3], _edge[0][l - 1]);
		if (_edge[0][l - 2] == nullptr || tempY[1] < tempY[2])
			std::swap(_edge[0][l - 2], _edge[0][l - 1]);

		//if (_edge[0][id * 3] == nullptr)std::cout << "find nullptr" << std::endl;
	}
	std::cout << "There are " << paraCull 
		<< " triangles parallel to Cam direction been culled" << std::endl;
}

void PolyList::initOctree()
{
	calcRangeTEMP(_model);
	clearGlobal();

	_poly.clear();
	_edge.clear();
	_poly.push_back(std::vector<PolyListNode*>(0));
	_edge.push_back(std::vector<EdgeListNode*>(0));
	int paraCull = 0;
	std::cout << "Totol num of faces: " << _model->_face.size() << std::endl;

	AABB2if object;

	for (int id = 0; id < _model->_face.size(); id++)
	{
		if (id % 1000 == 0)
		{
			std::cout << "Initializing: "
				<< float(id) / float(_model->_face.size()) * 100 << " %\n";
		}

		auto face = _model->_face[id];
		Dz.push_back(dzxy(face._nml, _scaleZ, _cam->_u, _cam->_v, _cam->_w));
		TriClosest.push_back(-DBL_MAX);
		AABB2if temp;
		AABB.push_back(temp);
		//octId.push_back(-1);

		if (abs(face._nml.dot(_cam->_w)) < 0.01)
		{
			paraCull++;
			continue;
		}
		std::vector<vec2if> p(0);
		for (int i = 0; i < 3; i++)
		{
			p.push_back(projectTEMP(face._vtx[i]));
			//p.push_back(projectVertex(face._vtx[i]));
			if (p[i].z > TriClosest[id]) TriClosest[id] = p[i].z;
			AABB[id].add(p[i].x, p[i].y, p[i].z);
		}
		
		octId.push_back(octId.size() - 1);
		//octId.pop_back();
		//octId.push_back(id);
		object.add(AABB[id]);
		sortTriVec2(p);
		//Polygon List:
		_poly[0].push_back(
			new PolyListNode(face._nml, id, p[0].y - p[2].y, p[0].z)
		);
		_polyY.push_back(p[0].y);

		//Edge List: MANY COMPLEX SITUATIONS CANNOT HANDLE
		std::vector<int> tempY(0);
		for (int i = 0; i < 3; i++)
		{
			std::vector<vec2if> v(0);
			calcCut(p[i], p[(i + 1) % 3], v);

			if (!v.empty())
			{
				_edge[0].push_back(
					new EdgeListNode(v[0].x, v[0].x - v[1].x, v[0].y - v[1].y, id, v[0].z)
				);
				tempY.push_back(v[0].y);
			}
			else
			{
				tempY.push_back(0);
				_edge[0].push_back(nullptr);
			}
		}

		int l = _edge[0].size();

		if (_edge[0][l - 3] == nullptr || tempY[0] < tempY[1])
			std::swap(_edge[0][l - 3], _edge[0][l - 2]);
		if (_edge[0][l - 3] == nullptr || tempY[0] < tempY[2])
			std::swap(_edge[0][l - 3], _edge[0][l - 1]);
		if (_edge[0][l - 2] == nullptr || tempY[1] < tempY[2])
			std::swap(_edge[0][l - 2], _edge[0][l - 1]);
	}

	std::cout << "There are " << paraCull
		<< " triangles parallel to Cam direction been culled" << std::endl;

	std::cout << "Start building Oct-tree ...\n";
	_oct = new Octree(object);
	for (auto id : octId)
	{
		//int id = _poly[0][i]->id;
		if (id == -1) continue;
		_oct->putin(AABB[_poly[0][id]->id], id);
	}
	/*
	for (int i = 0; i < _oct->_octNode.size(); i++)
	{
		auto temp = _oct->_octNode[i];

		if (temp->_inc.size()!=0)
			std::cout << "size: "<<temp->_width<<' '<<temp->_height<<' '<<temp->_depth
				<<"   includeTriNum: "<<temp->_inc.size()<< std::endl;
	}
	*/
	std::cout << "Finish building Oct-tree! HOW QUICKLY!\n";
}

void PolyList::activeP(int y)
{
	for (auto a : _poly[y])
	{
		_actList->_actpoly.push_back(a);
		std::vector<int> temp(2);
		int id1=-1, id2=-1;
		for (int i = 0; i < _edge[y].size(); i++)
		{
			if (_edge[y][i]->id == a->id && _edge[y][i]->dy!=0)
			{
				if (id1 == -1) id1 = i;
				else { id2 = i; break;}
			}
		}
		if (id1 == -1 || id2 == -1) continue;
		_actList->_actedge.push_back(
			new ActiveEdgeNode(_edge[y][id1], _edge[y][id2], a->nml,
				_cam->_u, _cam->_v, _cam->_w, a->z,_scaleZ)
		);
	}
}

void PolyList::activePv2(int y)
{
	for (auto a : _poly[y])
	{
		_actList->_actpoly.push_back(a);
		std::vector<EdgeListNode*> temp(0);
		if (edges[a->id * 3 + 0] != nullptr && edges[a->id * 3 + 1] != nullptr)
		{
			temp.push_back(edges[a->id * 3 + 0]);
			temp.push_back(edges[a->id * 3 + 1]);
			_actList->_actedgev2.push_back(temp);
		}
		
	}
}

void PolyList::activePinter(int y)
{
	
	for (auto a : _poly[y])
	{
		_actList->_actpoly.push_back(a);
		std::vector<int> temp(2);
		int id1 = -1, id2 = -1;
		for (int i = 0; i < _edge[y].size(); i++)
		{
			if (_edge[y][i]->id == a->id && _edge[y][i]->dy != 0)
			{
				if (id1 == -1) id1 = i;
				else { id2 = i; break; }
			}
		}
		if (id1 == -1 || id2 == -1) continue;
		_actList->_actedgeInter.push_back(_edge[y][id1]);
		_actList->_actedgeInter.push_back(_edge[y][id2]);
	}
	//std::cout << y << ' ' << _poly[y].size()<<' '
	//	<< _actList->_actedgeInter.size() << std::endl;
}

void PolyList::rastrizeTri(std::vector<std::vector<int>>& output, std::vector<std::vector<double>>& depth)
{
	for (int i = 0; i < _poly[0].size(); i++)
	{
		rastrizeOneTri(output, depth, _polyY[i], _poly[0][i],
			_edge[0][i * 3 + 0], _edge[0][i * 3 + 1], _edge[0][i * 3 + 2]);
	}
}

void PolyList::rastrizeTriQtree(std::vector<std::vector<int>>& output, 
	std::vector<std::vector<double>>& depth, QtreeNode* qt,
	std::vector<std::vector<QtreeNode*>>& QtPtr)
{
	int skip = 0;
	for (int i = 0; i < _poly[0].size(); i++)
	{
		int id = _poly[0][i]->id;
		QtreeNode* test = qt->zTest(AABBmin[id], AABBmax[id], TriClosest[id]);
		if (test == nullptr)
		{
			skip++;
			continue;
		}
		rastrizeOneTri(output, depth, _polyY[i], _poly[0][i],
			_edge[0][i * 3 + 0], _edge[0][i * 3 + 1], _edge[0][i * 3 + 2]);
		
		for (auto& a : update)
		{
			QtPtr[a.y][a.x]->update(depth);
			QtPtr[a.y][a.x]->popup();
		}
	}
	std::cout << skip << " faces have been culled by Q-Tree. How amazing!"<< std::endl;
}

bool isPointInTriangle(vec2 A, vec2 B, vec2 C, vec2 P)
{
	//THIS PART is COPIED from 
	//https://www.cnblogs.com/graphics/archive/2010/08/05/1793393.html
	vec2 v0 = C - A;
	vec2 v1 = B - A;
	vec2 v2 = P - A;

	float dot00 = v0.dot(v0);
	float dot01 = v0.dot(v1);
	float dot02 = v0.dot(v2);
	float dot11 = v1.dot(v1);
	float dot12 = v1.dot(v2);

	float inverDeno = 1 / (dot00 * dot11 - dot01 * dot01);

	float u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
	if (u < 0 || u > 1) // if u out of range, return directly
	{
		return false;
	}

	float v = (dot00 * dot12 - dot01 * dot02) * inverDeno;
	if (v < 0 || v > 1) // if v out of range, return directly
	{
		return false;
	}

	return u + v <= 1;
}

void zTestAndUpdate(std::vector<std::vector<int>>& output, 
	std::vector<std::vector<double>>& depth,
	int pos, int id, QtreeNode* qt,
	std::vector<std::vector<QtreeNode*>>& QtPtr)
{
	double z = TriClosest[id];
	vec2i Min = AABBmin[id];
	vec2i Max = AABBmax[id];
	if (z < qt->_z) return;

	std::deque<QtreeNode*> dq(0);
	dq.push_back(qt);
	while (!dq.empty())
	{
		QtreeNode* h = dq.front();
		//std::cout << h << std::endl;
		dq.pop_front();
		if (h->isLeaf())
		{
			
			for (int i = h->getLeft(); i <= h->getRight(); i++)
			{
				for (int j = h->getDown(); j <= h->getUp(); j++)
				{
					if (isPointInTriangle(projects[pos * 3], projects[pos * 3 + 1],
						projects[pos * 3 + 2], vec2(i, j)))
					{
						double d= polyZ[pos] + (projects[pos * 3].y - j) * Dz[id].dzy
							+ (i - projects[pos * 3].x) * Dz[id].dzx;
						if (d > depth[j][i])
						{
							output[j][i] = id;
							depth[j][i] = d;
							QtPtr[j][i]->update(depth);
							QtPtr[j][i]->popup();
						}
						
					}
				}
			}
		 
			continue;
		}
		for (int i = 0; i < 4; i++)
			if (h->_cld[i]->_z < z && h->overlap(Min, Max))
				dq.push_back(h->_cld[i]);
	}
	return;
}

void PolyList::rastrizeTriQtreeFine(std::vector<std::vector<int>>& output, 
	std::vector<std::vector<double>>& depth, QtreeNode* qt, 
	std::vector<std::vector<QtreeNode*>>& QtPtr)
{
	int skip = 0;
	for (int i = 0; i < _poly[0].size(); i++)
	{
		int id = _poly[0][i]->id;
		QtreeNode* test = qt->zTestFine(AABBmin[id], AABBmax[id], TriClosest[id]);
		if (test == nullptr)
		{
			skip++;
			continue;
		}
		rastrizeOneTri(output, depth, _polyY[i], _poly[0][i],
			_edge[0][i * 3 + 0], _edge[0][i * 3 + 1], _edge[0][i * 3 + 2]);

		for (auto& a : update)
		{
			QtPtr[a.y][a.x]->update(depth);
			QtPtr[a.y][a.x]->popup();
		}
	}
	std::cout << skip << " faces have been culled by Q-Tree(Fine version). How amazing!" << std::endl;
}

void PolyList::rastrizeTriQtreeFinev2(std::vector<std::vector<int>>& output, 
	std::vector<std::vector<double>>& depth, 
	QtreeNode* qt, std::vector<std::vector<QtreeNode*>>& QtPtr)
{
	//int skip = 0;
	for (int i = 0; i < _poly[0].size(); i++)
	{
		int id = _poly[0][i]->id;
		//std::cout << id << ' ' << std::endl;
		zTestAndUpdate(output, depth, i, id, qt, QtPtr);
	}
	//std::cout << skip << " faces have been culled by Q-Tree(Fine version). How amazing!" << std::endl;

}

void PolyList::rastrizeTriQtreeComp(std::vector<std::vector<int>>& output, 
	std::vector<std::vector<double>>& depth, QtreeNode* qt,
	std::vector<std::vector<QtreeNode*>>& QtPtr)
{
	int skip = 0;
	auto oct = _oct->_octNode;
	int numtry = 0;
	std::deque<std::pair<OctreeNode*, QtreeNode*>> dq(0);
	dq.push_back(std::make_pair(oct[0], qt));

	while (!dq.empty())
	{
		OctreeNode* t8 = dq.front().first;
		QtreeNode* t4 = dq.front().second;
		
		dq.pop_front();
		QtreeNode* test;
		test = t4->zTest(t8->get2dMin(), t8->get2dMax(), t8->getNear());
		if (test == nullptr)
		{
			numtry += t8->_inc.size();
			continue;
		}
		for (auto i : t8->_inc)
		{
			int id = _poly[0][i]->id;
		
			auto test1 = t4->zTest(AABB[id]._min, AABB[id]._max, TriClosest[id]);
			if (test1 != nullptr)
			{
				rastrizeOneTri(output, depth, _polyY[i], _poly[0][i],
					_edge[0][i * 3 + 0], _edge[0][i * 3 + 1], _edge[0][i * 3 + 2]);
				for (auto& a : update)
				{
					//std::cout << "1" << std::endl;
					QtPtr[a.y][a.x]->update(depth);
					QtPtr[a.y][a.x]->popup();
				}
				/*
				if (numdraw % (_poly[0].size() / 40) == 0)
				{
					qt->update(depth);
				}
				*/
				//test1->update(depth);
				//test->popup();
				
			}
			else numtry++;
		}

		if (!t8->isLeaf())
		{
			for (int j = 0; j < 8; j++)
			{
				bool f = 0;
				for (int i = 0; i < 4; i++)
				{
					vec2i Mintemp = oct[t8->_cld[j]]->get2dMin();
					vec2i Maxtemp = oct[t8->_cld[j]]->get2dMax();
					if (t4->_cld[i]->inside(Mintemp, Maxtemp))
					{
						dq.push_front(std::make_pair(oct[t8->_cld[j]], t4->_cld[i]));
						f = 1; break;
					}
				}
				if (!f)
				{
					dq.push_front(std::make_pair(oct[t8->_cld[j]], t4));
				}
			}
		}
	}
	std::cout << "Num culled faces: "<<numtry << std::endl;
}


void PolyList::rastrizeTriQtreeCompFine(std::vector<std::vector<int>>& output, std::vector<std::vector<double>>& depth, QtreeNode* qt, std::vector<std::vector<QtreeNode*>>& QtPtr)
{
	int skip = 0;
	auto oct = _oct->_octNode;
	int numtry = 0;
	std::deque<std::pair<OctreeNode*, QtreeNode*>> dq(0);
	dq.push_back(std::make_pair(oct[0], qt));

	while (!dq.empty())
	{
		OctreeNode* t8 = dq.front().first;
		QtreeNode* t4 = dq.front().second;

		dq.pop_front();
		QtreeNode* test;
		test = t4->zTestFine(t8->get2dMin(), t8->get2dMax(), t8->getNear());
		if (test == nullptr)
		{
			numtry += t8->_inc.size();
			continue;
		}
		for (auto i : t8->_inc)
		{
			int id = _poly[0][i]->id;

			auto test1 = t4->zTestFine(AABB[id]._min, AABB[id]._max, TriClosest[id]);
			if (test1 != nullptr)
			{
				rastrizeOneTri(output, depth, _polyY[i], _poly[0][i],
					_edge[0][i * 3 + 0], _edge[0][i * 3 + 1], _edge[0][i * 3 + 2]);
				for (auto& a : update)
				{
					QtPtr[a.y][a.x]->update(depth);
					QtPtr[a.y][a.x]->popup();
				}
			}
			else numtry++;
		}

		if (!t8->isLeaf())
		{
			for (int j = 0; j < 8; j++)
			{
				bool f = 0;
				for (int i = 0; i < 4; i++)
				{
					vec2i Mintemp = oct[t8->_cld[j]]->get2dMin();
					vec2i Maxtemp = oct[t8->_cld[j]]->get2dMax();
					if (t4->_cld[i]->inside(Mintemp, Maxtemp))
					{
						dq.push_front(std::make_pair(oct[t8->_cld[j]], t4->_cld[i]));
						f = 1; break;
					}
				}
				if (!f)
				{
					dq.push_front(std::make_pair(oct[t8->_cld[j]], t4));
				}
			}
		}
	}
	std::cout << "Num culled faces: " << numtry << std::endl;
}

void PolyList::rastrizeTriQtreeCompFinev2(std::vector<std::vector<int>>& output, std::vector<std::vector<double>>& depth, QtreeNode* qt, std::vector<std::vector<QtreeNode*>>& QtPtr)
{
}

void PolyList::rastrizeOneTri(std::vector<std::vector<int>>& output, 
	std::vector<std::vector<double>>& depth, int y,
	PolyListNode* poly, EdgeListNode* e1, EdgeListNode* e2, EdgeListNode* e3)
{
	update.clear();
	EdgeListNode* el = e1;
	EdgeListNode* er = e2;
	while (poly->dy > 0)
	{
		if (el->dy == 0)
		{
			el = e3;
		}
		if (er->dy == 0)
		{
			er = e3;
		}
		if (el->x > er->x) std::swap(el, er);

		double z = el->z;
		for (int j = int(el->x); j<int(er->x); j++)
		{
			if (z > depth[y][j])
			{
				depth[y][j] = z;
				output[y][j] = poly->id;
				update.push_back(vec2i(j, y));
			}
			z += Dz[poly->id].dzx;
		}
		el->x += el->dx;
		er->x += er->dx;
		el->z += Dz[poly->id].dzy;
		el->z += Dz[poly->id].dzx * el->dx;
		el->dy--;
		er->dy--;
		poly->dy--;
		y--;
	}
}

void ActiveList::delActiveP(int y)
{
	for (auto p = _actpoly.begin(); p != _actpoly.end(); )
	{
		if ((*p)->dy == 0)
		{
			//for naive scan Z buffer:
			for (auto q = _actedge.begin(); q != _actedge.end();)
			{
				if ((*p)->id == (*q)->id)
				{
					q = _actedge.erase(q);
				}
				else q++;
			}
			/*
			//for interval scan Z buffer
			for (auto q = _actedgeInter.begin(); q != _actedgeInter.end();)
			{
				if ((*p)->id == (*q)->id)
				{
					q = _actedgeInter.erase(q);
				}
				else q++;
			}
			*/
			p = _actpoly.erase(p);
			delActivePolyNum++;
		}
		else p++;
	}
	for (auto p = _actedgev2.begin(); p != _actedgev2.end();)
	{
		if ((*p)[0]->dy == 0 && (*p)[1]->dy == 0)
		{
			p = _actedgev2.erase(p);
		}
		else p++;
	}
	for (auto q = _actedgeInter.begin(); q != _actedgeInter.end();)
	{
		if ((*q)->dy==0)
		{
			q = _actedgeInter.erase(q);
		}
		else q++;
	}
}

void ActiveList::updataActiveE(int y)
{
	for (auto p = _actedge.begin(); p != _actedge.end(); p++)
	{
		auto& e = *p;
		e->zl += e->dzy;
		e->zl += e->dxl * e->dzx;
		e->xl += e->dxl;
		e->xr += e->dxr;
		if (e->dyl == 0)
		{
			for (auto a : _polyList->_edge[y])
			{
				if (a->id == e->id)// && abs(e->dxl - a->x) < 1e-2)
				{
					e->xl = a->x;
					e->dxl = a->dx;
					e->dyl = a->dy;
				}
			}
		}
		if (e->dyr == 0)
		{
			for (auto a : _polyList->_edge[y])
			{
				if (a->id == e->id)// && abs(e->dxr - a->x) < 1e-2)
				{
					e->xr = a->x;
					e->dxr = a->dx;
					e->dyr = a->dy;
				}
			}
		}
	}
}

void ActiveList::updateActiveEv2(int y)
{
	for (auto p = _actedgev2.begin(); p != _actedgev2.end(); p++)
	{
		(*p)[0]->x += (*p)[0]->dx;
		(*p)[1]->x += (*p)[1]->dx;
		if ((*p)[0]->x > (*p)[1]->x) std::swap((*p)[0], (*p)[1]);
		(*p)[0]->z += Dz[(*p)[0]->id].dzy;
		(*p)[0]->z += Dz[(*p)[0]->id].dzx * (*p)[0]->dx;
		if ((*p)[0]->dy == 0) (*p)[0] = edges[(*p)[0]->id * 3 + 2];
		if ((*p)[1]->dy == 0) (*p)[1] = edges[(*p)[0]->id * 3 + 2];
	}
}

void ActiveList::draw(int y, std::vector<double>& depth, std::vector<int>& buffer)
{
	for (auto& e : _actedge)
	{
		double z = e->zl;
		for (int i = int(e->xl); i < int(e->xr); i++)
		{
			if (z > depth[i])
			{
				depth[i] = z;
				buffer[i] = e->id;
			}
			z += e->dzx;
		}
	}
}

void ActiveList::drawv2(int y, std::vector<double>& depth, std::vector<int>& buffer)
{
	for (auto p : _actedgev2)
	{
		double z = p[0]->z;
		for (int i = int(p[0]->x); i<int(p[1]->x); i++)
		{
			if (z > depth[i])
			{
				depth[i] = z;
				buffer[i] = p[0]->id;
			}
			z += Dz[p[0]->id].dzx;
		}
	}
}

void ActiveList::decDy()
{
	for (auto& a : _actpoly)
	{
		a->dy--;
	}
	for (auto& a : _actedge)
	{
		a->dyl--;
		a->dyr--;
	}
	for (auto& a : _actedgev2)
	{
		a[0]->dy--;
		a[1]->dy--;
	}
}

bool cmpInter(EdgeListNode* a, EdgeListNode* b)
{
	return int(a->x) < int(b->x);
}

double sortUseTime = 0;
void ActiveList::updateActiveEInter(int y)
{
	for (auto p = _actedgeInter.begin(); p != _actedgeInter.end(); p++)
	{
		auto& e = *p;
		e->z += Dz[e->id].dzy;
		e->z += Dz[e->id].dzx * e->dx;
		e->x += e->dx;
		if (e->dy == 0)
		{
			for (auto a : _polyList->_edge[y])
			{
				if (a->id == e->id)// && abs(e->dxl - a->x) < 1e-2)
				{
					*e = *a;
				}
			}
		}
	}
	/*
	LARGE_INTEGER nFreq;
	LARGE_INTEGER nBeginTime;
	LARGE_INTEGER nEndTime;

	QueryPerformanceFrequency(&nFreq);
	QueryPerformanceCounter(&nBeginTime);

	
	QueryPerformanceCounter(&nEndTime);
	sortUseTime += (double)(nEndTime.QuadPart - nBeginTime.QuadPart) / (double)nFreq.QuadPart;
	*/
	_actedgeInter.sort(cmpInter);
	//std::sort(_actedgeInter.begin(), _actedgeInter.end(), cmpInter);
}
std::ofstream fout("debug.txt");
void ActiveList::drawInter(int y, std::vector<double>& depth, std::vector<int>& buffer)
{
	//std::set<int> nowIn;
	std::map<int, EdgeListNode*> m;
	m.clear();
	std::stack<EdgeListNode*> st;
	std::set<EdgeListNode*> edgeIn;
	edgeIn.clear();
	EdgeListNode* nowe = NULL;
	for (int i = 0; i < in.size(); i++) in[i] = 0;
	
	int color = -1;
	double z = -DBL_MAX;
	
	auto p = _actedgeInter.begin();
	for (int i = 0; i < U_PIX_NUM; i++)
	{
		while (p!=_actedgeInter.end() && int((*p)->x) == i)
		{
			if (in[(*p)->id] == false)
			{
				in[(*p)->id] = true;
				edgeIn.insert(*p);
				if (color == -1)
				{
					color = (*p)->id;
					z = (*p)->z;
				}
				else
				{//compare
					if ((*p)->z + Dz[(*p)->id].dzx > z + Dz[color].dzx)
					{
						//std::cout << i << ":  change " << color << " to " << (*p)->id << std::endl;
						z = (*p)->z;
						color = (*p)->id;
					}
				}
			}
			else
			{
				in[(*p)->id] = false;
				edgeIn.erase(*p);
				color = -1;
				z = -DBL_MAX;
				
				//for (auto q = _actedgeInter.begin(); q != p; q++)
				for (auto q=edgeIn.begin();q!=edgeIn.end();q++)
				{
					if (in[(*q)->id] && (*q)->id != (*p)->id &&
						(color == -1 ||
						((*q)->z + Dz[(*q)->id].dzx * (i - int((*q)->x) + 1) > z + Dz[color].dzx)))
					{
						z = (*q)->z + Dz[(*q)->id].dzx * (i - int((*q)->x));
						color = (*q)->id;
					}
				}
				
			}
			p++;
		}
		if (p == _actedgeInter.end())
		{
			buffer[i] = -1;
			continue;
		}
		buffer[i] = color;
		
		
		if (color != -1) z += Dz[color].dzx;
	}
}

void ActiveList::decDyInter()
{
	for (auto& a : _actpoly)
	{
		a->dy--;
	}
	for (auto& a : _actedgeInter)
	{
		a->dy--;
	}
}
