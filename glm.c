#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "glm.h"

static void matrix_mul_matrix(double *c, double *a, double *b)
{
#define A(i, j) a[4*j + i]
#define B(i, j) b[4*j + i]
#define C(i, j) c[4*j + i]
	int i, j;

	for (i = 0; i < 4; i++)
	{
		const double Ai0 = A(i,0), Ai1 = A(i,1), Ai2 = A(i,2), Ai3 = A(i,3);
		for (j = 0; j < 4; j++)
			C(i,j) = Ai0*B(0,j) + Ai1*B(1,j) + Ai2*B(2,j) + Ai3*B(3,j);
	}
#undef C
#undef B
#undef A
}

static void mat4_from_mat3(double a[16], Mat3 b)
{
#define A(i, j) a[4*j + i]
#define B(i, j) b[3*j + i]
	A(0,0) = B(0,0); A(0,1) = B(0,1); A(0,2) = B(0,2); A(0,3) = 0.0;
	A(1,0) = B(1,0); A(1,1) = B(1,1); A(1,2) = B(1,2); A(1,3) = 0.0;
	A(2,0) = B(2,0); A(2,1) = B(2,1); A(2,2) = B(2,2); A(2,3) = 0.0;
	A(3,0) = 0.0;    A(3,1) = 0.0;    A(3,2) = 0.0;    A(3,3) = 1.0;
#undef B
#undef A
}

void glmPrintMatrix(Matrix *mat)
{
	const double *m = mat->m;
	printf("%g\t%g\t%g\t%g\n", m[ 0], m[ 4], m[ 8], m[12]);
	printf("%g\t%g\t%g\t%g\n", m[ 1], m[ 5], m[ 9], m[13]);
	printf("%g\t%g\t%g\t%g\n", m[ 2], m[ 6], m[10], m[14]);
	printf("%g\t%g\t%g\t%g\n", m[ 3], m[ 7], m[11], m[15]);
}

void glmLoadIdentity(Matrix *mat)
{
	memset(mat->m, 0, 16 * sizeof(double));
	mat->m[0] = mat->m[5] = mat->m[10] = mat->m[15] = 1.0;
}

Matrix *glmNewMatrixStack(void)
{
	Matrix *new = NULL;
	glmPushMatrix(&new);
	glmLoadIdentity(new);
	return new;
}

void glmPopMatrix(Matrix **mat)
{
	Matrix *next;

	next = (*mat)->next;
	free(*mat);
	*mat = next;
}

void glmPushMatrix(Matrix **mat)
{
	Matrix *new;

	new = malloc(sizeof(Matrix));
	if (new == NULL)
		return;

	if (*mat != NULL)
		memcpy(new->m, (*mat)->m, sizeof(double) * 16);

	new->next = *mat;
	*mat = new;
}

void glmFreeMatrixStack(Matrix *mat)
{
	while(mat != NULL)
		glmPopMatrix(&mat);
}

void glmLoadMatrix(Matrix *mat, double m[16])
{
	memcpy(mat->m, m, sizeof(double) * 16);
}

void glmSaveMatrix(Matrix *mat, double m[16])
{
	memcpy(m, mat->m, sizeof(double) * 16);
}

void glmMultMatrix(Matrix *mat, double m[16])
{
	matrix_mul_matrix(mat->m, mat->m, m);
}

void glmMultQuaternion(Matrix *mat, Quaternion q)
{
	Mat3 m3;
	double m4[16];

	mat3_from_quat(m3, q);
	mat4_from_mat3(m4, m3);

	glmMultMatrix(mat, m4);
}

/* Homogeneous transform */
Vec3 glmTransformVector(Matrix *mat, Vec3 v)
{
	const double v0 = v.x, v1 = v.y, v2 = v.z, v3 = 1;
	double w;
	Vec3 out;

#define M(i,j) mat->m[4*j+i]
	out.x = M(0,0)*v0 + M(0,1)*v1 + M(0,2)*v2 + M(0,3)*v3;
	out.y = M(1,0)*v0 + M(1,1)*v1 + M(1,2)*v2 + M(1,3)*v3;
	out.z = M(2,0)*v0 + M(2,1)*v1 + M(2,2)*v2 + M(2,3)*v3;
	    w = M(3,0)*v0 + M(3,1)*v1 + M(3,2)*v2 + M(3,3)*v3;
#undef M

	out.x /= w;
	out.y /= w;
	out.z /= w;

	return out;
}

