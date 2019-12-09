#include <stdio.h>
#include <stdlib.h>

#include "addgen.h"
#include "ast.h"
#include "hash_table.h"
#include "liveness.h"

extern FILE * yyout;

Node * createNode(char * val) {
	Node * head = (Node*)malloc(sizeof(Node));
	if (head == NULL) {
		return;
	}
	head->val = val;
	head->next = NULL;

	return head;
}

//add a new node at the end of the list
void insertNode(Node * head, char * val) {
	Node * curr = head;
	while(curr->next != NULL) {
		curr = curr->next;
	}

	curr->next = createNode(val);
}

void printList(Node * head) {
	Node * ptr = head;

	while (ptr != NULL) {
		printf("%s\n", ptr->val);
		ptr = ptr->next;
	}
}

int count(Node * head) {
	Node * curr = head;
	int c = 0;
	while (curr != NULL) {
		c++;
		curr = curr->next;
	}
	return c;
}
//delete the whole list
void deleteList(Node * head){
	Node * curr, *temp;
	if (head != NULL) {
		curr = head->next;
		head->next = NULL;
		while(curr != NULL)
		{
			temp = curr->next;
			free(curr);
			curr = temp;
		}
	}
}

//delete node from front of list
Node * deleteFrontNode(Node * head) {
	if (head == NULL) {
		return NULL;
	}
	Node * front =  head;
	head = head->next;
	front->next = NULL;

	if (front == head) {
		head = NULL;
	}
	free(front);
	return head;
}

//delete node from back of list
Node * deleteBackNode(Node * head) {
	if (head == NULL) {
		return NULL;
	}
	Node * curr = head;
	Node * back = NULL;

	while (curr->next != NULL) {
		back = curr;
		curr = curr->next;
	}
	if (back != NULL) {
		back->next = NULL;
	}
	if (curr == head) {
		head = NULL;
	}
	free(curr);
	return head;

}

//delete a node from the list
Node * removeMiddleNode(Node * head, Node * nd) {
	if (nd == NULL) {
		return NULL;
	}

	if (nd == head) {
		return deleteFrontNode(head);
	}

	if (nd->next == NULL) {
		return deleteBackNode(head);
	}
	Node * curr = head;
	while (curr != NULL) {
		if (curr->next == nd)
			break;
		curr = curr->next;
	}
	if (curr != NULL) {
		Node * temp = curr->next;
		curr->next = temp->next;
		temp->next = NULL;
		free(temp);
	}	
	return head;
}

// generate gen, kill, in and out sets
// Approach: traverse to the end of one route first, work backwards.
void generate_set(Tree * node){

	if(node->succ == NULL){
		// node is the last statement in the current route
		// generate use, def, in, and out sets
		printf("reached an END\n");
		return;
	}
	
	// FUNC_NODE
	else if(node->node_type == FUNC_NODE){
		// move to successor
		generate_set(node->succ); // first statement under then func_node
	}
	else if(node->node_type == ASSIGN_NODE || node->node_type == WRITE_LIST){
		
	}


	/*
	if (list == NULL)
		return;
	
	Tree * curr = list->left;
	if (curr == NULL)
		return;
	while (curr != NULL) {
		traverse_cfg(curr);
		curr = curr->next;
	}
	*/
}

void add_elem_to_node_kill(Tree * node, Tree * elem){
	
	// check if kill set is empty
	if(node->kill_head == NULL){
		node->kill_head = elem; 
		node->kill_tail = elem; 
		return;
	}

	// at this point, kill set is not empty
	Tree * curr = node->kill_head;
	
	//check if elem is in kill set, if yes, return.
	while(curr != NULL){
		if(curr->name == elem->name){
			return;
		}
		curr = curr->kill_next;
	}

	// at this point, elem is new; so add to set.
	node->kill_tail->kill_next = elem;
	node->kill_tail = node->kill_tail->kill_next;

	printf("New KILL(s): ");
	curr = node->kill_head; 
	while(curr != NULL){
		printf("%s ", curr->name);
		curr = curr->kill_next;
	}
	printf("\n");
}

void add_elem_to_node_gen(Tree * node, Tree * elem){

	// check if gen set is empty
	if(node->gen_head == NULL){
		node->gen_head = elem;
		node->gen_tail = elem;
		return;
	}

	// at this point, gen set is not empty
	Tree * curr = node->gen_head;
	
	//check if elem is in gen set, if yes, return.
	while(curr != NULL){
		if(strcmp(curr->name,elem->name) == 0){
			return;
		}
		curr = curr->gen_next;
	}

	// at this point, elem is new; so add to set.
	node->gen_tail->gen_next = elem;
	node->gen_tail = node->gen_tail->gen_next;

	/*
	printf("New GEN(s): ");
	curr = node->gen_head; 
	while(curr != NULL){
		printf("%s ", curr->name);
		curr = curr->gen_next;
	}
	printf("\n");
	*/
}

