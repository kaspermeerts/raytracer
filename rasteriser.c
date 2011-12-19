#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "colour.h"
#include "scene.h"
#include "ppm.h"
#include "raster.h"

static struct {
	Vec3 normal;
	Vec3 light_dir, cam_dir;
} varying[3];

static struct {
	Mat4 mv;
	Mat4 mvp;
	Mat4 normal_matrix;
	Vec3 light_pos;
} uniform;

typedef struct Screen3 {
	int x;
	int y;
	float z;
} Screen3;

static void tesselate_shape(Shape *shape)
{
	switch(shape->type)
	{
	case SHAPE_MESH:
		break;
	default:
		printf("Cannot tesselate shape \"%s\"\n", shape->name);
		exit(1);
		break;
	}

	shape->type = SHAPE_MESH;
}

/* The third argument gives the index of the vertex of the current triangle */
static Vec4 vertex_shader(Vec3 position, Vec3 normal, int i)
{
	Vec3 eye_pos_xyz; /* Position in eye space */
	Vec3 light_dir;
	Vec4 eye_pos, homo_pos; /* Homogeneous position vector; */

	homo_pos = vec4_from_vec3(position, 1.0);
	eye_pos = mat4_transform(uniform.mv, homo_pos);
	/* No homogeneous divide necessary, as the modelview matrix holds no
	 * perspective. */
	eye_pos_xyz.x = eye_pos.x;
	eye_pos_xyz.y = eye_pos.y;
	eye_pos_xyz.z = eye_pos.z;
	assert(fabs(eye_pos.w - 1.0) < 0.01);

	light_dir = vec3_sub(uniform.light_pos, eye_pos_xyz);
	varying[i].normal = mat4_transform3_hetero(uniform.normal_matrix, normal);
	varying[i].light_dir = vec3_normalize(light_dir);
	varying[i].cam_dir = vec3_scale(-1, vec3_normalize(eye_pos_xyz));

	return mat4_transform(uniform.mvp, homo_pos);
}

static Screen3 vec3_to_screen3(Vec4 v4)
{
	int nx = config->width;
	int ny = config->height;
	Vec3 v3;
	Screen3 coord;

	v3 = vec4_homogeneous_divide(v4);

	coord.x = nx/2*(v3.x + 1) - 0.5;
	coord.y = ny/2*(v3.y + 1) - 0.5;
	coord.z = v3.z;
	return coord;
}

static Vec3 vec3_interpolate(float a, float b, float c,
		Vec3 v1, Vec3 v2, Vec3 v3)
{
	return vec3_add(vec3_add(
			vec3_scale(a, v1),
			vec3_scale(b, v2)),
			vec3_scale(c, v3));
}

static Colour fragment_shader(Material *mat, float a, float b, float c)
{
	mat = mat;
	Vec3 L, C, N;
	Light *light = scene->light[0];

	N = vec3_interpolate(a, b, c, 
			varying[0].normal, varying[1].normal, varying[2].normal);
	N = vec3_normalize(N);
	C = vec3_interpolate(a, b, c, 
			varying[0].cam_dir, varying[1].cam_dir, varying[2].cam_dir);
	L = vec3_interpolate(a, b, c, 
			varying[0].light_dir, varying[1].light_dir, varying[2].light_dir);

	return light_mat_colour(light, mat, C, L, N);
}

static void raster_triangle(Raster *raster, Material *mat, Screen3 coord[3])
{
	int xmin, ymin, xmax, ymax;
	float fa, fb, fc;
	float fao, fbo, fco; /* Coefficients for offscreen point */
	int x0 = coord[0].x, x1 = coord[1].x, x2 = coord[2].x;
	int y0 = coord[0].y, y1 = coord[1].y, y2 = coord[2].y;
	float z0 = coord[0].z, z1 = coord[1].z, z2 = coord[2].z;

	xmin = MIN(x0, MIN(x1, x2));
	ymin = MIN(y0, MIN(y1, y2));
	xmax = MAX(x0, MAX(x1, x2));
	ymax = MAX(y0, MAX(y1, y2));

	fa = (y1 - y2)*x0 + (x2 - x1)*y0 + x1*y2 - x2*y1;
	fb = (y2 - y0)*x1 + (x0 - x2)*y1 + x2*y0 - x0*y2;
	fc = (y0 - y1)*x2 + (x1 - x0)*y2 + x0*y1 - x1*y0;

	fao = -(y1 - y2) + -(x2 - x1) + x1*y2 - x2*y1;
	fbo = -(y2 - y0) + -(x0 - x2) + x2*y0 - x0*y2;
	fco = -(y0 - y1) + -(x1 - x0) + x0*y1 - x1*y0;

	xmin = MAX(0, xmin);
	xmax = MIN(raster->width, xmax);

	ymin = MAX(0, ymin);
	ymax = MIN(raster->height, ymax);
	for (int y = ymin; y <= ymax; y++)
	{
		for (int x = xmin; x <= xmax; x++)
		{
			float a, b, c, z;
			Colour col;

			a = ((y1 - y2)*x + (x2 - x1)*y + x1*y2 - x2*y1)/fa;
			b = ((y2 - y0)*x + (x0 - x2)*y + x2*y0 - x0*y2)/fb;
			c = ((y0 - y1)*x + (x1 - x0)*y + x0*y1 - x1*y0)/fc;

			z = a*z0 + b*z1 + c*z2;

			if (a >= 0 && b >= 0 && c >= 0)
			{
				/*
				if (a > 0 || fa*fao > 0)
				if (b > 0 || fb*fbo > 0)
				if (c > 0 || fc*fco > 0)*/
				{
					if (raster_z_pixel(raster, x, y, z))
					{
						col = fragment_shader(mat, a, b, c);
						//col = colour_scale(z, RED);
						raster_pixel(raster, x, y, col);
					}
				}
			}
		}
	}
}

