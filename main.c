#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "schema.h"
#include "error.h"

extern FILE *yyin;
extern int yyparse(void);
extern ASTNode *root;
extern int yydebug;

int main(int argc, char **argv) {
    int print_ast = 0;
    const char *outdir = ".";
    const char *infile = NULL;

    /* Parse command-line options */
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--print-ast")) {
            print_ast = 1;
        } else if (!strcmp(argv[i], "--out-dir")) {
            if (i+1 >= argc) {
                fprintf(stderr, "Missing directory after --out-dir\n");
                return 1;
            }
            outdir = argv[++i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option %s\n", argv[i]);
            return 1;
        } else {
            infile = argv[i];
        }
    }

    /* Open input file for Flex if provided */
    if (infile) {
        yyin = fopen(infile, "r");
        if (!yyin) {
            perror(infile);
            return 1;
        }
    }

    /* Enable Bison debug tracing */
    yydebug = 1;

    /* Parse JSON to AST */
    yyparse();
    if (!root) {
        fprintf(stderr, "No JSON parsed\n");
        return 1;
    }

    /* Derive base name from input file */
    char basename[256] = "root";
    if (infile) {
        const char *p = strrchr(infile, '/');
        strncpy(basename, p ? p+1 : infile, sizeof(basename)-1);
        char *d = strrchr(basename, '.');
        if (d) *d = '\0';
    }

    /* Semantic analysis → CSV files */
    semantic_analyze(root, outdir, basename, print_ast);

    /* Cleanup */
    free_ast(root);
    return 0;
}
