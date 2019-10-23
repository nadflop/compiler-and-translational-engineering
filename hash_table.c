#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

/*function to initialize an item in the hash table*/
ht_item * ht_new_item(const char * k, const char * n, const char * type, const char * strval){
	ht_item * i = malloc(sizeof(ht_item));
	i->key = strdup(k);
	if (n == NULL) {
		i->name = NULL; 
	}
	else {
		i->name = strdup(n);
	}

	if (type == NULL) {
		i->type = NULL; 
	}
	else {
		i->type = strdup(type);
	}

	if (strval == NULL){
		i->strval = NULL;  
	}
	else {
		i->strval = strdup(strval); 
	}
	i->next = NULL;
	return i;
}

/*function to initialize a hash table*/
ht_hash_table * ht_new() {
	ht_hash_table * ht = malloc(sizeof(ht_hash_table));
	if (ht == NULL) {
		return NULL;
	}
	ht->size = 100;
	/*calloc fills allocated memory with NULL bytes*/
	ht->items = calloc((size_t)ht->size, sizeof(ht_item*));
	if (ht->items == NULL) {
		return NULL;
	}
	return ht;
}

/*delete the whole hash table*/
void ht_del_hash_table(ht_hash_table * ht) {
	ht_item * temp;
	int i;

	if (ht == NULL) {
		return;
	}
	
	for (i = 0; i < ht->size; ++i) {
		if(ht->items[i] != NULL) {
			/*go to the head of the node*/
			/*traverse the list and free the nodes*/
			while (ht->items[i] != NULL) {
				temp = ht->items[i]->next;
				/*temp->next = item->next;
				ht_del_item(item);
				item = temp;*/
				free(ht->items[i]->key);
				free(ht->items[i]->name); 
				free(ht->items[i]->type);
				free(ht->items[i]->strval);
				free(ht->items[i]);
				ht->items[i] = temp;
			}
			free(ht->items[i]);
		}
	}
	free(ht->items);
	free(ht);

	//printf("\n\nHash Table Freed.\n"); 
}

/*function to calculate the hash value based on the string*/
unsigned int ht_hash(const char * key, const int size) {
	unsigned int hash;
	/*const int len_s = strlen(key);*/
	int i;

	hash = 0;
	i = 0;
	while (key && key[i]) {
		hash = (hash + key[i]) % size;
		++i;
	}
	return hash;
}

/*insert a new value into the hash table*/
/*
void ht_insert(ht_hash_table * ht, const char * key, const char* name, const char* type, const char * strval) {
	ht_item * item = ht_new_item(key, name, type, strval);
	int index = ht_hash(item->key, ht->size);
	ht_item * cur_item = ht->items[index];

	//check if there is already a hash table with the hash index
	if (ht->items[index] != NULL) {
		cur_item = ht->items[index];
		//if the item is not empty
		while (cur_item != NULL) {
			if (strcmp(cur_item->name, item->name) == 0) {
				//there's already a declaration of the item in the hash table
				printf("Error");
				break;
			}
			cur_item = cur_item->next;
		}
		//cur_item must now points at the end of the list
		if (cur_item == NULL) {
			item->next = ht->items[index];
			ht->items[index] = item;
		}
	}
	//just add the item into the hash_table
	else {
		item->next = NULL;
		ht->items[index] = item;
	}
}
*/

void ht_insert(ht_hash_table * ht, const char * key, const char* name, const char* type, const char * strval) {	
	ht_item * item = ht_new_item(key, name, type, strval); 
	int index = ht_hash(item->key, ht->size);
	ht_item * cur_item = ht->items[index]; 

	//printf("Function Enter: %s, %s, %s, %s\n", key, name, type, strval); 

	if (ht->items[index] != NULL) {
		//printf("Key Found\n");  
		if (cur_item->name == NULL) {
			// Key exists, but no entry; 
			cur_item->name = item->name; 
			cur_item->type = item->type; 
			cur_item->strval = item->strval; 
			cur_item->next = NULL; 
		}
		else if (strcmp(cur_item->name, name) == 0){
			printf("DECLARATION ERROR %s\n", item->name); 
			free(item->key);
			free(item->name);
			free(item->type);
			free(item->strval);
			free(item->next); 
			free(item);
			ht_del_hash_table(ht); 
			exit(0); 
		}
		else {
			while(cur_item->next != NULL){ 
				if (strcmp(cur_item->next->name, item->name) == 0){
					printf("DECLARATION ERROR %s\n", item->name);
					free(item->key);
					free(item->name);
					free(item->type);
					free(item->strval);
					free(item->next); 
					free(item);
					ht_del_hash_table(ht); 
					exit(0);  
				}
				//printf("Now at: %s\n", cur_item->name); 
				cur_item = cur_item->next; 
			}
			cur_item->next = item;
		}
	}
	else {
		ht->items[index] = item; 
	}

	//printf("Adding new item\n"); 

	return; 

}


/*find the contents of a hash table?*/
// check if key exists?
int ht_search(ht_hash_table * ht, const char * key, const char * name){
	int index = ht_hash(key, ht->size);
	ht_item * iptr = ht->items[index];
	
	if (iptr == NULL) {
		printf("key doesn't exist!\n");
		return -1; 
	}
	while (iptr != NULL) { 
		if (strcmp(iptr->name, name) == 0) {
			printf("entry already exists!"); 
			return 1; 
		}
		iptr = iptr->next;
	}
	// iptr has traversed all the way to the end of entry list, now pointing to NULL.
	printf("Key Exists, entry has not been declared\n");
	return 0; 
}


// get an entry (pointer form) from the hash table
ht_item * ht_get_item(ht_hash_table * ht, const char * key, const char * name){
	int index = ht_hash(key, ht->size); 
	ht_item * item = ht->items[index]; 

	while (item != NULL){ 
		if (strcmp(item->name, name) == 0){
			// KEY FOUND
			break; 
		}
		item = item->next; 
	}
	
	return item; 
}

/*
int main() {
	// To test hash table separately from compiler
	ht_hash_table * ht = ht_new();
	ht_insert(ht, "Main", "Cendol", "INT", "NULL");
	ht_insert(ht, "Main", "Hello", "VOID", "NULL");
	ht_insert(ht, "Main", "~~~", "FLOAT", "NULL");
	ht_insert(ht, "Main", "Haha", "STRING", "NULL");

	ht_insert(ht, "F1", NULL, "INT", "NULL"); 
	printf("Here\n"); 
 
	ht_item * item = ht_get_item(ht, "Main", "Haha");
	if (item == NULL){
		printf("Item does not exist!\n"); 
	}
	else {
		printf("%s, %s, %s, %s\n", item->key, item->name, item->type, item->strval); 
	}

	ht_del_hash_table(ht);

	return 0;
}
*/

