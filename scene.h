#ifndef CG_SCENE
#define CG_SCENE

#include "cgmath.h"
#include "colour.h"
#include "mesh.h"

enum { MAX_LIGHTS=8 };

typedef struct Camera {
	Vec3 position;
	Vec3 u, v, w; /* For the raytracer */
	Quaternion orientation; /* For the rasteriser */
	float fov;
	char *name;
} Camera;

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
	enum { MATERIAL_DIFFUSE, MATERIAL_PHONG, MATERIAL_COMBINED } type;
	Colour colour;
	int shininess;
	float weight1;
	struct Material *mat1;
	float weight2;
	struct Material *mat2;
	char *name;
} Material;

typedef struct Sphere {
	float radius;
} Sphere;

typedef struct Cylinder {
	float radius;
	float height;
	bool capped;
} Cylinder;

typedef struct Cone {
	float radius;
	float height;
	bool capped;
} Cone;

typedef struct Torus {
	float inner_radius;
	float outer_radius;
} Torus;

typedef struct Shape {
	enum { SHAPE_SPHERE, SHAPE_CYLINDER, SHAPE_CONE, SHAPE_TORUS, SHAPE_MESH }
			type;
	union {
		Sphere sphere;
		Cylinder cylinder;
		Cone cone;
		Torus torus;
		Mesh *mesh;
	} u;
	char *name;
} Shape;

typedef struct Surface {
	Shape *shape;
	Material *material;
	double model_to_world[16];
	double world_to_model[16];
	struct Surface *next;
} Surface;

typedef struct Scene {
	Camera *camera;
	int num_lights;
	Light *light[MAX_LIGHTS];
	Colour background;
	/*struct Node graph;*/ /* One node? */
	Surface *root;
} Scene;

typedef struct Sdl {
	/* Resources */
	int num_cameras;
	Camera *camera;
	int num_lights;
	Light *light;
	int num_shapes;
	Shape *shape;
	int num_textures;
	Texture *texture;
	int num_materials;
	Material *material;

	Scene scene;
} Sdl;

Sdl *sdl_load(const char *filename);

#endif
