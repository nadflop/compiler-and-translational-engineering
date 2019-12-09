#ifndef LIVENESS
#define LIVENESS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "hash_table.h"
#include "addgen.h"

typedef struct Node {
	char * val;
	struct Node * next;
}Node;

//need a linked list set for gen, kill, in and out

Node * createNode(char * val);
void insertNode(Node * head, char * val);
void printList(Node * head);
int count(Node * head);
void deleteList(Node * head);
Node * deleteFrontNode (Node * head);
Node * deleteBackNode (Node * head);
Node * deleteMiddleNode (Node * head, Node * nd);
void traverse_cfg(Tree * root);

#endif
