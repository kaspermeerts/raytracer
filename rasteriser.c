#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "glm.h"
#include "colour.h"
#include "scene.h"
#include "ppm.h"

#define WIDTH  512
#define HEIGHT 512

typedef float Depth;

static void tesselate_shape(Shape *shape)
{
	switch(shape->type)
	{
	case SHAPE_MESH:
		break;
	default:
		printf("Cannot tesselate shape \"%s\"\n", shape->name);
		break;
	}

	shape->type = SHAPE_MESH;
}

static void cam_view_matrix(const Camera *cam, Matrix *view)
{
	glmLoadIdentity(view);
	/* Because the matrix transforms vertices, we have to give
	 * the inverse transformation */
	glmMultQuaternion(view, quat_conjugate(cam->orientation));
	glmTranslateVector(view, vec3_scale(-1, cam->position));
}

static void rasterise(Scene *scene, Colour *buffer)
{
	int i;
	Matrix *mvp, *projection, *view, *model;
	double proj_m[16], view_m[16], model_m[16];
	Depth *zbuffer;
	Surface *surface = scene->root;
	assert(surface->shape->type == SHAPE_MESH);
	Mesh *mesh = surface->shape->u.mesh;

	zbuffer = calloc(WIDTH*HEIGHT, sizeof(Depth));
	projection = glmNewMatrixStack();
	view = glmNewMatrixStack();
	model = glmNewMatrixStack();
	mvp = glmNewMatrixStack();

	glmLoadIdentity(projection);
//	glmOrtho(projection, -1, 1, -1, 1, 1, 5);
	cam_view_matrix(scene->camera, view);
	glmLoadMatrix(model, surface->model_to_world);

	glmSaveMatrix(projection, proj_m);
	glmSaveMatrix(view, view_m);
	glmSaveMatrix(model, model_m);

	glmLoadIdentity(mvp);
	glmMultMatrix(mvp, proj_m);
	glmMultMatrix(mvp, view_m);
	glmMultMatrix(mvp, model_m);

	for (i = 0; i < mesh->num_triangles; i++)
	{
		Vec3 v, w;
		v = mesh->vertex[mesh->triangle[i].vertex[0].vertex_index];
		printf("%g\t%g\t%g\n", v.x, v.y, v.z);
		w = glmTransformVector(mvp, v);
		printf("%g\t%g\t%g\n", w.x, w.y, w.z);

	}

}

int main(int argc, char **argv)
{
	Sdl *sdl;
	Colour *buffer;
	FILE *out;

	if (argc < 2)
	{
		printf("Give an SDL file as an argument\n");
		return 1;
	}

	sdl = sdl_load(argv[1]);
	if (sdl == NULL)
		return 1;

	for (int i = 0; i < sdl->num_shapes; i++)
		tesselate_shape(&sdl->shape[i]);

	buffer = calloc(WIDTH*HEIGHT, sizeof(Colour));

	rasterise(&sdl->scene, buffer);

	out = fopen("raster.ppm", "wb");
	ppm_write(buffer, WIDTH, HEIGHT, out);
	fclose(out);

	return 0;
}
