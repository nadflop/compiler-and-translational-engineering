D	[0-9]
L	[A-Za-z]
W	[ \t]*

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
