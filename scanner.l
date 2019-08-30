D	[0-9]
L	[A-Za-z]

%{
#include <stdio.h>
int count = 0; 
void WriteOutput(char * tokentype); 
%} 

%% 

(prog|func|read|write)			WriteOutput("KEYWORD"); 

(begin|if|else|while|for)		WriteOutput("KEYWORD"); 

(end|endif|endwhile|endfor)		WriteOutput("KEYWORD"); 

(int|void|str|float)			WriteOutput("KEYWORD"); 

(return|break)					WriteOutput("KEYWORD");

(==|!=|<|>|<=|>=)				WriteOutput("OPERATOR");

=								WriteOutput("OPERATOR"); 

[\+\-\*\/]						WriteOutput("OPERATOR");

\(								WriteOutput("OPERATOR"); 

\)								WriteOutput("OPERATOR"); 

;								WriteOutput("OPERATOR"); 

,								WriteOutput("OPERATOR"); 

{L}({L}|{D})*					WriteOutput("IDENTIFIER"); 

{D}*\.{D}+						WriteOutput("FLOATLITERAL");

{D}+							WriteOutput("INTLITERAL");

\"[^"]+\"						WriteOutput("STRINGLITERAL");

#.*			

[^0-9]

%% 

void WriteOutput (char * tokentype){
	fprintf(yyout, "Token Type: %s\n", tokentype); 
	fprintf(yyout, "Value: %s\n", yytext); 
	return; 
}

int main(int argc, char** argv){

	// argv[1]: input file name
	// argv[2]: output file name

	yyin = fopen(argv[1], "r"); 
	yyout = fopen(argv[2], "w"); 

	yylex(); 
	
	fclose(yyin); 
	fclose(yyout); 

	return 0; 
}
