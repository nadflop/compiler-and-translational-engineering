#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"

/*basic ast node that can be either assign, write*/
Tree * new_node(NodeType node_type, Tree * left, Tree * right) {
	Tree * t = malloc(sizeof(Tree));
	t->node_type = node_type;
	t->left = left;
	t->right = right;
	return t;
}
/*ast node for arithmetic operations*/
Tree * new_opnode(NodeType node_type, enum Operator op, Tree * left, Tree * right) {
	Tree * t = malloc(sizeof(Tree));
	t->node_type = node_type;
	t->op = op;
	t->left = left;
	t->right = right;
	return t;
}

/*var reference leaf*/
Tree * new_varleaf(char * name, char * type, ht_item ** items) {
	Tree * t = malloc(sizeof(Tree));	
	t->node_type = VAR_REF;
	t->name = name;
	t->type = type;
	t->items = items;
	return t;
}

/*keeping track of the literal value for assignments*/
Tree * new_litleaf(char * literal) {
	Tree * t = calloc(sizeof(Tree),1);
	t->node_type = LIT_VAL;
	t->literal = literal;
	return t;
}

void ast_print_node(Tree * node) {
	Tree * temp = node;

	if (node->node_type == BASIC_NODE) {
		printf("Basic Node\n");
	}
	else if (node->node_type == ASSIGN_NODE) {
		printf("Assign node of entry %s\n", temp->left->name);
	}
	else if (node->node_type == ARITHM_NODE) {
		printf("Arithmetic node of operator %d\n", temp->op);
	}
	else if (node->node_type == WRITE_NODE) {
		printf("Write node of %s\n", temp->left->name);
	}
	else if (node->node_type == VAR_REF) {
		printf("Variable reference of %s\n with type %s\n", temp->name, temp->type);
	}
	else if (node->node_type == LIT_VAL) {
		printf("Literal value %s\n", temp->literal);
	}
	else {
		printf("its' empty?");
	}
}

/*checking if ast makes sense fucntion, using post-order*/
void ast_traversal(Tree * root) {
	if (root == NULL) {
		return;
	}

	if(root->node_type == BASIC_NODE || root->node_type == ASSIGN_NODE || root->node_type == ARITHM_NODE || root->node_type == WRITE_NODE) {
		ast_traversal(root->left);
		ast_traversal(root->right);
		ast_print_node(root);
	}
	else {
		ast_print_node(root);
	}
}

void deleteTree (Tree * node) {
	if (node == NULL) return;

	deleteTree(node->left);
	deleteTree(node->right);

	free(node);
}

int main () {
	/*for noe, ht item is NULL because we only want to check the functionality*/
	Tree * const_val1 = new_varleaf("a", "INT", NULL);
	Tree * const_val2 = new_varleaf("b", "INT", NULL);
	Tree * const_val3 = new_varleaf("c", "INT", NULL);
	Tree * mulop = new_opnode(ARITHM_NODE, MUL, const_val2, const_val3);
	Tree * addop = new_opnode(ARITHM_NODE, ADD, const_val1, mulop);
	ast_traversal(addop);
	deleteTree(addop);
	return 0;
}
