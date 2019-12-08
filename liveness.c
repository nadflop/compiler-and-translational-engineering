#include <stdio.h>
#include <stdlib.h>

#include "addgen.h"
#include "ast.h"
#include "hash_table.h"
#include "liveness.h"

extern FILE * yyout;

node createNode(char * val) {
	node * head = (node*)malloc(sizeof(node));
	if (head == NULL) {
		return;
	}
	head->val = val;
	head->next = NULL;

	return head;
}

//add a new node at the end of the list
void insertNode(node * head, char * val) {
	node * curr = head;
	while(curr->next != NULL) {
		curr = curr->next;
	}

	curr->next = createNode(val);
}

void printList(node * head) {
	node * ptr = head;

	while (ptr != NULL) {
		printf("%s\n", ptr->val);
		ptr = ptr->next;
	}
}

int count(node * head) {
	node * curr = head;
	int c = 0;
	while (curr != NULL) {
		c++;
		curr = curr->next;
	}
	return c;
}
//delete the whole list
void deleteList(node * head){
	node * curr, *temp;
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
node * deleteFrontNode(node * head) {
	if (head == NULL) {
		return NULL;
	}
	node * front =  head;
	head = head->next;
	front->next = NULL;

	if (front == head) {
		head = NULL;
	}
	free(front);
	return head;
}

//delete node from back of list
node * deleteBackNode(node * head) {
	if (head == NULL) {
		return NULL;
	}
	node * curr = head;
	node * back = NULL;

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
node * removeMiddleNode(node * head, node * nd) {
	if (nd == NULL) {
		return NULL;
	}

	if (nd == head) {
		return deleteFrontNode(head);
	}

	if (nd->next == NULL) {
		return deleteBackNode(head);
	}

	node * curr = head;
	while (curr != NULL) {
		if (curr->next == nd)
			break;
		curr = curr->next;
	}

	if (curr != NULL) {
		node * temp = curr->next;
		curr->next = temp->next;
		tmp->next = NULL;
		free(temp);
	}
	
	return head;
}

void generate_set(Tree * list){
	if (list == NULL)
		return;
	
	Tree * curr = list->left;
	if (curr == NULL)
		return;
	while (curr != NULL) {
		traverse_cfg(curr);
		curr = curr->next;
	}
}

//traverse the ast/cfg tree to compute gen,kill,in and out sets
void traverse_cfg(Tree * root) {
	if (root == NULL)
		return;
	
	Tree * curr;

	if (root->node_type == PROG_NODE) {
		curr = node->left;
		while(curr != NULL) {
			traverse_cfg(curr);
			curr = curr->next;
		}
	}
	if (root->node_type == FUNC_NODE || root->node_type == ASSIGN_NODE) {
			generate_set(root);
			traverse_cfg(root->succ);
	}
}

