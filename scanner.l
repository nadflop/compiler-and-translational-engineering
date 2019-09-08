D	[0-9]
L	[A-Za-z]

%{
#include <stdio.h> 
#include "parser.tab.h" 
%} 

%% 

prog|func|read|write		return KEYWORD;

begin|if|else|while			return KEYWORD; 

end|endif|endwhile			return KEYWORD; 

int|void|str|float			return KEYWORD; 

return						return KEYWORD;

true|false					return BOOLEAN;

==|!=|<|>|<=|>=				return OPERATOR;

=							return OPERATOR; 

[\+\-\*\/]					return OPERATOR;

\(							return OPERATOR; 

\)							return OPERATOR; 

;							return OPERATOR; 

,							return OPERATOR; 

{L}({L}|{D})*				return IDENTIFIER; 

{D}*\.{D}+					{	//yylval.fval = atof(yytext);
								return FLOATLITERAL;	}

{D}+						{	//yylval.fval = atoi(yytext);
								return INTLITERAL;	}

\"[^"]+\"					return STRINGLITERAL;

[ \t\n\r]+ 

#.*			

%% 

