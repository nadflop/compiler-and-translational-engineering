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
	CodeObject* t = new_data();
			
	if (node->node_type == VAR_REF) {
		t->temp = (strcmp(node->entry->key, "GLOBAL") == 0) ? (node->name) : (node->entry->str_offset); 		
		t->result_type = node->type;
		node->tac = t;
	}
	else if (node->node_type == LIT_VAL){ 
		newTemp(s);
		t->temp = strdup(s);   
		t->result_type = (strcmp(node->type, "INT") == 0) ? ("INT") : ("FLOAT"); 
		t->data->op = (strcmp(t->result_type,"INT") == 0) ? ("STOREI") : ("STOREF");
		t->data->src1 = node->literal; 
		t->data->src2 = NULL; 
		node->tac = t;	
		fprintf(yyout, ";%s %s %s\n", t->data->op, t->data->src1, node->tac->temp);
		printf(";%s %s %s\n", t->data->op, t->data->src1, node->tac->temp);

	}
	else if (node->node_type == WHILE_LIST) {
		//check if we have enough info to generate 3ac
		if(node->left->tac->temp != NULL && node->right->right->tac->temp != NULL) {
			printf(";LABEL %s\n", node->endlabel);
			fprintf(yyout,";LABEL %s\n", node->endlabel);
			return;
		}
	}
	else if (node->node_type == RETURN_STMT) {
		if (node->left->node_type == VAR_REF) {
			newTemp(s);
			node->left->tac->data->op = (strcmp(node->left->tac->result_type,"INT") == 0) ? ("STOREI") : ("STOREF");
			node->left->tac->temp = strdup(s);
			node->left->tac->data->src1 = (strcmp(node->left->entry->key, "GLOBAL") == 0) ? (node->left->name) : (node->left->entry->str_offset); 
			fprintf(yyout, ";%s %s %s\n", node->left->tac->data->op, node->left->tac->data->src1, node->left->tac->temp);
			printf(";%s %s %s\n", node->left->tac->data->op, node->left->tac->data->src1, node->left->tac->temp);
		}
		t->data->op = (strcmp(node->left->tac->result_type,"INT") == 0) ? ("STOREI") : ("STOREF");
		t->data->src1 = node->left->tac->temp; //<-- here, the temp val is the offset of the var
		t->data->src2 = NULL;
		t->temp = node->str_offset;
		t->result_type = node->left->type;
		node->tac = t;
		fprintf(yyout, ";%s %s %s\n", t->data->op, t->data->src1, t->temp);
		printf(";%s %s %s\n", t->data->op, t->data->src1, t->temp);
		printf(";UNLINK\n");
		printf(";RET\n");
		fprintf(yyout, ";UNLINK\n");
		fprintf(yyout, ";RET\n");
	}
	else if (node->node_type == CALL_LIST) {
		printf(";PUSH\n");
		printf(";PUSHREGS\n");
		fprintf(yyout, ";PUSH\n");
		fprintf(yyout, ";PUSHREGS\n");
		Tree * curr = node->left;
		while (curr != NULL) {
			if (curr->node_type == VAR_REF) {
				printf(";PUSH %s\n", curr->entry->str_offset);
				fprintf(yyout, ";PUSH %s\n", curr->entry->str_offset);
			}
			else {
				printf(";PUSH %s\n", curr->tac->temp);
				fprintf(yyout, ";PUSH %s\n", curr->tac->temp);
			}
			curr = curr->next;		
		}
		printf(";JSR FUNC_%s\n", node->startlabel);
		fprintf(yyout, ";JSR FUNC_%s\n", node->startlabel);
		int i;
		for (i = 0; i < node->varcount; i++) {
			printf(";POP\n");
			fprintf(yyout, ";POP\n");
		}
		printf(";POPREGS\n");
		fprintf(yyout, ";POPREGS\n");
		newTemp(s);
		t->temp = strdup(s);
		node->tac = t; 
		printf(";POP %s\n", node->tac->temp);
		fprintf(yyout, ";POP %s\n", node->tac->temp);
	}
	else if (node->node_type == STMT_LIST || node->node_type == IF_LIST || node->node_type == WHILE_STMT_LIST || node->node_type == IF_STMT_LIST || node->node_type == ELSE_LIST || node->node_type == PROG_NODE || node->node_type == PARAM_LIST || node->node_type == DECL_LIST || node->node_type == FUNC_NODE){
		return;  
	}
	else {
		/*check if we have enough info to generate 3ac*/
		if(node->left->tac->temp != NULL && node->right->tac->temp != NULL) {
			switch(node->node_type) {
				case ASSIGN_NODE:
					t->data->op = (strcmp(node->left->tac->result_type, "INT") == 0) ? ("STOREI") : ("STOREF");
					if (node->right->node_type == VAR_REF) {
						newTemp(s);
						node->right->tac->data->op = (strcmp(node->right->tac->result_type,"INT") == 0) ? ("STOREI") : ("STOREF");
						node->right->tac->temp = strdup(s);
						node->right->tac->data->src1 = (strcmp(node->right->entry->key, "GLOBAL") == 0) ? (node->right->name) : (node->right->entry->str_offset);						
						//print out the variable store assignment
						printf(";%s %s %s\n", node->right->tac->data->op, node->right->tac->data->src1, node->right->tac->temp);
						fprintf(yyout, ";%s %s %s\n", node->right->tac->data->op, node->right->tac->data->src1, node->right->tac->temp);
					}
					
					t->data->src1 = node->right->tac->temp;
					t->data->src2 = NULL;
					t->temp = node->left->tac->temp;
					t->result_type = node->left->type;
					node->tac = t;
					fprintf(yyout, ";%s %s %s\n", t->data->op, t->data->src1, t->temp);
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
					fprintf(yyout, ";%s %s %s %s\n", node->tac->data->op, node->tac->data->src1, node->tac->data->src2, t->temp);
					printf(";%s %s %s %s\n", node->tac->data->op, node->tac->data->src1, node->tac->data->src2, t->temp);
					break;

				case COMP_NODE:
					//newTemp(s);
					//t->temp = strdup(s);
					t->data->src1 = node->left->tac->temp;
					if (node->right->node_type == VAR_REF) {
						newTemp(s);
						node->right->tac->data->op = (strcmp(node->right->tac->result_type,"INT") == 0) ? ("STOREI") : ("STOREF");
						node->right->tac->temp = strdup(s);
						node->right->tac->data->src1 = (strcmp(node->right->entry->key, "GLOBAL") == 0) ? (node->right->name) : (node->right->entry->str_offset);	
						//print out the variable store assignment
						printf(";%s %s %s\n", node->right->tac->data->op, node->right->tac->data->src1, node->right->tac->temp);
						fprintf(yyout, ";%s %s %s\n", node->right->tac->data->op, node->right->tac->data->src1, node->right->tac->temp);
					}
					t->data->src2 = node->right->tac->temp;
					/*determine result type*/
					if(strcmp(node->left->tac->result_type,"INT") == 0)
						t->result_type = "INT";
					else
						t->result_type = "FLOAT";
					/*determine opposite comparator*/
					switch(node->comp) {
						case GT:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("LEI") : ("LEF");
							break;
						case GE:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("LTI") : ("LTF");
							break;
						case LT:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("GEI") : ("GEF");								
							break;
						case LE:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("GTI") : ("GTF");								
							break;
						case NE:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("EQI") : ("EQF");								
							break;
						case EQ:
							t->data->op = (strcmp(t->result_type, "INT") == 0) ? ("NEI") : ("NEF");								
							break;
					}
					node->tac = t;
					char s= node->startlabel[0];
					if (s == 'W') {
						fprintf(yyout, ";%s %s %s %s\n", node->tac->data->op, node->tac->data->src1, node->tac->data->src2, node->endlabel);
						printf(";%s %s %s %s\n", node->tac->data->op, node->tac->data->src1, node->tac->data->src2, node->endlabel);
					}
					else {
						fprintf(yyout, ";%s %s %s %s\n", node->tac->data->op, node->tac->data->src1, node->tac->data->src2, node->startlabel);
						printf(";%s %s %s %s\n", node->tac->data->op, node->tac->data->src1, node->tac->data->src2, node->startlabel);

					}
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
						printf(";%s %s\n", curr->tac->data->op, curr->tac->temp); 
						curr = curr->next;
					}
					break;
				
				case READ_LIST:
					free(t->data); 
					free(t); 
					Tree * curr1 = node->left;
					while (curr1 != NULL) {
						if(strcmp(curr1->tac->result_type,"INT") == 0) 
							curr1->tac->data->op = "READI";
						else if (strcmp(curr1->tac->result_type,"FLOAT") == 0)
							curr1->tac->data->op = "READF";
						fprintf(yyout, ";%s %s\n", curr1->tac->data->op, curr1->tac->temp);
						printf(";%s %s\n", curr1->tac->data->op, curr1->tac->temp);
						curr1 = curr1->next;
					}
					break;

				default:
					break;
			}	
		}
	}

}

