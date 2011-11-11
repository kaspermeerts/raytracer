#include <stdio.h>

#include "scene.h"

int main(int argc, char **argv)
{
	Sdl *sdl;

	if (argc < 2)
		return 1;

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;

	return 0;
}
