%option reentrant noyywrap yylineno nounput noinput

%{
#include <stdio.h>
#include "objtokens.h"
%}

D [0-9]
I [a-zA-Z][a-zA-z0-9]*

%%

#.*$   ;
[ \t]+ ;
v    return T_VERTEX;
f    return T_FACE;
g    return T_GROUP;
{I}    return T_IDENTIFIER;
[\-\+]?{D}*\.{D}+([eE][\-\+]?{D}+)? return T_FLOAT;
[\-\+]?{D}+ return T_INTEGER;
\n   ;
.    printf("Error unmatched: %s\n", yytext); return 0;
