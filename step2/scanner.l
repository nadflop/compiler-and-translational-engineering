D	[0-9]
L	[A-Za-z]

%{
#include <stdio.h> 
#include "parser.tab.h" 
%}

%option yylineno

%% 

prog	return _PROG;
func	return _FUNC;
read	return _READ;
write	return _WRITE;

begin	return _BEGIN;
if		return _IF;
else	return _ELSE;
while	return _WHILE;

end			return _END;
endif		return _ENDIF;
endwhile	return _ENDWHILE;

int		return INT;
void	return _VOID;
str		return STRING;
float	return FLOAT;

return	return _RETURN;

true	return _TRUE;
false	return _FALSE;

==|!=|<|>|<=|>=		return COMPARATOR;
=					return ASSIGNMENT; 

[\+\-]		return ADDOP;
[\*\/]		return MULOP;

\(				return OPENPARENT; 
\)				return CLOSEPARENT; 
;				return TERMINATOR; 
,				return COMMA; 

{L}({L}|{D})*	return IDENTIFIER; 
{D}*\.{D}+		return FLOATLITERAL;
{D}+			return INTLITERAL;	
\"[^"]+\"		return STRINGLITERAL;

[ \t\n\r]+ 

#.*


%% 

