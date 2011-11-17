#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <objreader/objreader.h>

#include "mesh.h"

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

	tri->vertex[msb->cur_triangle_vertex].vertex_index = v - 1;
	tri->vertex[msb->cur_triangle_vertex].texcoord_index = vt - 1;
	tri->vertex[msb->cur_triangle_vertex].normal_index = vn - 1;

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
