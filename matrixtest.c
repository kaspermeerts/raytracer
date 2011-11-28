#include <stdio.h>

#include "scene.h"
#include "cgmath.h"

int main(int argc, char **argv)
{
	Sdl *sdl;
	Surface *surface;

	if (argc < 2)
		return 1;

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;

	surface = sdl->scene.root;
	while (surface)
	{
		Mat4 prod;
		printf("%s\n", surface->shape->name);
		mat4_print(surface->model_to_world);
		printf("\n");
		mat4_print(surface->world_to_model);
		printf("\n");
		mat4_mult(prod, surface->model_to_world, surface->world_to_model);
		mat4_print(prod);
		printf("\n\n");
		surface = surface->next;
	}


	return 0;
}
