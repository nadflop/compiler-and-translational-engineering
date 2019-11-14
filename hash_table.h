#ifndef HASH_TABLE
#define HASH_TABLE

#include <stdio.h>
#include <string.h>

typedef struct ht_item {
	char * key;
	char * name;
	char * type;
	char * strval;

	int offset;

	struct ht_item * next;
}ht_item;

typedef struct ht_hash_table {
	int size;
	struct ht_item ** items;
}ht_hash_table;

ht_item * ht_new_item(const char * k, const char * n, const char * type, const char * strval);
ht_hash_table * ht_new();
void ht_del_item(ht_item * i);
void ht_del_hash_table(ht_hash_table * ht);
unsigned int ht_hash(const char * key, const int size);
void ht_insert(ht_hash_table * ht, const char * key, const char * name, const char * type, const char * strval);
int ht_search(ht_hash_table * ht, const char * key, const char * name);
ht_item * ht_get_item(ht_hash_table * ht, const char * key, const char * name); 

#endif
