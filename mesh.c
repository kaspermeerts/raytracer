#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <objreader/objreader.h>

#include "mesh.h"

/***********************
 * Parsing and loading *
 ***********************/
static int count_vertex(float x, float y, float z, float w, void *data)
{
	x = y = z = w = 0; /* Unused */
	Mesh *rec = (Mesh *)data;
	rec->num_vertices++;
	return 0;
}

static int count_normal(float x, float y, float z, void *data)
{
	x = y = z = 0; /* Unused */
	Mesh *rec = (Mesh *)data;
	rec->has_normals = true;
	rec->num_normals++;
	return 0;
}

static int count_texcoord(float u, float v, void *data)
{
	u = v = 0; /* Unused */
	Mesh *rec = (Mesh *)data;
	rec->has_texcoords = true;
	rec->num_texcoords++;
	return 0;
}

static int count_triangle(void *data)
{
	Mesh *rec = (Mesh *)data;
	rec->num_triangles++;
	return 0;
}

static bool obj_first_pass(FILE *fd, Mesh *mesh)
{
	int res;
	ObjParseCallbacks ocb;

	mesh->num_vertices = mesh->num_normals = mesh->num_texcoords = 0;
	mesh->num_triangles = 0;
	mesh->has_normals = mesh->has_texcoords = false;

	ocb.onVertex = count_vertex;
	ocb.onTexel = count_texcoord;
	ocb.onNormal = count_normal;
	ocb.onStartLine = NULL;
	ocb.onAddToLine = NULL;
	ocb.onStartFace = count_triangle;
	ocb.onAddToFace = NULL;
	ocb.onStartObject = NULL;
	ocb.onStartGroup = NULL;
	ocb.onGroupName = NULL;
	ocb.onRefMaterialLib = NULL;
	ocb.onUseMaterial = NULL;
	ocb.onSmoothingGroup = NULL;
	ocb.userData = mesh;
	res = ReadObjFile(fd, &ocb);

	return res == 0;
}

/* Second pass */

typedef struct MeshBuilder {
	int cur_vertex;
	int cur_normal;
	int cur_texcoord;
	int cur_triangle;
	int cur_triangle_vertex;
	Mesh *mesh;
} MeshBuilder;

static int add_vertex(float x, float y, float z, float w, void *data)
{
	MeshBuilder *msb = (MeshBuilder *) data;
	w = w; /* Unused */

	assert(msb->cur_vertex < msb->mesh->num_vertices);

	msb->mesh->vertex[msb->cur_vertex].x = x;
	msb->mesh->vertex[msb->cur_vertex].y = y;
	msb->mesh->vertex[msb->cur_vertex].z = z;
	msb->cur_vertex++;

	return 0;
}

static int add_texcoord(float u, float v, void *data)
{
	MeshBuilder *msb = (MeshBuilder *) data;

	assert(msb->cur_texcoord < msb->mesh->num_texcoords);

	msb->mesh->texcoord[msb->cur_texcoord].u = u;
	msb->mesh->texcoord[msb->cur_texcoord].v = v;
	msb->cur_texcoord++;

	return 0;
}

static int add_normal(float x, float y, float z, void *data)
{
	MeshBuilder *msb = (MeshBuilder *) data;

	assert(msb->cur_normal < msb->mesh->num_normals);

	msb->mesh->normal[msb->cur_normal].x = x;
	msb->mesh->normal[msb->cur_normal].y = y;
	msb->mesh->normal[msb->cur_normal].z = z;
	msb->cur_normal++;

	return 0;
}

static int add_triangle(void *data)
{
	MeshBuilder *msb = (MeshBuilder *) data;

	msb->cur_triangle++;
	msb->cur_triangle_vertex = 0;

	return 0;
}

static int add_index(size_t v, size_t vt, size_t vn, void *data)
{
	MeshBuilder *msb = (MeshBuilder *) data;
	Triangle *tri = &msb->mesh->triangle[msb->cur_triangle];

	assert(msb->cur_triangle < msb->mesh->num_triangles);
	assert(msb->cur_triangle_vertex < 3);
	assert((int) v < msb->mesh->num_vertices + 1);
	assert(!msb->mesh->has_texcoords || (int) vt < msb->mesh->num_texcoords + 1);
	assert(!msb->mesh->has_normals   || (int) vn < msb->mesh->num_normals + 1);

	tri->vertex_index[msb->cur_triangle_vertex] = v - 1;
	tri->texcoord_index[msb->cur_triangle_vertex] = vt - 1;
	tri->normal_index[msb->cur_triangle_vertex] = vn - 1;

	msb->cur_triangle_vertex++;

	return 0;
}

