#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "hash_table.h"
#include "addgen.h"

/*basic ast node that can be either assign, write, return stmt*/
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
//ast node for comparator operations (like boolean, relational or equivalent//
Tree * new_compnode(NodeType node_type, char * comp, Tree * left, Tree * right) {
	Tree * t = malloc(sizeof(Tree));
	t->node_type = node_type;
	t->left = left;
	t->right = right;
	
	
	if (strcmp(">", comp) == 0)
		t->comp = GT;
	else if (strcmp(">=", comp) == 0)
		t->comp = GE;
	else if (strcmp("<", comp) == 0)
		t->comp = LT;
	else if (strcmp("<=", comp) == 0)
		t->comp = LE;
	else if (strcmp("!=", comp) == 0)
		t->comp = NE;
	else if (strcmp("==", comp) == 0)
		t->comp = EQ;
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

//only for STMT_LIST, WRITE_LIST, READ_LIST, IF_LIST, ELSE_LIST, WHILE_LIST, PROG_NODE, DECL_LIST, PARAM_LIST, CALL_LIST
Tree * new_list(NodeType node_type, const char * startlabel, const char * endlabel){
	Tree * node = malloc(sizeof(Tree)); 
	node->node_type = node_type;
	node->left = NULL;
	node->right = NULL;
	node->next = NULL;

	node->startlabel = startlabel;
	node->endlabel = endlabel;

	return node; 
}

void ast_add_node_to_list(Tree * list, Tree * node){
	// check if list is empty
	if (list->left == NULL) {
		list->left = node; 
		list->right = node; 
		//list->head = node; 
		//list->tail = node;
		//node->startlabel = list->startlabel;
		//node->endlabel = list->endlabel;
		return; 
	}

	// at this point, list is not empty
	list->right->next = node; 
	list->right = list->right->next; 
	list->right->next = NULL;

	/*
	Tree * curr = list->left;
	printf("\nin list: %s");
	while(curr != NULL){
		printf("<%d> ", curr->node_type);
		curr = curr->next;
	}
	printf("\n");
	*/

	return;
}

void ast_print(Tree * node){
	Tree * curr = node->left; 
	Tree * rw; 

	if(curr == NULL){
		printf("List is empty\n"); 
	}

	else if(node->node_type == COMP_NODE){
		//printf("COMP_NODE: %d\n", node->comp);	// TODO: labels for JMP
	}

	else if(node->node_type == FUNC_NODE){
		printf("%s\n", node->startlabel); // TODO: name label "LABEL FUNC_funcname"
		ast_print(curr);
	}

	else if(node->node_type == STMT_LIST ||  node->node_type == WHILE_STMT_LIST || node->node_type == IF_STMT_LIST){
		while(curr != NULL){
			ast_print(curr); 
			curr = curr->next; 
		}	
	}

	else if(node->node_type == IF_LIST){
		// curr points to bool_expr (compnode)
		printf("IF_LIST\n");
		//printf("---%s\n", node->startlabel);
		//printf("---%s\n", node->endlabel);
		while(curr != NULL){
			ast_print(curr); 
			curr = curr->next; 
		}
		//printf("---%s\n", node->endlabel);

	}

	else if(node->node_type == ELSE_LIST){
		printf("ELSE_LIST\n");
		// curr is stmt_list
		ast_print(curr); // else's stmt_list
		printf("ENDIF\n");
	}

	else if(node->node_type == WHILE_LIST){
		// curr points to bool_expr (compnode)
		printf("WHILE_LIST\n");
		//printf("---%s\n", node->startlabel); 
		//printf("---%s\n", node->endlabel);

		while(curr != NULL){
			ast_print(curr);
			curr = curr->next;
		}
		//printf("---%s\n", node->endlabel);
	}

	else if(node->node_type == ASSIGN_NODE){
		printf("ASSIGN NODE for: %s\n", node->left->name); // print lhs of assignment expression for reference 
	}

	else if(node->node_type == WRITE_LIST){
		printf("WRITE_LIST: ");
		while(curr != NULL){
			printf("%s ", curr->name);
			curr = curr->next; 
		}
		printf("\n");
	}

	else if(node->node_type == READ_LIST){
		printf("READ_LIST: "); 
		while(curr != NULL){
			printf("%s ", curr->name); 
			curr = curr->next;
		}
		printf("\n");
	}

	return; 
}

/*keeping track of the literal value for assignments*/
Tree * new_litleaf(char * literal, char * type) {
	Tree * t = calloc(sizeof(Tree),1);
	t->node_type = LIT_VAL;
	t->type = type; 
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

	if(root->node_type == BASIC_NODE || root->node_type == ASSIGN_NODE || root->node_type == ARITHM_NODE || root->node_type == COMP_NODE || root->node_type == FUNC_NODE) {
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
