#pragma once
#include"vec3.h"

class ray
{
private:
	vec3 _origin;
	//vec3 _direction;
	vec3 _destination;
public:
	ray() :_origin(vec3(0.f, 0.f, 0.f)), _destination(vec3(1.f, 1.f, 1.f)) {};
	ray(const vec3& origin, const vec3& destination) :_origin(origin), _destination(destination) {};
	vec3 getOrigin() const { return _origin; }
	vec3 getDirection() const { return _destination - _origin; }
	vec3 getDestination() const { return _destination; }
	vec3 pointAtParameter(float t) const { return _origin + t * getDirection(); };
};