void enter_node_expression(Tree * node, Tree * expr){
	
	// recursively go into expression (post_order) and find var refs and add them to node's gen set
	if(expr->left == NULL){
		// at left/right leaf: can either be literal or var_ref
		if(expr->node_type == VAR_REF){
			//printf("leaf: %s\n", expr->name);
			add_elem_to_node_gen(node, expr);
		}
		return;
	}

	enter_node_expression(node, expr->left); 
	enter_node_expression(node, expr->right);

	return;
}

void add_elem_to_node_out(Tree * node, char * name){

	Node * newnode; // just in case

	// check if out set is empty
	if(node->out_head == NULL){
		newnode = createNode(name);
		node->out_head = newnode;
		node->out_tail = newnode;
		return;
	}

	// at this point, out set is not empty
	Node * curr = node->out_head;
	
	//check if elem is in out set, if yes, return.
	while(curr != NULL){
		if(strcmp(curr->val,name) == 0){
			return;
		}
		curr = curr->next;
	}

	// at this point, elem is new; so add to set.
	// create newnode
	newnode = createNode(name);

	node->out_tail->next = newnode;
	node->out_tail = node->out_tail->next;

	/*
	printf("New OUT(s): ");
	curr = node->out_head; 
	while(curr != NULL){
		printf("%s ", curr->val);
		curr = curr->next;
	}
	printf("\n");
	*/

}


void generate_node_out(Tree * node){
	//printf("Generating OUT(s)\n");
	
	if(node->succ == NULL){
		return;
	}

	Node * curr;

	if(node->node_type == IF_LIST || node->node_type == WHILE_LIST){
		curr = node->succ_T->in_head;
		while(curr != NULL){
			add_elem_to_node_out(node, curr->val);
			curr = curr->next; 
		}

		curr = node->succ_F->in_head;
		while(curr != NULL){
			add_elem_to_node_out(node, curr->val);
			curr = curr->next; 
		}
	}
	else{
		// only for statements with one successor (no succ_T or succ_F)
		curr = node->succ->in_head; 
		while(curr != NULL){
			add_elem_to_node_out(node, curr->val);
			curr = curr->next; 
		}
	}

}

void add_elem_to_node_in(Tree * node, char * name){

	Node * newnode;
	
	// check if out set is empty
	if(node->in_head == NULL){
		newnode = createNode(name);
		node->in_head = newnode;
		node->in_tail = newnode;
		return;
	}

	// at this point, gen set is not empty
	Node * curr = node->in_head;
	
	//check if elem is in IN set, if yes, return.
	while(curr != NULL){
		if(strcmp(curr->val,name) == 0){
			return;
		}
		curr = curr->next;
	}

	// at this point, elem is new; so add to set.
	newnode = createNode(name);
	node->in_tail->next = newnode;
	node->in_tail = node->in_tail->next;

	/*
	printf("New IN(s): ");
	curr = node->in_head; 
	while(curr != NULL){
		printf("%s ", curr->name);
		curr = curr->next;
	}
	printf("\n");
	*/

}

void add_out_to_node_in(Tree * node){

	Node * curr; 
	curr = node->out_head; 
	while(curr != NULL){
		add_elem_to_node_in(node, curr->val);
		curr = curr->next;
	}
}

void sub_kill_from_node_in(Tree * node){

	Node * curr_in; 
	Tree * curr_kill; 
	Node * temp;

	curr_kill = node->kill_head; 
	while(curr_kill != NULL){
		//printf("curr_kill: %s\n", curr_kill->name);
		// check if in_head == curr_kill; 
		if(strcmp(node->in_head->val, curr_kill->name) == 0){
			// move in_head to the next one
			temp = node->in_head; 
			node->in_head = temp->next;
			temp->next = NULL;
			if(node->in_head == NULL){
				node->in_tail = NULL;
			}
		}
		else{
			// go through in set to find and eliminate curr_kill
			curr_in = node->in_head; 
			while(curr_in != NULL){
				if(curr_in->next != NULL){
					if(strcmp(curr_in->next->val, curr_kill->name) == 0){
						if(curr_in->next == node->in_tail){
							node->in_tail = curr_in;
							curr_in->next = NULL; 
						}
						else{
							temp = curr_in->next; 
							curr_in->next = temp->next; 
							temp->next = NULL; 
						}
					}
				}
				curr_in = curr_in->next; 
			}
		}
		curr_kill = curr_kill->kill_next; 
	}

}

void add_gen_to_node_in(Tree * node){
	
	Tree * curr; 
	curr = node->gen_head; 
	while(curr != NULL){
		add_elem_to_node_in(node, curr->name); 
		curr = curr->gen_next; 
	}

}

