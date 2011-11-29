#include <assert.h>
#include <math.h>

#include "material.h"

Colour diff_colour(Light *light, Material *mat, Vec3 cam_dir, Vec3 light_dir,
		Vec3 normal)
{
	Colour light_col;
	float ndotv;

	cam_dir = cam_dir;

	ndotv = MAX(0, vec3_dot(light_dir, normal));

	light_col = colour_scale(light->intensity, colour_scale(ndotv, light->colour));
	return colour_mul(mat->colour, light_col);
}

Colour spec_colour(Light *light, Material *mat, Vec3 light_dir, Vec3 cam_dir,
		Vec3 normal)
{
	float hdotn;
	Vec3 half;
	Colour light_col;

	half = vec3_normalize(vec3_add(light_dir, cam_dir));
	hdotn = powf(MAX(0, vec3_dot(half, normal)), mat->shininess);

	light_col = colour_scale(light->intensity,
			colour_scale(hdotn, light->colour));
	return colour_mul(mat->colour, light_col);
}

Colour light_mat_colour(Light *light, Material *mat, Vec3 cam_dir,
		Vec3 light_dir, Vec3 normal)
{
	Colour final, col1, col2;

	switch(mat->type)
	{
	case MATERIAL_DIFFUSE:
		return diff_colour(light, mat, cam_dir, light_dir, normal);
		break;
	case MATERIAL_PHONG:
		return spec_colour(light, mat, cam_dir, light_dir, normal);
		break;
	case MATERIAL_COMBINED:
		col1 = light_mat_colour(light, mat->mat1, cam_dir, light_dir, normal);
		col2 = light_mat_colour(light, mat->mat2, cam_dir, light_dir, normal);
		final = colour_add(
				colour_scale(mat->weight1, col1),
				colour_scale(mat->weight2, col2));
		break;
	default:
		assert("Unknown material!" == NULL);
	}

	return final;
}
