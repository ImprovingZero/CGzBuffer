#include "camera.h"

#define _USE_MATH_DEFINES
#include<math.h>

camera::camera(vec3 lookFrom, vec3 lookAt, vec3 up,
	float alpha, float ratio, float focusDist)
	:_origin(lookFrom)
{
	float theta = alpha * float(M_PI) / 180.f;
	float halfHeight = tan(theta / 2);
	float halfWidth = ratio * halfHeight;

	_w = Unit(lookFrom - lookAt);
	_u = Unit(Cross(up, _w));
	_v = Cross(_w, _u);

	_lowerLeftConer = lookFrom - _w * focusDist
		- halfWidth * focusDist * _u - halfHeight * focusDist * _v;
	_horizontal = 2 * halfWidth * focusDist * _u;
	_vertical = 2 * halfHeight * focusDist * _v;
}