void generate_node_in(Tree * node){

	Tree * curr_gk;
	Node * curr; 
	printf("GEN(s): ");
	curr_gk = node->gen_head; 
	while(curr_gk != NULL){
		printf("%s ", curr_gk->name);
		curr_gk = curr_gk->gen_next;
	}
	printf("\n");

	printf("KILL(s): ");
	curr_gk = node->kill_head; 
	while(curr_gk != NULL){
		printf("%s ", curr_gk->name);
		curr_gk = curr_gk->kill_next;
	}
	printf("\n");
 
	printf("OUT(s): ");
	curr = node->out_head; 
	while(curr != NULL){
		printf("%s ", curr->val);
		curr = curr->next;
	}
	printf("\n");
	
	// formula: in(s) = gen(s) U (out(s)-kill(s))
	add_out_to_node_in(node);

	/*
	printf("AFTER ADDING OUT - IN(s): ");
	curr = node->in_head; 
	while(curr != NULL){
		printf("%s ", curr->val);
		curr = curr->next;
	}
	printf("\n");
	*/

	sub_kill_from_node_in(node);

	/*
	printf("AFTER SUB KILL - IN(s): ");
	curr = node->in_head; 
	while(curr != NULL){
		printf("here?\n");
		printf("%s ", curr->val);
		curr = curr->next;
	}
	printf("\n");
	*/
	
	add_gen_to_node_in(node);
 
	printf("IN(s): ");
	curr = node->in_head; 
	while(curr != NULL){
		printf("%s ", curr->val);
		curr = curr->next;
	}
	printf("\n");

}

void generate_node_in_out_sets(Tree * node){
	printf("\nGenerating IN and OUT at node %d\n", node->node_type);
	if(node->node_type == ASSIGN_NODE){
		printf("LHS: %s\n", node->left->name);
	}

	Tree * curr;

	// calculate out set
	// check node_type for the number of successors
	if(node->node_type == IF_LIST){
		generate_node_out(node);
		generate_node_in(node);
	
	}
	else if(node->node_type == WHILE_LIST){
		generate_node_in(node);
		generate_node_out(node);		
	}
	else if(node->node_type == RETURN_STMT){
		// for a return_stmt, OUT(s) is empty, so only generate IN(s)
		generate_node_in(node);
	}
	else{
		generate_node_out(node);
		generate_node_in(node);
	}

	

}

void traverse_backwards(Tree * node){

	if(node == NULL){
		return;
	}
	
	//printf("now at node: %d\n", node->node_type);
	if(node->node_type == ASSIGN_NODE){
		//printf("lhs: %s --------\n", node->left->name);
	}
	
	/*
	if(node->node_type == IF_LIST){
		printf("Here\n\n");
		// check if all succ already has in/out sets, if not, go till the end
		generate_node_in_out_sets(node); 
		return;
	}
	*/

	 Tree * curr_pred;
	
	 curr_pred = node->pred_head;
	 if(curr_pred == NULL){
	 	//printf("Here: %d lhs - %s\n", node->node_type, node->left->name);
		generate_node_in_out_sets(node);
	 }


	while(curr_pred != NULL){
		if(curr_pred->node_type == IF_LIST){
			// IF_LIST can have three successors: succ, succ_T and succ_F
			if(curr_pred->succ == node){
				curr_pred = curr_pred->pred_next;
			}
			else{
				// at this point, node must be an IF_LIST's succ_T or succ_F
				generate_node_in_out_sets(node);
				traverse_backwards(curr_pred);
				return; 
			}
		}

		else if(curr_pred->node_type == WHILE_LIST){
			generate_node_in_out_sets(node); 
			traverse_backwards(curr_pred); 
		}

		//printf("PRED: %d\n", curr_pred->node_type);
		generate_node_in_out_sets(node);
		traverse_backwards(curr_pred);
		
	 	curr_pred = curr_pred->pred_next; 
	 }

	
}

