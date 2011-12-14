#ifndef CG_TEXTURE
#define CG_TEXTURE

#include "cgmath.h"

enum CUBE_DIRECTION {
	NEGATIVE_X = 0, POSITIVE_X,
	NEGATIVE_Y    , POSITIVE_Y,
	NEGATIVE_Z    , POSITIVE_Z,
};

extern const char *cube_direction_str[];

typedef struct Texture {
	int width;
	int height;
	Colour *buffer;
} Texture;

typedef struct CubeMap {
	Texture *texture[6];
} CubeMap;

Texture *texture_load_png(const char *filename);
Colour texture_texel(Texture *texture, double u, double v);
CubeMap *cubemap_load(const char *prefix);
Colour cubemap_colour(CubeMap *map, Vec3 d);

#endif
