#ifndef AST
#define AST

#include "hash_table.h"

/*Declare different type of nodes*/
typedef enum NodeType { 
	BASIC_NODE, 
	ASSIGN_NODE, 
	STMT_LIST, 
	ARITHM_NODE, 
	WRITE_LIST,
	READ_LIST,
	FUNC_NODE, 
	VAR_REF,
	OPEN_PARENT, 
	CLOSE_PARENT,
	LIT_VAL,
	IF_LIST, //when we first see an 'if' statement
	COMP_NODE,//the comparation happening inside the if stmt
	ELSE_LIST, //similar to stmt list but this is local to the else body
	WHILE_LIST
}NodeType;

typedef enum Operator{
	ADD,	/* + operator */
	SUB, 	/* - operator */
	MUL,  	/* x operator */
	DIV		/* / operator */

}Operator;

typedef enum Comparator{
	GT, // >
	GE, // >=
	LT, // <
	LE, // <=
	NE, // !=
	EQ  // ==
}Comparator;

/*structure of the tree*/
typedef struct Tree {
    enum NodeType node_type; // every node has a nodetype
	
	/*if it's a node, it only has two attributes*/
	struct Tree * left;/*left child*/
	struct Tree * right;/*right child*/

	struct Tree * next; // only for list nodes
	struct Tree * head; 
	struct Tree * tail; 

	/*only for arithmetic nodes*/
	enum Operator op;

	//only for comparator node
	enum Comparator comp;
	
	/*if it's a leaf, it needs a name, type and reference to symbtab entry*/
	char * name;
	char * type;
	char * literal; /*intliteral or floatliteral*/
	ht_item * entry; /*pointer to symbtab entry that has the var*/

	struct CodeObject * tac; 
	char * startlabel;
	char * endlabel;

}Tree;

Tree * new_node(NodeType node_type, Tree * left, Tree * right);
Tree * new_opnode(NodeType node_type, enum Operator op, Tree * left, Tree * right);
Tree * new_compnode(NodeType node_type, char * comp, Tree * left, Tree * right);
Tree * new_varleaf(ht_hash_table * ht, char * key, char * name);
Tree * new_litleaf(char * literal, char * type);
Tree * new_list(NodeType node_type, char * startlabel, char * endlabel);
void ast_add_node_to_list(Tree * list, Tree * node); 
void ast_print(Tree * node);
void ast_print_node(Tree * node);
void ast_traversal(Tree * root);
void deleteTree (Tree * node);

#endif
