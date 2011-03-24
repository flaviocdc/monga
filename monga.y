%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtab.h"

int yylex (void);
void yyerror (char const *);

DeclrListNode* ast;

extern int yylineno;

Type tvoid = { 0, 0, 0, NULL };

%}

%union {
  double fval;
  int ival;
  char* sval;
  Declr* declr;
  StrListNode* slist;
  Type* type;
  Block* block;
  DeclrListNode* dlist;
  CommListNode* clist;
  Command* comm;
  Exp* exp;
  Var* var;
  ExpListNode* elist;
}

%token <ival>	TK_INT
%token <fval>	TK_FLOAT
%token <sval>	TK_STRING
%token <sval>	TK_ID
%token <ival>	TK_WHILE
%token <ival>	TK_IF
%token <ival>	TK_ELSE
%token <ival>	TK_TINT
%token <ival>	TK_TFLOAT
%token <ival>	TK_TCHAR
%token <ival>	TK_RETURN
%token <ival>	TK_TVOID
%token <ival>	TK_EQ
%token <ival>	TK_LEQ
%token <ival>	TK_NEQ
%token <ival>	TK_GEQ
%token <ival>	TK_AND
%token <ival>	TK_OR
%token 		TK_MANY

%token 		ERR_SCAN
%token 		ERR_MEM
%token 		ERR_IRANGE
%token 		ERR_FRANGE

%type <declr>	declr_func param
%type <slist>	list_names
%type <type>	type
%type <ival>	base_type
%type <block>	block
%type <dlist>	declr declr_var prog declr_varl paraml params
%type <clist>	commandl
%type <comm>	command
%type <exp>	exp funcall
%type <var>	var
%type <elist>	expl nexpl

%token <ival>	TK_SEMIC ';'
%token <ival>   TK_LBRACK '['
%token <ival>   TK_RBRACK ']'
%token <ival>   TK_LPAR '(' 
%token <ival>   TK_RPAR ')' 
%token <ival>   TK_LCBRACK '{'
%token <ival>   TK_RCBRACK '}'
%token <ival>	TK_ASSIG '=' 
%token <ival>	TK_TIMES '*' 
%token <ival>	TK_DIV '/' 
%token <ival>	TK_PLUS '+'
%token <ival>	TK_MINUS '-'
%token <ival>	TK_LT '<' 
%token <ival>	TK_GT '>' 
%token <ival>	TK_LNEG '!'
%token <ival>	TK_COMMA ',' 

%left TK_OR
%left TK_AND
%left TK_LEQ TK_GEQ '<' '>' TK_NEQ TK_EQ
%nonassoc '!'
%left '-' '+'
%left '*' '/'
%nonassoc UNM

%start prog

%%

prog: 		/* empty */	{ $$ = NULL; }
		| declr prog	{
				  DeclrListNode *declrs = $1;
                                  while(declrs->next) declrs = declrs->next;
				  declrs->next = $2;
				  $$ = $1;
				  ast = $$;
				}
		;

declr:		declr_var	{ $$ = $1; }
		| declr_func	{ 
				  ALLOC($$, DeclrListNode);
				  $$->declr = $1;
				  $$->next = NULL;
				}
		;

declr_var:	type list_names ';'	{
					  FILE *log;
					  StrListNode *declr;
					  DeclrListNode *list1, *list2;
					  list1 = NULL;
					  declr = $2;
					  do {
					    list2 = list1;
					    ALLOC(list1, DeclrListNode);
					    ALLOC(list1->declr, Declr);
					    list1->next = list2;
					    list1->declr->tag = DECLR_VAR;
					    list1->declr->type = $1;
					    list1->declr->line = $3;
					    list1->declr->u.name = declr->name;
					  } while(declr = declr->next);
					  $$ = list1;
					}
		;

list_names:	TK_ID	{
			  ALLOC($$, StrListNode);
			  ALLOCS($$->name, strlen($1) + 1);
			  strcpy($$->name, $1);
			  $$->next = NULL;
			}
		| list_names ',' TK_ID	{
					  ALLOC($$, StrListNode);
					  ALLOCS($$->name, strlen($3) + 1);
					  strcpy($$->name, $3);
					  $$->next = $1;
					}
		;

