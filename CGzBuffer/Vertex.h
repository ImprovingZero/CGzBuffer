#pragma once
#include"common.h"
class Vertex
{
public:
	int _pos;
	int _nml;
	int _tex;
	Vertex() :_pos(-1), _nml(-1), _tex(-1) {};
	Vertex(int pos, int nml, int tex) :_pos(pos), _nml(nml), _tex(tex) {};
};

