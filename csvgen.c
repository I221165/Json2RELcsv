#include "csvgen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/* Internal field and row structures */
typedef struct Field {
    char *col, *val;
    struct Field *next;
} Field;

typedef struct Row {
    int id, parent_id, seq_index;
    Field *fields;
    struct Row *next;
} Row;

typedef struct Col {
    char *name;
    struct Col *next;
} Col;

typedef struct Table {
    char *name;
    Col  *cols;
    Row  *rows;
    struct Table *next;
} Table;

struct CSVManager {
    char basedir[1024];
    Table *tables;
};

/* Helper: append a new column name to a column list */
static Col *append_col(Col *head, const char *name) {
    Col *c = calloc(1, sizeof(*c));
    c->name = strdup(name);
    if (!head) return c;
    Col *it = head;
    while (it->next) it = it->next;
    it->next = c;
    return head;
}

/* Find or create a Table by name */
static Table *find_or_create_table(CSVManager *mgr, const char *name) {
    for (Table *t = mgr->tables; t; t = t->next)
        if (!strcmp(t->name, name)) return t;
    Table *t = calloc(1, sizeof(*t));
    t->name = strdup(name);
    t->cols = NULL;
    t->rows = NULL;
    t->next = mgr->tables;
    mgr->tables = t;
    return t;
}

/* Open the manager and create output directory if needed */
CSVManager *csv_open_manager(const char *basedir) {
    CSVManager *mgr = calloc(1, sizeof(*mgr));
    if (basedir) strncpy(mgr->basedir, basedir, sizeof(mgr->basedir)-1);
    if (mgr->basedir[0] && mkdir(mgr->basedir, 0755) && errno != EEXIST) {
        perror("mkdir");
        exit(1);
    }
    return mgr;
}

/* Begin a new row, assign a unique id, and set up PK/FK/seq columns */
int csv_begin_row(CSVManager *mgr,
                  const char *tablename,
                  int parent_id,
                  int seq_index)
{
    Table *t = find_or_create_table(mgr, tablename);

    /* Create new row */
    Row *r = calloc(1, sizeof(*r));
    r->parent_id = parent_id;
    r->seq_index = seq_index;
    /* Count existing rows to assign ID = count+1 */
    int count = 0;
    for (Row *x = t->rows; x; x = x->next) count++;
    r->id = count + 1;
    r->fields = NULL;
    /* Prepend to list */
    r->next = t->rows;
    t->rows = r;

    /* Ensure columns: id */
    t->cols = append_col(t->cols, "id");
    /* Foreign key column if needed */
    if (parent_id >= 0) {
        char fk[128];
        snprintf(fk, sizeof(fk), "%s_id", tablename);
        t->cols = append_col(t->cols, fk);
    }
    /* Sequence column if needed */
    if (seq_index >= 0) {
        t->cols = append_col(t->cols, "seq");
    }

    return r->id;
}

/* Emit a single field (col:value) into the most recent row of the table */
void csv_emit_field(CSVManager *mgr,
                    const char *tablename,
                    const char *colname,
                    const char *value)
{
    Table *t = find_or_create_table(mgr, tablename);

    /* Add column to schema if it's new */
    int found = 0;
    for (Col *c = t->cols; c; c = c->next) {
        if (!strcmp(c->name, colname)) { found = 1; break; }
    }
    if (!found) t->cols = append_col(t->cols, colname);

    /* Attach field to latest row */
    Row *r = t->rows;  /* newest row at head */
    Field *f = calloc(1, sizeof(*f));
    f->col = strdup(colname);
    f->val = strdup(value);
    f->next = r->fields;
    r->fields = f;
}

/* No-op in this simple design */
void csv_end_row(CSVManager *mgr, const char *tablename) {
    (void)mgr; (void)tablename;
}

/* Helper: convert AST scalar to string */
char *scalar_to_string(ASTNode *v) {
    if (!v) return strdup("");
    switch (v->type) {
      case NODE_STRING:
        return strdup(v->data.string);
      case NODE_NUMBER: {
        char buf[64];
        snprintf(buf, sizeof(buf), "%g", v->data.number);
        return strdup(buf);
      }
      case NODE_BOOL:
        return strdup(v->data.boolean ? "true" : "false");
      case NODE_NULL:
        return strdup("");
      default:
        return strdup("");
    }
}

/* Write out a single table to CSV */
static void write_table(CSVManager *mgr, Table *t) {
    char path[2048];
    if (mgr->basedir[0])
        snprintf(path, sizeof(path), "%s/%s.csv", mgr->basedir, t->name);
    else
        snprintf(path, sizeof(path), "%s.csv", t->name);

    FILE *f = fopen(path, "w");
    if (!f) { perror(path); exit(1); }

    /* Build ordered header list */
    Col *hdr = NULL;
    for (Col *c = t->cols; c; c = c->next)
        hdr = append_col(hdr, c->name);

    /* Print header */
    for (Col *c = hdr; c; c = c->next) {
        fprintf(f, "%s", c->name);
        if (c->next) fprintf(f, ",");
    }
    fprintf(f, "\n");

    /* Print rows */
    for (Row *r = t->rows; r; r = r->next) {
        for (Col *c = hdr; c; c = c->next) {
            const char *val = "";
            /* find matching field */
            for (Field *fld = r->fields; fld; fld = fld->next) {
                if (!strcmp(fld->col, c->name)) {
                    val = fld->val;
                    break;
                }
            }
            /* id / fk / seq special columns */
            if (!strcmp(c->name, "id")) {
                fprintf(f, "%d", r->id);
            } else if (strlen(c->name) > 3 && 
                       !strcmp(c->name + strlen(c->name) - 3, "_id")) {
                fprintf(f, "%d", r->parent_id);
            } else if (!strcmp(c->name, "seq")) {
                fprintf(f, "%d", r->seq_index);
            } else {
                /* standard value: quote if needed */
                if (strchr(val, ',') || strchr(val, '\n'))
                    fprintf(f, "\"%s\"", val);
                else
                    fprintf(f, "%s", val);
            }
            if (c->next) fprintf(f, ",");
        }
        fprintf(f, "\n");
    }
    fclose(f);

    /* free temporary header list */
    for (Col *c = hdr; c; ) {
        Col *n = c->next;
        free(c->name);
        free(c);
        c = n;
    }
}

/* Write all tables, then free the manager */
void csv_write_all(CSVManager *mgr) {
    for (Table *t = mgr->tables; t; t = t->next)
        write_table(mgr, t);
}

void csv_close_manager(CSVManager *mgr) {
    /* TODO: free all allocated Tables, Rows, Fields, Cols */
    free(mgr);
}
