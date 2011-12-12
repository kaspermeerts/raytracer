#ifndef CG_TEXTURE
#define CG_TEXTURE

enum CUBE_DIRECTION {
	NEGATIVE_X = 0, POSITIVE_X,
	NEGATIVE_Y    , POSITIVE_Y,
	NEGATIVE_Z    , POSITIVE_Z,
};

static const char *cube_direction[] = {
		"negative_x", "positive_x", "negative_y", "positive_y",
		"negative_z", "positive_z" };

typedef struct CubeMap {
	int width, height;
	Colour *buffer[6];
} CubeMap;

CubeMap *cubemap_load(const char *prefix);

#endif
