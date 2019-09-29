#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbtab.h"

struct ht *createHashTable(int size) {
	int i;
	struct ht *hash_table = malloc(size * sizeof(struct ht));
	for(i = 0; i < size; i++) {
		hash_table->entry[i] = NULL;
	}
	hash_table->size = size;

	return hash_table;
}

unsigned int createHashCode(char * key) {
	//using djb2 string hash function
	unsigned int hash = 5381;
	char *temp = key;
	
	for(; *temp != '\0'; temp++) {
		hash = (hash << 5) + hash + *temp;
	}

	return hash;
}

struct node * createNode(char *name, char *type) {
	struct node * newNode = (node *) malloc(sizeof(struct node));
	strcpy(newNode->name, name, sizeof(newNode->name));
	strcpy(newNode->type, type, sizeof(newNode->type));
	//find a way to update value easily
	newNode->next = NULL;

	return newNode;
}

//function to insert a new entry inside the hash table
void updateHT(struct ht *hash_table, char *name, char *type) {
	unsigned int bin = createHashCode(name);
	//create a new node based on the info given
	struct node *tempNode = createNode(name, type);		
	//check if hash table for that index is NULL or not
	if (!hash_table[bin]->entry) {
		hash_table[bin]->entry = tempNode;
		hash_table[bin].size = 1;
		return;
	}

	tempNode->next = hash_table[bin]->entry;
	hash_table[bin]->entry = tempNode;
	hash_table[bin].size++;
	return;
}
//not sure if we need delete ht in our case?
//since if there is a duplicate entry in the same scope, we will just throw an error?
/*void deleteHT(struct ht *hash_table, char * key) {

}*/

//return symbol if found, NULL if not found
void searchHT(struct ht * hash_table, char * name) { 
	unsigned int bin = createHashCode(name);
	//step through the hastable entry to look for our value
	struct node *temp = hash_table[bin];
	while (temp != NULL) {
		//strcmp compares two strings if its identical
		//returns 0 if yes
		if(strcmp(name, temp->name) == 0) {
			return temp;
		}
		temp = temp->next; //traverse to the next element
	}
	return NULL;
}