static bool obj_second_pass(FILE *fd, Mesh *mesh)
{
	int res;
	ObjParseCallbacks ocb;
	MeshBuilder msb;

	/* This gets incremented at the start of each triangle */
	msb.cur_triangle = -1;
	msb.cur_triangle_vertex = 0;
	msb.cur_vertex = msb.cur_normal = msb.cur_texcoord = 0;
	msb.mesh = mesh;

	ocb.onVertex = add_vertex;
	ocb.onTexel = add_texcoord;
	ocb.onNormal = add_normal;
	ocb.onStartLine = NULL;
	ocb.onAddToLine = NULL;
	ocb.onStartFace = add_triangle;
	ocb.onAddToFace = add_index;
	ocb.onStartObject = NULL;
	ocb.onStartGroup = NULL;
	ocb.onGroupName = NULL;
	ocb.onRefMaterialLib = NULL;
	ocb.onUseMaterial = NULL;
	ocb.onSmoothingGroup = NULL;
	ocb.userData = &msb;

	res = ReadObjFile(fd, &ocb);

	return res == 0;
}

Mesh *mesh_load(const char *filename)
{
	FILE *fd;
	Mesh *mesh;

	if ((fd = fopen(filename, "r")) == NULL)
	{
		printf("Opening file %s failed: %s\n", filename, strerror(errno));
		return NULL;
	}
	mesh = malloc(sizeof(Mesh));
	if (!obj_first_pass(fd, mesh))
	{
		printf("Error parsing file %s\n", filename);
		fclose(fd);
		return NULL;
	}
	fclose(fd);

	mesh->vertex = calloc(mesh->num_vertices, sizeof(mesh->vertex[0]));
	if (mesh->has_normals)
		mesh->normal = calloc(mesh->num_normals, sizeof(mesh->normal[0]));
	if (mesh->has_texcoords)
		mesh->texcoord = calloc(mesh->num_texcoords, sizeof(mesh->texcoord[0]));
	mesh->triangle = calloc(mesh->num_triangles, sizeof(mesh->triangle[0]));

	if ((fd = fopen(filename, "r")) == NULL)
	{
		printf("Opening file %s failed: %s\n", filename, strerror(errno));
		return NULL;
	}
	if (!obj_second_pass(fd, mesh))
	{
		printf("Error parsing file %s\n", filename);
		fclose(fd);
		return NULL;
	}
	fclose(fd);

	return mesh;
}

/********************
 * kd-tree building *
 ********************/

static KdNode *kd_node_new(void)
{
	KdNode *node;

	node = malloc(sizeof(KdNode));
	node->leaf = false;
	node->axis = -1;
	node->location = HUGE_VAL;
	node->left = node->right = NULL;
	node->num_triangles = 0;
	node->triangle = NULL;

	return node;
}

static void split_kd_tree(const Vec3 *vertex_list, KdNode *tree, enum AXIS axis,
		float location)
{
	int lefti, righti;

	tree->location = location;
	tree->left = kd_node_new();
	tree->right = kd_node_new();
	for (int i = 0; i < tree->num_triangles; i++)
	{
		bool v_left[3]; /* v_left[i]: is vertex i left or right */
		for (int j = 0; j < 3; j++)
		{
			Vec3 v = vertex_list[tree->triangle[i].vertex_index[j]];
			if (axis == X_AXIS)
				v_left[j] = v.x <= tree->location;
			else if (axis == Y_AXIS)
				v_left[j] = v.y <= tree->location;
			else /* axis == Z_AXIS */
				v_left[j] = v.z <= tree->location;
		}

		if (v_left[0] || v_left[1] || v_left[2])
			tree->left->num_triangles++;
		if (!v_left[0] || !v_left[1] || !v_left[2])
			tree->right->num_triangles++;
	}

	tree->left->triangle = calloc(tree->left->num_triangles, sizeof(Triangle));
	tree->right->triangle = calloc(tree->right->num_triangles, sizeof(Triangle));

	lefti = righti = 0;
	for (int i = 0; i < tree->num_triangles; i++)
	{
		bool v_left[3]; /* v_left[i]: is vertex i left or right */
		for (int j = 0; j < 3; j++)
		{
			Vec3 v = vertex_list[tree->triangle[i].vertex_index[j]];
			if (axis == X_AXIS)
				v_left[j] = v.x <= tree->location;
			else if (axis == Y_AXIS)
				v_left[j] = v.y <= tree->location;
			else /* axis == Z_AXIS */
				v_left[j] = v.z <= tree->location;
		}

		if (v_left[0] || v_left[1] || v_left[2])
		{
			tree->left->triangle[lefti] = tree->triangle[i];
			lefti++;
		}
		if (!v_left[0] || !v_left[1] || !v_left[2])
		{
			tree->right->triangle[righti] = tree->triangle[i];
			righti++;
		}
	}
	assert(tree->left->num_triangles == lefti);
	assert(tree->right->num_triangles == righti);
	free(tree->triangle);
	tree->triangle = NULL;
	tree->num_triangles = 0;
	tree->leaf = false;
}

