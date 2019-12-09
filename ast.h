#ifndef AST
#define AST

#include "hash_table.h"

/*Declare different type of nodes*/
typedef enum NodeType { 
	BASIC_NODE, //0
	ASSIGN_NODE, //1
	STMT_LIST, //2
	ARITHM_NODE, //3
	WRITE_LIST,	//4
	READ_LIST,	//5
	FUNC_NODE, 	//6
	VAR_REF,	//7
	OPEN_PARENT, 	//8
	CLOSE_PARENT,	//9
	LIT_VAL,	//10
	IF_LIST, //11 when we first see an 'if' statement
	COMP_NODE,//12 the comparation happening inside the if stmt
	ELSE_LIST, //13 similar to stmt list but this is local to the else body
	WHILE_LIST, //14
	IF_STMT_LIST, //15
	WHILE_STMT_LIST, //16
	PROG_NODE,//17
	DECL_LIST,//18
	PARAM_LIST,//19
	RETURN_STMT,//20
	CALL_LIST//21
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
	
	int varcount; // number of variables in any list (call, decl, param, etc.)

	int offset; 	// the offset for AR/stack purposes (calculated from CALLEE's frame pointer)
	char * str_offset;

	int address; 	// for function call (CALL_LIST)'s return address

	struct CodeObject * tac; 
	const char * startlabel;
	const char * endlabel;

	// CFG stuffs
	struct Tree * succ;
	struct Tree * succ_T;
	struct Tree * succ_F; 

	struct Tree * pred_head;
	struct Tree * pred_next;
	struct Tree * pred_tail; 

	struct Tree * gen_head; // use
	struct Tree * gen_next;
	struct Tree * gen_tail; 

	struct Tree * kill_head; // def
	struct Tree * kill_next;
	struct Tree * kill_tail; 

	struct Node * in_head; // in set
	struct Node * in_next;
	struct Node * in_tail;

	struct Node * out_head; // out set
	struct Node * out_next;
	struct Node * out_tail; 

}Tree;

Tree * new_node(NodeType node_type, Tree * left, Tree * right);
Tree * new_opnode(NodeType node_type, enum Operator op, Tree * left, Tree * right);
Tree * new_compnode(NodeType node_type, char * comp, Tree * left, Tree * right);
Tree * new_varleaf(ht_hash_table * ht, char * key, char * name);
Tree * new_litleaf(char * literal, char * type);
Tree * new_list(NodeType node_type, const char * startlabel, const char * endlabel);
void ast_add_node_to_list(Tree * list, Tree * node); 
void ast_print(Tree * node);
void ast_print_node(Tree * node);
void ast_traversal(Tree * root);
void deleteTree (Tree * node);

#endif
