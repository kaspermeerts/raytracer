#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

xmlDoc *doc = NULL;

static void print_names(int n, xmlNode *node)
{
	xmlNode *cur;
	xmlAttr *prop;

	for (cur = node; cur; cur = cur->next)
	{
		printf("%d: %s: %s\n", n, cur->name, cur->content);
		prop = cur->properties;
		while (prop)
		{
			printf("%s\n", prop->name);
			if (prop->children)
			{
				printf("%d\n", prop->children->type);
				printf("%s\n", prop->children->content);
				printf("%p\n", prop->children->next);
			}
			prop = prop->next;
		}


		print_names(n + 1, cur->children);
	}
}

int main(int argc, char **argv)
{
	xmlNode *root = NULL;

	if (argc != 2)
		return 1;

	LIBXML_TEST_VERSION

	doc = xmlReadFile(argv[1], NULL, XML_PARSE_DTDVALID);
	if (doc == NULL)
		printf("Error opening file %s\n", argv[1]);

	root = xmlDocGetRootElement(doc);

	print_names(0, root);

	xmlFreeDoc(doc);

	xmlCleanupParser();

	return 0;
}
