
%{

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include "ast.h"
#include "y.tab.h"

int alloc_sval = 0;

%}

%option yylineno

%%

"while"		{ yylval.ival = yylineno; return TK_WHILE; }

"if"		{ yylval.ival = yylineno; return TK_IF; }

"else"		{ yylval.ival = yylineno; return TK_ELSE; }

"int"		{ yylval.ival = yylineno; return TK_TINT; }

"char"		{ yylval.ival = yylineno; return TK_TCHAR; }

"float"		{ yylval.ival = yylineno; return TK_TFLOAT; }

"return"	{ yylval.ival = yylineno; return TK_RETURN; }

"void"		{ yylval.ival = yylineno; return TK_TVOID; }

[=]		{ yylval.ival = yylineno; return '='; }

[*]		{ yylval.ival = yylineno; return '*'; }

[/]		{ yylval.ival = yylineno; return '/'; }

[+]		{ yylval.ival = yylineno; return '+'; }

[-]		{ yylval.ival = yylineno; return '-'; }

"=="		{ yylval.ival = yylineno; return TK_EQ; }

"!="		{ yylval.ival = yylineno; return TK_NEQ; }

"<="		{ yylval.ival = yylineno; return TK_LEQ; }

">="		{ yylval.ival = yylineno; return TK_GEQ; }

"&&"		{ yylval.ival = yylineno; return TK_AND; }

"||"		{ yylval.ival = yylineno; return TK_OR; }

[<]		{ yylval.ival = yylineno; return '<'; }

[>]		{ yylval.ival = yylineno; return '>'; }

[!]		{ yylval.ival = yylineno; return '!'; }

[(]		{ yylval.ival = yylineno; return '('; }

[)]		{ yylval.ival = yylineno; return ')'; }

[;]		{ yylval.ival = yylineno; return ';'; }

[,]		{ yylval.ival = yylineno; return ','; }

[\[]		{ yylval.ival = yylineno; return '['; }

[\]]		{ yylval.ival = yylineno; return ']'; }

[{]		{ yylval.ival = yylineno; return '{'; }

[}]		{ yylval.ival = yylineno; return '}'; }

"..."		{ return TK_MANY; }

[_a-zA-Z][_0-9a-zA-Z]*	{ 
			  yylval.sval = (char*)malloc(yyleng + 1);
			  if(!yylval.sval) return ERR_MEM;
			  strcpy(yylval.sval, yytext);
			  return TK_ID;
			}

[0-9]+  { 
	  long val;
	  val = strtol(yytext, NULL, 10);
	  if(errno != ERANGE && val <= INT_MAX && val >= INT_MIN) { yylval.ival = (int)val; return TK_INT; } else return ERR_IRANGE;
	}

"0x"[0-9a-fA-F]+ { 
		   long val;
		   val = strtol(yytext+2, NULL, 16);
		   if(errno != ERANGE && val <= INT_MAX && val >= INT_MIN) { yylval.ival = (int)val; return TK_INT; } else return ERR_IRANGE; 
		 }

[0-9]+"."[0-9]*([eE][+-]?[0-9]+)?  |
"."[0-9]+([eE][+-]?[0-9]+)?        |
[0-9]+([eE][+-]?[0-9]+)            {
				     yylval.fval = strtod(yytext, NULL);
				     if(errno != ERANGE) return TK_FLOAT; else return ERR_FRANGE;
                                   }

"\""([^\n\\\"]|"\\n"|"\\t"|"\\\\"|"\\\"")*"\""	{
						  int i, j;
						  i = 1; j = 0;
						  yylval.sval = (char*)malloc(yyleng);
						  if(!yylval.sval) return ERR_MEM;
						  while(i < (yyleng -1)) {
						    if(yytext[i]=='\\' && yytext[i+1]=='n') {
						      yylval.sval[j] = '\n';
						      i++;
						    } else if(yytext[i]=='\\' && yytext[i+1]=='t') {
						      yylval.sval[j] = '\t';
						      i++;
						    } else if(yytext[i]=='\\' && yytext[i+1]=='"') {
						      yylval.sval[j] = '\"';
						      i++;
						    } else if(yytext[i]=='\\' && yytext[i+1]=='\\') {
						      yylval.sval[j] = '\\';
						      i++;
						    } else yylval.sval[j] = yytext[i];
						    i++;
						    j++;
						  }
						  yylval.sval[j] = 0; 
						  return TK_STRING;
						}

"/*"([^*]|[*]+[^/*])*[*]*"*/"   ;

" "|\n|\r|\t ;

.  { return ERR_SCAN; }

%%

int yywrap() { return 1; }