type:		base_type	{
				  ALLOC($$, Type);
				  $$->type = $1;
				  $$->dimensions = 0;
				  $$->sizes = NULL;
				}
		| type '[' ']'	{
				  $1->dimensions = $1->dimensions + 1;
				  IntListNode* iln;
				  ALLOC(iln, IntListNode);
				  iln->n = 0;
				  iln->next = NULL;
				  if($1->sizes) {
				    IntListNode* last = $1->sizes;
				    while(last->next) last = last->next;
				    last->next = iln;
				  } else {
				    $1->sizes = iln;
				  }
				  $1->line = $2;
				  $$ = $1;
				}
		| type '[' TK_INT ']'	{
					  IntListNode* iln;
					  $1->dimensions = $1->dimensions + 1;
					  ALLOC(iln, IntListNode);
					  iln->n = $3;
					  iln->next = NULL;
					  if($1->sizes) {
					    IntListNode* last = $1->sizes;
					    while(last->next) last = last->next;
					    last->next = iln;
					  } else {
					    $1->sizes = iln;
					  }
					  $1->line = $2;
					  $$ = $1;
					}
		;

base_type:	TK_TINT		{ $$ = TK_TINT; }
		| TK_TFLOAT	{ $$ = TK_TFLOAT; }
		| TK_TCHAR	{ $$ = TK_TCHAR; }
		;

declr_func:	type TK_ID '(' params ')' block		{
							  ALLOC($$, Declr);
							  $$->tag = DECLR_FUNC;
							  $$->type = $1;
							  $$->line = $3;
							  ALLOCS($$->u.func.name, strlen($2) + 1);
							  strcpy($$->u.func.name, $2);
							  $$->u.func.params = $4;
							  $$->u.func.block = $6;
							}
		| TK_TVOID TK_ID '(' params ')' block	{
							  ALLOC($$, Declr);
                                                          tvoid.type = TK_TVOID;
							  $$->tag = DECLR_FUNC;
							  $$->type = &tvoid;
							  $$->line = $3;
							  ALLOCS($$->u.func.name, strlen($2) + 1);
							  strcpy($$->u.func.name, $2);
							  $$->u.func.params = $4;
							  $$->u.func.block = $6;
							}
		;

params:		/* empty */	{ $$ = NULL; }
		| paraml	{ $$ = $1; }
		;

paraml:		param		{
				  ALLOC($$, DeclrListNode);
				  $$->declr = $1;
				  $$->next = NULL;
				}
		| param ',' paraml	{
					  ALLOC($$, DeclrListNode);
					  $$->declr = $1;
					  $$->next = $3;
					}
		;

param:		TK_MANY		{ $$ = NULL; }
		| type TK_ID	{
				  ALLOC($$, Declr);
				  $$->tag = DECLR_VAR;
				  $$->type = $1;
				  ALLOCS($$->u.name, strlen($2)+1);
				  strcpy($$->u.name, $2);
				}
		;

block:		';'		{ $$ = NULL; }
		| '{' '}'	{
				  $$ = ALLOC($$, Block);
				  $$->declrs = NULL;
				  $$->comms = NULL;
				}
		| '{' declr_varl '}'	{
					  ALLOC($$, Block);
					  $$->declrs = $2;
					  $$->comms = NULL;
					}
		| '{' commandl '}'	{
					  ALLOC($$, Block);
					  $$->declrs = NULL;
					  $$->comms = $2;
					}	
		| '{' declr_varl commandl '}'	{
						  ALLOC($$, Block);
						  $$->declrs = $2;
						  $$->comms = $3;
						}	
		;

declr_varl:	declr_var	{ $$ = $1; }
		| declr_var declr_varl	{
					  DeclrListNode *list;
					  list = $1;
					  while(list->next) list = list->next;
					  list->next = $2;
					  $$ = $1;
					}
		;

commandl:	command		{
				  ALLOC($$, CommListNode);
				  $$->comm = $1;
				  $$->next = NULL;
				}	
		| command commandl	{
					  ALLOC($$, CommListNode);
					  $$->comm = $1;
					  $$->next = $2;
					}	
		;