/* Multiply m on the right with a scaling matrix */
void glmScale(Matrix *mat, double x, double y, double z)
{
	double *m = mat->m;

	m[ 0] *= x; m[ 4] *= y; m[ 8] *= z;
	m[ 1] *= x; m[ 5] *= y; m[ 9] *= z;
	m[ 2] *= x; m[ 6] *= y; m[10] *= z;
	m[ 3] *= x; m[ 7] *= y; m[11] *= z;
}

void glmScaleUniform(Matrix *mat, double r)
{
	glmScale(mat, r, r, r);
}

void glmTranslate(Matrix *mat, double tx, double ty, double tz)
{
	double *m = mat->m;
#define M(i, j) m[4*j + i]
	M(0, 3) = M(0, 0)*tx + M(0, 1)*ty + M(0, 2)*tz + M(0, 3);
	M(1, 3) = M(1, 0)*tx + M(1, 1)*ty + M(1, 2)*tz + M(1, 3);
	M(2, 3) = M(2, 0)*tx + M(2, 1)*ty + M(2, 2)*tz + M(2, 3);
	M(3, 3) = M(3, 0)*tx + M(3, 1)*ty + M(3, 2)*tz + M(3, 3);
#undef M
}

void glmTranslateVector(Matrix *mat, Vec3 v)
{
	glmTranslate(mat, v.x, v.y, v.z);
}

void glmRotate(Matrix *mat, double angle, double ax,
		double ay, double az)
{
	Quaternion q;

	q = quat_from_angle_axis(angle, ax, ay, az);
	glmMultQuaternion(mat, q);
}

/****************************
 * Perspective manipulation *
 ****************************/
void glmOrtho(Matrix *mat, double left, double right,
		double bottom, double top, double near, double far)
{
	double m[16];

	memset(m, 0, sizeof(m));

#define M(i, j) m[4*j + i]
	M(0, 0) = 2.0 / (right - left);
	M(0, 3) = -(right + left) / (right - left);

	M(1, 1) = 2.0 / (top - bottom);
	M(1, 3) = -(top + bottom) / (top - bottom);

	M(2, 2) = 2.0 / (far - near);
	M(2, 3) = -(far  +  near) / (far  -  near);

	M(3, 3) = 1.0;
#undef M

	glmMultMatrix(mat, m);
}

void glmFrustum(Matrix *mat, double l, double r,
		double b, double t, double near, double far)
{
	double x, y, A, B, C, D, m[16];

	x = 2*near/(r - l);
	y = 2*near/(t - b);
	A = (r + l)/(r - l);
	B = (t + b)/(t - b);
	C = (far + near)/(far - near);
	D = 2 * far * near / (far - near);

#define M(i, j) m[4*j + i]
   M(0,0) = x;  M(0,1) = 0;  M(0,2) =  A;  M(0,3) = 0;
   M(1,0) = 0;  M(1,1) = y;  M(1,2) =  B;  M(1,3) = 0;
   M(2,0) = 0;  M(2,1) = 0;  M(2,2) =  C;  M(2,3) = D;
   M(3,0) = 0;  M(3,1) = 0;  M(3,2) = -1;  M(3,3) = 0;
#undef  M

	glmMultMatrix(mat, m);
}

void glmPerspective(Matrix *mat, double fov, double aspect, double near,
		double far)
{
	double m[16];
	double cotan = 1/tan(fov);
	double depth = far - near;

#define M(i, j) m[4*j + i]
	M(0, 0) = cotan;
	M(0, 1) = 0;
	M(0, 2) = 0;
	M(0, 3) = 0;

	M(1, 0) = 0;
	M(1, 1) = aspect*cotan;
	M(1, 2) = 0;
	M(1, 3) = 0;

	M(2, 0) = 0;
	M(2, 1) = 0;
	M(2, 2) = -(far + near)/depth;
	M(2, 3) = -2*far*near/depth;

	M(3, 0) = 0;
	M(3, 1) = 0;
	M(3, 2) = -1;
	M(3, 3) = 0;
#undef M

	glmMultMatrix(mat, m);
}
