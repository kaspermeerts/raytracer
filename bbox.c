#include <assert.h>
#include <math.h>

#include "bbox.h"

BBox bbox_transform(Mat4 m, BBox box)
{
	BBox new_box;
	Vec3 v[8];

	v[0] = (Vec3) {box.xmin, box.ymin, box.zmin};
	v[1] = (Vec3) {box.xmax, box.ymin, box.zmin};
	v[2] = (Vec3) {box.xmin, box.ymax, box.zmin};
	v[3] = (Vec3) {box.xmax, box.ymax, box.zmin};
	v[4] = (Vec3) {box.xmin, box.ymin, box.zmax};
	v[5] = (Vec3) {box.xmax, box.ymin, box.zmax};
	v[6] = (Vec3) {box.xmin, box.ymax, box.zmax};
	v[7] = (Vec3) {box.xmax, box.ymax, box.zmax};

	new_box.xmin = new_box.ymin = new_box.zmin =  HUGE_VAL;
	new_box.xmax = new_box.ymax = new_box.zmax = -HUGE_VAL;

	for (int i = 0; i < 8; i++)
	{
		Vec3 tv;
		tv = mat4_transform3_homo(m, v[i]);
		if (tv.x < new_box.xmin)
			new_box.xmin = tv.x;
		if (tv.x > new_box.xmax)
			new_box.xmax = tv.x;
		if (tv.y < new_box.ymin)
			new_box.ymin = tv.y;
		if (tv.y > new_box.ymax)
			new_box.ymax = tv.y;
		if (tv.z < new_box.zmin)
			new_box.zmin = tv.z;
		if (tv.z > new_box.zmax)
			new_box.zmax = tv.z;
	}

	return new_box;
}

void bbox_split(BBox a, enum AXIS axis, float location, BBox *b, BBox *c)
{
	BBox left, right;

	left = right = a;
	switch(axis)
	{
	case X_AXIS:
		left.xmax = right.xmin = location;
		break;
	case Y_AXIS:
		left.ymax = right.ymin = location;
		break;
	case Z_AXIS:
		left.zmax = right.zmin = location;
		break;
	default:
		assert(axis == X_AXIS || axis == Y_AXIS || axis == Z_AXIS);
		break;
	}

	*b = left;
	*c = right;
}

double bbox_surface_area(BBox a)
{
	return 2 * fabs(a.zmax - a.zmin) * fabs(a.ymax - a.ymin) * fabs(a.xmax - a.xmin);
}

