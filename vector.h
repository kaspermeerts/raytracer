#ifndef CG_VECTOR_H
#define CG_VECTOR_H

typedef struct Vec3 {
	double x;
	double y;
	double z;
} Vec3;

typedef struct Vec4 {
	double x;
	double y;
	double z;
	double w;
} Vec4;

void vec3_print(Vec3 v);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_scale(double r, Vec3 a);
double vec3_dot(Vec3 a, Vec3 b);
double vec3_length2(Vec3 a);
double vec3_length(Vec3 a);
Vec3 vec3_normalize(Vec3 a);
Vec3 vec3_cross(Vec3 a, Vec3 b);
Vec3 vec3_lerp(Vec3 a, Vec3 b, double t);
Vec3 vec3_reflect(Vec3 d, Vec3 n);
Vec4 vec4_from_vec3(Vec3, double w);
Vec3 vec4_homogeneous_divide(Vec4 v);
Vec3 vec3_from_vec4(Vec4 v);

#endif
