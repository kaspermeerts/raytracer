#ifndef CG_MESH_H
#define CG_MESH_H

#include <stdbool.h>
#include "cgmath.h"

typedef struct Triangle {
	struct {
		int vertex_index;
		int normal_index;
		int texcoord_index;
	} vertex[3];
} Triangle;

typedef struct TexCoord {
	float u, v;
} TexCoord;

typedef struct Mesh {
	char *name;

	bool has_normals, has_texcoords;
	int num_vertices;
	Vec3 *vertex;
	int num_normals;
	Vec3 *normal;
	int num_texcoords;
	TexCoord *texcoord;

	int num_triangles;
	Triangle *triangle;
} Mesh;

Mesh *mesh_load(const char *filename);

#endif
