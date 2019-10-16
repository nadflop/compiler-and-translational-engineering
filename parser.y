%{
// C Definitions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


// AST struct
struct ASTNode{
	char * nodetype; 
	struct ASTNode * left; 
	struct ASTNode * right; 
};

struct numval{
	int nodetype; 
	double number; 
};

struct ASTNode * newast(char * nodetype, struct ASTNode * left, struct ASTNode * right);
struct ASTNode * genAddExpr(char * op, struct ASTNode * left, struct ASTNode * right);
struct ASTNode * newnum(double d); 

double eval(struct ASTNode * node); 

void freetree(struct ASTNode * node); 

// IR Rep
int tempnum = -1; 
void StoreOp(ht_item * entryptr, const char * val, const char * place);
Tree * lhs; 
Tree * rhs;
Tree * op_ptr; 
Tree * expr_ptr;
Tree * prim_ptr; 
int prim_done = 0;

void printExprTree(Tree * root); 

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
	struct ASTNode * a;

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

//type declare the type of semantic values for a non-terminal symbol
%type <strval> id str_literal string_decl
%type <strval> var_type any_type


%%
// Grammar Rules
// Program
// GLOBAL
program: 	_PROG id _BEGIN 
			{ 	ht = ht_new(); 
				scopename = "GLOBAL"; 
				ht_insert(ht, scopename, NULL, NULL, NULL); 
				updateArray(scopename); } 
			pgm_body 
			_END 
			{ 	printArray(); 
				ht_del_hash_table(ht); }
;
id: IDENTIFIER {$$ = $1; }
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
			{ 	if (declare == 1) { 
					ht_insert(ht, symtab[maxind]->key, $1, datatype, NULL);
				} 
			} id_tail
; 
id_tail:	COMMA id 
			{ 	if (declare == 1) {
					ht_insert(ht, symtab[maxind]->key, $2, datatype, NULL);
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
func_decl: _FUNC any_type id { ht_insert(ht, $3, NULL, NULL, NULL); updateArray($3); } OPENPARENT param_decl_list CLOSEPARENT _BEGIN func_body _END
; 
func_body: decl stmt_list
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
assign_stmt: 	assign_expr TERMINATOR { lhs = NULL; rhs = NULL; op_ptr = NULL; expr_ptr = NULL; }
; 
assign_expr:	id ASSIGNMENT expr { if (expr_ptr != NULL) { expr_ptr->right = lhs; printExprTree(expr_ptr); } }
; 
read_stmt: 		_READ OPENPARENT { declare = 0; } id_list CLOSEPARENT TERMINATOR
; 
write_stmt: 	_WRITE OPENPARENT { declare = 0; } id_list CLOSEPARENT TERMINATOR
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
primary: 		OPENPARENT expr CLOSEPARENT 
				| id 					{ 	lhs = new_varleaf(ht, "GLOBAL", $1);  
											if (op_ptr != NULL) {
												if (op_ptr->op == MUL || op_ptr->op == DIV){  
													op_ptr->right = lhs; 	
													lhs = op_ptr; 
													expr_ptr->right = op_ptr;
													lhs = expr_ptr; 
												}
											}
										}	
				| INTLITERAL 			{ 	lhs = new_litleaf($1);
											if (op_ptr != NULL && (op_ptr->op == MUL || op_ptr->op == DIV)){  
												op_ptr->right = lhs; 	
												lhs = op_ptr; 
												expr_ptr->right = op_ptr;
												lhs = expr_ptr; 

											} 
										}	
				| FLOATLITERAL			{	lhs = new_litleaf($1); 
											if (op_ptr != NULL && (op_ptr->op == MUL || op_ptr->op == DIV)){  
												op_ptr->right = lhs; 	
												lhs = op_ptr; 
												expr_ptr->right = op_ptr;
												lhs = expr_ptr;
											} 
										}
; 
addop: 			ADDOP 	{ 	printf("Plus\n");
							//if (op_ptr != NULL) { lhs = op_ptr; }
							rhs = new_opnode(ARITHM_NODE, ((strcmp("+", $1) == 0) ? ADD : SUB), lhs, NULL); 
							//printExprTree(expr_ptr);  
							op_ptr = rhs; 
							expr_ptr = rhs; 
						}
; 
mulop: 			MULOP 	{ 	printf("Multiply\n");
							rhs = new_opnode(ARITHM_NODE, ((strcmp("*", $1) == 0) ? MUL : DIV), lhs, NULL); 
							//printf("NNNNN----%s\n", rhs->left->name); 
							op_ptr = rhs;
							//printf("DDDD  %d\n", expr_ptr->op);
							//printExprTree(expr_ptr);
							if (expr_ptr == NULL){
								expr_ptr = rhs;  
							}
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
	yyparse();

	fclose(yyin); 
	fclose(yyout);
	return 0; 	
}

void printExprTree(Tree * root){
	printf("HEEEERE\n"); 
	ast_traversal(expr_ptr);

	/*if(root->left == NULL){
		printf("At leaf node: "); 
		ast_print_node(root);
		return; 
	}
	printExprTree(expr_ptr); 
	ast_print_node(root); 
	ast_print_node(root->right); 
	*/

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
	for(i = 0; i <= maxind; i++){
		printf("\nScope: %s === ", symtab[i]->key);
		eptr = symtab[i]; 
		while(eptr != NULL){
			printf("%s ", eptr->name); 
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

