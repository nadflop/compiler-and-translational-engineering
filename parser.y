%{
// C Definitions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "addgen.c"
#include "hash_table.c"
#include "ast.c"

extern int yylex(); 
extern int yyparse();
extern int yylineno;
extern char * yytext;
extern int yyleng; 
extern FILE *yyin;
extern FILE *yyout;  

char * inputfile; 
char * outputfile; 

int blocknum = 0;   
int declare = 0; 

void yyerror(const char *s);


// Integrating Symbol Table
ht_hash_table * ht; // hashtable pointer

ht_item * symtab[50];
int maxind = -1; 
void updateArray(const char * key); 
void printArray(void); 

char * scopename; 
char * name;
char * datatype; 
char buf[20];


// IR Rep
void printExprTree(Tree * root); 

// SL = Statement List
Tree * Stmt_List[50]; 
int SLmaxind = -1; // initialize index
void updateSL(Tree * newStmt);
void printSL(void); 

// IL = ID List
ht_item * idL[50]; 
int ILmaxind = -1; 
void updateIL(ht_item * id); 

int write = 0; 
int read = 0; 
int prim_done = 0;
int primary = 0; 
int openp = 0; 
int closep = 0; 

Tree * lhs; 
Tree * rhs; 
Tree * parent; 
Tree * opnode;
Tree * term;
Tree * root_expr;

Tree * func_node;
Tree * stmt_list; 
Tree * write_list; 
Tree * read_list; 
Tree * if_list; 
Tree * else_list; 
Tree * while_list; 
Tree * stmt;

Tree * inf_head; 
Tree * inf_tail; 
Tree * op_head; 
Tree * op_tail;
Tree * stack_head; 
Tree * stack_tail; 

int labelnum = 0;

void infix_add_node(Tree * node); 
void oplist_add_op(Tree * node); 
void oplist_extract(NodeType type); 
void infix_print(void); 
void oplist_print(void); 
void infix_build_expr_tree(void);

void printIR(void);

%}
// Bison Definitions
%define parse.error verbose
%locations

//Symbol semantic values

%start program

// Union to define value types from yylex
%union {
	int intval; 
	float floatval; 
	char * strval;
	char * op; 
	char * keyword;
	char * datatype;
	char * bool_val; 
	int integerExp; 
	float floatExp;  
}

//token declare terminal symbols with no precedence or associativity specified
%token <keyword> _PROG _FUNC _READ _WRITE
%token <keyword> _BEGIN _IF _ELSE _WHILE
%token <keyword>_END _ENDIF _ENDWHILE
%token <strval> _INT _VOID _STRING _FLOAT
%token <keyword> _RETURN
%token <keyword> _TRUE _FALSE
%token <operator> COMPARATOR 
%token <operator> ASSIGNMENT
%token <op> ADDOP MULOP
%token <operator> TERMINATOR OPENPARENT CLOSEPARENT COMMA
%token <strval> IDENTIFIER 
%token <strval> STRINGLITERAL
%token <strval> INTLITERAL 
%token <strval> FLOATLITERAL
%token <a> add_op mul_op

//precedence declaration
%left '='
%left COMPARATOR
%left ADDOP
%left MULOP
%left OPENPARENT CLOSEPARENT

//type declare the type of semantic values for a non-terminal symbol
%type <strval> id str_literal string_decl
%type <strval> var_type any_type var_decl id_list


%%
// Grammar Rules
// Program
// GLOBAL
program: 	_PROG id _BEGIN 
			{ 	
				ht = ht_new(); 
				scopename = "GLOBAL"; 
				ht_insert(ht, scopename, NULL, NULL, NULL); 
				updateArray(scopename); 	
			} 
			pgm_body _END 
;
id: IDENTIFIER
; 
pgm_body: 	decl func_declarations
;
decl: 	string_decl decl
		| var_decl decl 
		| 
; 

// String Declaration
string_decl: 	_STRING { declare = 1; datatype = "STRING"; } id ASSIGNMENT str_literal { ht_insert(ht, symtab[maxind]->key, $3, datatype, $5); declare = 0; } TERMINATOR
;
str_literal: STRINGLITERAL
; 

// Variable Declaration
var_decl:  var_type id_list TERMINATOR
; 
var_type: 	_FLOAT 	{ declare = 1; datatype = "FLOAT"; $$ = "FLOAT"; }
			| _INT	{ declare = 1; datatype = "INT"; $$ = "INT"; }
; 
any_type: 	var_type
			| _VOID	{ declare = 1; datatype = "VOID"; $$ = "VOID"; }
