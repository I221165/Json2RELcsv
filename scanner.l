%option noyywrap yylineno

%{
#include "parser.h"
#include "error.h"
#include "util.h"

int yycol = 1;
#define YY_USER_ACTION \
  for (int _i = 0; _i < yyleng; ++_i) { \
    if (yytext[_i]=='\n') yycol=1; else yycol++; \
  }

static char *make_string(const char *s, int len) {
    char *raw = strndup(s+1, len-2);
    char *un  = unescape(raw);
    free(raw);
    return un;
}
%}

%%

[ \t\r\n]+                         ;

\"([^\"\\\n]|\\.)*\"               {
                                       yylval.string = make_string(yytext, yyleng);
                                       return STRING;
                                   }

-?[0-9]+(\.[0-9]+)?([eE][+\-]?[0-9]+)? {
                                       yylval.number = strtod(yytext, NULL);
                                       return NUMBER;
                                   }

"true"                             { yylval.boolean=1; return TRUE;  }
"false"                            { yylval.boolean=0; return FALSE; }
"null"                             { return NULLVAL; }

"{"                                { return LBRACE; }
"}"                                { return RBRACE; }
"["                                { return LBRACK; }
"]"                                { return RBRACK; }
":"                                { return COLON;  }
","                                { return COMMA;  }

.                                  { report_lex_error(yylineno,yycol,"Unexpected '%s'",yytext); }

%%
