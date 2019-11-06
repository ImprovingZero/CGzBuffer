#pragma once
#include"common.h"
#include"Vertex.h"

class Face
{
public:
	std::vector<Vertex> _vtx;
	vec3 _nml;
	vec3 _color;

	Vertex& operator[](int i) { return _vtx[i]; }

	Face(std::vector<Vertex> v) :_vtx(v)
	{
		//_color = (_nml.dot(vec3(0.f, 0.f, 1.f))*0.5+0.5)*vec3(1.f,1.f,1.f);
		//_color = vec3(0.f, 0.f, 0.f);
		//_color = vec3(0.5+0.25*cos(v[0]._pos+2), 0.5+0.5*sin(v[1]._pos+2), 1+0.5*sin(v[2]._pos+3));
	}
	void calcNormal(std::vector<vec3>& p)
	{
		if (p.size() - 1 < _vtx[0]._pos || p.size() - 1 < _vtx[1]._pos || p.size() - 1 < _vtx[2]._pos)
		{
			std::cout << "num of vertices: " << p.size() << "   " << _vtx[0]._pos << ' ' << _vtx[1]._pos << ' ' << _vtx[2]._pos << std::endl;
			std::cout << "ERROR::Face::calcNormal::Invalid position index" << std::endl;
			return;
		}
		vec3 a = p[_vtx[1]._pos] - p[_vtx[0]._pos];
		vec3 b = p[_vtx[2]._pos] - p[_vtx[1]._pos];
		_nml = Unit(Cross(a, b));
		double d = Dot(Unit(_nml), Unit(vec3(0.f, 0.f, 1.f)));
		if (d > -1e-6) _color = d * vec3(1.f, 1.f, 1.f);
		else _color = -d * (vec3(0.f, 1.f, 1.f));
	
	}
};

