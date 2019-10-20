#ifndef ADDGEN
#define ADDGEN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"

/*IR Nodes*/
typedef struct data_object {
	char * src1;
	char * src2;
	char * op;
}data_object;

typedef struct CodeObject {
	int temp_num;
	char * temp; /*temporary variable used to store the result*/
	char * result_type; /*type of the result (INT or FLOAT)*/
	struct data_object * data;
}CodeObject;

void newTemp(char * s);
CodeObject * new_data();
void generate_self (Tree * node);
void generate_list (Tree * list);
void generate_code (Tree * node);
void deleteCode (CodeObject * cur_item, NodeType n_type);
void generateTiny(Tree * node);
void walkAST(Tree * node);
#endif
