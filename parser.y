%{
// C Definitions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "addgen.c"
#include "hash_table.c"
#include "ast.c"

#define regcount 4

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
ht_item * symboltable;
ht_item * entry;

ht_item * symtab[50];
int maxind = -1; 
void updateArray(const char * key); 
void printArray(void); 
void test_print_collection(void);

char * scopename; 
char * name;
char * datatype; 
char buf[100];

// IR Rep
void printExprTree(Tree * root); 

/*
// SL = Statement List
Tree * Stmt_List[50]; 
int SLmaxind = -1; // initialize index
void updateSL(Tree * newStmt);
void printSL(void); 

// IL = ID List
ht_item * idL[50]; 
int ILmaxind = -1; 
void updateIL(ht_item * id); 
*/

//store comparator operator
char * cp;

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
Tree * temp;
Tree * expr;

Tree * prog_node;
Tree * ast_root;
Tree * func_node;
Tree * stmt_list; 
Tree * decl_list; 
Tree * param_list; 
Tree * write_list; 
Tree * read_list; 
Tree * if_list; 
Tree * else_list; 
Tree * while_list; 
Tree * comp_node;
Tree * call_list;
Tree * ret_stmt; 

Tree * inf_head; 
Tree * inf_tail; 
Tree * op_head; 
Tree * op_tail;
Tree * stack_head; 
Tree * stack_tail; 
Tree * list_head; 
Tree * list_tail; 

int labelnum = 0;
int i;

Tree * infix_pop(void);
void infix_add_node(Tree * node); 
void oplist_add_op(Tree * node); 
void oplist_extract(NodeType type); 
void infix_print(void); 
void oplist_print(void); 
void infix_build_expr_tree(void);
void list_push(Tree * node);
Tree * list_pop(void);

void printIR(void);

char buf_FUNC[30]; 
char buf_WHILE_START[20];
char buf_WHILE_END[20];
char buf_ELSE[20]; 
char buf_END_IF_ELSE[20];
char buf_OFFSET[20]; 

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
%token <strval> _TRUE _FALSE
%token <strval> COMPARATOR 
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
%type <strval> compop cond


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
				prog_node = new_list(PROG_NODE, $2, NULL);
				decl_list = new_list(DECL_LIST, NULL, NULL); 
				ast_add_node_to_list(prog_node, decl_list);
			} 
			pgm_body _END 
			{
				printf("\nPrinting AST Tree..\n");
				ast_print(prog_node);
				printf("printing the IR Code..\n\n");
				generate_code(prog_node);
				printf("\n\nprinting the Tiny Code..\n\n");
				walkAST(prog_node);
				//printf("\n\n\nCFG------\n\n");
				//ast_build_cfg(prog_node);
				//ast_cfg_print(prog_node);
			}
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
string_decl: 	_STRING { declare = 1; datatype = "STRING"; } 
				id ASSIGNMENT str_literal 
				{
					ht_insert(ht, symtab[maxind]->key, $3, datatype, $5); 
					lhs = new_varleaf(ht, symtab[maxind]->key, $3);
					ast_add_node_to_list(decl_list, lhs);
					decl_list->varcount = decl_list->varcount + 1;
					declare = 0; 
				} 
				TERMINATOR
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
					lhs = new_varleaf(ht, symtab[maxind]->key, $1);
					ast_add_node_to_list(decl_list, lhs);
					decl_list->varcount = decl_list->varcount + 1;
					lhs->entry->offset = -(decl_list->varcount);

					snprintf(buf_OFFSET, sizeof buf_OFFSET, "$%d", lhs->entry->offset);
					lhs->entry->str_offset = strdup(buf_OFFSET);
				}
				else if(write == 1){
					lhs = new_varleaf(ht, symtab[maxind]->key, $1);
					if(lhs == NULL){
						i = 0;
						while(lhs == NULL){
							i++; 
							lhs = new_varleaf(ht, symtab[maxind-i]->key, $1);
						}
					}

					ast_add_node_to_list(write_list, lhs);
				}
				else if(read == 1){
					lhs = new_varleaf(ht, symtab[maxind]->key, $1);
					if(lhs == NULL){
						i = 0;
						while(lhs == NULL){
							i++; 
							lhs = new_varleaf(ht, symtab[maxind-i]->key, $1);
						}
					}

					ast_add_node_to_list(read_list, lhs); 
				}

			} id_tail
