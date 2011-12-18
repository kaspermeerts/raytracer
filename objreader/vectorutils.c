#include <objreader/vectorutils.h>
#include <math.h>

#define EPSILON (1e-8)

void
NormalizeVector(Vector3D *c)
{
  float l, m;

  l=(float)sqrt(c->x*c->x + c->y*c->y + c->z*c->z);
  if (fabs(l) < EPSILON) {
    if ((c->x>=c->y) && (c->x>=c->z)) {
      c->x=1.0f;
      c->y=c->z=0.0f;
    }
    else
    if (c->y>=c->z) {
      c->y=1.0f;
      c->x=c->z=0.0f;
    }
    else {
      c->z=1.0f;
      c->x=c->y=0.0f;
    }
  }
  else {
    m=1.0f/l;
    c->x*=m;
    c->y*=m;
    c->z*=m;
  }
}



/** 
 * Compute the dot product of two vectors
 * @return dot product of u and v
 */
float 
DotProduct(Vector3D *u, Vector3D *v)
{
    return u->x*v->x + u->y*v->y + u->z*v->z;
}

/**
 * Compute the cross product of two vectors u and v
 * @param n - to return the cross product in
 */
void
CrossProduct(Vector3D *u, Vector3D *v, Vector3D *n)
{
    n->x = u->y*v->z - u->z*v->y;
    n->y = u->z*v->x - u->x*v->z;
    n->z = u->x*v->y - u->y*v->x;
}