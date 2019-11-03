#pragma once
#include"common.h"
#include"vec3.h"
#include"vec2.h"
#include"Face.h"
class Model
{
public:
	std::vector<vec3> _pos;
	std::vector<vec3> _nml;
	std::vector<vec2> _tex;
	std::vector<Face> _face;

	Model()
	{
		_pos.clear();
		_nml.clear();
		_tex.clear();
		_face.clear();
	}
};