static void rasterise_mesh(Raster *raster, Mesh *mesh, Material *mat)
{
	for (int i = 0; i < mesh->num_triangles; i++)
	{
		Vec4 pos[3];
		Screen3 coord[3];
		Triangle tri = mesh->triangle[i];

		for (int j = 0; j < 3; j++)
		{
			Vec3 vertex = mesh->vertex[tri.vertex_index[j]];
			Vec3 normal = mesh->normal[tri.normal_index[j]];
			pos[j] = vertex_shader(vertex, normal, j);
			coord[j] = vec3_to_screen3(pos[j]);
		}

#if 0
		raster_line(raster, coord[0].x, coord[0].y, coord[1].x, coord[1].y);
		raster_line(raster, coord[1].x, coord[1].y, coord[2].x, coord[2].y);
		raster_line(raster, coord[2].x, coord[2].y, coord[0].x, coord[0].y);
#else
		raster_triangle(raster, mat, coord);
#endif
	}
}

static void cam_proj_matrix(const Camera *cam, Mat4 proj, float znear, float zfar)
{
	const float aspect = ((float) config->width) / ((float) config->height);
	mat4_perspective(proj, cam->fov*M_TWO_PI/360., aspect, znear, zfar);
}

static void cam_view_matrix(const Camera *cam, Mat4 view)
{
	Mat4 d;

	mat4_identity(view);
	/* Because the matrix transforms vertices, we have to give
	 * the inverse transformation */
	mat4_quaternion(d, quat_conjugate(cam->orientation));
	mat4_rmul(view, d);
	mat4_translate_vector(d, vec3_scale(-1, cam->position));
	mat4_rmul(view, d);
}

static void cam_view_inv_matrix(const Camera *cam, Mat4 inv_view)
{
	mat4_quaternion(inv_view, cam->orientation);
}

static void rasterise(Raster *raster)
{
	Mat4 proj, view, model, inv_view, inv_model, tmp;
	Surface *surface = scene->root;
	assert(surface->shape->type == SHAPE_MESH);
	Mesh *mesh = surface->shape->u.mesh;

	/* Projection */
	//mat4_ortho(proj, -1, 1, -1, 1, -1, 1);
	cam_proj_matrix(scene->camera, proj, -1, -100);
	/* View */
	cam_view_matrix(scene->camera, view);
	cam_view_inv_matrix(scene->camera, inv_view);
	/* Model */
	mat4_copy(model, surface->model_to_world);
	mat4_copy(inv_model, surface->world_to_model);

	/* MVP matrix */
	mat4_identity(tmp);
	mat4_rmul(tmp, proj);
	mat4_rmul(tmp, view);
	mat4_rmul(tmp, model);
	mat4_copy(uniform.mvp, tmp);

	/* MV matrix */
	mat4_identity(tmp);
	mat4_rmul(tmp, view);
	mat4_rmul(tmp, model);
	mat4_copy(uniform.mv, tmp);

	/* Normal matrix */
	mat4_identity(tmp);
	mat4_lmul(inv_view, tmp);
	mat4_lmul(inv_model, tmp);
	mat4_transpose(tmp);
	mat4_copy(uniform.normal_matrix, tmp);

	/* IMPORTANT: Transform the light position with the view matrix.
	 * Otherwise, the position will be interpreted as in model space, and thus
	 * fixed in the reference frame of the object. This will make lighting on an
	 * object appear static */
	uniform.light_pos = mat4_transform3_homo(view, scene->light[0]->position);

#if 0
	printf("Projection\n");
	mat4_print(proj);
	printf("\nView\n");
	mat4_print(view);
	printf("\nModel\n");
	mat4_print(model);
#endif

	rasterise_mesh(raster, mesh, surface->material);
}

int main(int argc, char **argv)
{
	Sdl *sdl;
	Raster *raster;
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

	raster = raster_new(config->width, config->height);
	raster_fill(raster, scene->background);
	rasterise(raster);

	out = fopen("raster.ppm", "wb");
	ppm_write(raster->buffer, raster->width, raster->height, out);
	fclose(out);
	raster_destroy(raster);

	return 0;
}
