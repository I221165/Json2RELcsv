#ifndef SCHEMA_H
#define SCHEMA_H
#include "ast.h"
void semantic_analyze(ASTNode *root,const char *outdir,const char *basename,int print_ast);
#endif
