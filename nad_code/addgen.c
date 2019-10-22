#include <stdio.h>
#include <stdlib.h>

#include "addgen.h"
#include "ast.h"
#include "ast.c"

int tempnum = 0;
char s[5];
/*func to generate new temp var name*/
void newTemp(char * s) {
	tempnum++;
	sprintf(s,"r%d", tempnum);
	return;
}

data_object * new_obj() {
	data_object * i = malloc(sizeof(data_object));
	if(i == NULL)
		return NULL;
	/*
	i->op = op;
	i->src1 = src1;
	i->src2 = src2;*/
	
	return i;
}

CodeObject * new_data() {
	CodeObject *t = malloc(sizeof(CodeObject));
	data_object *i = malloc(sizeof(data_object));
	if (t == NULL) {
		return NULL;	
	}
	
	t->data = i;//new_obj();
	i = NULL;		
	return t;
}

void generate_self(Tree * node) {
	CodeObject* t = new_data();
			
	if (node->node_type == VAR_REF) {
		t->temp = node->name;
		t->result_type = node->type;
		//t->data = NULL;
		free(t->data);
		t->data = NULL;
		node->tac = t;
	}
	else if (node->node_type == LIT_VAL){
		newTemp(s);
		t->temp = strdup(s);
		t->result_type = (strcmp(node->type, "INT") == 0) ? ("INT") : ("FLOAT");
		/*fill in the code part*/
		t->data->op = (strcmp(t->result_type,"INT") == 0) ? ("STOREI") : ("STOREF");
		t->data->src1 = node->literal;
		t->data->src2 = NULL;
		printf(";%s %s %s\n", t->data->op, t->data->src1, t->temp);
		node->tac = t;
	}
	else {
		/*check if we have enough info to generate 3ac*/
		if(node->left->tac->temp != NULL && node->right->tac->temp != NULL) {
			switch(node->node_type) {
				case ASSIGN_NODE:
					t->data->op = (strcmp(node->left->tac->result_type, "INT") == 0) ? ("STOREI") : ("STOREF");
					t->data->src1 = node->right->tac->temp;
					t->data->src2 = NULL;
					t->temp = node->left->tac->temp;
					t->result_type = node->left->type;
					node->tac = t;
					printf(";%s %s %s\n", t->data->op, t->data->src1, t->temp);
					break;

				case ARITHM_NODE:
					newTemp(s);
					t->temp = strdup(s);
					t->data->src1 = node->left->tac->temp;
					t->data->src2 = node->right->tac->temp;					
					/*determine result type*/
					if(strcmp(node->left->tac->result_type,"INT") == 0)
						t->result_type = "INT";
					else
						t->result_type = "FLOAT";
					/*determine operand*/
					switch(node->op) {
						case ADD:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("ADDI") : ("ADDF");
							break;
						case SUB:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("SUBI") : ("SUBF");
							break;
						case MUL:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("MULI") : ("MULF");								
							break;
						case DIV:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("DIVI") : ("DIVF");								
							break;
					}
					node->tac = t;
					printf(";%s %s %s %s\n", node->tac->data->op, node->tac->data->src1, node->tac->data->src2, t->temp);
					break;

				case WRITE_NODE:
					if(strcmp(node->left->tac->result_type,"INT") == 0) 
						t->data->op = "WRITEI";
					else if (strcmp(node->left->tac->result_type,"FLOAT") == 0)
						t->data->op = "WRITEF";
					else
						t->data->op = "WRITES";
					t->data->src1 = node->left->name;
					t->data->src2 = NULL;
					t->temp = NULL;
					t->result_type = node->left->type;
					node->tac = t;
					printf(";%s %s\n", t->data->op, t->data->src1);
					break;

				case READ_NODE:
					if(strcmp(node->left->tac->result_type,"INT") == 0) 
						t->data->op = "READII";
					else 
						t->data->op = "READF";
					t->data->src1 = NULL;
					t->data->src2 = NULL;
					newTemp(s);
					t->temp = strdup(s);
					t->result_type = node->left->tac->result_type;
					node->tac = t;
					printf(";%s %s\n", t->data->op, t->temp);
					break;

				default:
					break;
			}	
		}
	}
	
	return;
}

void generate_code(Tree * root) {
	if(root == NULL) 
		return;
	if(root->node_type == ASSIGN_NODE || root->node_type == ARITHM_NODE || root->node_type == WRITE_NODE || root->node_type == READ_NODE) {
		generate_code(root->left);
		generate_code(root->right);
	}
	generate_self(root);
	return;
}

