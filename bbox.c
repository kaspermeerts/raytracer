#include <assert.h>
#include <math.h>

#include "bbox.h"

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