; 
id_list: 	id 
			{ 	if(declare == 1){
					ht_insert(ht, symtab[maxind]->key, $1, datatype, NULL); 
				}
				else if(write == 1){
					lhs = new_varleaf(ht, "GLOBAL", $1); 
					ast_add_node_to_list(write_list, lhs);
				}
				else if(read == 1){
					lhs = new_varleaf(ht, "GLOBAL", $1); 
					ast_add_node_to_list(read_list, lhs); 
				}

			} id_tail
; 
id_tail:	COMMA id 
			{ 	if(declare == 1) {
					ht_insert(ht, symtab[maxind]->key, $2, datatype, NULL);	
				}
				else if(write == 1){
					lhs = new_varleaf(ht, "GLOBAL", $2); 
					ast_add_node_to_list(write_list, lhs); 
				}
				else if(read == 1){
					lhs = new_varleaf(ht, "GLOBAL", $2); 
					ast_add_node_to_list(read_list, lhs); 
				}
			} id_tail
			|
;

// Function Parameter List
param_decl_list:  	param_decl param_decl_tail
					|
;
param_decl:	 var_type id { ht_insert(ht, symtab[maxind]->key, $2, datatype, NULL); } 
;
param_decl_tail:	COMMA param_decl param_decl_tail
					|
; 

// Function Declarations
func_declarations: 	func_decl func_declarations
					|
; 
func_decl: _FUNC any_type id 
			{	
				ht_insert(ht, $3, NULL, NULL, NULL); updateArray($3); 
			}
			OPENPARENT param_decl_list CLOSEPARENT _BEGIN func_body _END
; 
func_body: 	decl {	stmt_list = new_list(STMT_LIST, NULL, NULL); } stmt_list 
			{	
				func_node = new_node(FUNC_NODE, stmt_list, NULL); 
				ast_print(func_node);
				printIR();
			}
; 

// Statement List
stmt_list: 	stmt stmt_list
			|
;
stmt: 	base_stmt
		| if_stmt
		| loop_stmt
; 
base_stmt:		assign_stmt	
				| read_stmt 
				| write_stmt 	
				| control_stmt
;

// Basic Statements
assign_stmt: 	assign_expr TERMINATOR
; 
assign_expr:	id ASSIGNMENT {	inf_head = NULL; op_head = NULL; inf_tail = NULL; op_tail = NULL; } expr 
				{	
					if (op_head != NULL) {	
						oplist_extract(100); 	// 100 > 50 tells oplist_extract to extract every opnode until end of oplist. 
					}
					
					if (inf_head != inf_tail) {			// if the 'expr' is a mathematical expression 
						infix_build_expr_tree();
					} 

					lhs = new_varleaf(ht, "GLOBAL", $1); 
					rhs = inf_head; 
					root_expr = new_node(ASSIGN_NODE, lhs, rhs); 
					ast_add_node_to_list(stmt_list, root_expr); 
				}
; 
read_stmt: 		_READ { read_list = new_list(READ_LIST, NULL, NULL); ast_add_node_to_list(stmt_list, read_list); read = 1; } OPENPARENT { declare = 0; } id_list CLOSEPARENT TERMINATOR { read = 0; }
; 
write_stmt: 	_WRITE { write_list = new_list(WRITE_LIST, NULL, NULL); ast_add_node_to_list(stmt_list, write_list); write = 1; } OPENPARENT { declare = 0; } id_list CLOSEPARENT TERMINATOR { write = 0; }
; 
return_stmt: 	_RETURN expr TERMINATOR
; 

// Expressions
expr: 			expr_prefix factor 
; 
expr_prefix: 	expr_prefix factor addop
				|
;
factor: 		factor_prefix postfix_expr
; 
factor_prefix: 	factor_prefix postfix_expr mulop
				|
; 
postfix_expr: 	primary
				| call_expr
; 
call_expr: 		id OPENPARENT expr_list CLOSEPARENT 
; 
expr_list: 		expr expr_list_tail 
				|
; 
expr_list_tail: COMMA expr expr_list_tail
				|
; 
primary: 		OPENPARENT { oplist_add_op(new_node(OPEN_PARENT, NULL, NULL)); } expr CLOSEPARENT { oplist_extract(CLOSE_PARENT); }
				| id
				{
					term = new_varleaf(ht, "GLOBAL", $1); 
					infix_add_node(term); 
				}

				| INTLITERAL
				{
					term = new_litleaf($1, "INT"); 
					infix_add_node(term);
				}

				| FLOATLITERAL	
				{
					term = new_litleaf($1, "FLOAT"); 
					infix_add_node(term); 
				}
; 
addop: 			ADDOP 
				{	
					opnode = new_opnode(ARITHM_NODE, ((strcmp("+", $1) == 0) ? ADD : SUB), term, NULL); 
					oplist_add_op(opnode);
				}
