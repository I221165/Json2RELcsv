#ifndef AST_H
#define AST_H

typedef enum {
    NODE_OBJECT,
    NODE_ARRAY,
    NODE_STRING,
    NODE_NUMBER,
    NODE_BOOL,
    NODE_NULL
} NodeType;

typedef struct Pair {
    char           *key;
    struct ASTNode *value;
    struct Pair    *next;
} Pair;

typedef struct ASTNodeList {
    struct ASTNode *node;
    struct ASTNodeList *next;
} ASTNodeList;

typedef struct ASTNode {
    NodeType type;
    union {
        Pair           *object;
        ASTNodeList    *array;
        char           *string;
        double          number;
        int             boolean;
    } data;
} ASTNode;

/* Constructors */
ASTNode *create_object_node(Pair *members);
ASTNode *create_array_node(ASTNodeList *elems);
ASTNode *create_string_node(const char *s);
ASTNode *create_number_node(double d);
ASTNode *create_bool_node(int b);
ASTNode *create_null_node(void);

/* Pair/list helpers */
Pair *create_pair(char *k, ASTNode *v);
Pair *append_pair(Pair *head, Pair *p);
ASTNodeList *create_nodelist(ASTNode *n);
ASTNodeList *append_nodelist(ASTNodeList *head, ASTNode *n);

/* Destructor & debug */
void free_ast(ASTNode *node);
void ast_print(ASTNode *node, int indent);

#endif
