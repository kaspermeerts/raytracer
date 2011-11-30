#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cgmath.h"
#include "matrix.h"

static void mat4_from_mat3(Mat4 a, Mat3 b)
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

MatrixStack *matstack_new(void)
{
	MatrixStack *stack = malloc(sizeof(MatrixStack));
	stack->top = NULL;
	return stack;
}

void matstack_destroy(MatrixStack *stack)
{
	while (stack->top)
		matstack_pop(stack);
}

void matstack_pop(MatrixStack *stack)
{
	Matrix *m;

	m = stack->top;
	stack->top = stack->top->next;
	free(m);
}

void matstack_push(MatrixStack *stack)
{
	Matrix *new;

	new = malloc(sizeof(Matrix));
	new->next = stack->top;
	stack->top = new;
	if (new->next == NULL)
	{
		mat4_identity(new->matrix);
		mat4_identity(new->inverse);
	} else
	{
		mat4_copy(new->matrix, new->next->matrix);
		mat4_copy(new->inverse, new->next->inverse);
	}
}

Vec4 mat4_transform(Mat4 m, Vec4 v)
{
	const double v0 = v.x, v1 = v.y, v2 = v.z, v3 = v.w;
	Vec4 out;

#define M(i,j) m[4*j+i]
	out.x = M(0,0)*v0 + M(0,1)*v1 + M(0,2)*v2 + M(0,3)*v3;
	out.y = M(1,0)*v0 + M(1,1)*v1 + M(1,2)*v2 + M(1,3)*v3;
	out.z = M(2,0)*v0 + M(2,1)*v1 + M(2,2)*v2 + M(2,3)*v3;
	out.w = M(3,0)*v0 + M(3,1)*v1 + M(3,2)*v2 + M(3,3)*v3;
#undef M

	return out;
}

Vec3 mat4_transform3(Mat4 m, Vec3 v)
{
	const double v0 = v.x, v1 = v.y, v2 = v.z;
	Vec3 out;

#define M(i,j) m[4*j+i]
	out.x = M(0,0)*v0 + M(0,1)*v1 + M(0,2)*v2 + M(0,3)*0;
	out.y = M(1,0)*v0 + M(1,1)*v1 + M(1,2)*v2 + M(1,3)*0;
	out.z = M(2,0)*v0 + M(2,1)*v1 + M(2,2)*v2 + M(2,3)*0;
	//out.w = M(3,0)*v0 + M(3,1)*v1 + M(3,2)*v2 + M(3,3)*0;
#undef M

	return out;
}

void mat4_print(Mat4 m)
{
	printf("%g\t%g\t%g\t%g\n", m[ 0], m[ 4], m[ 8], m[12]);
	printf("%g\t%g\t%g\t%g\n", m[ 1], m[ 5], m[ 9], m[13]);
	printf("%g\t%g\t%g\t%g\n", m[ 2], m[ 6], m[10], m[14]);
	printf("%g\t%g\t%g\t%g\n", m[ 3], m[ 7], m[11], m[15]);
}

void mat4_copy(Mat4 dst, Mat4 src)
{
	memcpy(dst, src, sizeof(Mat4));
}

void mat4_mult(Mat4 c, Mat4 a, Mat4 b)
{
	int i, j;

#define A(i,j) a[4*j + i]
#define B(i,j) b[4*j + i]
#define C(i,j) c[4*j + i]
	for (i = 0; i < 4; i++)
	{
		double A0 = A(i, 0), A1 = A(i, 1), A2 = A(i, 2), A3 = A(i, 3);
		for (j = 0; j < 4; j++)
			C(i,j) = A0*B(0,j) + A1*B(1,j) + A2*B(2,j) + A3*B(3,j);
	}
#undef C
#undef B
#undef A
}

/* b * a --> a */
void mat4_lmul(Mat4 b, Mat4 a)
{
	int i, j;
#define A(i,j) a[4*j + i]
#define B(i,j) b[4*j + i]
	for (j = 0; j < 4; j++)
	{
		double A0 = A(0, j), A1 = A(1, j), A2 = A(2, j), A3 = A(3, j);
		for (i = 0; i < 4; i++)
			A(i,j) = B(i,0)*A0 + B(i,1)*A1 + B(i,2)*A2 + B(i,3)*A3;
	}
#undef B
#undef A
}

/* a * b --> a */
void mat4_rmul(Mat4 a, Mat4 b)
{
	int i, j;

#define A(i,j) a[4*j + i]
#define B(i,j) b[4*j + i]
	for (i = 0; i < 4; i++)
	{
		double A0 = A(i, 0), A1 = A(i, 1), A2 = A(i, 2), A3 = A(i, 3);
		for (j = 0; j < 4; j++)
			A(i,j) = A0*B(0,j) + A1*B(1,j) + A2*B(2,j) + A3*B(3,j);
	}
#undef B
#undef A
}

