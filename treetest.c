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

	mesh_build_kd_tree(mesh);

	return 0;
}
