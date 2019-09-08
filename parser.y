%{
// C Definitions
#include <stdio.h>
#include <stdlib.h>

extern int yylex(); 
extern int yyparse(); 
extern FILE *yyin;
extern FILE *yyout; 

void yyerror(const char *s);

%}
// Bison Definitions
%token KEYWORD IDENTIFIER BOOLEAN OPERATOR INTLITERAL FLOATLITERAL STRINGLITERAL

%%
// Grammar Rules
// Program
program:	KEYWORD id KEYWORD pgm_body KEYWORD
;

id:		IDENTIFIER
; 

pgm_body: 
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
	printf("Parse Error: %s\n", s); 
	fprintf(yyout, "Not Accepted"); 
}