; 
id_tail:	COMMA id 
			{ 	if(declare == 1) {
					ht_insert(ht, symtab[maxind]->key, $2, datatype, NULL);
					lhs = new_varleaf(ht, symtab[maxind]->key, $2);
					ast_add_node_to_list(decl_list, lhs);
					decl_list->varcount = decl_list->varcount + 1;
					lhs->entry->offset = -(decl_list->varcount);

					snprintf(buf_OFFSET, sizeof buf_OFFSET, "$%d", lhs->entry->offset);
					lhs->entry->str_offset = strdup(buf_OFFSET);
				}
				else if(write == 1){
					lhs = new_varleaf(ht, symtab[maxind]->key, $2); 
					if(lhs == NULL){
						i = 0;
						while(lhs == NULL){
							i++; 
							lhs = new_varleaf(ht, symtab[maxind-i]->key, $2);
						}
					}

					ast_add_node_to_list(write_list, lhs); 
				}
				else if(read == 1){
					lhs = new_varleaf(ht, symtab[maxind]->key, $2);
					if(lhs == NULL){
						i = 0;
						while(lhs == NULL){
							i++; 
							lhs = new_varleaf(ht, symtab[maxind-i]->key, $2);
						}
					}

					ast_add_node_to_list(read_list, lhs); 
				}
			} id_tail
			|
;

// Function Parameter List
param_decl_list:  	param_decl param_decl_tail
					|
;
param_decl:	 var_type id 
			{
				ht_insert(ht, symtab[maxind]->key, $2, datatype, NULL); 
				lhs = new_varleaf(ht, symtab[maxind]->key, $2); 
				ast_add_node_to_list(param_list, lhs);
				param_list->varcount = param_list->varcount + 1;
			} 
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
				ht_insert(ht, $3, NULL, NULL, NULL); 
				updateArray($3);
				
				snprintf(buf_FUNC, sizeof buf_FUNC, "FUNC_%s", $3); 
				func_node = new_list(FUNC_NODE, strdup(buf_FUNC), NULL);
				ast_add_node_to_list(prog_node, func_node);
				param_list = new_list(PARAM_LIST, NULL, NULL);
				ast_add_node_to_list(func_node, param_list); 
			}
			OPENPARENT param_decl_list CLOSEPARENT 
			{
				lhs = param_list->left;
				i = param_list->varcount;
				while(i > 0){
					lhs->entry->offset = i + 1;	// add 1 to account for return address slot
					
					snprintf(buf_OFFSET, sizeof buf_OFFSET, "$%d", lhs->entry->offset);
					lhs->entry->str_offset = strdup(buf_OFFSET);

					i--;
					lhs = lhs->next;	// moce to the next argument
				}
			}
			_BEGIN 
			{
				decl_list = new_list(DECL_LIST, NULL, NULL); 
				ast_add_node_to_list(func_node, decl_list); 
			}
			func_body _END
