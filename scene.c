#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "scene.h"
#include "mesh.h"
#include "glm.h"

static Vec3 parse_vec3(const char *string)
{
	assert(string != NULL);
	Vec3 v;
	char *end;

	v.x = strtof(string, &end);
	v.y = strtof(end, &end);
	v.z = strtof(end, &end);

	return v;
}

static double parse_double(const char *string)
{
	assert(string != NULL);
	char *end;

	return strtod(string, &end);
}

static int parse_int(const char *string)
{
	assert(string != NULL);
	char *end;

	return strtol(string, &end, 10);
}

static Colour parse_colour(const char *string)
{
	assert(string != NULL);
	Colour c;
	char *end;

	c.r = strtof(string, &end);
	c.g = strtof(end, &end);
	c.b = strtof(end, &end);

	return c;
}

static bool parse_bool(const char *string)
{
	assert(string != NULL);
	return (strcmp(string, "true") == 0);
}

static char *strdup(const char *string)
{
	assert(string != NULL);
	char *new;
	size_t len;

	len = strlen(string);
	new = malloc(len+1);
	memcpy(new, string, len);
	new[len] = '\0';

	return new;
}

static bool import_cameras(Sdl *sdl, xmlNode *node, int n)
{
	int i;
	xmlNode *cur_node;
	Vec3 direction, up;
	Mat3 m;

	sdl->num_cameras = n;
	sdl->camera = calloc(n, sizeof(Camera));

	for (i = 0, cur_node = xmlFirstElementChild(node); cur_node; 
			i++, cur_node = xmlNextElementSibling(cur_node))
	{
		Camera *cam = &sdl->camera[i];
		assert(strcmp(cur_node->name, "Camera") == 0);

		cam->position = parse_vec3(xmlGetProp(cur_node, "position"));
		cam->fov = parse_double(xmlGetProp(cur_node, "fovy"));
		cam->width = parse_int(xmlGetProp(cur_node, "width"));
		cam->height = parse_int(xmlGetProp(cur_node, "height"));
		cam->name = strdup(xmlGetProp(cur_node, "name"));

		direction = parse_vec3(xmlGetProp(cur_node, "direction"));
		up = parse_vec3(xmlGetProp(cur_node, "up"));
		cam->w = vec3_scale(-1, vec3_normalize(direction));
		cam->u = vec3_normalize(vec3_cross(up, cam->w));
		cam->v = vec3_cross(cam->w, cam->u);
#define M(i, j) m[3*j + i]
		M(0, 0) = cam->u.x; M(0, 1) = cam->v.x; M(0, 2) = cam->w.x;
		M(1, 0) = cam->u.y; M(1, 1) = cam->v.y; M(1, 2) = cam->w.y;
		M(2, 0) = cam->u.z; M(2, 1) = cam->v.z; M(2, 2) = cam->w.z;
#undef M
		cam->orientation = quat_from_mat3(m);
	}
	assert(i == n);

	return true;
}

static bool import_lights(Sdl *sdl, xmlNode *node, int n)
{
	int i;
	xmlNode *cur_node;

	sdl->num_lights = n;
	sdl->light = calloc(n, sizeof(Light));

	for (i = 0, cur_node = xmlFirstElementChild(node); cur_node;
			i++, cur_node = xmlNextElementSibling(cur_node))
	{
		Light *light = &sdl->light[i];

		if (strcmp(cur_node->name , "DirectionalLight") == 0)
		{
			light->type = LIGHT_DIRECTIONAL;
			light->direction = vec3_normalize(
					parse_vec3(xmlGetProp(cur_node, "direction")));
		} else if (strcmp(cur_node->name, "PointLight") == 0)
		{
			light->type = LIGHT_POINT;
			light->position = parse_vec3(xmlGetProp(cur_node, "position"));
		} else if (strcmp(cur_node->name, "SpotLight") == 0)
		{
			light->type = LIGHT_SPOT;
			light->position = parse_vec3(xmlGetProp(cur_node, "position"));
			light->direction = vec3_normalize(
					parse_vec3(xmlGetProp(cur_node, "direction")));
			light->angle = parse_double(xmlGetProp(cur_node, "angle"));
		} else
		{
			printf("Unknown light type: \"%s\"\n", cur_node->name);
			return false;
		}

		light->colour = parse_colour(xmlGetProp(cur_node, "color"));
		light->intensity = parse_double(xmlGetProp(cur_node, "intensity"));
		light->name = strdup(xmlGetProp(cur_node, "name"));
	}
	assert(i == n);

	return true;
}

