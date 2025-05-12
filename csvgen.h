#ifndef CSVGEN_H
#define CSVGEN_H
#include "ast.h"

typedef struct CSVManager CSVManager;
CSVManager *csv_open_manager(const char *basedir);
int         csv_begin_row(CSVManager*, const char*, int parent_id,int seq_index);
void        csv_emit_field(CSVManager*,const char*,const char*,const char*);
void        csv_end_row(CSVManager*,const char*);
void        csv_write_all(CSVManager*);
void        csv_close_manager(CSVManager*);
char       *scalar_to_string(ASTNode *v);

#endif