void deleteCode(CodeObject * cur_item, NodeType n_type) {
	if (cur_item == NULL)
		return;
	if(cur_item->data != NULL ) {
		free(cur_item->data);
		if (cur_item->temp != NULL) {
			if(n_type != ASSIGN_NODE)
				free(cur_item->temp);
		}
	}
		
	free(cur_item);
	
}

char * strlwr(char * str) {
	unsigned char * p = (unsigned char *) str;

	while (*p) {
		*p = tolower((unsigned char)*p);
		p++;
	}
	return str;
}

void generateTiny(Tree * node) {

	if (node->node_type == VAR_REF) 
		return;
	else if(node->node_type == LIT_VAL)
		return;
	else {
		
		char * opcode = node->tac->data->op;
		if (strcmp(opcode, "STOREF") == 0)
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
		if (strcmp(opcode, "STOREI") == 0)
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
		if (strcmp(opcode, "WRITEI") == 0)
			printf("sys writei  %s\n", node->tac->data->src1);
		if (strcmp(opcode, "WRITEF") == 0)
			printf("sys writef %s\n", node->tac->data->src1);
		if (strcmp(opcode, "WRITES") == 0)
			printf("sys writes %s\n", node->tac->data->src1);
		if (strcmp(opcode, "READI") == 0)
			printf("sys readi %s\n", node->tac->temp);	
		if (strcmp(opcode, "READFF") == 0)
			printf("sys readf %s\n", node->tac->temp);	
		if (strcmp(opcode, "ADDI") == 0) {
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("addi %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "ADDF") == 0) {
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("addf %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "SUBI") == 0) {
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("subi %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "SUBF") == 0) {
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("subf %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "MULI") == 0) {
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("muli %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "MULF") == 0) {
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("mulf %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "DIVI") == 0) {
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("divi %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "DIVF") == 0) {
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("divf %s %s\n", node->tac->data->src2, node->tac->temp);
		}
	}
}
void walkAST(Tree * node) {
	if (node == NULL)
		return;
	if(node->node_type != VAR_REF && node->node_type != LIT_VAL) {
		walkAST(node->left);
		walkAST(node->right);
	}

	generateTiny(node);
}

int main() {
	/*Tree, pass it to generate code*/
	Tree * const_val1 = new_varleaf("a", "INT");
	Tree * const_val2 = new_varleaf("b", "INT");
	Tree * const_val3 = new_varleaf("c", "INT");
	Tree * const_val4 = new_varleaf("d", "INT");
	Tree * const_val5 = new_varleaf("e", "INT");
	//Tree * const_val6 = new_varleaf("s", "STRING");
	/*
	Tree * lit1 = new_litleaf("20", "INT");
	Tree * lit2 = new_litleaf("30", "INT");
	Tree * lit3 = new_litleaf("40", "INT");
	Tree * a = new_node(ASSIGN_NODE, const_val1, lit1);
	Tree * b = new_node(ASSIGN_NODE, const_val2, lit2);
	Tree * c = new_node(ASSIGN_NODE, const_val3, lit3); */
	Tree * mulop1 = new_opnode(ARITHM_NODE, MUL, const_val1, const_val4);
	Tree * mulop2 = new_opnode(ARITHM_NODE, MUL, const_val2, const_val3);
	Tree * addop1 = new_opnode(ARITHM_NODE, ADD, mulop2, mulop1);
	Tree * assgn = new_node(ASSIGN_NODE, const_val5, addop1);
	//Tree * writep = new_node(WRITE_NODE, const_val6, NULL);
	/*ast_traversal(assgn);*/
	printf(";IR code\n");
	printf(";LABEL FUNC_main\n");
	printf(";LINK\n");
	/*generate_code(a);
	generate_code(b);
	generate_code(c);*/
	generate_code(assgn);
	//generate_code(writep);
	printf(";RET\n");
/*	deleteTree(a);
	deleteTree(b);
	deleteTree(c);*/
	walkAST(assgn);
	printf("sys halt");
	//walkAST(writep);
	deleteTree(assgn);
	//char s[6] = "opcode";
	//printf("%s", s);
	/*
	deleteTree(addop1);	
	deleteTree(mulop2);	
	deleteTree(mulop1);	
	deleteTree(lit2);	
	deleteTree(lit1);	
	deleteTree(const_val3);	
	deleteTree(const_val2);	
	deleteTree(const_val1);*/
	return 0;	
}