; 
func_body: 	decl 
			{	
				stmt_list = new_list(STMT_LIST, NULL, NULL);
				ast_add_node_to_list(func_node, stmt_list);
				list_push(stmt_list);
			} 
			stmt_list 
			{	
				stmt_list = list_pop();
				//printf("\n\n");
				//printIR();
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
assign_expr:	id ASSIGNMENT {	inf_head = NULL; op_head = NULL; inf_tail = NULL; op_tail = NULL; call_list = NULL; } expr 
				{	
					if(call_list != NULL){
						rhs = call_list; 
					}

					else{
						if (op_head != NULL) {	
							oplist_extract(100); // 100 > 50 tells oplist_extract to extract every opnode until end of oplist. 
						}
						if (inf_head != inf_tail) {			// if the 'expr' is a mathematical expression 
							infix_build_expr_tree();
						}	
						rhs = inf_head; 
					}

					lhs = new_varleaf(ht, symtab[maxind]->key, $1);
					if(lhs == NULL){
						i = 0;
						while(lhs == NULL){
							i++; 
							lhs = new_varleaf(ht, symtab[maxind-i]->key, $1);
						}
					}

					root_expr = new_node(ASSIGN_NODE, lhs, rhs); 
					ast_add_node_to_list(list_head, root_expr);
					inf_head = NULL;
					call_list = NULL; 
				}
; 
read_stmt: 		_READ { read_list = new_list(READ_LIST, NULL, NULL); ast_add_node_to_list(list_head, read_list); read = 1; } OPENPARENT { declare = 0; } id_list CLOSEPARENT TERMINATOR { read = 0; }
; 
write_stmt: 	_WRITE { write_list = new_list(WRITE_LIST, NULL, NULL); ast_add_node_to_list(list_head, write_list); write = 1; } OPENPARENT { declare = 0; } id_list CLOSEPARENT TERMINATOR { write = 0; }
; 
return_stmt: 	_RETURN expr 
				{
					ret_stmt = new_list(RETURN_STMT, NULL, NULL);
					ret_stmt->offset = 1 + param_list->varcount + regcount + 1;

					snprintf(buf_OFFSET, sizeof buf_OFFSET, "$%d", ret_stmt->offset);
					ret_stmt->str_offset = strdup(buf_OFFSET);


					if (op_head != NULL) {	
						oplist_extract(100); // 100 > 50 tells oplist_extract to extract every opnode until end of oplist. 
					}

					// check if 'expr' is a mathematical expression
					if(inf_head != inf_tail){
						// make inf_head to be the 'root' of expression
						infix_build_expr_tree();
					}

					ast_add_node_to_list(ret_stmt, inf_head);
					inf_head = NULL;
					ast_add_node_to_list(list_head, ret_stmt);
					ret_stmt = NULL;
				}
				TERMINATOR
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
call_expr: 		id 
				{
					call_list = new_list(CALL_LIST, $1, NULL);
				}
				OPENPARENT expr_list CLOSEPARENT
; 
expr_list: 		expr 
				{
					if (op_head != NULL) {	
						oplist_extract(100); // 100 > 50 tells oplist_extract to extract every opnode until end of oplist. 
					}

					if(inf_head != inf_tail){
						infix_build_expr_tree();
					}

					ast_add_node_to_list(call_list, inf_head);
					call_list->varcount = call_list->varcount + 1;
					inf_head = NULL; 
				}
				expr_list_tail 
				|
; 
expr_list_tail: COMMA expr
				{
					if (op_head != NULL) {	
						oplist_extract(100); // 100 > 50 tells oplist_extract to extract every opnode until end of oplist. 
					}

					if(inf_head != inf_tail){
						infix_build_expr_tree();
					}

					ast_add_node_to_list(call_list, inf_head);
					call_list->varcount = call_list->varcount + 1;
					inf_head = NULL; 
				}
				expr_list_tail
				|
; 
primary: 		OPENPARENT { oplist_add_op(new_node(OPEN_PARENT, NULL, NULL)); } expr CLOSEPARENT { oplist_extract(CLOSE_PARENT); }
				| id
				{
					term = new_varleaf(ht, symtab[maxind]->key, $1);
					if(term == NULL){
						i = 0;
						while(term == NULL){
							i++; 
							term = new_varleaf(ht, symtab[maxind-i]->key, $1);
						}
					}
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
//
//TODO: edit if_stmt and else_stmt
if_stmt: 	_IF 
			{ 	blocknum++; 
				snprintf(buf, 100, "BLOCK %d", blocknum); 
				ht_insert(ht, buf, NULL, NULL, NULL); 
				updateArray(buf);
				
				labelnum++; 
				snprintf(buf_ELSE, sizeof buf_ELSE, "ELSE_%d", labelnum); 
				labelnum++;
				snprintf(buf_END_IF_ELSE, sizeof buf_END_IF_ELSE, "END_IF_ELSE%d", labelnum);
				
				stmt_list = new_list(IF_LIST, strdup(buf_ELSE), strdup(buf_END_IF_ELSE)); 	// CREATE IF_LIST
				ast_add_node_to_list(list_head, stmt_list); // add IF_LIST to current list_head (any stmt_list)
				list_push(stmt_list);	// make IF_LIST to be the current head
			} 
			OPENPARENT cond 
			{
				ast_add_node_to_list(list_head, comp_node);	// add COMP_NODE to IF_LIST	
			}
			CLOSEPARENT decl {
				stmt_list = new_list(IF_STMT_LIST, strdup(buf_ELSE), strdup(buf_END_IF_ELSE)); // create IF_STMT_LIST
				ast_add_node_to_list(list_head, stmt_list); 	// add IF_STMT_LIST to IF_LIST
				list_push(stmt_list); 	// make IF_STMT_LIST to be the current head
			}
			stmt_list {
				stmt_list = list_pop(); // pop IF_STMT_LIST
				stmt_list = new_list(ELSE_LIST, strdup(buf_ELSE), strdup(buf_END_IF_ELSE)); // create ELSE_LIST
				ast_add_node_to_list(list_head, stmt_list); // add ELSE_LIST to IF_LIST
				list_push(stmt_list); // make ELSE_LIST to be the current head
				stmt_list = new_list(STMT_LIST, strdup(buf_ELSE), strdup(buf_END_IF_ELSE)); // create STMT_LIST
				ast_add_node_to_list(list_head, stmt_list); // add STMT_LIST to ELSE_LIST
				list_push(stmt_list); // make STMT_LIST to be the current head				
			}
			else_part {
				stmt_list = list_pop(); // pop ELSE's STMT_LIST
				stmt_list = list_pop(); // pop ELSE_LIST
				stmt_list = list_pop(); // pop IF_LIST
			}
			_ENDIF
; 
else_part: 	_ELSE 
			{	
				blocknum++; 
				snprintf(buf, 100, "BLOCK %d", blocknum); 
				ht_insert(ht, buf, NULL, NULL, NULL); 
				updateArray(buf);  
			}
			stmt_list 
			|
; 

cond: 		expr 
			{ 
				if (op_head != NULL) {	
					//printf("ophead is not null\n"); 
					oplist_extract(100); 	// 100 > 50 tells oplist_extract to extract every opnode until end of oplist. 
				}

				if (inf_head != inf_tail) {	// if the 'expr' is a mathematical expression 
					infix_build_expr_tree(); 
				} 
				
				temp = inf_head; 
				list_push(temp); 
				inf_head = NULL; 
			} 
			compop expr 
			{	
				if (op_head != NULL) {	
					//printf("ophead is not null\n");
					oplist_extract(100); 	// 100 > 50 tells oplist_extract to extract every opnode until end of oplist. 
				}
					
				if (inf_head != inf_tail) {	// if the 'expr' is a mathematical expression 
					//printf("is a tree\n");
					infix_build_expr_tree(); 
				} 
				

				lhs = list_pop();
				rhs = inf_head; 	
				comp_node = new_compnode(COMP_NODE, $3, lhs, rhs);
				comp_node->startlabel = list_head->startlabel;
				comp_node->endlabel = list_head->endlabel;
				inf_head = NULL;
			}
			| _TRUE
			| _FALSE
; 
compop: 	COMPARATOR {$$ = $1; }
; 
while_stmt: _WHILE {
				blocknum++; 
				snprintf(buf, 100, "BLOCK %d", blocknum); 
				ht_insert(ht, buf, NULL, NULL, NULL); 
				updateArray(buf);
				
				labelnum++;
				snprintf(buf_WHILE_START, sizeof buf_WHILE_START, "WHILE_START_%d", labelnum);
				labelnum++;
				snprintf(buf_WHILE_END, sizeof buf_WHILE_END, "WHILE_END_%d", labelnum);	

				stmt_list = new_list(WHILE_LIST, strdup(buf_WHILE_START), strdup(buf_WHILE_END)); // CREATE WHILE_LIST 
				ast_add_node_to_list(list_head, stmt_list);		// add WHILE_LIST to current list_head (any stmt_list)
				list_push(stmt_list);	// make WHILE_LIST to be the current head
			} 
			OPENPARENT cond	{
				ast_add_node_to_list(list_head, comp_node); // add COMP_NODE to WHILE_LIST	
			}
			CLOSEPARENT decl {
				stmt_list = new_list(WHILE_STMT_LIST, buf_WHILE_START, buf_WHILE_END); // create WHILE_STMT_LIST
				ast_add_node_to_list(list_head, stmt_list); // add WHILE_STMT_LIST to WHILE_LIST
				list_push(stmt_list); // make WHILE_STMT_LIST to be the current head		
			} 
			stmt_list 
			{	
				stmt_list = list_pop(); // pop WHILE_STMT_LIST
				stmt_list = list_pop(); // pop WHILE_LIST
			
			} 
			_ENDWHILE	
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
	
	//printf("\nBegin parsing..\n"); 
	yyparse();

	//test_print_collection();
	
	//deleteTree(stmt_list); 
	ht_del_hash_table(ht);
	//printf("\n_________________________________________________________________________________\n");

	fclose(yyin); 
	fclose(yyout);
	return 0; 	
}

void printIR(){
	
	fprintf(yyout, ";IR code\n");
	//printf(";IR code\n"); 
	
	fprintf(yyout, ";PUSH\n");
	//printf(";PUSH\n"); 
	
	fprintf(yyout, ";PUSHREGS\n");
	//printf(";PUSHREGS\n"); 
	
	fprintf(yyout, ";JSR FUNC_main\n"); 
	//printf(";JSR FUNC_main\n"); 
	
	fprintf(yyout, ";HALT\n");
	//printf(";HALT\n");

	fprintf(yyout, ";LABEL FUNC_main\n");
	//printf(";LABEL FUNC_main\n");

	fprintf(yyout, ";LINK 1\n");
	//printf(";LINK 1\n");

	generate_code(stmt_list); 

	fprintf(yyout, ";RET\n");
	//printf(";RET\n"); 

	printArray();
	
	fprintf(yyout, "push\n");
	//printf("push\n");

	fprintf(yyout, "jsr FUNC_main\n");
	//printf("jsr FUNC_main\n");

	fprintf(yyout, "sys halt\n");
	//printf("sys halt\n");

	fprintf(yyout, "label FUNC_main\n");
	//printf("label FUNC_main\n");

	fprintf(yyout, "link 1\n");
	//printf("link 1\n");

	walkAST(stmt_list);
	
	fprintf(yyout, "unlnk\n");
	//printf("unlnk\n");
	
	fprintf(yyout, "ret\n");
	//printf("ret\n");

	return; 

}

void list_push(Tree * node){
	//printf("Pushing..\n");

	// if scope stack is empty
	if(list_head == NULL){
		list_head = node; 
		list_tail = NULL;
		list_head->next = list_tail;

		return;
	}

	// add node to scope stack
	node->next = list_head; 
	list_head = node;

	if(node->node_type == WHILE_LIST){
		/*
		Tree * curr = list_head; 
		printf("LIST_STACK: ");
		while(curr != NULL){
			printf("<%d> ", curr->node_type);
			curr = curr->next; 
		}
		printf("\n");
		*/

		//printf("LLLSTART: %s\n", node->startlabel);
		//printf("LLLEND: %s\n", node->endlabel);

	}

	

	return;
}

Tree * list_pop(void){
	//printf("Popping..\n");

	if(list_head == NULL){
		printf("ERROR: Scope stack is empty!");
		exit(-1); 
	}

	Tree * popnode = list_head;  
	list_head = list_head->next;
	popnode->next = NULL;

	/*
	Tree * curr = list_head; 
	printf("STACK: ");
	while(curr != NULL){
		printf("<%d> ", curr->node_type);
		curr = curr->next; 
	}
	printf("\n");
	*/

	if(popnode->node_type == WHILE_LIST){
		/*
		Tree * curr = list_head; 
		printf("LIST_STACK: ");
		while(curr != NULL){
			printf("<%d> ", curr->node_type);
			curr = curr->next; 
		}
		printf("\n");
		*/

		//printf("PPPSTART: %s\n", popnode->startlabel);
		//printf("PPPEND: %s\n", popnode->endlabel);

	}


	return popnode;
}

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

Tree * infix_pop(){
	
	// stack should not be empty!
	if (stack_head == NULL){
		printf("infix_pop ERROR: Popping from an empty stack.\n"); 
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
				//printf("%s %s\n", eptr->name, eptr->strval);
				fprintf(yyout, "str %s %s\n", eptr->name, eptr->strval);
			}
			else{
				//printf("var %s\n", eptr->name);
				fprintf(yyout, "var %s\n", eptr->name); 
			}
			eptr = eptr->next; 
		}
	}

}

void test_print_collection(){
	int i; 
	ht_item * eptr; 

	printf("\nTesting print collection");

	for(i = 0; i <= maxind; i++){
		printf("\nScope: %s === ", symtab[i]->key); 
		eptr = symtab[i]; 
		while(eptr != NULL){
			printf("%s[%d]  ", eptr->name, eptr->offset); 
			eptr = eptr->next; 
		}
	}

	printf("\n");

	return;
}

void yyerror(const char *s){
	printf("%s at Line %d\n%s\n", s, yylineno, yytext); 
	//fprintf(yyout, "Not Accepted"); 
	//print the var_name
	//fprintf(yyout, "DECLARATION ERROR %s", yytext);
	return; 
}

