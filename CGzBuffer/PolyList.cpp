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
std::vector<EdgeListNode*>edges(0);

inline const int minInd(const double a, const double b, const double c)
{
	if (a < b && a < c) return 0;
	else if (b < c) return 1;
	else return 2;
}
inline void sortTriVec2(std::vector<vec2if>& p)
{
	//std::cout << "sort-----" << std::endl;
	//std::cout << p[0].y << ' '<<p[1].y << ' ' << p[2].y << std::endl;
	if (p[0].y < p[1].y) std::swap(p[0], p[1]);
	if (p[0].y < p[2].y) std::swap(p[0], p[2]);
	if (p[1].y < p[2].y) std::swap(p[1], p[2]);
	//std::cout << p[0].y << ' '<<p[1].y << ' ' << p[2].y << std::endl;
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
	//x0 = x0 * 0.9 + U_PIX_NUM * 0.05;
	//y0 = y0 * 0.9 + V_PIX_NUM * 0.05;
	//std::cout << v._pos << ' ' << z << std::endl;
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
	//_scaleZ = (_utemp - _dtemp) / double(V_PIX_NUM);
	//_scaleZ = 1.f;
	//std::cout << _ltemp << ' ' << _rtemp << ' ' << _utemp << ' ' << _dtemp << std::endl;
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
	calcRangeTEMP(_model);

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
		if (id % 1000 == 0)
		{
			std::cout << "Initializing PolyList: "
				<< float(id) / float(_model->_face.size())*100 << " %\n";
		}
		auto face = _model->_face[id];
		Dz.push_back(dzxy(face._nml, _scaleZ, _cam->_u, _cam->_v, _cam->_w));
		in.push_back(0);
		for (int k = 0; k < 3; k++) edges.push_back(nullptr);
		if (abs(face._nml.dot(_cam->_w)) < 0.05)
		{
			paraCull++;
			continue;
		}
		std::vector<vec2if> p(0);
		
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

		int EdgeNum = 0;
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
				EdgeNum++;
			}
		}
	}
	std::cout << "There are " << paraCull << " triangles parallel to Cam direction been culled" << std::endl;
}

void PolyList::initNaive()
{
	calcRangeTEMP(_model);
	int c1 = 0, c2 = 0, c3 = 0;
	_actList = new ActiveList(this);
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

		if (abs(face._nml.dot(_cam->_w)) < 0.05)
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
		}
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
		//std::cout << l<<' '<<l-3 << std::endl;

		if (_edge[0][l - 3] == nullptr || tempY[0] < tempY[1])
			std::swap(_edge[0][l - 3], _edge[0][l - 2]);
		if (_edge[0][l - 3] == nullptr || tempY[0] < tempY[2]) 
			std::swap(_edge[0][l - 3], _edge[0][l - 1]);
		if (_edge[0][l - 2] == nullptr || tempY[1] < tempY[2])
			std::swap(_edge[0][l - 2], _edge[0][l - 1]);
		//if (_edge[0][l - 3] == nullptr) c1++;
		//if (_edge[0][l - 2] == nullptr) c2++;
		//if (_edge[0][l - 1] == nullptr) c3++;
		
		//std::cout << "dead" << std::endl;
	}
	std::cout << "There are " << paraCull << " triangles parallel to Cam direction been culled" << std::endl;
	//std::cout << c1 << ' ' << c2 << ' ' << c3 << std::endl;

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
	/*
	using namespace std;
	for (int i=0;i<_poly[0].size();i++)
	{
		PolyListNode* poly = _poly[0][i];
		EdgeListNode* el = _edge[0][i * 3 + 0];
		EdgeListNode* er = _edge[0][i * 3 + 1];
		cout << el->x << ' ' << er->x << ' ' << _edge[0][i * 3 + 2]->x << endl;
		int y = _polyY[i];
		while (poly->dy > 0)
		{
			if (el->dy == 0)
			{
				el = _edge[0][i * 3 + 2];
			}
			if (er->dy == 0)
			{
				er = _edge[0][i * 3 + 2];
			}
			if (el->x > er->x) std::swap(el, er);

			double z = el->z;
			for (int j = int(el->x); j<int(er->x); j++)
			{
				if (z > depth[y][j])
				{
					depth[y][j] = z;
					output[y][j] = poly->id;
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
 */
}

