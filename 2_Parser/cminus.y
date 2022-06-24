/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
//to saved ID, NUM value, Type
static char * savedID;
static int savedNum;
static int savedType;

//to prevent Error
static int yylex(void);
int yyerror(char * message);

%}

%token IF ELSE WHILE RETURN INT VOID
%token ID NUM 
%token LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI PLUS MINUS TIMES OVER COMMA ASSIGN
%token NE EQ GT GE LT LE
%token ERROR 

// for conflict
%nonassoc NO_ELSE
%nonassoc ELSE

%% /* Grammar for TINY */

program     : declar_list
                  { savedTree = $1;} 
            ;
declar_list : declar_list declar
                  { YYSTYPE t = $1;
                    if(t != NULL){
                      while (t->sibling != NULL) t = t->sibling;
                      t->sibling = $2;
                      $$ = $1;
                    }
                    else $$ = $2;
                  }
            | declar
                 { $$ = $1; }
            ;
declar      : var_declar
                 { $$ = $1; }
            | fun_declar
                 { $$ = $1; }
            ;
var_declar  : type_spec identifier SEMI
                 { $$ = newDeclarNode(VarK); 
                   $$->child[0] = $1;
                   $$->attr.id = savedID;
                   $$->type = savedType;
                   $$->lineno = lineno;
                 }
            | type_spec identifier LBRACE num RBRACE SEMI
                 { $$ = newDeclarNode(ArrVarK);
                   $$->attr.id = savedID;
                   $$->type = savedType +2;
                   $$-> child[0] = newExpNode(ConstK);
                   $$-> child[0]->attr.val=savedNum;
                   $$->lineno = lineno;
                 }
            ;
identifier   : ID 
                 { savedID = copyString(tokenString);
                   savedLineNo = lineno;}
            ;
num         : NUM 
                 { savedNum = atoi(tokenString); 
                   savedLineNo = lineno;}
            ;
type_spec   : INT
                 { savedType = Integer; }
            | VOID
                 { savedType = Void; }
            ;
fun_declar  : type_spec identifier
                 { $$ = newDeclarNode(FunK);
                   $$->attr.id = savedID;
                   $$->type = savedType;
                   $$->lineno = lineno;
                 }
                 LPAREN params RPAREN comp_stmt
                 {
                   $$ = $3;
                   $$->child[0] = $5;
                   $$->child[1] = $7;
                 }
            ;
params      : param_list
                 { $$ = $1;}
            | VOID
                 { $$ = newDeclarNode(ParamVoidK); }
            ;
param_list  : param_list COMMA param
                 { YYSTYPE t = $1;
                   if(t != NULL)
                   { while (t->sibling != NULL)
                       t = t->sibling;
                     t->sibling = $3;
                     $$ = $1;
                   }
                   else $$ = $3;
                 }
            | param
                 { $$ = $1; }
            ;
param       : type_spec identifier
                 { $$ = newDeclarNode(ParamK);
                   $$->attr.id = savedID;
                   $$->type = savedType;
                   $$->lineno = lineno;
                 }
            | type_spec identifier LBRACE RBRACE
                 { $$ = newDeclarNode(ParamK);
                   $$->attr.id = savedID;
                   $$->type = savedType+2;
                   $$->lineno = lineno;
                 }
            ;
comp_stmt   : LCURLY local_declar stmt_list RCURLY
                 { $$ = newStmtNode(CompK);
                   $$->child[0] = $2;
                   $$->child[1] = $3;
                   $$->lineno = lineno;
                 }
            ;
local_declar  : local_declar var_declar
                 { YYSTYPE t = $1;
                   if( t != NULL )
                   { while (t->sibling != NULL)
                       t = t->sibling;
                     t->sibling = $2;
                     $$ = $1;
                   }
                   else $$ = $2;
                 }
              |  {$$ = NULL;}
              ;
stmt_list   : stmt_list stmt
                 { YYSTYPE t = $1;
                   if(t != NULL)
                   { while (t->sibling != NULL)
                       t = t->sibling;
                     t->sibling = $2;
                     $$ = $1;
                   }
                   else $$ = $2;
                 }
            |    {$$ = NULL;}
            ;
stmt        : exp_stmt {$$ = $1;}
            | comp_stmt {$$ = $1;}
            | select_stmt {$$ = $1;}
            | iter_stmt {$$ = $1;}
            | return_stmt {$$ = $1;}
            ;
