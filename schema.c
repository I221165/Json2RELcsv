#include "schema.h"
#include "csvgen.h"
#include <stdio.h>
#include <stdlib.h>

static void walk(ASTNode *node,const char *tbl,int parent,int seq,CSVManager *m){
    if(!node) return;
    if(node->type==NODE_OBJECT){
        int id = csv_begin_row(m,tbl,parent,seq);
        for(Pair*p=node->data.object;p;p=p->next){
            ASTNode *v = p->value;
            if(v->type==NODE_OBJECT||v->type==NODE_ARRAY)
                walk(v,p->key,id,-1,m);
            else {
                char *s = scalar_to_string(v);
                csv_emit_field(m,tbl,p->key,s);
                free(s);
            }
        }
    } else if(node->type==NODE_ARRAY){
        int idx=0;
        for(ASTNodeList*l=node->data.array;l;l=l->next,idx++){
            ASTNode*v=l->node;
            if(v->type==NODE_OBJECT||v->type==NODE_ARRAY)
                walk(v,tbl,parent,idx,m);
            else {
                int rid = csv_begin_row(m,tbl,parent,-1);
                char buf[32]; snprintf(buf,sizeof(buf),"%d",idx);
                csv_emit_field(m,tbl,"index",buf);
                char*s = scalar_to_string(v);
                csv_emit_field(m,tbl,"value",s);
                free(s);
            }
        }
    }
}

void semantic_analyze(ASTNode *root,const char *outdir,const char *basename,int print_ast){
    if(print_ast) ast_print(root,0);
    CSVManager *mgr = csv_open_manager(outdir);
    walk(root,basename,-1,-1,mgr);
    csv_write_all(mgr);
    csv_close_manager(mgr);
}
