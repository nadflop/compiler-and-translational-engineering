D	[0-9]
L	[A-Za-z]

%{
#include <stdio.h>
#include <string.h>
#include "parser.tab.h" 
%}

%option yylineno

%% 

prog	{	yylval.keyword = (char *) strdup(yytext); return _PROG;}
func	{	yylval.keyword = (char *) strdup(yytext); return _FUNC;}
read	{return _READ;}
write	{return _WRITE;}

begin	{	yylval.keyword = (char *) strdup(yytext); return _BEGIN;}
if		{return _IF;}
else	{return _ELSE;}
while	{return _WHILE;}

end			{	yylval.keyword = (char *) strdup(yytext); return _END;}
endif		{return _ENDIF;}
endwhile	{return _ENDWHILE;}

int		{return _INT;}
void	{return _VOID;}
str		{	yylval.datatype = (char *) strdup("STRING"); return _STRING;}
float	{return _FLOAT;}

return	{return _RETURN;}

true	{yylval.strval = (char * ) strdup(yytext); return _TRUE;}
false	{yylval.strval = (char* ) strdup (yytext); return _FALSE;}

==|!=|<|>|<=|>=		{yylval.strval = (char *) strdup(yytext); return COMPARATOR;}
=					{yylval.strval = (char *) strdup(yytext); return ASSIGNMENT;} 

[\+\-]		{	yylval.op = (char *) strdup(yytext); return ADDOP;}
[\*\/]		{	yylval.op = (char *) strdup(yytext); return MULOP;}

\(				{return OPENPARENT;}
\)				{return CLOSEPARENT;} 
;				{return TERMINATOR;} 
,				{return COMMA;} 

{L}({L}|{D})*	{   // insert identifier into symbol table
					yylval.strval = (char *) strdup(yytext); 
					return IDENTIFIER;
				} 

{D}*\.{D}+		{
					yylval.strval = (char *) strdup(yytext);
					return FLOATLITERAL;
				}

{D}+			{ 
					yylval.strval = (char *) strdup(yytext);
					return INTLITERAL;
				}

\"[^"]+\"		{
					yylval.strval = strdup(yytext); 
					return STRINGLITERAL;
				}

[ \t\n\r]+ 

#.*			


%% 

