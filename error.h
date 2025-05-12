#ifndef ERROR_H
#define ERROR_H
#include <stdarg.h>
void report_lex_error(int line,int col,const char *fmt,...);
void report_parse_error(int line,int col,const char *fmt,...);
#endif
