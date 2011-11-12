#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "scene.h"

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

	sdl->num_cameras = n;
	sdl->camera = calloc(n, sizeof(Camera));

	for (i = 0, cur_node = xmlFirstElementChild(node); cur_node; 
			i++, cur_node = xmlNextElementSibling(cur_node))
	{
		Camera *cam = &sdl->camera[i];
		assert(strcmp(cur_node->name, "Camera") == 0);

		cam->position = parse_vec3(xmlGetProp(cur_node, "position"));
		cam->direction = parse_vec3(xmlGetProp(cur_node, "direction"));
		cam->up = parse_vec3(xmlGetProp(cur_node, "up"));
		cam->fov = parse_double(xmlGetProp(cur_node, "fovy"));
		cam->name = strdup(xmlGetProp(cur_node, "name"));
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
			shape->type = SHAPE_CONE;
			shape->u.torus.inner_radius =
					parse_double(xmlGetProp(cur_node, "innerRadius"));
			shape->u.torus.outer_radius =
					parse_double(xmlGetProp(cur_node, "outerRadius"));
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
			printf("Unimplemented material type: %s\n", cur_node->name);
			return false;
		}
	}

	return true;
}

static bool import_scene(Sdl *sdl, xmlNode *node, int n)
{
	Scene *scene = &sdl->scene;
	int i;
	const char *cam_name, *light_name;

	if (n != 1)
	{
		printf("No root nodes yet :p\n");
		return false;
	}

	/* Camera */
	if (!xmlHasProp(node, "camera"))
	{
		printf("At least one camera has to be defined\n");
		return false;
	}
	cam_name = xmlGetProp(node, "camera");
	scene->camera = NULL;
	for (i = 0; i < sdl->num_cameras; i++)
	{
		if (strcmp(sdl->camera[i].name, cam_name) == 0)
			scene->camera = &sdl->camera[i];
	}
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
	light_name = xmlGetProp(node, "lights");
	if (strchr(light_name, ',') != NULL)
	{
		printf("Only one light for now\n");
		return false;
	}
	scene->light[0] = NULL;
	for (i = 0; i < sdl->num_lights; i++)
	{
		if (strcmp(sdl->light[i].name, light_name) == 0)
			scene->light[0] = &sdl->light[i];
	}
	if (scene->light[0] == NULL)
	{
		printf("Requested light \"%s\" not found\n", light_name);
		return false;
	}

	/* Background */
	scene->background = parse_colour(xmlGetProp(node, "background"));

	/* The actual scene */
	if (xmlChildElementCount(node) != 1)
	{
		printf("Blargh\n");
		return false;
	}
	xmlNode *shape_node = xmlFirstElementChild(node);
	assert(strcmp(shape_node->name, "Shape") == 0);
	const char *shape_name = xmlGetProp(shape_node, "geometry");
	scene->graph.shape = NULL;
	for (i = 0; i < sdl->num_shapes; i++)
	{
		if (strcmp(sdl->shape[i].name, shape_name) == 0)
			scene->graph.shape = &sdl->shape[i];
	}
	if (scene->graph.shape == NULL)
	{
		printf("Requested shape \"%s\" not found\n", shape_name);
		return false;
	}
	assert(xmlHasProp(shape_node, "material"));

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
