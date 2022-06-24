/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

/* counter for variable memory locations */
static int location = 0;
static ScopeList globalScope = NULL;
static char * functionId;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

static void undefinedError(TreeNode *t){
  Error=TRUE;
  if (t->kind.exp == IDK || t->kind.exp == ArrIDK)
    fprintf(listing, "Error: Undefined variable \"%s\" at line %d\n", t->attr.id, t->lineno);
  else if (t->kind.exp == CallK)
    fprintf(listing, "Error: Undefined function \"%s\" at line %d\n", t->attr.id,t->lineno);
}

static void redefinedError(TreeNode *t){
  Error=TRUE;
  if (t->kind.declar == VarK || t->kind.declar == ArrVarK)
    fprintf(listing, "Error: Redefined variable \"%s\" at line %d\n", t->attr.id, t->lineno);
  else if (t->kind.declar == FunK)
    fprintf(listing, "Error: Redefined function \"%s\" at line %d\n", t->attr.id, t->lineno);
}

static void typeError(TreeNode *t, char *errorDetail){
  Error=TRUE;
  fprintf(listing, "Error: Type error at line %d : %s\n",t->lineno, errorDetail);
}


/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case DeclarK:
      switch (t->kind.declar){
        case FunK:
          functionId = t->attr.id;
          if(st_top_lookup(t->attr.id) >=0){
            redefinedError(t);
            break;
          }
          st_insert(functionId, t->lineno, addLocation(), t);
          scopeStackPush(scopeCreate(functionId));
          switch (t->child[0]->type){
            case Integer:
              t->type = Integer;
              break;
            case Void:
              t->type = Void;
              break;
            default:
              typeError(t,"It can't be return type");
              break;
          }
          break;
        case VarK:
        case ArrVarK:
          char * id;
          if (t->kind.declar == VarK){
            id = t->attr.id;
            t->type = Integer;
          }
          else{
            id = t->attr.id;
            t->type = IntegerArr;
          }
          if (st_top_lookup(id)<0)
            st_insert(id, t->lineno, addLocation(), t);
          else
            redefinedError(t);
          break;
        case ParamK:
          if(t->child[0]->type == Void) break;
          if(st_lookup(t->attr.id)==-1){
            st_insert(t->attr.id, t->lineno, addLocation(), t);
            t->type = Integer;
          }
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          ScopeList sl = scopeCreate(functionId);
          scopeStackPush(sl);
          location++;
          t->scope = scopeStackTop();
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case IDK:
        case ArrIDK:
        case CallK:
          if (st_lookup(t->attr.id) == -1)
            undefinedError(t);
          else
            st_add_lineno(t->attr.id, t->lineno);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

static void postInsertNode(TreeNode *t){
  if(t->nodekind == StmtK && t->kind.stmt==CompK)
    scopeStackPop();
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ globalScope = scopeCreate("global");
  scopeStackPush(globalScope);
  traverse(syntaxTree, insertNode, postInsertNode);
  scopeStackPop();
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case OpK:
          if ((t->child[0]->type != Integer) ||
              (t->child[1]->type != Integer))
            typeError(t,"Op applied to non-integer");
          if ((t->attr.op == EQ) || (t->attr.op == LT))
            t->type = Boolean;
          else
            t->type = Integer;
          break;
        case ConstK:
          t->type = Integer;
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
          if (t->child[0]->type == Integer)
            typeError(t->child[0],"if test is not Boolean");
          break;
        case AssignK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"assignment of non-integer value");
          break;
        case WriteK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"write of non-integer value");
          break;
        case RepeatK:
          if (t->child[1]->type == Integer)
            typeError(t->child[1],"repeat test is not Boolean");
          break;
        default:
          break;
      }
      break;
    default:
      break;

  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ traverse(syntaxTree,nullProc,checkNode);
}
