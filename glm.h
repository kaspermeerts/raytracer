#ifndef CG_GLM_H
#define CG_GLM_H

#include "cgmath.h"

typedef struct Matrix {
	double m[16];
	struct Matrix *next;
} Matrix;

void glmPrintMatrix(Matrix *mat);
void glmLoadIdentity(Matrix *mat);
Matrix *glmNewMatrixStack(void);
void glmPopMatrix(Matrix **mat);
void glmPushMatrix(Matrix **mat);
void glmFreeMatrixStack(Matrix *mat);
void glmLoadMatrix(Matrix *mat, double m[16]);
void glmSaveMatrix(Matrix *mat, double m[16]);
void glmMultMatrix(Matrix *mat, double m[16]);
void glmMultQuaternion(Matrix *mat, Quaternion q);
Vec4 glmTransformVector(Matrix *mat, Vec4);
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
