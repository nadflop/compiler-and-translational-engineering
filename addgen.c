#include <stdio.h>
#include <stdlib.h>

#include "addgen.h"
#include "ast.h"
#include "hash_table.h"

extern FILE * yyout;

int tempnum = -1;
char s[5];
/*func to generate new temp var name*/

void newTemp(char * s) {
	tempnum++;
	sprintf(s,"r%d", tempnum);
	//printf("newTemp Created: %s\n", s); 
	return;
}

CodeObject * new_data() {
	CodeObject *t = malloc(sizeof(CodeObject));
	data_object *i = malloc(sizeof(data_object));
	if (t == NULL) {
		return NULL;	
	}
	
	t->data = i;
	i = NULL;		
	return t;
}

void generate_self(Tree * node) {

	//printf("GEN_SELF\n"); 
	CodeObject* t = new_data();
			
	if (node->node_type == VAR_REF) {
		t->temp = node->name;
		t->result_type = node->type;
		//free(t->data);
		//t->data = NULL;
		node->tac = t;
	}
	else if (node->node_type == LIT_VAL){
		//printf("LIT_VAL found\n"); 
		newTemp(s);
		t->temp = strdup(s);  
		//printf("---- %s\n", node->type); 
		t->result_type = (strcmp(node->type, "INT") == 0) ? ("INT") : ("FLOAT"); 
		/*fill in the code part*/
		t->data->op = (strcmp(t->result_type,"INT") == 0) ? ("STOREI") : ("STOREF");
		t->data->src1 = node->literal; 
		t->data->src2 = NULL; 
		fprintf(yyout, ";%s %s %s\n", t->data->op, t->data->src1, t->temp);
		node->tac = t;
	}
	else if (node->node_type == STMT_LIST){
		return;  
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
					fprintf(yyout, ";%s %s %s\n", t->data->op, t->data->src1, t->temp);
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
					fprintf(yyout, ";%s %s %s %s\n", node->tac->data->op, node->tac->data->src1, node->tac->data->src2, t->temp);
					break;				
				case COMP_NODE:
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
					switch(node->comp) {
						case GT:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("GTI") : ("GTF");
							break;
						case GE:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("GEI") : ("GEF");
							break;
						case LT:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("LTI") : ("LTF");								
							break;
						case LE:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("LEI") : ("LEF");								
							break;
						case NE:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("NEI") : ("NEF");								
							break;
						case EQ:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("EQI") : ("EQF");								
							break;
					}
					node->tac = t;
					fprintf(yyout, ";%s %s %s %s\n", node->tac->data->op, node->tac->data->src1, node->tac->data->src2, t->temp);
					break;
				case WRITE_LIST:
					free(t->data); 
					free(t); 
					Tree * curr = node->left;
					while (curr != NULL) {
						if(strcmp(curr->tac->result_type,"INT") == 0) 
							curr->tac->data->op = "WRITEI";
						else if (strcmp(curr->tac->result_type,"FLOAT") == 0)
							curr->tac->data->op = "WRITEF";
						else
							curr->tac->data->op = "WRITES";
						fprintf(yyout, ";%s %s\n", curr->tac->data->op, curr->tac->temp); 
						//printf("%s ", curr->tac->temp);
						curr = curr->next;
					}
					break;
				
				case READ_LIST:
					free(t->data); 
					free(t); 
					//t->result_type = node->left->tac->result_type;
					//node->tac = t;
					//printf(";%s %s\n", t->data->op, t->temp);
					Tree * curr1 = node->left;
					while (curr1 != NULL) {
						if(strcmp(curr1->tac->result_type,"INT") == 0) 
							curr1->tac->data->op = "READI";
						else if (strcmp(curr1->tac->result_type,"FLOAT") == 0)
							curr1->tac->data->op = "READF";
						fprintf(yyout, ";%s %s\n", curr1->tac->data->op, curr1->name);
						curr1 = curr1->next;
					}
					break;
				case IF_LIST:
					break;
				case ELSE_LIST:
					break;
				case WHILE_LIST:
					break;
				default:
					break;
			}	
		}
	}
	
	return;
}
//TODO: Think how to generate 3ac for if-else statement and while loops
void generate_list(Tree * list) {
	Tree * curr = list->left;

	if(curr == NULL)
		return;
	if(list->node_type == WRITE_LIST || list->node_type == READ_LIST) {
		while(curr != NULL) {
			generate_code(curr);
			curr = curr->next;
		}
	}
	else {
		while(curr != NULL) {
			generate_code(curr);
			curr = curr->next;
		}
	}
}