void generate_list(Tree * list) {
	if (list == NULL)
		return;
	
	if (list->node_type == PROG_NODE) {
		printf(";IR code\n");
		printf(";PUSH\n");
		printf(";PUSHREGS\n");
		printf(";JSR FUNC_main\n");
		printf(";HALT\n");
		fprintf(yyout, ";IR code\n");
		fprintf(yyout, ";PUSH\n");
		fprintf(yyout, ";PUSHREGS\n");
		fprintf(yyout, ";JSR FUNC_main\n");
		fprintf(yyout, ";HALT\n");
	}

	if (list->node_type == FUNC_NODE) {
		printf(";LABEL %s\n", list->startlabel);
		fprintf(yyout, ";LABEL %s\n", list->startlabel);
		if (list->left->next->varcount == 0) {
			printf(";LINK\n");
			fprintf(yyout, ";LINK\n");
		}
		else {
			printf(";LINK %d\n", list->left->next->varcount);
			fprintf(yyout, ";LINK %d\n", list->left->next->varcount);
		}
	}

	Tree * curr = list->left;

	if(curr == NULL)
		return;
	while(curr != NULL) {
		//before calling generate code
		if (curr->node_type == ELSE_LIST){
			//we want to print the jump label here
			printf(";JUMP %s\n", list->endlabel);
			fprintf(yyout,";JUMP %s\n", list->endlabel);
			printf(";LABEL %s\n", list->startlabel);
			fprintf(yyout,";LABEL %s\n", list->startlabel);
		}
		
		generate_code(curr);

		//after calling generate code
		if (curr->node_type == ELSE_LIST) {
			printf(";LABEL %s\n", list->endlabel);
			fprintf(yyout,";LABEL %s\n", list->endlabel);
		}
		curr = curr->next;
	}
	if (list->node_type == PROG_NODE) {
		printf(";UNLINK\n");
		printf(";RET\n");
		fprintf(yyout, ";UNLINK\n");
		fprintf(yyout, ";RET\n");
	}
}

