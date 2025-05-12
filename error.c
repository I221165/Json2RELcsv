#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static void vdie(const char* phase,int line,int col,const char *fmt,va_list ap){
    fprintf(stderr,"Error %s at %d:%d: ",phase,line,col);
    vfprintf(stderr,fmt,ap);
    fprintf(stderr,"\n");
    exit(1);
}

void report_lex_error(int line,int col,const char *fmt,...){
    va_list ap; va_start(ap,fmt);
    vdie("lexing",line,col,fmt,ap);
}

void report_parse_error(int line,int col,const char *fmt,...){
    va_list ap; va_start(ap,fmt);
    vdie("parsing",line,col,fmt,ap);
}