exp_stmt    : exp SEMI
                 { $$ = $1;
                 }
            | SEMI
                 { $$ = NULL;
                 }
            ;
select_stmt : IF LPAREN exp RPAREN stmt %prec NO_ELSE
                 { $$ = newStmtNode(IfK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->lineno = lineno;
                 }
            | IF LPAREN exp RPAREN stmt ELSE stmt
                 { $$ = newStmtNode(IfElseK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->child[2] = $7;
                   $$->lineno = lineno;
                 }
            ;
iter_stmt   : WHILE LPAREN exp RPAREN stmt
                 { $$ = newStmtNode(WhileK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->lineno = lineno;
                 }
            ;
return_stmt : RETURN SEMI
                 { $$ = newStmtNode(ReturnOnlyK);
                   $$->lineno = lineno;
                 }
            | RETURN exp SEMI
                 { $$ = newStmtNode(ReturnK);
                   $$->child[0] = $2;
                   $$->lineno = lineno;
                 }
            ;
exp         : var ASSIGN exp
                 { $$ = newExpNode(AssignK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->lineno = lineno;
                 }
            | simp_exp
                 { $$ =$1; }
            ;
var         : identifier
                 { $$ = newExpNode(IDK);
                   $$->attr.id = savedID;
                   $$->lineno = lineno;
                 }
            | identifier
                 { $$ = newExpNode(ArrIDK);
                   $$->attr.id = savedID;
                   $$->lineno = lineno;
                 }
                 LBRACE exp RBRACE
                 {
                   $$=$2;
                   $$->child[0] = $4;
                 }
            ;
simp_exp    : add_exp reolp add_exp
                 { $$ = $2;
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                 }
            | add_exp
                 { $$ = $1;}
            ;
reolp       : EQ
                 { $$ = newExpNode(OpK);
                   $$->attr.op = EQ;
                   $$->lineno = lineno;
                 }
            | NE
                 { $$ = newExpNode(OpK);
                   $$->attr.op = NE;
                   $$->lineno = lineno;
                 }
            | LT
                 { $$ = newExpNode(OpK);
                   $$->attr.op = LT;
                   $$->lineno = lineno;
                 }
            | LE
                 { $$ = newExpNode(OpK);
                   $$->attr.op = LE;
                   $$->lineno = lineno;
                 }
            | GT
                 { $$ = newExpNode(OpK);
                   $$->attr.op = GT;
                   $$->lineno = lineno;
                 }
            | GE
                 { $$ = newExpNode(OpK);
                   $$->attr.op = GE;
                   $$->lineno = lineno;
                 }
            ;
add_exp     : add_exp addop term
                 { $$ = $2;
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                 }
            | term
                 { $$ = $1; }
            ;
addop       : PLUS
                 { $$ = newExpNode(OpK);
                   $$->attr.op = PLUS;
                   $$->lineno = lineno;
                 }
            | MINUS
                 { $$ = newExpNode(OpK);
                   $$->attr.op = MINUS;
                   $$->lineno = lineno;
                 }
            ;
term        : term mulop factor
                 { $$ = $2;
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                 }
            | factor
                 { $$ = $1;}
            ;
mulop       : TIMES
                 { $$ = newExpNode(OpK);
                   $$->attr.op = TIMES;
                   $$->lineno = lineno;
                 }
            | OVER
                 { $$ = newExpNode(OpK);
                   $$->attr.op = OVER;
                   $$->lineno = lineno;
                 }
            ;
factor      : LPAREN exp RPAREN
                 { $$ = $2;}
            | var
                 { $$ = $1;}
            | call
                 { $$ = $1; }
            | num
                 { $$ = newExpNode(ConstK);
                   $$->attr.val = savedNum;
                   $$->lineno = lineno;
                 }
            ;
call        : identifier
                 { $$ = newExpNode(CallK);
                   $$->attr.id = savedID;
                   $$->lineno = lineno;
                 }
                 LPAREN args RPAREN{
                   $$=$2;
                   $$->child[0] = $4;
                 }
            ;
args        : arg_list
                 { $$ = $1;}
            | {$$ = NULL;}
            ;
arg_list    : arg_list COMMA exp
                 { YYSTYPE t = $1;
                   if(t != NULL)
                   { while (t->sibling != NULL)
                       t = t->sibling;
                     t->sibling = $3;
                     $$ = $1;
                   }
                   else $$ = $3;
                 }
            | exp
                 { $$ = $1; }
            ;
%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