command:	TK_IF '(' exp ')' command	{
						  ALLOC($$, Command);
						  $$->tag = COMMAND_IF;
						  $$->line = $1;
						  $$->u.cif.exp = $3;
						  $$->u.cif.comm = $5;
						  $$->u.cif.celse = NULL;
						}
		| TK_IF '(' exp ')' command TK_ELSE command	{
								  ALLOC($$, Command);
								  $$->tag = COMMAND_IF;
								  $$->line = $1;
								  $$->u.cif.exp = $3;
								  $$->u.cif.comm = $5;
								  $$->u.cif.celse = $7;
								}
		| TK_WHILE '(' exp ')' command	{
						  ALLOC($$, Command);
						  $$->tag = COMMAND_WHILE;
						  $$->line = $1;
						  $$->u.cwhile.exp = $3;
						  $$->u.cwhile.comm = $5;
						}
		| var '=' exp ';'	{
					  ALLOC($$, Command);
					  $$->tag = COMMAND_ATTR;
					  $$->line = $2;
					  $$->u.attr.lvalue = $1;
					  $$->u.attr.rvalue = $3;
					}
		| TK_RETURN ';'		{
					  ALLOC($$, Command);
					  $$->tag = COMMAND_RET;
					  $$->line = $1;
					  $$->u.ret = NULL;
					}
		| TK_RETURN exp ';'	{
					  ALLOC($$, Command);
					  $$->tag = COMMAND_RET;
					  $$->line = $1;
					  $$->u.ret = $2;
					}
		| funcall ';'	{
				  ALLOC($$, Command);
				  $$->tag = COMMAND_FUNCALL;
				  $$->line = $1->line;
				  $$->u.funcall = $1;
				}
		| block	{
			  ALLOC($$, Command);
			  $$->tag = COMMAND_BLOCK;
			  $$->u.block = $1;
			}
		;

var:		TK_ID	{
			  ALLOC($$, Var);
			  ALLOCS($$->name, strlen($1)+1);
			  strcpy($$->name, $1);
			  $$->line = yylineno;
			  $$->idxs = NULL;
			}
		| var '[' exp ']'	{
					  ExpListNode* eln;
					  $$ = $1;
					  ALLOC(eln, ExpListNode);
					  eln->exp = $3;
					  eln->next = NULL;
					  if($$->idxs) {
					    ExpListNode* last = $$->idxs;
					    while(last->next) last = last->next;
					    last->next = eln;
					  } else $$->idxs = eln;
					}
		;

