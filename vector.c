#include <math.h>

#include "vector.h"

Vec3 vec3_add(Vec3 a, Vec3 b)
{
	Vec3 c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;

	return c;
}

Vec3 vec3_sub(Vec3 a, Vec3 b)
{
	Vec3 c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;

	return c;
}

double vec3_dot(Vec3 a, Vec3 b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

double vec3_length(Vec3 a)
{
	return sqrt(vec3_dot(a, a));
}

Vec3 vec3_scale(double scale, Vec3 a)
{
	Vec3 b;
	b.x = a.x * scale;
	b.y = a.y * scale;
	b.z = a.z * scale;

	return b;
}

Vec3 vec3_normalize(Vec3 a)
{
	return vec3_scale(1/vec3_length(a), a);
}

Vec3 vec3_cross(Vec3 a, Vec3 b)
{
	Vec3 c;
	c.x = a.y*b.z - a.z*b.y;
	c.y = a.z*b.x - a.x*b.z;
	c.z = a.x*b.y - a.y*b.x;

	return c;
}

Vec3 vec3_lerp(Vec3 a, Vec3 b, double t)
{
	Vec3 c;
	/* c = a*t + b*(1-t) */
	a = vec3_scale(t,     a);
	b = vec3_scale(1 - t, b);
	c = vec3_add(a, b);

	return c;
}

Vec3 vec3_reflect(Vec3 d, Vec3 n)
{
	/* d - 2(d.n)n */
	return vec3_add(d, vec3_scale(-2*vec3_dot(d,n),n));
}

Vec4 vec4_from_vec3(Vec3 v3, double w)
{
	Vec4 v4;
	v4.x = v3.x;
	v4.y = v3.y;
	v4.z = v3.z;
	v4.w =    w;

	return v4;
}

Vec3 vec4_project(Vec4 v)
{
	Vec3 v3;

	v3.x = v.x / v.w;
	v3.y = v.y / v.w;
	v3.z = v.z / v.w;

	return v3;
}

Vec3 vec3_from_vec4(Vec4 v4)
{
	Vec3 v3;

	v3.x = v4.x;
	v3.y = v4.y;
	v3.z = v4.z;

	return v3;
}