static bool import_shapes(Sdl *sdl, xmlNode *node, int n)
{
	int i;
	xmlNode *cur_node;

	sdl->num_shapes = n;
	sdl->shape = calloc(n, sizeof(Shape));

	for (i = 0, cur_node = xmlFirstElementChild(node); cur_node;
			i++, cur_node = xmlNextElementSibling(cur_node))
	{
		Shape *shape = &sdl->shape[i];
		if (strcmp(cur_node->name, "Sphere") == 0)
		{
			shape->type = SHAPE_SPHERE;
			shape->u.sphere.radius =
					parse_double(xmlGetProp(cur_node, "radius"));
		} else if (strcmp(cur_node->name, "Cylinder") == 0)
		{
			shape->type = SHAPE_CYLINDER;
			shape->u.cylinder.radius =
					parse_double(xmlGetProp(cur_node, "radius"));
			shape->u.cylinder.height =
					parse_double(xmlGetProp(cur_node, "height"));
			shape->u.cylinder.capped =
					parse_bool(xmlGetProp(cur_node, "capped"));
		} else if (strcmp(cur_node->name, "Cone") == 0)
		{
			shape->type = SHAPE_CONE;
			shape->u.cone.radius = parse_double(xmlGetProp(cur_node, "radius"));
			shape->u.cone.height = parse_double(xmlGetProp(cur_node, "height"));
			shape->u.cone.capped = parse_bool(xmlGetProp(cur_node, "capped"));
		} else if (strcmp(cur_node->name, "Torus") == 0)
		{
			shape->type = SHAPE_TORUS;
			shape->u.torus.inner_radius =
					parse_double(xmlGetProp(cur_node, "innerRadius"));
			shape->u.torus.outer_radius =
					parse_double(xmlGetProp(cur_node, "outerRadius"));
		} else if (strcmp(cur_node->name, "Mesh") == 0)
		{
			const char *filename;
			shape->type = SHAPE_MESH;
			filename = xmlGetProp(cur_node, "src");
			shape->u.mesh = mesh_load(filename);
			if (shape->u.mesh == NULL)
			{
				printf("Couldn't load mesh %s\n", filename);
				return false;
			}
		} else
		{
			printf("Unknown geometry type: %s\n", cur_node->name);
			return false;
		}
		shape->name = strdup(xmlGetProp(cur_node, "name"));

	}
	assert(i == n);
	return true;
}

static bool import_textures(Sdl *sdl, xmlNode *node, int n)
{
	int i;
	xmlNode *cur_node;

	sdl->num_textures = n;
	sdl->texture = calloc(n, sizeof(Texture));

	for (i = 0, cur_node = xmlFirstElementChild(node); cur_node;
			i++, cur_node = xmlNextElementSibling(cur_node))
	{
		Texture *tex = &sdl->texture[i];
		assert(strcmp(cur_node->name, "Texture") == 0);
		tex->source = strdup(xmlGetProp(cur_node, "src"));
		tex->name = strdup(xmlGetProp(cur_node, "name"));
		assert("Texture should be loaded here" == NULL);
		/* TODO: Load texture here? */
	}
	assert(i == n);

	return true;
}

