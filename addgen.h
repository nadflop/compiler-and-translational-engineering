#ifndef ADDGEN
#define ADDGEN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "hash_table.h"
#include "liveness.h"

/*IR Nodes*/
typedef struct data_object {
	char * src1;
	char * src2;
	char * op;
}data_object;

typedef struct CodeObject {
	int temp_num;
	char * startlabel;//only for if-else, while
	char * endlabel;
	char * temp; /*temporary variable used to store the result*/
	char * result_type; /*type of the result (INT or FLOAT)*/
	struct data_object * data;
}CodeObject;

void generate_reg_list();
void check_status(Tree * root, char * var);
void ensure(Tree * var);
void ensure_both(Tree * var);
void free_everything();
void allocate(Tree * parent, char * var);
void newTemp(char * s);
CodeObject * new_data();
void generate_self (Tree * node);
void generate_list (Tree * list);
void generate_code (Tree * node);
void deleteCode (CodeObject * cur_item, NodeType n_type);
void generateTiny(Tree * node);
void walkAST(Tree * node);
#endif
