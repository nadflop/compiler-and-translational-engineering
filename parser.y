%{
// C Definitions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.c"

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

int var_exists = 0;
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
%token <intval> INTLITERAL 
%token <floatval> FLOATLITERAL
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
assign_stmt: 	assign_expr TERMINATOR
; 
assign_expr:	id ASSIGNMENT expr
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
				| id  
				| INTLITERAL
				| FLOATLITERAL
; 
addop: 			ADDOP { $<a>$ = genAddExpr($1, NULL, NULL); }
; 
mulop: 			MULOP
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

struct ASTNode * genAddExpr(char * op, struct ASTNode * left, struct ASTNode * right){
	struct ASTNode * AddExpr = newast(op, NULL, NULL); 

	return AddExpr; 
}

struct ASTNode * newast(char * nodetype, struct ASTNode * left, struct ASTNode * right){
	struct ASTNode * a = malloc(sizeof(struct ASTNode)); 

	if (!a) {
		yyerror("no space\n"); 
		exit(0); 
	}
	a->nodetype = nodetype; 
	a->left = left; 
	a->right = right; 

	return a; 
}

struct ASTNode * newnum(double d){
	struct numval * a = malloc(sizeof(struct numval)); 
	if (!a) {
		yyerror("no space\n"); 
		exit(0); 
	}
	a->nodetype = 'K'; 
	a->number = d; 

	return (struct ASTNode *) a; 
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

