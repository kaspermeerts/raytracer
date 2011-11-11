#ifndef CG_SCENE
#define CG_SCENE

#include "cgmath.h"
#include "colour.h"

enum { MAX_LIGHTS=8 };

typedef struct Sphere {
	Vec3 center;
	float radius;
} Sphere;

typedef struct Camera {
	Vec3 position;
	Vec3 direction; /* Keep this normalized */
	Vec3 up;
	float fov;
	char *name;
} Camera;

typedef struct Light {
	enum { LIGHT_DIRECTIONAL, LIGHT_POINT, LIGHT_SPOT } type;
	Vec3 position;
	Vec3 direction; /* Keep this normalized */
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
	enum { DIFFUSE, PHONG, COMBINED } type;
	Colour colour;
	int shininess;
} Material;

typedef struct Shape {
	Sphere sphere;
} Shape;

typedef struct Scene {
	Camera cam;
	int num_lights;
	Light light[MAX_LIGHTS]; /* XXX */
	Colour background;
	struct Node *graph;
} Scene;

typedef struct Node {
	enum { SHAPE, TRANSFORM } type;
} Node;

typedef struct Sdl {
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

	Scene *scene;
} Sdl;

Sdl *sdl_load(const char *filename);

#endif