Vec3 mat3_transform(Mat3 a, Vec3 v)
{
	Vec3 w;

#define A(i, j) a[3*j + i]
	w.x = A(0,0)*v.x + A(0,1)*v.y + A(0,2)*v.z;
	w.y = A(1,0)*v.x + A(1,1)*v.y + A(1,2)*v.z;
	w.z = A(2,0)*v.x + A(2,1)*v.y + A(2,2)*v.z;
#undef A

	return w;
}

void mat4_identity(Mat4 m)
{
#define M(i, j) m[4*j + i]
	M(0,0) = 1.0; M(0, 1) = 0.0; M(0,2) = 0.0; M(0,3) = 0.0;
	M(1,0) = 0.0; M(1, 1) = 1.0; M(1,2) = 0.0; M(1,3) = 0.0;
	M(2,0) = 0.0; M(2, 1) = 0.0; M(2,2) = 1.0; M(2,3) = 0.0;
	M(3,0) = 0.0; M(3, 1) = 0.0; M(3,2) = 0.0; M(3,3) = 1.0;
#undef M
}

void mat4_scale(Mat4 m, double x, double y, double z)
{
#define M(i, j) m[4*j + i]
	M(0,0) = x; M(0,1) = 0; M(0,2) = 0; M(0,3) = 0;
	M(1,0) = 0; M(1,1) = y; M(1,2) = 0; M(1,3) = 0;
	M(2,0) = 0; M(2,1) = 0; M(2,2) = z; M(2,3) = 0;
	M(3,0) = 0; M(3,1) = 0; M(3,2) = 0; M(3,3) = 1;
#undef M
}

void mat4_translate(Mat4 m, double x, double y, double z)
{
#define M(i, j) m[4*j + i]
	M(0,0) = 1; M(0,1) = 0; M(0,2) = 0; M(0,3) = x;
	M(1,0) = 0; M(1,1) = 1; M(1,2) = 0; M(1,3) = y;
	M(2,0) = 0; M(2,1) = 0; M(2,2) = 1; M(2,3) = z;
	M(3,0) = 0; M(3,1) = 0; M(3,2) = 0; M(3,3) = 1;
#undef M
}

void mat4_translate_vector(Mat4 m, Vec3 v)
{
	mat4_translate(m, v.x, v.y, v.z);
}

void mat4_rotate(Mat4 m, double angle, double x, double y, double z)
{
	Quaternion q;

	q = quat_from_angle_axis(angle, x, y, z);
	mat4_quaternion(m, q);
}

void mat4_quaternion(Mat4 m, Quaternion q)
{
	Mat3 m3;
	mat3_from_quat(m3, q);
	mat4_from_mat3(m, m3);
}

/* Perspective */
void mat4_ortho(Mat4 m, double left, double right, double bottom, double top,
		double near, double far)
{
#define M(i,j) m[4*j + i]
	M(0,0) = 2.0 / (right - left);
	M(0,1) = 0.0;
	M(0,2) = 0.0;
	M(0,3) = -(right + left) / (right - left);

	M(1,0) = 0.0;
	M(1,1) = 2.0 / (top - bottom);
	M(1,2) = 0.0;
	M(1,3) = -(top + bottom) / (top - bottom);

	M(2,0) = 0.0;
	M(2,1) = 0.0;
	M(2,2) = 2.0 / (far - near);
	M(2,3) = -(far  +  near) / (far  -  near);

	M(3,0) = 0.0;
	M(3,1) = 0.0;
	M(3,2) = 0.0;
	M(3,3) = 1.0;
#undef M
}

void mat4_frustum(Mat4 m, double l, double r, double b, double t,
		double near, double far)
{
	double x, y, A, B, C, D;

	x = 2*near/(r - l);
	y = 2*near/(t - b);
	A = (l + r)/(l - r);
	B = (b + t)/(b - t);
	C = (far + near)/(near - far);
	D = 2 * far * near / (far - near);

#define M(i, j) m[4*j + i]
   M(0,0) = x;  M(0,1) = 0;  M(0,2) =  A;  M(0,3) = 0;
   M(1,0) = 0;  M(1,1) = y;  M(1,2) =  B;  M(1,3) = 0;
   M(2,0) = 0;  M(2,1) = 0;  M(2,2) =  C;  M(2,3) = D;
   M(3,0) = 0;  M(3,1) = 0;  M(3,2) =  1;  M(3,3) = 0;
#undef  M
}

void mat4_perspective(Mat4 m, double fovy, double aspect, double near,
		double far)
{
	double right, top;

	top = abs(near) * tan(fovy/2.0);
	right = top * aspect;
	mat4_frustum(m, -right, right, -top, top, near, far);
}