static bool import_materials(Sdl *sdl, xmlNode *node, int n)
{
	int i;
	xmlNode *cur_node;

	sdl->num_materials = n;
	sdl->material = calloc(n, sizeof(Material));

	for (i = 0, cur_node = xmlFirstElementChild(node); cur_node;
			i++, cur_node = xmlNextElementSibling(cur_node))
	{
		Material *mat = &sdl->material[i];
		if (strcmp(cur_node->name, "DiffuseMaterial") == 0)
		{
			mat->type = MATERIAL_DIFFUSE;
			mat->colour = parse_colour(xmlGetProp(cur_node, "color"));
		} else if (strcmp(cur_node->name, "PhongMaterial") == 0)
		{
			mat->type = MATERIAL_PHONG;
			mat->colour = parse_colour(xmlGetProp(cur_node, "color"));
			mat->shininess = parse_double(xmlGetProp(cur_node, "shininess"));
		} else
		{
			const char *mat1_name, *mat2_name;
			int j;
			mat->type = MATERIAL_COMBINED;
			mat->weight1 = parse_double(xmlGetProp(cur_node, "weight1"));
			mat->weight2 = parse_double(xmlGetProp(cur_node, "weight2"));
			mat1_name = xmlGetProp(cur_node, "material1");
			mat2_name = xmlGetProp(cur_node, "material2");
			mat->mat1 = mat->mat2 = NULL;
			for (j = 0; j < i; j++)
			{
				if (strcmp(mat1_name, sdl->material[j].name) == 0)
					mat->mat1 = &sdl->material[j];
				if (strcmp(mat2_name, sdl->material[j].name) == 0)
					mat->mat2 = &sdl->material[j];
			}
			if (mat->mat1 == NULL)
			{
				printf("Couln't find requested material \"%s\"\n",
					mat1_name);
				return false;
			}
			if (mat->mat2 == NULL)
			{
				printf("Couln't find requested material \"%s\"\n",
					mat2_name);
				return false;
			}
		}
		mat->name = strdup(xmlGetProp(cur_node, "name"));
	}

	return true;
}

static bool import_light_refs(Sdl *sdl, const char *light_names)
{
	int i;
	const char *name, *end;
	Scene *scene = &sdl->scene;

	if (*light_names == '\0')
	{
		scene->num_lights = 0;
		return true;
	}

	i = 1;
	for (name = light_names; *name; name++)
		if (*name == ',')
			i++;
	scene->num_lights = i;

	if (scene->num_lights > MAX_LIGHTS)
	{
		printf("Too many lights: %d\n", scene->num_lights);
		return false;
	}

	i = 0;
	name = end = light_names;
	while (*end != '\0')
	{
		end++;
		if (*end == ',' || *end == '\0')
		{
			scene->light[i] = NULL;
			for (int j = 0; j < sdl->num_lights; j++)
				if (strncmp(sdl->light[j].name, name, end - name) == 0)
					scene->light[i] = &sdl->light[j];
			if (scene->light[i] == NULL)
			{
				printf("Couldn't find light %d of %s\n", i, light_names);
				return false;
			}
			name = end + 1;
			i++;
		}
	}

	return true;
}

static bool import_graph(Sdl *sdl, Surface **root, xmlNode *xml_node, Matrix *stack)
{
	xmlNode *child_node;

	if (strcmp(xml_node->name, "Shape") == 0)
	{
		const char *shape_name, *material_name;
		Surface *surf;
		surf = calloc(1, sizeof(Surface));
		surf->next = *root;
		*root = surf;
		shape_name = xmlGetProp(xml_node, "geometry");
		surf->shape = NULL;
		for (int i = 0; i < sdl->num_shapes; i++)
			if (strcmp(sdl->shape[i].name, shape_name) == 0)
				surf->shape = &sdl->shape[i];
		if (surf->shape == NULL)
		{
			printf("Requested shape \"%s\" not found\n", shape_name);
			return false;
		}
		if (xmlHasProp(xml_node, "texture"))
		{
			printf("Sorry, no texture support yet\n");
			return false;
		}
		material_name = xmlGetProp(xml_node, "material");
		surf->material = NULL;
		for (int i = 0; i < sdl->num_materials; i++)
			if (strcmp(sdl->material[i].name, material_name) == 0)
				surf->material = &sdl->material[i];
		if (surf->material == NULL)
		{
			printf("Requested material \"%s\" not found\n", material_name);
			return false;
		}
		glmSaveMatrix(stack, surf->model_to_world);
		return true;
	} else
	{

		glmPushMatrix(&stack);

		if (strcmp(xml_node->name, "Rotate") == 0)
		{
			double angle;
			Vec3 axis;

			angle = parse_double(xmlGetProp(xml_node, "angle")) *
					M_TWO_PI / 360.;
			axis = parse_vec3(xmlGetProp(xml_node, "axis"));

			glmRotate(stack, angle, axis.x, axis.y, axis.z);
		} else if (strcmp(xml_node->name, "Translate") == 0)
		{
			Vec3 v;

			v = parse_vec3(xmlGetProp(xml_node, "vector"));

			glmTranslateVector(stack, v);

		} else if (strcmp(xml_node->name, "Scale") == 0)
		{

		} else
		{
			printf("Unknown node: \"%s\"\n", xml_node->name);
			return false;
		}

		child_node = xmlFirstElementChild(xml_node);
		while (child_node)
		{
			if (!import_graph(sdl, root, child_node, stack))
				return false;
			child_node = xmlNextElementSibling(child_node);
		}

		glmPopMatrix(&stack);
	}
	return true;
}