; 
mulop: 			MULOP
				{	
					opnode = new_opnode(ARITHM_NODE, ((strcmp("*", $1) == 0) ? MUL : DIV), term, NULL);
					oplist_add_op(opnode); 
				}
; 

// Complex Statements and Condition
if_stmt: 	_IF OPENPARENT cond CLOSEPARENT 
			{ 	blocknum++; 
				snprintf(buf, 20, "BLOCK %d", blocknum); 
				ht_insert(ht, buf, NULL, NULL, NULL); 
				updateArray(buf); 
			} decl stmt_list else_part _ENDIF
; 
else_part: 	_ELSE {	blocknum++; 
					snprintf(buf, 20, "BLOCK %d", blocknum); 
					ht_insert(ht, buf, NULL, NULL, NULL); 
					updateArray(buf);  
			} decl stmt_list
			|
; 
cond: 		expr compop expr
			| _TRUE
			| _FALSE
; 
compop: 	COMPARATOR
; 
while_stmt: _WHILE OPENPARENT cond CLOSEPARENT decl 
				{ 	blocknum++; 
					snprintf(buf, 20, "BLOCK %d", blocknum); 
					ht_insert(ht, buf, NULL, NULL, NULL); 
					updateArray(buf); 
				} stmt_list _ENDWHILE
; 
control_stmt: 	return_stmt
; 
loop_stmt: 		while_stmt
;

%%
//Data structure for our symbol table
// Additional C Code
int main(int argc, char **argv){
	inputfile = argv[1];
	outputfile = argv[2]; 
	
	yyin = fopen(argv[1], "r"); 
	yyout = fopen(argv[2], "w");
	
	//printf("\n"); 
	yyparse();

	//printArray();
	//deleteTree(stmt_list); 
	ht_del_hash_table(ht);
	//printf("\n_________________________________________________________________________________\n");

	fclose(yyin); 
	fclose(yyout);
	return 0; 	
}

void printIR(){
	
	fprintf(yyout, ";IR code\n");
	printf(";IR code\n"); 

	fprintf(yyout, ";LABEL FUNC_main\n"); 
	printf(";LABEL FUNC_main\n"); 

	fprintf(yyout, ";LINK\n");
	printf(";LINK\n");

	generate_code(stmt_list); 

	fprintf(yyout, ";RET\n");
	printf(";RET\n"); 

	printArray();

	walkAST(stmt_list);
	
	fprintf(yyout, "sys halt\n");
	printf("sys halt\n");

	return; 

}

// Work in Progress
void infix_push(Tree * node){
	
	// stack is empty
	if (stack_head == NULL){
		stack_head = node; 
		stack_tail = NULL; 
		stack_head->next = stack_tail; 
		// at this point, node(head)-NULL(tail)
		return; 
	}

	// add node to stack
	// stack_tail and stack_head is pointing to something
	node->next = stack_head; 
	stack_head = node; 

	return; 
}

// Work in Progress
Tree * infix_pop(){
	
	// stack should not be empty!
	if (stack_head == NULL){
		printf("ERROR: Popping from an empty stack.\n"); 
		exit(-1); 
	}

	Tree * popnode = stack_head; 
	stack_head = stack_head->next; 

	return popnode; 
}

void infix_build_expr_tree(){
	//printf("Building Expression Tree..\n"); 
	
	Tree * temp; 
	//infix_print();  
	stack_head = NULL; 
	stack_tail = NULL; 

	while (inf_head != inf_tail){
		if (inf_head->node_type == LIT_VAL || inf_head->node_type == VAR_REF){
			temp = inf_head; 
			inf_head = inf_head->next; 
			infix_push(temp); 
		} 
		else if(inf_head->node_type == ARITHM_NODE){
			rhs = infix_pop(); 
			lhs = infix_pop(); 
			inf_head->right = rhs; 
			inf_head->left = lhs; 
			temp = inf_head; 
			inf_head = inf_head->next; 
			infix_push(temp); 
		}
	}
	rhs = infix_pop(); 
	lhs = infix_pop(); 
	inf_head->right = rhs; 
	inf_head->left = lhs; 

	//printExprTree(inf_head); 

	return; 
}

void infix_print(){
	Tree * curr = inf_head;
	printf("\nINFIX-POSTFIX EXPRESSION: ");
	while(curr != NULL){
		if(curr->node_type == LIT_VAL){
			printf("[%s]", curr->literal); 		
		}
		else if(curr->node_type == VAR_REF){
			printf("%s", curr->name); 
		}
		else if(curr->node_type == ARITHM_NODE){
			printf("<%d>", curr->op); 
		}
		curr = curr->next; 
	}
	printf("\n"); 
	return; 
}

