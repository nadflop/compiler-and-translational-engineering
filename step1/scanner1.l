D	[0-9]
L	[A-Za-z]
W	[ \t\r\n]*

%{
#include <stdio.h>
void scannerOutput(char * tokenType);
%}

%%

(prog|func|read|write)		scannerOutput("KEYWORD");
(if|else|while|begin|for)	scannerOutput("KEYWORD");
(in|range|continue)			scannerOutput("KEYWORD");
(end|endif|endwhile|endfor)	scannerOutput("KEYWORD");
(int|char|void|str|float)	scannerOutput("KEYWORD");
(return|break)				scannerOutput("KEYWORD");
(==|!=|<|>|<=|>=)			scannerOutput("OPERATOR");
(&&|\|\||!)					scannerOutput("OPERATOR");
(&|\||<<|>>|~|\^)			scannerOutput("OPERATOR");
(=|\+=|\-=|\*=|\/=|%=)		scannerOutput("OPERATOR");
(\+|\-|\*|\/|%)				scannerOutput("OPERATOR");
\(							scannerOutput("OPERATOR");
\)							scannerOutput("OPERATOR");
;							scannerOutput("OPERATOR");
,							scannerOutput("OPERATOR");
{L}({L}|{D})*				scannerOutput("IDENTIFIER");
{D}*\.{D}+					scannerOutput("FLOATLITERAL");
{D}+						scannerOutput("INTLITERAL");
\"[^"]+\"					scannerOutput("STRINGLITERAL");
{W}							
#.*

%%

//
//general format:
//	{definitions}
//	%%
//	{rules}
//	%%
//	{user subroutines}
//

void scannerOutput (char * tokenType){
	fprintf(yyout, "Token Type: %s\n", tokenType);
	fprintf(yyout, "Value: %s\n", yytext);
	return;
}

int main(int argc, char ** argv){
	yyin = fopen(argv[1], "r");
	yyout = fopen(argv[2], "w");

	yylex();

	fclose(yyin);
	fclose(yyout);

	return 0;
}
