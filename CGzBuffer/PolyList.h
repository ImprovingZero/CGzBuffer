#pragma once
#include"common.h"
#include"Model.h"
#include"camera.h"

struct PolyListNode
{
	vec3 nml;
	int id;
	int dy;
	double z;
	vec3 color;
	PolyListNode(vec3 n, int i, int y, double Z)
		:nml(n), id(i), dy(y), z(Z){}
};

struct EdgeListNode
{
	double x;
	double dx; // equals to (-1/k)
	double z;
	int dy;
	int id;
	EdgeListNode(double X, double DX, int DY, int i,double Z)
		:x(X), dy(DY), id(i), z(Z)
	{
		dx = -double(DX) / double(DY);
	}
};

struct ActiveEdgeNode
{
	double xl, dxl;
	int dyl;
	double xr, dxr;
	int dyr;
	double zl, dzx, dzy;
	int id;
	ActiveEdgeNode(EdgeListNode* e1, EdgeListNode* e2, 
		vec3& nml, vec3& u, vec3& v, vec3& w, double z, double sc)
	{
		if (e1->x == e2->x)
		{
			if (e1->dx > e2->dx) std::swap(e1, e2);
		}
		else
		{
			if (e1->x > e2->x) std::swap(e1, e2);
		}
		
		xl = e1->x; xr = e2->x;
		dxl = e1->dx; dxr = e2->dx;
		dyl = e1->dy; dyr = e2->dy;
		id = e1->id;
		double a = nml.dot(u);
		double b = nml.dot(v);
		double c = nml.dot(-w);
		dzx = -a / c * sc;
		dzy = b / c * sc;
		zl = z;
		//std::cout << xl << ' ' << xr << ' ' << dxl << ' ' << dxr << ' ' << dyl << ' ' << dyr
		//	<< id << ' ' << dzx << ' ' << dzy << ' ' << zl << std::endl;
	}
};

class ActiveList;

class PolyList
{
private:
	vec2if projectVertex(vec3 pos);
		//calc the coordinate when the vertex is projected on screen
	vec2if projectVertex(Vertex* v) 
	{ 
		return projectVertex(_model->_pos[v->_pos]); 
	}
	vec2if projectVertex(Vertex& v)
	{
		return projectVertex(_model->_pos[v._pos]);
	}
	vec2if projectTEMP(Vertex& v);
	void calcRangeTEMP(Model* model);
	double _ltemp, _dtemp, _rtemp, _utemp;
	double _scaleZ;

	void initScan();
	void initNaive();
	void calcCut(vec2if p1, vec2if p2, std::vector<vec2if>& v);
		//p1 p2 may not in screen, 
		//calc the ends that can represent this line on screen
		//THIS FUNCTION HAS NOT BEEN FINISHED !!!!

public:
	std::vector<std::vector<PolyListNode*>> _poly;
	std::vector<std::vector<EdgeListNode*>> _edge;
	std::vector<int> _polyY;
	Model* _model;
	ActiveList* _actList;
	camera* _cam;

	PolyList(Model* mdl, camera* cam) 
		:_model(mdl),_cam(cam) { }

	void refreshList() { initScan(); }
	void refreshNaive() { initNaive(); }
	void activeP(int i);
	void activePinter(int i);
	//void activeE(int i);
	void rastrizeTri(std::vector<std::vector<int>>& output,
		std::vector<std::vector<double>>& depth);
	void rastrizeOneTri(std::vector<std::vector<int>>& output,
		std::vector<std::vector<double>>& depth, int y,
		PolyListNode* poly,
		EdgeListNode* e1, EdgeListNode* e2, EdgeListNode* e3);
};

class ZBuffer;

class ActiveList
{
public:
	std::list<PolyListNode*> _actpoly;
	std::list<ActiveEdgeNode*> _actedge;
	std::list<EdgeListNode*> _actedgeInter;
	PolyList* _polyList;

	ActiveList(PolyList* plyList) :_polyList(plyList) 
	{ 
		_actpoly.clear(); 
		_actedge.clear(); 
		_actedgeInter.clear();
	};

	void delActiveP(int y);
	void updataActiveE(int y);
	void draw(int y, std::vector<double>& depth, std::vector<int>& buffer);
	void decDy();

	void updateActiveEInter(int y);
	void drawInter(int y, std::vector<double>& depth,
		std::vector<int>& buffer);
	void decDyInter();
};
