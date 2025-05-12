%define parse.trace

%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "error.h"

extern int yylex(void);
extern int yylineno, yycol;
ASTNode *root = NULL;
%}

/* Make AST types visible */
%code requires { #include "ast.h" }

%union {
  char           *string;
  double          number;
  int             boolean;
  ASTNode        *ast;
  Pair           *pair;
  ASTNodeList    *nodelist;
}

%token LBRACE RBRACE LBRACK RBRACK COMMA COLON
%token <string>  STRING
%token <number>  NUMBER
%token <boolean> TRUE FALSE
%token           NULLVAL

%type <ast>      json value object array
%type <pair>     pair
%type <nodelist> members elements

%%

json:
    value             { root = $1; }
  ;

value:
    object            { $$ = $1; }
  | array             { $$ = $1; }
  | STRING            { $$ = create_string_node($1); free($1); }
  | NUMBER            { $$ = create_number_node($1); }
  | TRUE              { $$ = create_bool_node(1); }
  | FALSE             { $$ = create_bool_node(0); }
  | NULLVAL           { $$ = create_null_node(); }
  ;

object:
    LBRACE RBRACE               { $$ = create_object_node(NULL); }
  | LBRACE members RBRACE       { $$ = create_object_node($2); }
  ;

members:
    pair                         { $$ = $1; }
  | members COMMA pair           { $$ = append_pair($1,$3); }
  ;

pair:
    STRING COLON value           { $$ = create_pair($1,$3); free($1); }
  ;

array:
    LBRACK RBRACK                { $$ = create_array_node(NULL); }
  | LBRACK elements RBRACK       { $$ = create_array_node($2); }
  ;

elements:
    value                        { $$ = create_nodelist($1); }
  | elements COMMA value         { $$ = append_nodelist($1,$3); }
  ;

%%

void yyerror(const char *msg) {
    report_parse_error(yylineno, yycol, msg);
}
