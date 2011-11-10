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
		if (cur->type == XML_ELEMENT_NODE)
		{
			for (int i = 0; i < n; i++)
				printf("\t");
			printf("%s\n", cur->name);
		}
		else
			continue;
		prop = cur->properties;
		while (prop)
		{
			for (int i = 0; i < n; i++)
				printf("\t");
			printf("%s: %s\n", prop->name, xmlGetProp(cur, prop->name));
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
