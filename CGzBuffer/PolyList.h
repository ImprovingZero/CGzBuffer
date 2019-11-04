#pragma once
#include"common.h"
#include"Model.h"
#include"camera.h"

struct PolyListNode
{
	vec3 nml;
	int id;
	int dy;
	float z;
	vec3 color;
	PolyListNode(vec3 n, int i, int y, float Z)
		:nml(n), id(i), dy(y), z(Z) {}
};

struct EdgeListNode
{
	float x;
	float dx; // equals to (-1/k)
	int dy;
	int id;
	EdgeListNode(int X, float DX, int DY, int i)
		:x(float(X)), dy(DY), id(i)
	{
		dx = -float(DX) / float(DY);
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
	float _ltemp, _dtemp, _rtemp, _utemp;
	float _scaleZ;

	void calcCut(vec2if p1, vec2if p2, std::vector<vec2if>& v);
		//p1 p2 may not in screen, 
		//calc the ends that can represent this line on screen
		//THIS FUNCTION HAS NOT BEEN FINISHED !!!!

public:
	std::vector<std::vector<PolyListNode*>> _poly;
	std::vector<std::vector<EdgeListNode*>> _edge;
	Model* _model;
	ActiveList* _actList;
	camera* _cam;

	void init();
	PolyList(Model* mdl, camera* cam) 
		:_model(mdl),_cam(cam) { }
	void refreshList() { init(); }
	void activeP(int i);
	//void activeE(int i);
};

class ZBuffer;

class ActiveList
{
public:
	std::list<PolyListNode*> _actpoly;
	std::list<ActiveEdgeNode*> _actedge;
	PolyList* _polyList;

	ActiveList(PolyList* plyList) :_polyList(plyList) 
	{ 
		_actpoly.clear(); 
		_actedge.clear(); 
	};

	void delActiveP(int y);
	void updataActiveE(int y);
	void draw(int y, std::vector<float>& depth, std::vector<int>& buffer);
	void decDy();
};