void generate_code(Tree * root) {
	
	//printf("Generating Code..\n");
	if(root == NULL) 
		return;

	if(root->node_type == ASSIGN_NODE || root->node_type == ARITHM_NODE || root->node_type == COMP_NODE) {
		generate_code(root->left);
		generate_code(root->right);
	}
	else if (root->node_type == STMT_LIST || root->node_type == WRITE_LIST || root->node_type == READ_LIST || root->node_type == IF_LIST || root->node_type == ELSE_LIST || root->node_type == WHILE_LIST) {
		//since we know they only have a left child
		generate_list(root);		
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

void generateTiny(Tree * node) {

	if (node->node_type == VAR_REF) //&& node->tac->data == NULL) 
		return;
	else if(node->node_type == WRITE_LIST || node->node_type == READ_LIST || node->node_type == STMT_LIST || node->node_type == IF_LIST || node->node_type == ELSE_LIST || node->node_type == WHILE_LIST){
		return;
	}
	else {
		char * opcode = node->tac->data->op;
		if (strcmp(opcode, "STOREF") == 0){
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
		}
		if (strcmp(opcode, "STOREI") == 0){
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
		}
		if (strcmp(opcode, "WRITEI") == 0)
			fprintf(yyout, "sys writei %s\n", node->tac->temp);
		if (strcmp(opcode, "WRITEF") == 0)
			fprintf(yyout, "sys writer %s\n", node->tac->temp);
		if (strcmp(opcode, "WRITES") == 0)
			fprintf(yyout, "sys writes %s\n", node->tac->temp);
		if (strcmp(opcode, "READI") == 0)
			fprintf(yyout, "sys readi %s\n", node->tac->temp);	
		if (strcmp(opcode, "READF") == 0)
			fprintf(yyout, "sys readr %s\n", node->tac->temp);	
		if (strcmp(opcode, "ADDI") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "addi %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "ADDF") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "addr %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "SUBI") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "subi %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "SUBF") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "subr %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "MULI") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "muli %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "MULF") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "mulr %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "DIVI") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "divi %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "DIVF") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "divr %s %s\n", node->tac->data->src2, node->tac->temp);
		}
	}
}
void walkAST(Tree * node) {
	if (node == NULL)
		return;
	if(node->node_type == STMT_LIST){
		Tree * curr = node;
		walkAST(curr->left);
		curr = curr->left->next; 
		while(curr != NULL) { 
			//printf("in while loop\n"); 
			walkAST(curr); 
			curr = curr->next; 
		}
		//printf("end of stmtlist while loop\n");
	}
	else if(node->node_type == WRITE_LIST || node->node_type == READ_LIST) {
		Tree * curr1 = node->left;
		while(curr1 != NULL) {

			char * opcode = curr1->tac->data->op;
			if (strcmp(opcode, "WRITEI") == 0)
				fprintf(yyout, "sys writei %s\n", curr1->tac->temp);
			if (strcmp(opcode, "WRITEF") == 0)
				fprintf(yyout, "sys writer %s\n", curr1->tac->temp);
			if (strcmp(opcode, "WRITES") == 0)
				fprintf(yyout, "sys writes %s\n", curr1->tac->temp);
			if (strcmp(opcode, "READI") == 0)
				fprintf(yyout, "sys readi %s\n", curr1->tac->temp);	
			if (strcmp(opcode, "READF") == 0)
				fprintf(yyout, "sys readr %s\n", curr1->tac->temp);	

			curr1 = curr1->next;
		}
	}
	else if(node->node_type != VAR_REF && node->node_type != LIT_VAL) {
		walkAST(node->left);
		walkAST(node->right);
	}

	generateTiny(node);
}

/*
int main() {
	// Tree, pass it to generate code
	Tree * const_val1 = new_varleaf("a", "INT");
	Tree * const_val2 = new_varleaf("b", "INT");
	Tree * const_val3 = new_varleaf("c", "INT");
	Tree * const_val4 = new_varleaf("d", "INT");
	Tree * const_val5 = new_varleaf("e", "INT");
	//Tree * const_val6 = new_varleaf("s", "STRING");
	Tree * lit1 = new_litleaf("20", "INT");
	Tree * lit2 = new_litleaf("30", "INT");
	Tree * lit3 = new_litleaf("40", "INT");
	Tree * a = new_node(ASSIGN_NODE, const_val1, lit1);
	Tree * b = new_node(ASSIGN_NODE, const_val2, lit2);
	Tree * c = new_node(ASSIGN_NODE, const_val3, lit3); 
	Tree * mulop1 = new_opnode(ARITHM_NODE, MUL, const_val1, const_val4);
	Tree * mulop2 = new_opnode(ARITHM_NODE, MUL, const_val2, const_val3);
	Tree * addop1 = new_opnode(ARITHM_NODE, ADD, mulop2, mulop1);
	Tree * assgn = new_node(ASSIGN_NODE, const_val5, addop1);
	//Tree * writep = new_node(WRITE_NODE, const_val6, NULL);
	ast_traversal(assgn);
	printf(";IR code\n");
	printf(";LABEL FUNC_main\n");
	printf(";LINK\n");
	generate_code(a);
	generate_code(b);
	generate_code(c);
	//generate_code(assgn);
	//generate_code(writep);
	printf(";RET\n");
	deleteTree(a);
	deleteTree(b);
	deleteTree(c);
	//walkAST(assgn);
	printf("sys halt");
	//walkAST(writep);
	//deleteTree(assgn);
	//char s[6] = "opcode";
	//printf("%s", s);
	deleteTree(addop1);	
	deleteTree(mulop2);	
	deleteTree(mulop1);	
	deleteTree(lit2);	
	deleteTree(lit1);	
	deleteTree(const_val3);	
	deleteTree(const_val2);	
	deleteTree(const_val1);
	return 0;	
}

*/