static float calculate_cost(const Vec3 *vertex_list, const KdNode *tree,
		enum AXIS axis,	float location, BBox tree_box)
{
	int left_tris, right_tris;
	BBox left_box, right_box;

	for (int i = 0; i < tree->num_triangles; i++)
	{
		Triangle tri = tree->triangle[i];
		bool v_left[3]; /* v_left[i]: is vertex i left or right */
		for (int j = 0; j < 3; j++)
		{
			Vec3 v = vertex_list[tri.vertex_index[j]];
			if (axis == X_AXIS)
				v_left[j] = v.x <= location;
			else if (axis == Y_AXIS)
				v_left[j] = v.y <= location;
			else /* axis == Z_AXIS */
				v_left[j] = v.z <= location;
		}

		if (v_left[0] || v_left[1] || v_left[2])
			left_tris++;
		if (!v_left[0] || !v_left[1] || !v_left[2])
			right_tris++;
	}

	bbox_split(tree_box, axis, location, &left_box, &right_box);
	return left_tris * bbox_surface_area(left_box) +
			right_tris * bbox_surface_area(right_box);

}

static void build_kd_subtree(const Vec3 *vertex_list, KdNode *tree, int depth,
		enum AXIS axis, BBox bbox)
{
	enum AXIS next_axis;
	float best_cost, best_location;
	BBox left_box, right_box;

	if (tree->num_triangles <= 10 || depth == 8)
	{
		tree->leaf = true;
		tree->left = tree->right = NULL;
		return;
	}
	tree->axis = axis;

	/* Determine the optimum location using the surface area heuristic */
	best_cost = HUGE_VAL;
	best_location = 0;
	for (int i = 0; i < tree->num_triangles; i++)
	{
		Triangle tri = tree->triangle[i];
		for (int j = 0; j < 3; j++)
		{
			float loc, cost;
			Vec3 v = vertex_list[tri.vertex_index[j]];

			if (axis == X_AXIS)
				loc = v.x;
			else if (axis == Y_AXIS)
				loc = v.y;
			else
				loc = v.z;

			cost = calculate_cost(vertex_list, tree, axis, loc, bbox);
			if (cost < best_cost)
			{
				best_cost = cost;
				best_location = loc;
			}
		}
	}

	/* Now, split the tree in twain at this location */
	split_kd_tree(vertex_list, tree, axis, best_location);
	bbox_split(bbox, axis, best_location, &left_box, &right_box);

	switch (axis)
	{
	case X_AXIS:
		next_axis = Y_AXIS;
		break;
	case Y_AXIS:
		next_axis = Z_AXIS;
		break;
	case Z_AXIS:
		next_axis = X_AXIS;
		break;
	default:
		printf("Invalid axis %d\n", axis);
		exit(1);
		break;
	}

	build_kd_subtree(vertex_list, tree->left, depth + 1, next_axis, left_box);
	build_kd_subtree(vertex_list, tree->right, depth + 1, next_axis, right_box);
}

void mesh_build_kd_tree(Mesh *mesh)
{
	BBox bbox;

	bbox.xmin = bbox.ymin = bbox.zmin =  HUGE_VAL;
	bbox.xmax = bbox.ymax = bbox.zmax = -HUGE_VAL;

	for (int i = 0; i < mesh->num_triangles; i++)
	{
		Triangle tri = mesh->triangle[i];
		for (int j = 0; j < 3; j++)
		{
			Vec3 v = mesh->vertex[tri.vertex_index[j]];
			if (v.x < bbox.xmin)
				bbox.xmin = v.x;
			if (v.x > bbox.xmax)
				bbox.xmax = v.x;
			if (v.y < bbox.ymin)
				bbox.ymin = v.y;
			if (v.y > bbox.ymax)
				bbox.ymax = v.y;
			if (v.z < bbox.zmin)
				bbox.zmin = v.z;
			if (v.z > bbox.zmax)
				bbox.zmax = v.z;
		}
	}
	mesh->kd_tree = kd_node_new();
	mesh->kd_tree->num_triangles = mesh->num_triangles;
	mesh->kd_tree->triangle = calloc(mesh->kd_tree->num_triangles,
			sizeof(Triangle));
	memcpy(mesh->kd_tree->triangle, mesh->triangle,
			mesh->num_triangles * sizeof(Triangle));
	build_kd_subtree(mesh->vertex, mesh->kd_tree, 0, X_AXIS, bbox);
}
