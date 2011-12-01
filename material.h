#ifndef CG_MATERIAL_H
#define CG_MATERIAL_H

#include "colour.h"
#include "cgmath.h"

typedef struct Light {
	enum { LIGHT_DIRECTIONAL, LIGHT_POINT, LIGHT_SPOT } type;
	Vec3 position;
	Vec3 direction; /* Not necessarily normalized */
	float angle;
	Colour colour;
	float intensity;
	char *name;
} Light;

typedef struct Texture {
	char *source;
	int width, height;
	Colour *image;
	char *name;
} Texture;

typedef struct Material {
	Colour diffuse_colour;
	Colour specular_colour;
	int shininess;
	char *name;
} Material;

Colour diff_colour(Light *light, Material *mat, Vec3 cam_dir, Vec3 light_dir,
		Vec3 normal);
Colour spec_colour(Light *light, Material *mat, Vec3 light_dir, Vec3 cam_dir,
		Vec3 normal);
Colour light_mat_colour(Light *light, Material *mat, Vec3 cam_dir,
		Vec3 light_dir, Vec3 normal);

#endif
