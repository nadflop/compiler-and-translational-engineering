#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "hash_table.h"
#include "addgen.h"

extern int yylineno;

Tree * parent_head;
Tree * parent_tail; 

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
	if (entry == NULL)
	{
		entry = ht_get_item(ht, "GLOBAL", name);
		if (entry == NULL){
			// variable is in a BLOCK scope -- need to search in previous scope that is not a BLOCK scope.
			return NULL;
		}
	}

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

	node->address = yylineno;

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

	Tree * curr; 
	Tree * rw;
	
	if(node == NULL){
		return;
	}

	else if(node->node_type == PROG_NODE){
		printf("PROGRAM: %s\n", node->startlabel); 
		curr = node->left; 
		while(curr != NULL){
			ast_print(curr); 
			curr = curr->next;
		}
	}

	else if(node->node_type == VAR_REF){
		printf("VAR_REF: %s\n", node->name);
	}

	else if(node->node_type == DECL_LIST){
		printf("DECL_LIST of %d variables\n", node->varcount);
		curr = node->left; 
		while(curr != NULL){
			ast_print(curr);
			curr = curr->next; 
		}
	}

	else if(node->node_type == FUNC_NODE){
		printf("%s\n", node->startlabel); // LABEL FUNC_<funcname>
		curr = node->left; 
		while(curr != NULL){
			ast_print(curr);
			curr = curr->next;
		}
	}

	else if(node->node_type == PARAM_LIST){
		printf("PARAM_LIST with %d parameters\n", node->varcount);
		curr = node->left; 
		while(curr != NULL){
			ast_print(curr);
			curr = curr->next;
		}
	}

	else if(node->node_type == STMT_LIST){
		printf("STMT_LIST\n");
		curr = node->left; 
		while(curr != NULL){
			ast_print(curr);
			curr = curr->next; 
		} 
	}

	else if(node->node_type == WHILE_STMT_LIST){
		printf("WHILE_STMT_LIST\n");
		curr = node->left; 
		while(curr != NULL){
			ast_print(curr); 
			curr = curr->next;
		}
	}

	else if(node->node_type == IF_STMT_LIST){
		printf("IF_STMT_LIST\n");
		curr = node->left; 
		while(curr != NULL){
			ast_print(curr); 
			curr = curr->next;
		}
	}

	else if(node->node_type == ASSIGN_NODE){
		printf("ASSIGN NODE for: %s\n", node->left->name); // print lhs of assignment expression for reference 
		if(node->right->node_type == CALL_LIST){
			ast_print(node->right);
		}
	}

	else if(node->node_type == CALL_LIST){
		printf("CALL_LIST with %d arguments at line %d\n", node->varcount, node->address); 
		curr = node->left; 
		while(curr != NULL){
			if(curr->node_type == ARITHM_NODE){
				printf("EXPRESSION\n");
			}
			else if(curr->node_type == LIT_VAL){
				printf("LITERAL: %s\n", curr->literal);
			}
			else if(curr->node_type == VAR_REF){
				printf("%s [%d]\n", curr->name, curr->entry->offset);
			}
			curr = curr->next; 
		}
	}

	else if(node->node_type == COMP_NODE){
		printf("COMP_NODE: %d\n", node->comp);	
	}

	else if(node->node_type == IF_LIST){
		printf("IF_LIST\n");
		curr = node->left;
		// curr points to bool_expr (compnode)
		while(curr != NULL){
			ast_print(curr); 
			curr = curr->next; 
		}
	}

	else if(node->node_type == ELSE_LIST){
		printf("ELSE_LIST\n");
		curr = node->left;
		// curr is stmt_list
		ast_print(curr); // else's stmt_list
		printf("ENDIF\n");
	}

	else if(node->node_type == WHILE_LIST){
		// curr points to bool_expr (compnode)
		printf("WHILE_LIST\n");
		//printf("---%s\n", node->startlabel); 
		//printf("---%s\n", node->endlabel);
		curr = node->left;
		while(curr != NULL){
			ast_print(curr);
			curr = curr->next;
		}
		printf("ENDWHILE\n");
		//printf("---%s\n", node->endlabel);
	}

	else if(node->node_type == WRITE_LIST){
		printf("WRITE_LIST: ");
		curr = node->left;
		while(curr != NULL){
			printf("%s ", curr->name);
			curr = curr->next; 
		}
		printf("\n");
	}

	else if(node->node_type == READ_LIST){
		printf("READ_LIST: "); 
		curr = node->left;
		while(curr != NULL){
			printf("%s ", curr->name); 
			curr = curr->next;
		}
		printf("\n");
	}

	else if (node->node_type == RETURN_STMT){
		printf("RETURN_STMT to offset [%d]\n", node->offset);
		if(node->left->node_type == ARITHM_NODE){
			printf("RETURN EXPRESSION\n");
		}
		else if(node->left->node_type == LIT_VAL){
			printf("RETURN LITERAL: %s\n", node->left->literal);
		}
		else{
			ast_print(node->left); // should be a var_ref
		}
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


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CFG

void cfg_update_node_pred(Tree * node, Tree * pred){
	// check if node has any pred
	if(node->pred_head == NULL){
		node->pred_head = pred; 
		node->pred_tail = pred; 
		return; 
	}

	// at this point, node has at least one pred
	node->pred_tail->pred_next = pred; 
	node->pred_tail = node->pred_tail->pred_next; 
}

void ast_build_cfg(Tree * node){
	
	Tree * curr;

	if(node->node_type == PROG_NODE){
		curr = node->left->next; // first func_node in ast
		while(curr != NULL){
			ast_build_cfg(curr);
			curr = curr->next;
		}
	}
	else if(node->node_type == FUNC_NODE){
		node->succ = node->right->left; // first statement in stmt_list
		ast_build_cfg(node->right);		// go to stmt_list
	}
	else if(node->node_type == STMT_LIST || node->node_type == IF_STMT_LIST){
		curr = node->left; 	// first statement in stmt_list
		while(curr != NULL){

			if(curr->node_type == IF_LIST){
				curr->succ_T = curr->left->next->left; 	// first statement in if_stmt_list
				cfg_update_node_pred(curr->succ_T, curr);	// update if_list to be if_stmt_list's predecessor				
				curr->succ_F = curr->right->left->left; // first statement under else_list's stmt_list
				cfg_update_node_pred(curr->succ_F, curr); 	// update if_list to be else_list's stmt_list predecessor

				ast_build_cfg(curr->left->next);	// if_stmt_list
				ast_build_cfg(curr->right->left); 	// stmt_list under else_list
			}
			else if(curr->node_type == WHILE_LIST){
				curr->succ_T = curr->left->next;	// while_stmt_list
				curr->succ_F = curr->next; 			// next stmt after while_list
			}

			curr->succ = curr->next; 
			cfg_update_node_pred(curr->succ, curr); 
			curr = curr->next; 
		}
		// at this point, curr reached the end of stmt_list
		
	}
}

void ast_cfg_print(Tree * node){
	if(node == NULL){
		return; 
	}

	Tree * curr;

	if(node->node_type == PROG_NODE){
		curr = node->left->next; // first func_node
		while(curr != NULL){
			ast_cfg_print(curr); 
			curr = curr->next;
		}
	}	
	else if(node->node_type == FUNC_NODE){
		printf("START: %s\n", node->startlabel);
		ast_cfg_print(node->succ);
		printf("END\n"); 
	}
	else if(node->node_type == ASSIGN_NODE){
		printf("ASSIGN_NODE: %s\n", node->left->name); 
		ast_cfg_print(node->succ);
	}
	else if(node->node_type == WRITE_LIST){
		printf("WRITE_LIST: ");
		curr = node->left;
		while(curr != NULL){
			printf("%s ", curr->name);
			curr = curr->next; 
		}
		printf("\n");
		ast_cfg_print(node->succ);
	}
	else if(node->node_type == READ_LIST){
		printf("READ_LIST: "); 
		curr = node->left;
		while(curr != NULL){
			printf("%s ", curr->name); 
			curr = curr->next;
		}
		printf("\n");
		ast_cfg_print(node->succ);
	}
	else if(node->node_type == IF_LIST){
		printf("IF_LIST!\n");
		ast_cfg_print(node->succ);
	}
	else if(node->node_type == WHILE_LIST){
		printf("WHILE_LIST!\n");  
		ast_cfg_print(node->succ);
	}
	else if(node->node_type == RETURN_STMT){
		printf("RETURN_STMT\n");
		ast_cfg_print(node->succ);
	}
}


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
