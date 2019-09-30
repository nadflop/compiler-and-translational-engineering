%{
// C Definitions
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define YYSTYPE double
#define MAX 100
//#include "symtab.c"
//#include "symtab.h"


extern int yylex(); 
extern int yyparse();
extern int yylineno;
extern char * yytext;
extern int yyleng; 
extern FILE *yyin;
extern FILE *yyout; 
extern YYSTYPE yylval; 

char * inputfile; 
char * outputfile; 

int scope = 0; 
int blocknum = 0;   
int datatype = 0; // 1 int, 2 float, 3 void, 4 str

int var_exists = 0;
int declare = 0; 
int maxind = -1; 
char var[20][10];

void yyerror(const char *s);
void scope_incr(int i); 
void scope_decr(int i);
void block_incr(void); 
void printVarDecl(void); 

void varCheck(void); 

void detDataType(void); 
void printName(void); 
void printDataType(void);
void printValue(void); 

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
	char * operator; 
	char * keyword;
	char * bool_val; 
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
%token <operator> ADDOP MULOP
%token <operator> TERMINATOR OPENPARENT CLOSEPARENT COMMA
%token <strval> IDENTIFIER 
%token <strval> STRINGLITERAL
%token <intval> INTLITERAL 
%token <floatval> FLOATLITERAL


//type declare the type of semantic values for a non-terminal symbol
%type <strval> id any_type var_type str_literal


%%
// Grammar Rules
// Program
// GLOBAL
program: _PROG id _BEGIN { fprintf(yyout, "Symbol table GLOBAL\n"); } pgm_body _END 
;
id: IDENTIFIER 
; 
pgm_body: 	decl { datatype = 0; }func_declarations
;
decl: 	string_decl decl 
		| var_decl decl 
		| 
; 

// String Declaration
string_decl: _STRING { declare = 1; detDataType(); } id { varCheck(); printName(); printDataType(); } ASSIGNMENT str_literal { printValue(); var_exists = 0; declare = 0; } TERMINATOR
;
str_literal: 	STRINGLITERAL
; 

// Variable Declaration
var_decl: { declare = 1; } var_type { detDataType(); } id_list { declare = 0; }TERMINATOR
; 
var_type: 	_FLOAT
			| _INT
; 
any_type: 	var_type
			| _VOID
; 
id_list: 	id { varCheck(); printName(); printDataType(); var_exists = 0; } id_tail
; 
id_tail:	COMMA id { varCheck(); printName(); printDataType(); var_exists = 0; } id_tail
			|
;

// Function Parameter List
param_decl_list:  	param_decl param_decl_tail
					|
;
param_decl: 	{ declare = 1; } var_type { detDataType(); } id { printName(); printDataType(); var_exists = 0; declare = 0; }
;
param_decl_tail:	COMMA param_decl param_decl_tail
					|
; 

// Function Declarations
func_declarations: 	func_decl func_declarations
					|
; 
func_decl: _FUNC any_type id { fprintf(yyout, "\nSymbol table %s\n", yytext);} OPENPARENT param_decl_list CLOSEPARENT _BEGIN func_body _END
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
read_stmt: 		_READ OPENPARENT id_list CLOSEPARENT TERMINATOR
; 
write_stmt: 	_WRITE OPENPARENT id_list CLOSEPARENT TERMINATOR
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
addop: 			ADDOP
; 
mulop: 			MULOP
; 

// Complex Statements and Condition
if_stmt: 	_IF OPENPARENT cond CLOSEPARENT { block_incr(); } decl stmt_list else_part _ENDIF
; 
else_part: 	_ELSE {	block_incr(); } decl stmt_list
			|
; 
cond: 		expr compop expr
			| _TRUE
			| _FALSE
; 
compop: 	COMPARATOR
; 
while_stmt: _WHILE OPENPARENT cond CLOSEPARENT decl { block_incr(); }stmt_list _ENDWHILE
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

/*
void printVarDecl(void){ // pass in data type
	if (type == "STRING"){
		printf("name %s type %s value %s\n");
	}
	else{
		if (value == NULL){
			printf("name %s type %s\n"); 
		}
		else {
			printf("name %s type %s value %s\n"); 
		}
	}
	value = NULL;
	return;
}
*/

void varCheck(){
	if (maxind == -1){
		strcpy(var[0], yytext);
		maxind++;
		//printf("First VAR %s\n", var[0]);
		return; 
	}
	else {
		int i;  
		//printf("checking new var %s\n", yytext); 
		//printf("declare = %d\n", declare); 
		//printf("IN LOOP: ");
		for(i = 0; i <= maxind; i++){
			//printf("%i. %s ", i, var[i]); 
			if(strcmp(var[i], yytext) == 0){
				//printf("\nvar already exists! datatype = %d\n", datatype); 
				if (declare) {
					if (datatype == 0){
						var_exists = 1; 
						break; 
					}
					else{
						//printf("var already declared!\n"); 
						fclose(yyout); 
						yyout = fopen(outputfile, "w"); 
						fprintf(yyout, "DECLARATION ERROR %s\n", yytext);
						fclose(yyin); 
						fclose(yyout); 
						exit(0); 
					}
				}
			}
		}
		//printf("\n"); 
		if(!var_exists){
			maxind++;
			//printf("maxind %d\n", maxind); 
			strcpy(var[maxind], yytext);
			//printf("%s\n", var[maxind]); 
		}
	}
}

void printName(void){ 
	if (!var_exists && (datatype != 0) && declare){
		fprintf(yyout, "name %s ", yytext);
	}
}

void printDataType(void){
	if (!var_exists && (datatype != 0) && declare){ 
		if (datatype == 1){
			fprintf(yyout, "type INT\n"); 
		}
		else if (datatype == 2){
			fprintf(yyout, "type FLOAT\n"); 
		}
		else if (datatype == 3){
			fprintf(yyout, "type VOID\n");
		}
		else if (datatype == 4){
			fprintf(yyout, "type STRING "); 
		}
	}
}

void printValue(void){
	if (!var_exists && declare){
		fprintf(yyout, "value %s\n", yytext); 
	}
}

void detDataType(void){
	if (strcmp(yytext, "int") == 0){
		datatype = 1; 
	}
	else if (strcmp(yytext, "float") == 0){ 
		datatype = 2; 
	}
	else if (strcmp(yytext, "void") == 0){
		datatype = 3; 
	}
	else if (strcmp(yytext, "str") == 0){
		datatype = 4; 
	}
	else{
		datatype = 0; 
	}
}

void block_incr(void){
	blocknum++;
	fprintf(yyout, "\nSymbol table BLOCK %d\n", blocknum); 
}

void scope_incr(int i){ 
	scope = scope + i; 
	printf("Scope Increased: %d\n", scope); 
}

void scope_decr(int i){
	scope = scope - i; 
	printf("Scope Decreased: %d\n", scope); 
}

void yyerror(const char *s){
	printf("%s at Line %d\n%s\n", s, yylineno, yytext); 
	//fprintf(yyout, "Not Accepted"); 
	//print the var_name
	//fprintf(yyout, "DECLARATION ERROR %s", yytext);
	return; 
}

