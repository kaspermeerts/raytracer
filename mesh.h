#ifndef CG_MESH_H
#define CG_MESH_H

#include <stdbool.h>
#include "cgmath.h"

typedef struct TexCoord {
	float u, v;
} TexCoord;

typedef struct Mesh {
	char *name;

	bool has_normals, has_texcoords;
	int num_vertices;
	Vec3 *vertex;
	Vec3 *normal;
	TexCoord *texcoord;

	int num_indices;
	int *index; /* Shorts? */
} Mesh;

#endif
