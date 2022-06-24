/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4
#define MAX_SCOPES_NUM 100

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

/* the hash table */
static BucketList hashTable[SIZE];

ScopeList scopes[MAX_SCOPES_NUM];
ScopeList scopeStack[MAX_SCOPES_NUM];
int scopeIndex = 0;
int scopeStackIndex =0;
int locatoin[MAX_SCOPES_NUM];

ScopeList scopeCreate(char * name){
  ScopeList newScope = (ScopeList)malloc(sizeof(struct ScopeListRec));
  newScope->name = name;
  newScope->nestedLevel=scopeStackIndex;
  newScope->parent = scopeStackTop();
  scopes[scopeIndex++]=newScope;
  return newScope;
}

void scopeStackPush(ScopeList scope){
  scopeStack[scopeStackIndex]=scope;
  location[scopeStackIndex++]=0;
}

ScopeList scopeStackTop(void){
  if(scopeStackIndex ==0) return NULL;
  return scopeStack[scopeStackIndex-1];
}

void scopeStackPop(void){
  if (scopeStackIndex !=0) scopeStackIndex--;
}

int addLocation(void){
  return location[scopeStackIndex -1]++;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, TreeNode * treeNode )
{ int h = hash(name);
  BucketList bl =  hashTable[h];
  ScopeList sl = scopeStackTop();
  while((bl != NULL) && (strcmp(name,bl->name)!=0)) bl = bl->next;
  if (bl==NULL){
    // in this case there is no variable in table
    bl = (BucketList)malloc(sizeof(struct BucketListRec));
    bl->name = name;
    bl->lines = (LineList)malloc(sizeof(struct LineListRec));
    bl->lines->lineno = lineno;
    bl->lines->next=NULL;
    bl->memloc = loc;
    bl->next = sl->hashTable[h];
    sl->hashTable[h] = bl;
    bl->treeNode = treeNode;
  }
} /* st_insert */


BucketList get_bucket(char *name){
  int h = hash(name);
  ScopeList sl = scopeStackTop();
  while(sl !=NULL){
    BucketList bl = sl->hashTable[h];
    while((bl!=NULL)&&(strcmp(name,bl->name)!=0)) bl=bl->next;
    if(bl!=NULL) return bl;
    sl = sl->parent;
  }
  return NULL;
}

int st_lookup(char*name){
  BucketList bl = get_bucket(name);
  if(bl!=NULL) return bl->memloc;
  return -1;
}

int st_top_lookup(char * name){
  int h = hash(name);
  ScopeList sl = scopeStackTop();
  BucketList bl = sl->hashTable[h];
  while((bl!=NULL)&&(strcmp(name,bl->name)!=0)) bl=bl->next;
  if(bl!=NULL) return bl->memloc;
  return -1;
}

void st_add_lineno(char * name, int lineno){
  BucketList bl = get_bucket(name);
  LineList ll = bl->lines;
  while (ll->next !=NULL) ll = ll->next;
  ll->next = (LineList)malloc(sizeof(struct LineListRec));
  ll->next->lineno = lineno;
  ll->next->next = NULL;
}
/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
  fprintf(listing,"Variable Name  Location   Line Numbers\n");
  fprintf(listing,"-------------  --------   ------------\n");
  for (i=0;i<SIZE;++i)
  { if (hashTable[i] != NULL)
    { BucketList l = hashTable[i];
      while (l != NULL)
      { LineList t = l->lines;
        fprintf(listing,"%-14s ",l->name);
        fprintf(listing,"%-8d  ",l->memloc);
        while (t != NULL)
        { fprintf(listing,"%4d ",t->lineno);
          t = t->next;
        }
        fprintf(listing,"\n");
        l = l->next;
      }
    }
  }
} /* printSymTab */