void generate_code(Tree * root) {	
	if(root == NULL) 
		return;

	if(root->node_type == ARITHM_NODE || root->node_type == COMP_NODE || root->node_type == ASSIGN_NODE) {
		generate_code(root->left);
		generate_code(root->right);
	}
	else if (root->node_type == STMT_LIST || root->node_type == IF_STMT_LIST || root->node_type == WHILE_STMT_LIST || root->node_type == WRITE_LIST || root->node_type == READ_LIST || root->node_type == IF_LIST || root->node_type == PROG_NODE || root->node_type == DECL_LIST || root->node_type == PARAM_LIST || root->node_type == FUNC_NODE || root->node_type == CALL_LIST || root->node_type == RETURN_STMT) {
		//since we know they only have a left child
		generate_list(root);
	}
	else if (root->node_type == WHILE_LIST) {
		printf(";LABEL %s\n", root->startlabel);
		fprintf(yyout, ";LABEL %s\n", root->startlabel);
		generate_list(root);
		printf(";JUMP %s\n", root->startlabel);
		fprintf(yyout, ";JUMP %s\n", root->startlabel);
	}
	else if (root->node_type == ELSE_LIST) {
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
	if (node->node_type == VAR_REF){
		return;
	}
	else if(node->node_type == WRITE_LIST || node->node_type == READ_LIST || node->node_type == STMT_LIST || node->node_type == IF_LIST || node->node_type == ELSE_LIST || node->node_type == WHILE_LIST || node->node_type == IF_STMT_LIST || node->node_type == WHILE_STMT_LIST || node->node_type == DECL_LIST || node->node_type == PARAM_LIST || node->node_type == PROG_NODE || node->node_type == FUNC_NODE || node->node_type == CALL_LIST){
		return;
	}
	else if (node->node_type == RETURN_STMT) {
		if (node->left->node_type == VAR_REF) {
			fprintf(yyout, "move %s %s\n", node->left->tac->data->src1, node->left->tac->temp);
		}
		fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
		fprintf(yyout,"unlnk\n");
		fprintf(yyout, "ret\n");

		if (node->left->node_type == VAR_REF) {
			printf("move %s %s\n", node->left->tac->data->src1, node->left->tac->temp);
		}
		printf("move %s %s\n", node->tac->data->src1, node->tac->temp);	
		printf("unlnk\n");
		printf("ret\n");
	}
	else {
		char * opcode = node->tac->data->op;
		if (strcmp(opcode, "STOREF") == 0){
			if (node->right != NULL) {
				if (node->right->node_type == VAR_REF) {
					char * rhs_op = node->right->tac->data->op;
					if (strcmp(rhs_op, "STOREF") == 0){
						fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
						printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					}
					if (strcmp(rhs_op, "STOREI") == 0){
						fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
						printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					}
				}
			}
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
		}
		if (strcmp(opcode, "STOREI") == 0){
			if (node->right != NULL) {
				if (node->right->node_type == VAR_REF) {
					char * rhs_op = node->right->tac->data->op;
					if (strcmp(rhs_op, "STOREF") == 0){
						fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
						printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					}
					if (strcmp(rhs_op, "STOREI") == 0){
						fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
						printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					}
				}
			}
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
		}
		if (strcmp(opcode, "WRITEI") == 0){
			fprintf(yyout, "sys writei %s\n", node->tac->temp);
			printf("sys writei %s\n", node->tac->temp);
		}
		if (strcmp(opcode, "WRITEF") == 0){
			fprintf(yyout, "sys writer %s\n", node->tac->temp);
			printf("sys writer %s\n", node->tac->temp);
		}
		if (strcmp(opcode, "WRITES") == 0){
			fprintf(yyout, "sys writes %s\n", node->tac->temp);
			printf("sys writes %s\n", node->tac->temp);
		}
		if (strcmp(opcode, "READI") == 0){
			fprintf(yyout, "sys readi %s\n", node->tac->temp);	
			printf("sys readi %s\n", node->tac->temp);	
		}
		if (strcmp(opcode, "READF") == 0){
			fprintf(yyout, "sys readr %s\n", node->tac->temp);	
			printf("sys readr %s\n", node->tac->temp);	
		}
		if (strcmp(opcode, "ADDI") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "addi %s %s\n", node->tac->data->src2, node->tac->temp);
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("addi %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "ADDF") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "addr %s %s\n", node->tac->data->src2, node->tac->temp);
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("addr %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "SUBI") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "subi %s %s\n", node->tac->data->src2, node->tac->temp);
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("subi %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "SUBF") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "subr %s %s\n", node->tac->data->src2, node->tac->temp);
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("subr %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "MULI") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "muli %s %s\n", node->tac->data->src2, node->tac->temp);
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("muli %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "MULF") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "mulr %s %s\n", node->tac->data->src2, node->tac->temp);
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("mulr %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "DIVI") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "divi %s %s\n", node->tac->data->src2, node->tac->temp);
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("divi %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "DIVF") == 0) {
			fprintf(yyout, "move %s %s\n", node->tac->data->src1, node->tac->temp);
			fprintf(yyout, "divr %s %s\n", node->tac->data->src2, node->tac->temp);
			printf("move %s %s\n", node->tac->data->src1, node->tac->temp);
			printf("divr %s %s\n", node->tac->data->src2, node->tac->temp);
		}
		if (strcmp(opcode, "LEI") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
			printf("cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jle %s\n", node->endlabel);
				fprintf(yyout, "jle %s\n", node->endlabel);
			}
			else {
				printf("jle %s\n", node->startlabel);
				fprintf(yyout, "jle %s\n", node->startlabel);
			}

		}
		if (strcmp(opcode, "LEF") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
		
			printf("cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jle %s\n", node->endlabel);
				fprintf(yyout, "jle %s\n", node->endlabel);
			}
			else {
				printf("jle %s\n", node->startlabel);
				fprintf(yyout, "jle %s\n", node->startlabel);
			}

		}
		if (strcmp(opcode, "LTI") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
		
			printf("cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jlt %s\n", node->endlabel);
				fprintf(yyout, "jlt %s\n", node->endlabel);
			}
			else {
				printf("jlt %s\n", node->startlabel);
				fprintf(yyout, "jlt %s\n", node->startlabel);
			}

		}
		if (strcmp(opcode, "LTF") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
		
			printf("cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jlt %s\n", node->endlabel);
				fprintf(yyout, "jlt %s\n", node->endlabel);
			}
			else {
				printf("jlt %s\n", node->startlabel);
				fprintf(yyout, "jlt %s\n", node->startlabel);
			}
	
		}
		if (strcmp(opcode, "GEI") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
		
			printf("cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jge %s\n", node->endlabel);
				fprintf(yyout, "jge %s\n", node->endlabel);
			}
			else {
				printf("jge %s\n", node->startlabel);
				fprintf(yyout, "jge %s\n", node->startlabel);
			}
	
		}
		if (strcmp(opcode, "GEF") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
		
			printf("cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jge %s\n", node->endlabel);
				fprintf(yyout, "jge %s\n", node->endlabel);
			}
			else {
				printf("jge %s\n", node->startlabel);
				fprintf(yyout, "jge %s\n", node->startlabel);
			}

		}
		if (strcmp(opcode, "GTI") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
		
			printf("cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jgt %s\n", node->endlabel);
				fprintf(yyout, "jgt %s\n", node->endlabel);
			}
			else {
				printf("jgt %s\n", node->startlabel);
				fprintf(yyout, "jgt %s\n", node->startlabel);
			}

		}
		if (strcmp(opcode, "GTF") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
		
			printf("cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jgt %s\n", node->endlabel);
				fprintf(yyout, "jgt %s\n", node->endlabel);
			}
			else {
				printf("jgt %s\n", node->startlabel);
				fprintf(yyout, "jgt %s\n", node->startlabel);
			}

		}
		if (strcmp(opcode, "EQI") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
		
			printf("cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jeq %s\n", node->endlabel);
				fprintf(yyout, "jeq %s\n", node->endlabel);
			}
			else {
				printf("jeq %s\n", node->startlabel);
				fprintf(yyout, "jeq %s\n", node->startlabel);
			}

		}
		if (strcmp(opcode, "EQF") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
		
			printf("cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jeq %s\n", node->endlabel);
				fprintf(yyout, "jeq %s\n", node->endlabel);
			}
			else {
				printf("jeq %s\n", node->startlabel);
				fprintf(yyout, "jeq %s\n", node->startlabel);
			}

		}
		if (strcmp(opcode, "NEI") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
		
			printf("cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpi %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jne %s\n", node->endlabel);
				fprintf(yyout, "jne %s\n", node->endlabel);
			}
			else {
				printf("jne %s\n", node->startlabel);
				fprintf(yyout, "jne %s\n", node->startlabel);
			}

		}
		if (strcmp(opcode, "NEF") == 0) {
			if (node->right->node_type == VAR_REF) {
				char * rhs_op = node->right->tac->data->op;
				if (strcmp(rhs_op, "STOREF") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
				if (strcmp(rhs_op, "STOREI") == 0){
					fprintf(yyout, "move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
					printf("move %s %s\n", node->right->tac->data->src1, node->right->tac->temp);
				}
			}
		
			printf("cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			fprintf(yyout,"cmpr %s %s\n", node->tac->data->src1, node->tac->data->src2);
			if (node->endlabel[0] == 'W') {
				printf("jne %s\n", node->endlabel);
				fprintf(yyout, "jne %s\n", node->endlabel);
			}
			else {
				printf("jne %s\n", node->startlabel);
				fprintf(yyout, "jne %s\n", node->startlabel);
			}

		}
		
	}
}

void walkAST(Tree * node) {
	//printf("on top of walkAST, the curr node is %d\n", node->node_type);
	if (node == NULL)
		return;
	if (node->node_type == PROG_NODE) {
		Tree * decl = node->left->left;
		while (decl != NULL) {
			if (strcmp(decl->type, "STRING") == 0 ) {
				printf("str %s %s\n", decl->name, decl->entry->strval);
				fprintf(yyout, "str %s %s\n", decl->name, decl->entry->strval);
			}
			else {
				printf("var %s\n", decl->name);
				fprintf(yyout, "var %s\n", decl->name);
			}
			decl = decl->next;
		}
		printf("push\n");
		printf("push r0\n");
		printf("push r1\n");
		printf("push r2\n");
		printf("push r3\n");
		printf("jsr FUNC_main\n");
		printf("sys halt\n");
		fprintf(yyout,"push\n");
		fprintf(yyout,"push r0\n");
		fprintf(yyout,"push r1\n");
		fprintf(yyout,"push r2\n");
		fprintf(yyout,"push r3\n");
		fprintf(yyout,"jsr FUNC_main\n");
		fprintf(yyout,"sys halt\n");
		decl = node->left->next; //skip the decl list since we already traversed through it
		//printf("the current node is %d\n", decl->node_type);
		while (decl != NULL) {
			walkAST(decl);
			decl = decl->next;
			//printf("the current node is %d\n", decl->node_type);
		}
		//printf("why are we outta here?\n");
		printf("unlnk\n");
		printf("ret\n");
		fprintf(yyout,"unlnk\n");
		fprintf(yyout,"ret\n");
	}
	else if(node->node_type == STMT_LIST || node->node_type == IF_STMT_LIST || node->node_type == WHILE_STMT_LIST || node->node_type == DECL_LIST || node->node_type == PARAM_LIST || node->node_type == RETURN_STMT) {
		Tree * curr = node;
		if (curr->left != NULL) {
			walkAST(curr->left);
			curr = curr->left->next; 
			while(curr != NULL) {  
				walkAST(curr); 
				curr = curr->next; 
			}
		}
	}
	else if(node->node_type == FUNC_NODE) {
		printf("label %s\n", node->startlabel);
		fprintf(yyout,"label %s\n", node->startlabel);
		printf("link %d\n", node->left->next->varcount);
		fprintf(yyout, "link %d\n", node->left->next->varcount);
		Tree * curr0 = node;
		if (curr0->left != NULL) {
			walkAST(curr0->left);
			curr0 = curr0->left->next; 
			while(curr0 != NULL) {  
				walkAST(curr0); 
				curr0 = curr0->next; 
			}
		}
	}
	else if(node->node_type == IF_LIST) {
		Tree * curr1 = node;
		walkAST(curr1->left);
		curr1 = curr1->left->next; 
		while(curr1 != NULL) {  
			if (curr1->node_type == ELSE_LIST) {
				printf("jmp %s\n", node->endlabel);
				fprintf(yyout, "jmp %s\n", node->endlabel);
				printf("label %s\n", node->startlabel);
				fprintf(yyout, "label %s\n", node->startlabel);
			}
			walkAST(curr1);
			curr1 = curr1->next; 
		}
		printf("label %s\n", node->endlabel);
		fprintf(yyout,"label %s\n", node->endlabel);

	}
	else if(node->node_type == WRITE_LIST || node->node_type == READ_LIST) {
		Tree * curr2 = node->left;
		while(curr2 != NULL) {

			char * opcode = curr2->tac->data->op;
			if (strcmp(opcode, "WRITEI") == 0){
				fprintf(yyout, "sys writei %s\n", curr2->tac->temp);
				printf("sys writei %s\n", curr2->tac->temp);
			}
			if (strcmp(opcode, "WRITEF") == 0){
				fprintf(yyout, "sys writer %s\n", curr2->tac->temp);
				printf("sys writer %s\n", curr2->tac->temp);
			}
			if (strcmp(opcode, "WRITES") == 0){
				fprintf(yyout, "sys writes %s\n", curr2->tac->temp);
				printf("sys writes %s\n", curr2->tac->temp);
			}
			if (strcmp(opcode, "READI") == 0){
				fprintf(yyout, "sys readi %s\n", curr2->tac->temp);
				printf("sys readi %s\n", curr2->tac->temp);	
			}
			if (strcmp(opcode, "READF") == 0){
				fprintf(yyout, "sys readr %s\n", curr2->tac->temp);
				printf("sys readr %s\n", curr2->tac->temp);	
			}

			curr2 = curr2->next;
		}
	}
	else if(node->node_type == WHILE_LIST) {		
		Tree * curr3 = node->left;
		printf("label %s\n", node->startlabel);
		fprintf(yyout, "label %s\n", node->startlabel);
		walkAST(curr3); 
		curr3 = curr3->next;
		while(curr3 != NULL) { 
			walkAST(curr3); 
			curr3 = curr3->next;
		}
		printf("jmp %s\n", node->startlabel);
		fprintf(yyout,"jmp %s\n", node->startlabel);
		printf("label %s\n", node->endlabel);
		fprintf(yyout, "label %s\n", node->endlabel);
	}
	else if(node->node_type == ELSE_LIST) {		
		Tree * curr4 = node;
		if (curr4->left != NULL) {
			walkAST(curr4->left);
		}
		curr4 = curr4->left->next; 
		while(curr4 != NULL) {  
			walkAST(curr4); 
			curr4 = curr4->next; 
		}
		
		return;
	}
	else if(node->node_type == CALL_LIST) {
		Tree * curr5 = node->left;
		if (curr5->node_type != VAR_REF && curr5->node_type != LIT_VAL) {
			walkAST(curr5); 
		}
		printf("push\n");
		printf("push r0\n");
		printf("push r1\n");
		printf("push r2\n");
		printf("push r3\n");
		fprintf(yyout,"push\n");
		fprintf(yyout,"push r0\n");
		fprintf(yyout,"push r1\n");
		fprintf(yyout,"push r2\n");
		fprintf(yyout,"push r3\n");
		while (curr5 != NULL) {
			if (curr5->node_type == VAR_REF) {
				//printf("i am here\n");
				printf("push %s\n", curr5->entry->str_offset);
				fprintf(yyout, "push %s\n", curr5->entry->str_offset);
			}
			else {
				printf("push %s\n", curr5->tac->temp);
				fprintf(yyout,"push %s\n", curr5->tac->temp);
			}
			curr5 = curr5->next;
		}
		printf("jsr FUNC_%s\n", node->startlabel);
		fprintf(yyout,"jsr FUNC_%s\n", node->startlabel);
		int j;
		for (j = 0; j < node->varcount; j++) {
			//printf("pop\n");
			fprintf(yyout, "pop\n");
		}
		printf("pop r3\n");
		printf("pop r2\n");
		printf("pop r1\n");
		printf("pop r0\n");
		printf("pop %s\n", node->tac->temp);
		fprintf(yyout, "pop r3\n");
		fprintf(yyout, "pop r2\n");
		fprintf(yyout, "pop r1\n");
		fprintf(yyout, "pop r0\n");
		fprintf(yyout, "pop %s\n", node->tac->temp);

		return;
	}

	//ONLY IF ITS A COMPNODE OR OPNODE
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

