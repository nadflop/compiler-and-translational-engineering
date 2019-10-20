#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "hash_table.h"
#include "hash_table.c"

/*basic ast node that can be either assign, write*/
Tree * new_node(NodeType node_type, Tree * left, Tree * right) {
	Tree * t = malloc(sizeof(Tree));
	t->node_type = node_type;
	t->left = left;
	t->right = right;
	t->next = NULL; 
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
Tree * new_varleaf(ht_hash_table * ht, char * key, char * name) {
	ht_item * entry = ht_get_item(ht, key, name); 
	
	Tree * t = malloc(sizeof(Tree));	
	t->node_type = VAR_REF;
	t->name = name;
	t->entry = entry;
	t->type = entry->type; 
	t->next = NULL; 

	return t;
}

Tree * new_list(NodeType node_type){
	Tree * node = malloc(sizeof(Tree)); 
	node->node_type = node_type; // for now can be either STMT_LIST, WRITE_LIST or READ_LIST
	node->left = NULL;
	node->right = NULL;
	node->next = NULL;

	return node; 
}

void ast_add_node_to_list(Tree * list, Tree * node){
	// check if list is empty
	if (list->left == NULL) {
		list->left = node; 
		list->right = node; 
		list->head = node; 
		list->tail = node;

		return; 
	}

	// at this point, list is not empty
	list->right->next = node; 
	list->right = list->right->next; 

	return;
}

void ast_print_list(Tree * list){
	Tree * curr = list->left; 
	Tree * rw; 

	// check if list is empty --> notify and return if yes
	if (curr == NULL){
		printf("List is empty\n"); 

		return; 
	}

	// at this point list has node(s) in it
	// check list type (for now either STMT_LIST or WRITE_LIST
	if (list->node_type == STMT_LIST){
		while (curr != NULL){
			if(curr->node_type == ASSIGN_NODE){
				printf("\nASSIGN_NODE with LHS: %s", curr->left->name); 
			}
			else if(curr->node_type == WRITE_LIST){
				rw = curr->left; // points to the first variable
				printf("\nWRITE_LIST: ");
				while(rw != NULL){
					printf("%s ", rw->name); 
					rw = rw->next; 
				}
				
			}
			else if (curr->node_type == READ_LIST){
				rw = curr->left; 
				printf("\nREAD_LIST: "); 
				while(rw != NULL){
					printf("%s ", rw->name); 
					rw = rw->next; 
				}
			}
			curr = curr->next; 
		}
	}


	return; 

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
		printf("Variable reference of %s with type %s\n", temp->name, temp->type);
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

	if(root->node_type == BASIC_NODE || root->node_type == ASSIGN_NODE || root->node_type == ARITHM_NODE || root->node_type == WRITE_NODE || root->node_type == FUNC_NODE) {
		ast_traversal(root->left);
		ast_traversal(root->right);
		ast_print_node(root);
	}
	else {
		ast_print_node(root);
	}
}

void deleteTree (Tree * node) {
	if (node == NULL) {
		return;
	}
	if (node->node_type != VAR_REF && node->node_type != LIT_VAL) {
		deleteTree(node->left);
		deleteTree(node->right);
	}
	free(node);

	return; 
}

/*
ListNode * new_listnode(){
	ListNode * node = (ListNode *) malloc(sizeof(ListNode)); 
	node->length = 0; 
	node->capacity = 16; 
	node->list = (Tree **) malloc(16*sizeof(Tree *)); 

	return node; 
}

void delete_listnode(ListNode * node){
	int i; 
	for(i = 0; i < node->length; i++){
		free(node->list[i]); 
	}
	free(node->list); 
	free(node); 

	return; 
}

void add_node_to_list(ListNode * list, Tree * node){
	if (list->length == list->capacity) {
		 
	}
	list->list[list->length] = node; 
	list->length++; 

	return; 
}


void print_listnode(ListNode * node){
	int i = 0; 
	printf("ListNode with %d elements\n", node->length); 
	for(i = 0; i < node->length; i++){
		printf("%s\n", node->list[i]->node_type); 
	}
}
*/


/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
int main () {
	// for now, ht item is NULL because we only want to check the functionality
	Tree * const_val1 = new_varleaf("a", "INT", NULL);
	Tree * const_val2 = new_varleaf("b", "INT", NULL);
	Tree * const_val3 = new_varleaf("c", "INT", NULL);
	Tree * mulop = new_opnode(ARITHM_NODE, MUL, const_val2, const_val3);
	Tree * addop = new_opnode(ARITHM_NODE, ADD, const_val1, mulop);
	ast_traversal(addop);
	deleteTree(addop);

	return 0;
}
*/
