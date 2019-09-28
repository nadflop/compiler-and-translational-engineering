%{
// C Definitions
#include <stdio.h>
#include <stdlib.h>
#define YYSTYPE double

extern int yylex(); 
extern int yyparse();
extern int yylineno;
extern char * yytext;
extern FILE *yyin;
extern FILE *yyout; 

void yyerror(const char *s);

%}
// Bison Definitions
%define parse.error verbose
%locations

//Symbol semantic values

//yystype union
//union declare the collection of data types that semantic val may have
%union {
	int int_val;
	float float_val;
	char * string_val;
}

//token declare a terminal symbol with no precedence or associativity sepcified
%token _PROG _FUNC _READ _WRITE
%token _BEGIN _IF _ELSE _WHILE
%token _END _ENDIF _ENDWHILE
%token _INT _VOID _STRING _FLOAT
%token _RETURN
%token _TRUE _FALSE
%token COMPARATOR ASSIGNMENT
%token ADDOP MULOP
%token TERMINATOR OPENPARENT CLOSEPARENT COMMA
%token IDENTIFIER 
%token INTLITERAL FLOATLITERAL STRINGLITERAL

//type declare the type of semantic values for a nonterminal symbol
%type <string_val> id string_decl var_type STRINGLITERAL
%type <int_val> INTLITERAL
%type <float_val> FLOATLITERAL

%%
// Grammar Rules
// Program
program: 	_PROG id _BEGIN pgm_body _END
;
id:	IDENTIFIER
; 
pgm_body: 	decl func_declarations
;
decl: 	string_decl decl 
		| var_decl decl 
		| 
; 

// Global String Declaration
string_decl: 	_STR id ASSIGNMENT str_literal TERMINATOR
;
str_literal: 	STRINGLITERAL
; 

// Variable Declaration
var_decl: 	var_type id_list TERMINATOR
; 
var_type: 	_FLOAT
			| _INT
; 
any_type: 	var_type
			| _VOID
; 
id_list: 	id id_tail
; 
id_tail:	COMMA id id_tail
			|
;

// Function Parameter List
param_decl_list: 	param_decl param_decl_tail
					|
;
param_decl: 	var_type id
;
param_decl_tail:	COMMA param_decl param_decl_tail
					|
; 

// Function Declarations
func_declarations: 	func_decl func_declarations
					|
; 
func_decl:		_FUNC any_type id OPENPARENT param_decl_list CLOSEPARENT _BEGIN func_body _END
; 
func_body:		decl stmt_list
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
if_stmt: 	_IF OPENPARENT cond CLOSEPARENT decl stmt_list else_part _ENDIF
; 
else_part: 	_ELSE decl stmt_list
			|
; 
cond: 		expr compop expr
			| _TRUE
			| _FALSE
; 
compop: 	COMPARATOR
; 
while_stmt: _WHILE OPENPARENT cond CLOSEPARENT decl stmt_list _ENDWHILE
; 
control_stmt: 	return_stmt
; 
loop_stmt: 		while_stmt
;


%%
// Additional C Code
int main(int argc, char **argv){
	yyin = fopen(argv[1], "r"); 
	yyout = fopen(argv[2], "w");
	
	if (yyparse() == 0){
		fprintf(yyout, "Accepted"); 
	}

	fclose(yyin); 
	fclose(yyout);
	return 0; 	
}

void yyerror(const char *s){
	//printf("%s at Line %d\n%s\n", s, yylineno, yytext); 
	fprintf(yyout, "Not Accepted"); 
}
