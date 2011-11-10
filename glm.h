#ifndef CG_GLM_H
#define CG_GLM_H

#include "cgmath.h"

typedef struct Vertex3 {
	float x, y, z;
} Vertex3;

typedef struct Vertex3N {
	float x, y, z;
	float nx, ny, nz;
} Vertex3N;

typedef struct Vertex3NT {
	float x, y, z;
	float nx, ny, nz;
	float u, v;
} Vertex3NT;

typedef struct Vertex3T {
	float x, y, z;
	float u, v;
} Vertex3T;

typedef struct Vertex2 {
	float x, y;
} Vertex2;

typedef struct Vertex2C {
	float x, y;
	float r, g, b, a;
} Vertex2C;

/* Colour and texture? The texture here is a luminance texture, with no colour
 * information. This is e.g. used for text */
typedef struct Vertex2CT {
	float x, y;
	float u, v;
	float r, g, b, a;
} Vertex2CT;

typedef struct Vertex2T {
	float x, y;
	float u, v;
} Vertex2T;

typedef struct Matrix Matrix;

extern Matrix *glmProjectionMatrix;
extern Matrix *glmViewMatrix;
extern Matrix *glmModelMatrix;

void glmPrintMatrix(Matrix *mat);
void glmLoadIdentity(Matrix *mat);
Matrix *glmNewMatrixStack(void);
void glmPopMatrix(Matrix **mat);
void glmPushMatrix(Matrix **mat);
void glmFreeMatrixStack(Matrix *mat);
void glmLoadMatrix(Matrix *mat, double m[16]);
void glmMultMatrix(Matrix *mat, double m[16]);
void glmMultQuaternion(Matrix *mat, Quaternion q);
Vec3 glmTransformVector(Matrix *mat, Vec3);
void glmScale(Matrix *mat, double x, double y, double z);
void glmScaleUniform(Matrix *mat, double r);
void glmTranslate(Matrix *mat, double tx, double ty, double tz);
void glmTranslateVector(Matrix *mat, Vec3 v);
void glmRotate(Matrix *mat, double angle, double ax,
		double ay, double az);
void glmOrtho(Matrix *mat, double left, double right,
		double bottom, double top, double near, double far);
void glmFrustum(Matrix *mat, double l, double r,
		double b, double t, double near, double far);
void glmPerspective(Matrix *mat, double fov, double aspect, double near,
		double far);
#endif
