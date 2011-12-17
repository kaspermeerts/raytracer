#ifndef CG_BBOX
#define CG_BBOX

#include "matrix.h"

enum AXIS {X_AXIS, Y_AXIS, Z_AXIS};

typedef struct BBox {
	float xmin, ymin, zmin;
	float xmax, ymax, zmax;
} BBox;

BBox bbox_transform(Mat4 m, BBox box);
void bbox_split(BBox a, enum AXIS axis, float location, BBox *b, BBox *c);
double bbox_surface_area(BBox a);
#endif
