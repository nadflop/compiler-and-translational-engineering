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
	char * temp; /*temporary variable used to store the result*/
	char * result_type; /*type of the result (INT or FLOAT)*/
	struct data_object * data;
}CodeObject;

/*char * newTemp();*/
data_object *  new_obj();
CodeObject * new_data();
void generate_self(Tree * node);
void generate_code(Tree * node);
void deleteCode(CodeObject * cur_item);
#endif
