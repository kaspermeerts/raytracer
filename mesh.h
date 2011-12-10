#ifndef CG_MESH_H
#define CG_MESH_H

#include <stdbool.h>
#include "cgmath.h"

enum axis {X_AXIS, Y_AXIS, Z_AXIS};

typedef struct Triangle {
	int vertex_index[3];
	int normal_index[3];
	int texcoord_index[3];
} Triangle;

typedef struct TexCoord {
	float u, v;
} TexCoord;

typedef struct Mesh {
	char *name;

	int num_vertices;
	Vec3 *vertex;

	bool has_normals;
	int num_normals;
	Vec3 *normal;

	bool has_texcoords;
	int num_texcoords;
	TexCoord *texcoord;

	int num_triangles;
	Triangle *triangle;

	struct KdNode *kd_tree;
} Mesh;

typedef struct KdNode {
	bool leaf;
	enum axis axis;
	struct KdNode *left;
	struct KdNode *right;
	float location;
	int num_triangles;
	Triangle *triangle;
} KdNode;

Mesh *mesh_load(const char *filename);
void mesh_build_kd_tree(Mesh *mesh);

#endif
