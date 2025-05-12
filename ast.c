#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ASTNode *create_object_node(Pair *members) {
    ASTNode *n = calloc(1, sizeof(*n));
    n->type = NODE_OBJECT;
    n->data.object = members;
    return n;
}

ASTNode *create_array_node(ASTNodeList *elems) {
    ASTNode *n = calloc(1, sizeof(*n));
    n->type = NODE_ARRAY;
    n->data.array = elems;
    return n;
}

ASTNode *create_string_node(const char *s) {
    ASTNode *n = calloc(1, sizeof(*n));
    n->type = NODE_STRING;
    n->data.string = strdup(s);
    return n;
}

ASTNode *create_number_node(double d) {
    ASTNode *n = calloc(1, sizeof(*n));
    n->type = NODE_NUMBER;
    n->data.number = d;
    return n;
}

ASTNode *create_bool_node(int b) {
    ASTNode *n = calloc(1, sizeof(*n));
    n->type = NODE_BOOL;
    n->data.boolean = b;
    return n;
}

ASTNode *create_null_node(void) {
    ASTNode *n = calloc(1, sizeof(*n));
    n->type = NODE_NULL;
    return n;
}

Pair *create_pair(char *k, ASTNode *v) {
    Pair *p = calloc(1, sizeof(*p));
    p->key = strdup(k);
    p->value = v;
    return p;
}

Pair *append_pair(Pair *head, Pair *p) {
    if (!head) return p;
    Pair *it = head;
    while (it->next) it = it->next;
    it->next = p;
    return head;
}

ASTNodeList *create_nodelist(ASTNode *n) {
    ASTNodeList *l = calloc(1, sizeof(*l));
    l->node = n;
    return l;
}

ASTNodeList *append_nodelist(ASTNodeList *head, ASTNode *n) {
    if (!head) return create_nodelist(n);
    ASTNodeList *it = head;
    while (it->next) it = it->next;
    it->next = create_nodelist(n);
    return head;
}

void free_ast(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
      case NODE_OBJECT:
        for (Pair *p = node->data.object; p; ) {
            Pair *nx = p->next;
            free(p->key);
            free_ast(p->value);
            free(p);
            p = nx;
        }
        break;
      case NODE_ARRAY:
        for (ASTNodeList *l = node->data.array; l; ) {
            ASTNodeList *nx = l->next;
            free_ast(l->node);
            free(l);
            l = nx;
        }
        break;
      case NODE_STRING:
        free(node->data.string);
        break;
      default:
        break;
    }
    free(node);
}

static void indent_print(int indent) {
    for (int i = 0; i < indent; i++) putchar(' ');
}

void ast_print(ASTNode *node, int indent) {
    if (!node) return;
    indent_print(indent);
    switch (node->type) {
      case NODE_OBJECT:
        printf("Object {\n");
        for (Pair *p = node->data.object; p; p = p->next) {
            indent_print(indent+2);
            printf("%s:\n", p->key);
            ast_print(p->value, indent+4);
        }
        indent_print(indent);
        printf("}\n");
        break;
      case NODE_ARRAY:
        printf("Array [\n");
        for (ASTNodeList *l = node->data.array; l; l = l->next) {
            ast_print(l->node, indent+2);
        }
        indent_print(indent);
        printf("]\n");
        break;
      case NODE_STRING:
        printf("String \"%s\"\n", node->data.string);
        break;
      case NODE_NUMBER:
        printf("Number %g\n", node->data.number);
        break;
      case NODE_BOOL:
        printf("Boolean %s\n", node->data.boolean?"true":"false");
        break;
      case NODE_NULL:
        printf("Null\n");
        break;
    }
}
