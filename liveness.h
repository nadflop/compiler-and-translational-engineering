#ifndef LIVENESS
#define LIVENESS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "hash_table.h"
#include "addgen.h"

typedef struct node {
	char * val;
	struct node * next;
}node;

//need a linked list set for gen, kill, in and out

node * createNode(char * val);
void insertNode(node * head, char * val);
void printList(node * head);
int count(node * head);
void deleteList(node * head);
node * deleteFrontNode (node * head);
node * deleteBackNode (node * head);
node * deleteMiddleNode (node * head, node * nd);
void traverse_cfg(Tree * root);

#endif