exp:		TK_INT	{
			  ALLOC($$, Exp);
			  $$->tag = EXP_INT;
			  $$->u.ival = $1;
			}
		| TK_FLOAT	{
				  ALLOC($$, Exp);
				  $$->tag = EXP_FLOAT;
				  $$->u.fval = $1;
				}
		| TK_STRING	{
				  ALLOC($$, Exp);
				  $$->tag = EXP_STRING;
				  ALLOCS($$->u.sval, strlen($1)+1);
				  strcpy($$->u.sval, $1);
				}
		| var	{
			  ALLOC($$, Exp);
			  $$->tag = EXP_VAR;
			  $$->line = $1->line;
			  $$->u.var = $1;
			}
		| '(' exp ')'	{ $$ = $2; }
		| funcall	{ $$ = $1; }
		| '-' exp %prec UNM	{
					  ALLOC($$, Exp);
					  switch($2->tag) {
					    case EXP_INT: { $2->u.ival = -$2->u.ival; $$ = $2; break; }
					    case EXP_FLOAT: { $2->u.fval = -$2->u.fval; $$ = $2; break; }
					    default: {
					      ALLOC($$, Exp);
					      $$->tag = EXP_NEG;
					      $$->line = $1;
					      $$->u.exp = $2;
					    }
					  }
					}
		| exp '+' exp	{
				  ALLOC($$, Exp);
				  $$->tag = EXP_BINOP;
				  $$->line = $2;
				  $$->u.binop.op = '+';
				  $$->u.binop.e1 = $1;
				  $$->u.binop.e2 = $3;
				}
		| exp '-' exp	{
				  ALLOC($$, Exp);
				  $$->tag = EXP_BINOP;
				  $$->line = $2;
				  $$->u.binop.op = '-';
				  $$->u.binop.e1 = $1;
				  $$->u.binop.e2 = $3;
				}
		| exp '*' exp 	{
				  ALLOC($$, Exp);
				  $$->tag = EXP_BINOP;
				  $$->line = $2;
				  $$->u.binop.op = '*';
				  $$->u.binop.e1 = $1;
				  $$->u.binop.e2 = $3;
				}	
		| exp '/' exp	{
				  ALLOC($$, Exp);
				  $$->tag = EXP_BINOP;
				  $$->line = $2;
				  $$->u.binop.op = '/';
				  $$->u.binop.e1 = $1;
				  $$->u.binop.e2 = $3;
				}	
		| exp TK_EQ exp	{
				  ALLOC($$, Exp);
				  $$->tag = EXP_BINOP;
				  $$->line = $2;
				  $$->u.binop.op = TK_EQ;
				  $$->u.binop.e1 = $1;
				  $$->u.binop.e2 = $3;
				}
		| exp TK_LEQ exp	{
					  ALLOC($$, Exp);
					  $$->tag = EXP_BINOP;
					  $$->line = $2;
					  $$->u.binop.op = TK_LEQ;
					  $$->u.binop.e1 = $1;
					  $$->u.binop.e2 = $3;
					}	
		| exp TK_GEQ exp	{
					  ALLOC($$, Exp);
					  $$->tag = EXP_BINOP;
					  $$->line = $2;
					  $$->u.binop.op = TK_GEQ;
					  $$->u.binop.e1 = $1;
					  $$->u.binop.e2 = $3;
					}	
		| exp TK_NEQ exp	{
					  ALLOC($$, Exp);
					  $$->tag = EXP_BINOP;
					  $$->line = $2;
					  $$->u.binop.op = TK_NEQ;
					  $$->u.binop.e1 = $1;
					  $$->u.binop.e2 = $3;
					}	
		| exp '<' exp	{
				  ALLOC($$, Exp);
				  $$->tag = EXP_BINOP;
				  $$->line = $2;
				  $$->u.binop.op = '<';
				  $$->u.binop.e1 = $1;
				  $$->u.binop.e2 = $3;
				}	
		| exp '>' exp	{
				  ALLOC($$, Exp);
				  $$->tag = EXP_BINOP;
				  $$->line = $2;
				  $$->u.binop.op = '>';
				  $$->u.binop.e1 = $1;
				  $$->u.binop.e2 = $3;
				}	
		| '!' exp	{
				  ALLOC($$, Exp);
				  $$->tag = EXP_LNEG;
				  $$->line = $1;
				  $$->u.exp = $2;
				}	
		| exp TK_AND exp	{
					  ALLOC($$, Exp);
					  $$->tag = EXP_BINOP;
					  $$->line = $2;
					  $$->u.binop.op = TK_AND;
					  $$->u.binop.e1 = $1;
					  $$->u.binop.e2 = $3;
					}	
		| exp TK_OR exp	{
				  ALLOC($$, Exp);
				  $$->tag = EXP_BINOP;
				  $$->line = $2;
				  $$->u.binop.op = TK_OR;
				  $$->u.binop.e1 = $1;
				  $$->u.binop.e2 = $3;
				}	
		;

funcall:	TK_ID '(' expl ')'	{
					  ALLOC($$, Exp);
					  $$->tag = EXP_FUNCALL;
					  $$->line = $2;
					  ALLOCS($$->u.funcall.name, strlen($1)+1);
					  strcpy($$->u.funcall.name, $1);
					  $$->u.funcall.expl = $3;
					}
		;

expl:		/* empty */	{ $$ = NULL; }
		| nexpl		{ $$ = $1; }
		;

nexpl:		exp	{
			  ALLOC($$, ExpListNode);
			  $$->exp = $1;
			  $$->next = NULL;
			}
		| exp ',' nexpl	{
				  ALLOC($$, ExpListNode);
				  $$->exp = $1;
				  $$->next = $3;
				}
		;

%%

void yyerror (char const *s) {
  print_error(s, yylineno);
}


