#include <stdio.h>

#include "objtokens.h"
#include "lex.yy.h"

int main(int argc, char **argv)
{
	int tok;
	yyscan_t scanner;

	if (argc < 2)
		return 1;

	yylex_init(&scanner);
	yyset_in(fopen(argv[1], "r"), scanner);
	while ((tok = yylex(scanner)) != 0)
	{
		if (tok == T_VERTEX)
			printf("Vertex\n");
	}
	yylex_destroy(scanner);
	return 0;
}
