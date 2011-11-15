#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <objreader/objreader.h>

#include "mesh.h"

typedef struct MeshRecord {
	int num_vertices;
	bool has_normals;
	int num_normals;
	bool has_texcoords;
	int num_texcoords;
	int num_faces;
} MeshRecord;

/* First pass */
static int count_vertex(float x, float y, float z, float w, void *data)
{
	x = y = z = w = 0; /* Unused */
	MeshRecord *rec = (MeshRecord *)data;
	rec->num_vertices++;
	return 0;
}

static int count_normal(float x, float y, float z, void *data)
{
	x = y = z = 0; /* Unused */
	MeshRecord *rec = (MeshRecord *)data;
	rec->has_normals = true;
	rec->num_vertices++;
	return 0;
}

static int count_texcoord(float u, float v, void *data)
{
	u = v = 0; /* Unused */
	MeshRecord *rec = (MeshRecord *)data;
	rec->has_texcoords = true;
	rec->num_vertices++;
	return 0;
}

static int count_face(void *data)
{
	MeshRecord *rec = (MeshRecord *)data;
	rec->num_faces++;
	return 0;
}

static bool obj_first_pass(FILE *fd, Mesh *mesh)
{
	int res;
	MeshRecord rec;
	ObjParseCallbacks ocb;

	rec.num_vertices = rec.num_normals = rec.num_texcoords = rec.num_faces = 0;
	rec.has_normals = rec.has_texcoords = false;

	ocb.onVertex = count_vertex;
	ocb.onTexel = count_texcoord;
	ocb.onNormal = count_normal;
	ocb.onStartFace = count_face;
	ocb.onAddToFace = NULL;
	ocb.onStartGroup = NULL;
	ocb.onGroupName = NULL;
	ocb.onRefMaterialLib = NULL;
	ocb.onUseMaterial = NULL;
	ocb.onSmoothingGroup = NULL;
	ocb.userData = &rec;

	res = ReadObjFile(fd, &ocb);

	if (res != 0)
	{
		printf("First pass failed\n");
		return false;
	}
	if (rec.has_normals && (rec.num_normals != rec.num_vertices))
	{
		printf("Inconsistent amount of vertices and normals: %d and %d\n",
				rec.num_vertices, rec.num_normals);
		return false;
	}
	if (rec.has_texcoords && (rec.num_texcoords != rec.num_vertices))
	{
		printf("Inconsistent amount of vertices and texcoords: %d and %d\n",
				rec.num_vertices, rec.num_texcoords);
		return false;
	}

	mesh->num_vertices = rec.num_vertices;

	return true;
}

/* Second pass */
static bool obj_second_pass(FILE *fd, Mesh *mesh)
{
	return true;
}

int main(int argc, char **argv)
{
	FILE *fd;
	int res;
	Mesh *mesh;

	if (argc < 2)
		return 1;

	mesh = malloc(sizeof(Mesh));

	if ((fd = fopen(argv[1], "r")) == NULL)
		return 1;
	if (!obj_first_pass(fd, mesh))
	{
		fclose(fd);
		return 1;
	}
	fclose(fd);

	if ((fd = fopen(argv[1], "r")) == NULL)
		return 1;
	if (!obj_second_pass(fd, mesh))
	{
		fclose(fd);
		return 1;
	}

	fclose(fd);
	return 0;
}