static bool import_scene(Sdl *sdl, xmlNode *node, int n)
{
	Scene *scene = &sdl->scene;
	int i;
	const char *cam_name, *light_names;
	Matrix *model_matrix;

	n = n; /* UNUSED */

	/* Camera */
	if (!xmlHasProp(node, "camera"))
	{
		printf("At least one camera has to be defined\n");
		return false;
	}
	cam_name = xmlGetProp(node, "camera");
	scene->camera = NULL;
	for (i = 0; i < sdl->num_cameras; i++)
		if (strcmp(sdl->camera[i].name, cam_name) == 0)
			scene->camera = &sdl->camera[i];

	if (scene->camera == NULL)
	{
		printf("Requested camera \"%s\" not found\n", cam_name);
		return false;
	}

	/* Light(s) */
	if (!xmlHasProp(node, "lights"))
	{
		printf("A scene without lights is pretty dark...\n");
		return false;
	}
	light_names = xmlGetProp(node, "lights");
	if (!import_light_refs(sdl, light_names))
		return false;

	/* Background */
	scene->background = parse_colour(xmlGetProp(node, "background"));

	/* The actual scene */
	model_matrix = glmNewMatrixStack();
	glmLoadIdentity(model_matrix);

	/* TODO: Do all child nodes */
	if (!import_graph(sdl, &scene->root, xmlFirstElementChild(node),
			model_matrix))
	{
		printf("Error importing the scene graph\n");
		glmFreeMatrixStack(model_matrix);
		return false;
	}
	glmFreeMatrixStack(model_matrix);
	return true;

	return true;
}

static bool import_sdl(Sdl *sdl, xmlDoc *doc)
{
	xmlNode *root, *node;

	root = xmlDocGetRootElement(doc);
	if (root == NULL)
	{
		printf("No root element\n");
		return false;
	}

	for (node = xmlFirstElementChild(root); node != NULL;
			node = xmlNextElementSibling(node))
	{
		int n;
		assert(node->type == XML_ELEMENT_NODE);

		n = xmlChildElementCount(node);

		if (strcmp(node->name, "Cameras") == 0)
		{
			if (!import_cameras(sdl, node, n))
				return false;
		} else if (strcmp(node->name, "Lights") == 0)
		{
			if (!import_lights(sdl, node, n))
				return false;
		} else if (strcmp(node->name, "Geometry") == 0)
		{
			if (!import_shapes(sdl, node, n))
				return false;
		} else if (strcmp(node->name, "Textures") == 0)
		{
			if (!import_textures(sdl, node, n))
				return false;
		} else if (strcmp(node->name, "Materials") == 0)
		{
			if (!import_materials(sdl, node, n))
				return false;
		} else if (strcmp(node->name, "Scene") == 0)
		{
			if (!import_scene(sdl, node, n))
				return false;
		} else
		{
			printf("Unknown node: %s\n", node->name);
			return false;
		}
	}

	return true;
}

Sdl *sdl_load(const char *filename)
{
	Sdl *sdl = NULL;
	xmlDoc *doc = NULL;

	sdl = malloc(sizeof(Sdl));

	LIBXML_TEST_VERSION

	doc = xmlReadFile(filename, NULL, XML_PARSE_DTDLOAD | XML_PARSE_DTDVALID);
	if (doc == NULL)
	{
		printf("Error opening file %s\n", filename);
		goto errorout;
	}

	if (!import_sdl(sdl, doc))
	{
		printf("Error importing SDL from file %s\n", filename);
		goto errorout;
	}

	xmlFreeDoc(doc);
	xmlCleanupParser();
	return sdl;
errorout:
	free(sdl);
	if (doc) xmlFreeDoc(doc);
	xmlCleanupParser();
	return NULL;
}
