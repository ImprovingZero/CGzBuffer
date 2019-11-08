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
	/*/
	for (auto &a: model->_pos)
	{
		_rtemp = std::max(_rtemp, a.x);
		_ltemp = std::min(_ltemp, a.x);
		_utemp = std::max(_utemp, a.y);
		_dtemp = std::min(_dtemp, a.y);
	}
	*/
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

void PolyList::init()
{
	calcRangeTEMP(_model);

	_actList = new ActiveList(this);
	_poly.clear();
	_edge.clear();
	for (int i = 0; i < V_PIX_NUM; i++)
	{
		_poly.push_back(std::vector<PolyListNode*>(0));
		_edge.push_back(std::vector<EdgeListNode*>(0));
	}
	
	int paraCull = 0;
	//int id = 0;
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
		//std::cout << "Face" << id << "  " << p[0].y
		//	<< ' ' << p[1].y << ' ' << p[2].y << std::endl;
		//Polygon List:
		_poly[p[0].y].push_back(
			new PolyListNode(face._nml, id, p[0].y - p[2].y, p[0].z)
		);

		//std::cout << "id: " << id << "  z: " << p[0].z << "  dzx: " << Dz[id].dzx 
		//	<< "  dzy: " << Dz[id].dzy << std::endl;

		int EdgeNum = 0;
		//Edge List: MANY COMPLEX SITUATIONS CANNOT HANDLE
		for (int i = 0; i < 3; i++)
		{
			//tempoutput[p[i].y][p[i].x] = id;
			std::vector<vec2if> v(0);
			calcCut(p[i], p[(i + 1) % 3], v);
			
			if (!v.empty())
			{
				_edge[v[0].y].push_back(
					new EdgeListNode(v[0].x, v[0].x - v[1].x, v[0].y - v[1].y, id, v[0].z)
				);
				EdgeNum++;
			}
		}
	}
	std::cout << "There are " << paraCull << " triangles parallel to Cam direction been culled" << std::endl;
	
	/*
	for (int i = V_PIX_NUM - 1; i >= 0; i--)
	{
		if (!_poly[i].empty())
			std::cout << i << ' ' << _poly[i].size() << std::endl;
	}
	system("pause");
	std::cout << "======================================================" << std::endl;
	for (int i = V_PIX_NUM-1; i >= 0; i--)
	{
		for (int j = 0; j < U_PIX_NUM; j++)
		{
			if (tempoutput[i][j] == -1) std::cout << ' ';
			else std::cout << char(tempoutput[i][j] + 'a');
		}
		std::cout << std::endl;
	}
	std::cout << "======================================================" << std::endl;
	*/
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
			//for interval scan Z buffer
			for (auto q = _actedgeInter.begin(); q != _actedgeInter.end();)
			{
				if ((*p)->id == (*q)->id)
				{
					q = _actedgeInter.erase(q);
				}
				else q++;
			}
			p = _actpoly.erase(p);
			delActivePolyNum++;
		}
		else p++;
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
	using namespace std;
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
				//nowIn.insert((*p)->id);
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
				//fout << i << ":  Dead " << (*p)->id << endl;
				//nowIn.erase(nowIn.find((*p)->id));
				color = -1;
				z = -DBL_MAX;

				for (auto q = _actedgeInter.begin(); q != p; q++)
				{
					/*
					if (y == 250)
					{
						//fout << "try: " << (*q)->id << std::endl;
						fout << (*q)->z + Dz[(*q)->id].dzx * (i - int((*q)->x) + 1)
							<< ' ' << z + Dz[color].dzx << endl;
					}
					*/
					
					if (in[(*q)->id] && (*q)->id != (*p)->id &&
						(color == -1 ||
						((*q)->z + Dz[(*q)->id].dzx * (i - int((*q)->x) + 1) > z + Dz[color].dzx)))
					{
						z = (*q)->z + Dz[(*q)->id].dzx * (i - int((*q)->x));
						color = (*q)->id;
						//if (y==250) fout << i << ":  Pick " << color << endl;
					}
				}
				//if (y == 250 && color==-1) fout << i << ":  no Cover " << color << endl;
			}
			p++;
		}
		//if (y == 250 && color != -1) fout << i << ' ' << color << endl;
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
