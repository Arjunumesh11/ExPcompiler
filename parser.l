%{
	#include <stdlib.h>
	#include <stdio.h>
	#include "y.tab.h"
	#include "exprtree.h"
	
    int number;
    char *varname;
%}

%%

[0-9]+		{number = atoi(yytext); yylval.no = CreateTree(number,INTE,"number",NUMBER,NULL,NULL,NULL,NULL); return NUM;}
"begin" 	{return BEGIN0;}
"end" 		{return END;}
"pow" 		{return POW;}
"read" 		{return READ;}
"write" 	{return WRITE;}
"main"		{return MAIN;}
"return" 	{return RETURN;}

"if" 		{return IF;}
"else" 		{return ELSE;}
"then" 		{return THEN;}
"endif" 	{return ENDIF;}
"do" 		{return DO;}
"while" 	{return WHILE; }
"endwhile"  {return ENDWHILE;}
"break" 	{return BREAK;}
"continue"  {return CONTINUE;}
"decl"		{return DECL;}
"enddecl"   {return ENDDECL;}
"int"		{yylval.name=strdup("int");return INT;}
"str"		{yylval.name=strdup("str");return STR;}
"type"		{return TYPE;}
"endtype"	{return ENDTYPE;}
"alloc"		{return ALLOC;}
"dealloc"	{return DEALLOC;}
"initialize"	{return INTIALIZE;}
"class"		{return CLASS;}
"endclass"	{return ENDCLASS;}
"extends"   {return Extends;}
"self"		{yylval.name=strdup("self");return SELF;}
"BRKP"      {return BRKP;}
"new"		{return NEW;}

"+"	{return PLUS;}
"-"	{return MINUS;}
"*"	{return MUL;}
"/"	{return DIV;}
"=" {return EQUAL;}

"==" {return EEQUAL;}
"<=" {return LSE;}
">=" {return GRE;}
">"  {return GRT;}
"<"  {return LST;}
"!=" {return NEQUAL;}
"null"	{return NULL0;}

[A-Za-z][0-9A-Za-z_]* 	{varname=strdup(yytext);    yylval.name =varname; return VAR;}
[\"][0-9A-Za-z .]+[\"] 	{varname=strdup(yytext);    yylval.no = CreateTree(0,STRE,varname,STRI,NULL,NULL,NULL,NULL); return NUM;}
[\n] 	{}
[ \t]	{}
[()]	{return yytext[0];}
"["    {return yytext[0];}
"]"    {return yytext[0];}

","		{return yytext[0];}
";"		{return yytext[0];}
[{}.]	{return yytext[0];}
.	{yyerror("unknown character\n");printf("%s",yytext);exit(1);}

%%

int yywrap(void) {
	return 1;
}