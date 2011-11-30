#ifndef CG_MATRIX_H
#define CG_MATRIX_H

typedef double Mat3[9];
typedef double Mat4[16];

#include "vector.h"
#include "quaternion.h"

typedef struct MatrixStack {
	struct Matrix *top;
} MatrixStack;

typedef struct Matrix {
	Mat4 matrix;
	Mat4 inverse;
	struct Matrix *next;
} Matrix;

Vec3 mat3_transform(Mat3 m, Vec3 v);
MatrixStack *matstack_new(void);
void matstack_destroy(MatrixStack *matstack);
void matstack_pop(MatrixStack *stack);
void matstack_push(MatrixStack *stack);
Vec4 mat4_transform(Mat4 m, Vec4 v);
Vec3 mat4_transform3(Mat4 m, Vec3 v);
void mat4_print(Mat4 m);
void mat4_copy(Mat4 dst, Mat4 src);
void mat4_transpose(Mat4 m);
void mat4_mult(Mat4 c, Mat4 a, Mat4 b);
void mat4_lmul(Mat4 b, Mat4 a);
void mat4_rmul(Mat4 a, Mat4 b);
void mat4_identity(Mat4 m);
void mat4_scale(Mat4 m, double x, double y, double z);
void mat4_translate(Mat4 m, double x, double y, double z);
void mat4_translate_vector(Mat4 m, Vec3 v);
void mat4_rotate(Mat4 m, double angle, double x, double y, double z);
void mat4_quaternion(Mat4 m, Quaternion q);
void mat4_ortho(Mat4 m,double l,double r,double b,double t,double n,double f);
void mat4_frustum(Mat4 m,double l,double r,double b,double t,double n,double f);
void mat4_perspective(Mat4 m, double fov, double aspect, double n, double f);
#endif