//traverse the ast/cfg tree to compute gen,kill,in and out sets
void traverse_cfg(Tree * node) {

	Tree * curr;

	if(node->node_type == PROG_NODE){
		curr = node->left->next;
		while(curr != NULL) {
			traverse_cfg(curr);
			curr = curr->next;
		}
	}
	else if(node->node_type == FUNC_NODE){
		printf("%s ------------------------------------------\n", node->startlabel);
		curr = node->succ; // go to first statement
		while(curr != NULL){
			traverse_cfg(curr); 
			if(curr->succ == NULL){
				break;
			}
			curr = curr->succ; 
		}
		
		// at this point, curr is the last statement in func's stmt_list.
		// start generating in and out sets
		// go through curr's predecessors (pred_head through pred_tail)
		printf("\n\nstart backwards traverse\n");
		traverse_backwards(curr);
	
	}
	else if(node->node_type == ASSIGN_NODE){
		printf("ASSIGN_NODE: %s = %d\n", node->left->name, node->right->node_type);
		
		// lhs should be a var_leaf: lhs is defined, add to def set
		add_elem_to_node_kill(node, node->left);

		// rhs can be a float/int literal, a var_ref, a call_list, or an expression, check type first.
		if(node->right->node_type == VAR_REF){
			add_elem_to_node_gen(node, node->right);
		}
		else if(node->right->node_type == CALL_LIST){
			// go through call_list: can be a var_leaf, or an expression
			curr = node->right->left; // first argument in call_list
			while(curr != NULL){
				if(curr->node_type == VAR_REF){
					add_elem_to_node_gen(node, curr);
				}
				else if(curr->node_type == ARITHM_NODE){
					enter_node_expression(node, curr);
				}
				curr = curr->next;
			}
		}
		else if(node->right->node_type == ARITHM_NODE){
			// go into expression and update gen set
			enter_node_expression(node, node->right);
		}
		else{
			node->gen_head = NULL;
			node->gen_tail = NULL;
		}

	}
	else if(node->node_type == WRITE_LIST){
		printf("WRITE_LIST\n");

		// go through write_list, and add var_refs found to gen(s)
		curr = node->left; 
		while(curr != NULL){
			if(curr->node_type == VAR_REF){
				/*
				if (strcmp(curr->type, "STRING") != 0){
					add_elem_to_node_gen(node, curr); 
				}
				*/
				add_elem_to_node_gen(node, curr);
			}
			curr = curr->next; 
		}
	}
	else if(node->node_type == READ_LIST){
		printf("READ_LIST\n");

		// go through read_list, and add var_refs found to kill(s)
		curr = node->left; 
		while(curr != NULL){
			if(curr->node_type == VAR_REF){
				add_elem_to_node_kill(node, curr);
			}
			curr = curr->next;
		}
	}
	else if(node->node_type == WHILE_LIST){
		printf("WHILE_LIST\n");

		// go through while's comp_node (node->left) first
		// lhs is a var_ref, add to gen(s)
		add_elem_to_node_gen(node, node->left->left);

		// rhs can either be a literal, a var_ref or an expr (call_list also? not sure.)
		if(node->left->right->node_type == VAR_REF){
			add_elem_to_node_gen(node, node->left->right);
		}
		else if(node->left->right->node_type == ARITHM_NODE){
			// go into expression and update gen set
			enter_node_expression(node, node->left->right);
		}

		// go through while_stmt_list's statements
		curr = node->right->left; // first node in while_stmt_list
		while(curr != NULL){
			traverse_cfg(curr);
			curr = curr->next; 
		}

	}
	else if(node->node_type == IF_LIST){
		printf("IF_LIST\n"); 

		// go through if's comp_node (node->left) first
		// lhs is a var_ref, add to gen(s)
		add_elem_to_node_gen(node, node->left->left);

		// rhs can either be a literal, a var_ref or an expr (call_list also? not sure.)
		if(node->left->right->node_type == VAR_REF){
			add_elem_to_node_gen(node, node->left->right);
		}
		else if(node->left->right->node_type == ARITHM_NODE){
			// go into expression and update gen set
			enter_node_expression(node, node->left->right);
		}

		// go through if_list's if_stmt_list (succ_T) head till the end of the cfg route
		curr = node->succ_T; 
		while(curr != NULL){
			traverse_cfg(curr); 
			curr = curr->succ; 
		}

		// go through if_list's else_list (succ_F)
		curr = node->succ_F; 
		while(curr != NULL){
			traverse_cfg(curr); 
			curr = curr->succ; 
		}

	}
	else if(node->node_type == RETURN_STMT){
		printf("RETURN_STMT\n");
		
		// return argument (node->left) can either be a literal, a var_ref or an expression
		if(node->left->node_type == VAR_REF){
			add_elem_to_node_gen(node, node->left);
		}
		else if(node->left->node_type == CALL_LIST){
			// go through call_list: can be a var_leaf, or an expression
			curr = node->left->left; // first argument in call_list
			while(curr != NULL){
				if(curr->node_type == VAR_REF){
					add_elem_to_node_gen(node, curr);
				}
				else if(curr->node_type == ARITHM_NODE){
					enter_node_expression(node, curr);
				}
				curr = curr->next;
			}
		}
		else if(node->left->node_type == ARITHM_NODE){
			// go into expression and update gen set
			enter_node_expression(node, node->right);
		}

		
		// need to start generate
		printf("\n\nRET_STMT: start backwards traverse\n");
		traverse_backwards(node);

	}

}