void oplist_print(){
	Tree * curr = op_head; 
	printf("OPLIST: "); 
	while(curr != NULL){ 
		if(curr->node_type == OPEN_PARENT){
			printf("OPENPARENT");
		}
		else{
			printf("<%d>", curr->op); 
		}
		curr = curr->next; 
	}
	printf("\n"); 
}

void infix_add_node(Tree * node){
	//printf("Adding new node to infix..\n"); 
	// add first node
	if (inf_head == NULL){
		inf_head = term; 
		inf_tail = term; 
		return; 
	}

	// append new node to tail
	inf_tail->next = node; 
	inf_tail = inf_tail->next;
	inf_tail->next = NULL; 
	return; 
}

void oplist_add_op(Tree * opnode){
	//printf("Adding new OP to list..%s\n", op_head); 
	if (op_head == NULL){
		//printf("oplist is empty!\n"); 
		op_tail = opnode; 
		op_head = opnode; 
		op_tail->next = NULL; 
		return; 
	}

	if((op_head->op == MUL || op_head->op == DIV) && (opnode->node_type == ARITHM_NODE) && (opnode->op == ADD || opnode->op == SUB) ){
		//call op_extract function, then add opnode into list (to be top of 'stack'?)
		oplist_extract(ARITHM_NODE); 
		opnode->next = op_head; 
		op_head = opnode; 
	}
	else if(op_head->op == SUB && (opnode->node_type != OPEN_PARENT && opnode->op != MUL && opnode->op != DIV)){
		oplist_extract(ARITHM_NODE); 
		opnode->next = op_head; 
		op_head = opnode; 
	}
	else{
		// do normal op addition 
		opnode->next = op_head; 
		op_head = opnode; 
	}

	return; 
}

void oplist_extract(NodeType type){
	inf_tail->next = op_head; 

	if (type == ARITHM_NODE){
		if (op_head->op == SUB){
			// extract only the subtraction?
			inf_tail = op_head; 
			op_head = op_head->next; 
		}
		/*
		else if(op_head->op == opnode->op){
			// extract similar ops
			inf_tail = op_head; 
			op_head = op_head->next; 
		}
		*/
		else {
		// should extract op until the bottom of the stack or an '(' excluded
			while(op_head != NULL && op_head->node_type != OPEN_PARENT){
				inf_tail = op_head; 
				op_head = op_head->next; 
			}
			// here op_head either points to NULL or an OPEN_PARENT
		}
		inf_tail->next = NULL; 
	}
	else if (type == CLOSE_PARENT){
	// should extract op until an '(' is seen
		while(op_head->node_type != OPEN_PARENT){
			inf_tail = op_head; 
			op_head = op_head->next; 
		}
		inf_tail->next = NULL; 
		// here op_head should be pointing to a OPEN_PARENT
		//printf("op_head's type: %d, next is null?: %s\n", op_head->node_type, (op_head->next == NULL) ? "Yes" : "No");  	
		Tree * temp = op_head; 
		if(op_head->next == NULL){
			op_tail == NULL;
			op_head = NULL;    
		}
		else{
			op_head = op_head->next; 	
		}
		free(temp); 
	}
	else if(type > 50){
	// should extract until the bottom of the oplist
		while(op_head != NULL){
			inf_tail = op_head; 
			op_head = op_head->next; 
		}
		inf_tail->next = NULL; 
	}

}


void printExprTree(Tree * root){
	//printf("\nPrinting Expression Tree ---------------------------------\n"); 
	ast_traversal(root);

	return; 
}	

void updateArray(const char * key){
	//printf("updating symtab array\n");
	maxind++;
	int i = ht_hash(key, ht->size);  
	symtab[maxind] = ht->items[i];
	//printf("symtab[maxind]: %s\n", symtab[maxind]->key); 

	return; 
}

void printArray(){ 
	int i;
	ht_item * eptr;
	/*
	for(i = 0; i <= maxind; i++){
		printf("\nScope: %s === ", symtab[i]->key);
		eptr = symtab[i]; 
		while(eptr != NULL){
			printf("%s ", eptr->name); 
			eptr = eptr->next; 
		}
	}
	*/

	for(i = 0; i <= maxind; i++){
		eptr = symtab[i]; 
		while(eptr != NULL){
			if(eptr->type == NULL){
				break; 
			}
			if(strcmp(eptr->type, "STRING") == 0){
				fprintf(yyout, "str %s %s\n", eptr->name, eptr->strval);
			}
			else{
				fprintf(yyout, "var %s\n", eptr->name); 
			}
			eptr = eptr->next; 
		}
	}

}

void yyerror(const char *s){
	printf("%s at Line %d\n%s\n", s, yylineno, yytext); 
	//fprintf(yyout, "Not Accepted"); 
	//print the var_name
	//fprintf(yyout, "DECLARATION ERROR %s", yytext);
	return; 
}

