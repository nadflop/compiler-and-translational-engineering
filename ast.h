#ifndef AST
#define AST

#include "hash_table.h"

/*Declare different type of nodes*/
typedef enum NodeType { 
	BASIC_NODE,
	ASSIGN_NODE,
	ARITHM_NODE,
	WRITE_NODE,
	VAR_REF,
	LIT_VAL
}NodeType;

typedef enum Operator{
	ADD, /* + operator*/
	SUB, /* - operator*/
	MUL /* x operator*/
}Operator;

/*structure of the tree*/
typedef struct Tree {
    enum NodeType node_type;
	/*if it's a node, it only has two attributes*/
	struct Tree * left;/*left child*/
	struct Tree * right;/*right child*/
	/*only for arithmetic nodes*/
	enum Operator op;
	/*if it's a leaf, it needs a name, type and reference to symbtab entry*/
	char * name;
	char * type;
	char * literal; /*intliteral or floatliteral*/
	struct ht_item ** items; /*pointer to symbtab entry that has the var*/ 
}Tree;

Tree * new_node(NodeType node_type, Tree * left, Tree * right);
Tree * new_opnode(NodeType node_type, enum Operator op, Tree * left, Tree * right);
Tree * new_varleaf(char * name, char * type, ht_item ** items);
Tree * new_litleaf(char * literal);
void ast_print_node(Tree * node);
void ast_traversal(Tree * root);
void deleteTree (Tree * node);

#endif
