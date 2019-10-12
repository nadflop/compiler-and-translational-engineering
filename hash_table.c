#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

/*function to initialize an item in the hash table*/
ht_item * ht_new_item(const char * k, const char * n, const char * type, const char * strval){
	ht_item * i = malloc(sizeof(ht_item));
	i->key = strdup(k);
	i->name = strdup(n);
	i->type = strdup(type);
	i->strval = strdup(strval);
	i->next = NULL;
	return i;
}
/*function to initialize a hash table*/
ht_hash_table * ht_new() {
	ht_hash_table * ht = malloc(sizeof(ht_hash_table));
	if (ht == NULL) {
		return NULL;
	}
	ht->size = 53;
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
void ht_insert(ht_hash_table * ht, const char * key, const char* name, const char* type, const char * strval) {
	ht_item * item = ht_new_item(key, name, type, strval);
	int index = ht_hash(item->key, ht->size);
	ht_item * cur_item = ht->items[index];

	/*check if there is already a hash table with the hash index*/
	if (ht->items[index] != NULL) {
		cur_item = ht->items[index];
		/*if the item is not empty*/
		while (cur_item != NULL) {
			if (strcmp(cur_item->name, item->name) == 0) {
				/*there's already a declaration of the item in the hash table*/
				printf("Error");
				break;
			}
			cur_item = cur_item->next;
		}
		/*cur_item must now points at the end of the list*/
		if (cur_item == NULL) {
			item->next = ht->items[index];
			ht->items[index] = item;
		}
	}
	/*just add the item into the hash_table*/
	else {
		item->next = NULL;
		ht->items[index] = item;
	}
}
/*find the contents of a hash table?*/
char * ht_search(ht_hash_table * ht, const char * key){
	int index= ht_hash(key, ht->size);
	ht_item * item = ht->items[index];
	
	while (item != NULL) {
		if (strcmp(item->key, key) == 0) {
			/*found the key!*/
			break;
		}
		item = item->next;
	}
	if (item == NULL) {
		return NULL;
	}
	return item->name;
}
 
int main() {
	ht_hash_table * ht = ht_new();
	ht_insert(ht, "Main", "Team Cendol", "INT", "NULL");
	printf("%s\n", ht_search(ht, "Main"));
	ht_del_hash_table(ht);
	return 0;
}
