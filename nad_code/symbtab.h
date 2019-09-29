#include <string.h> 

/*typedef union Value{
	int intval;
	float floatval;
	char *strval;
}Value;*/

typedef struct node {
	//parameter/var type and name
	char *name;
	char *type;
	//value of that symbol
	//Value val;<-- fancier application, uncomment when really understand
	int intval;
	float floatval;
	char *strval;
	//level of the scope, global, main, block, etc
	//int scope; <-- uncomment later when sure how to implement
	struct node *next;
};

typedef struct ht {
	int size;
	struct node **entry;
};

//insert function decl for hashtable create, insert etc
struct ht * createHashTable(int size);
unsigned int createHashCode(char *key, char *type);

void updateHT(struct ht * hash_table, char * name, char * type);
//void deleteHT();
void searchHT(struct ht * hash_table, char * name);