void PolyList::rastrizeTriQtree(std::vector<std::vector<int>>& output, 
	std::vector<std::vector<double>>& depth, QtreeNode* qt,
	std::vector<std::vector<QtreeNode*>>& QtPtr)
{
	//update.clear();
	for (int i = 0; i < _poly[0].size(); i++)
	{
		//std::cout << i << std::endl;
		//std::cout << AABBmin[i].x<<' '<<AABBmin[i].y<<' '
		//	<<AABBmax[i].x<<' '<<AABBmax[i].y<<' '<<TriClosest[i] << std::endl;
		int id = _poly[0][i]->id;
		QtreeNode* test = qt->zTest(AABBmin[id], AABBmax[id], TriClosest[id]);
		//std::cout << test << std::endl;
		if (test == nullptr) std::cout << TriClosest[i] << std::endl;
		if (test==nullptr) continue;
		rastrizeOneTri(output, depth, _polyY[i], _poly[0][i],
			_edge[0][i * 3 + 0], _edge[0][i * 3 + 1], _edge[0][i * 3 + 2]);
		//test->update(depth);
		//test->popup();
		
	}
}

void PolyList::rastrizeOneTri(std::vector<std::vector<int>>& output, 
	std::vector<std::vector<double>>& depth, int y,
	PolyListNode* poly, EdgeListNode* e1, EdgeListNode* e2, EdgeListNode* e3)
{
	using namespace std;

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

void ActiveList::draw(int y, std::vector<double>& depth, std::vector<int>& buffer)
{
	//std::cout << "ActiveList::Draw:: active poly num = " << _actpoly.size() << std::endl;
	//std::cout << "ActiveList::Draw:: active edge num = " << _actedge.size() << std::endl;
	for (auto e : _actedge)
	{
		//std::cout << e->id << std::endl;
		double z = e->zl;
		//std::cout  << " --- z: " << z << ' ' << e->id << std::endl;
		//std::cout << e->dxl << ' ' << e->dxr << std::endl;
		for (int i = int(e->xl); i <= int(e->xr)-1; i++)
		{
			//if (i >= U_PIX_NUM) break;
			if (z > depth[i])
			{
				//std::cout << "**********draw: " <<i<< std::endl;
				depth[i] = z;
				buffer[i] = e->id;
			}
			z += e->dzx;
		}
	}
	
	//std::cout << delActivePolyNum << std::endl;
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
}

bool cmpInter(EdgeListNode* a, EdgeListNode* b)
{
	//if (a->x < b->x || (abs(a->x - b->x) < 1e-6 && Dz[a->id].dzx > Dz[b->id].dzx))return 1;
	//else return 0;
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

	QueryPerformanceFrequency(&nFreq);//获取系统时钟频率
	QueryPerformanceCounter(&nBeginTime);//获取开始时刻计数值

	
	QueryPerformanceCounter(&nEndTime);//获取停止时刻计数值
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
	//if (y == 0) std::cout << sortUseTime*1000 << "ms"<<std::endl;
	/*
	for (auto& i : _actedgeInter)
		std::cout << i->id << ' ' << i->x << ' ' << i->z << ' ' << i->dy
		<< " dx: " << i->dx << " dzx: "<<Dz[i->id].dzx<<" dzy: "<<Dz[i->id].dzy<<std::endl;
	for (int i = 0; i < U_PIX_NUM; i++)
	{
		if (buffer[i] == -1) std::cout << ' ';
		else std::cout << char(buffer[i] + 'a');
	}
	std::cout << std::endl;

	if (y == 118) system("pause");
	*/
	
	//std::cout << delActivePolyNum << std::endl;
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
