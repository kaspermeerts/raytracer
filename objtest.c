#include <stdlib.h>
#include <stdio.h>
#include "mesh.h"

int main(int argc, char **argv)
{
	Mesh *mesh;

	if (argc < 2)
		return 1;

	mesh = mesh_load(argv[1]);
	if (mesh == NULL)
		return 1;

	printf("%d vertices\n%d normals\n%d texcoords\n",
			mesh->num_vertices, mesh->num_normals, mesh->num_texcoords);

	return 0;
}
