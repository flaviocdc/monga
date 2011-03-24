#ifndef _CODEGEN_H

#include <stdio.h>
#include "ast.h"

#define SIZE_INT    4
#define SIZE_P      4
#define SIZE_CHAR   1

#define GLOBAL_VAR  -1

#define DIR_COMMON  ".common"
#define DIR_GLOBL   ".globl"
#define DIR_TEXT    ".text"
#define DIR_DATA    ".data"
#define DIR_FILE    ".file"
#define DIR_STRING  ".string"

#define REG_ESP     "%esp"
#define REG_EBP     "%ebp"
#define REG_EAX     "%eax"
#define REG_ECX     "%ecx"
#define REG_EDX     "%edx"
#define REG_EBX     "%ebx"
#define REG_ESI     "%esi"
#define REG_EDI     "%edi"
#define REG_MEM_EAX "(%eax)"

#define OP_PUSH     "pushl"
#define OP_MOV      "movl"
#define OP_MOVB     "movb"
#define OP_SUB      "subl"
#define OP_CMP      "cmpl"
#define OP_ADD      "addl"
#define OP_SUB      "subl"
#define OP_IMUL     "imull"
#define OP_IDIV     "idivl"
#define OP_AND      "andl"
#define OP_JE       "je"
#define OP_JNE      "jne"
#define OP_JG       "jg"
#define OP_JL       "jl"
#define OP_JGE      "jge"
#define OP_JLE      "jle"
#define OP_PUSH     "pushl"
#define OP_POP      "popl"
#define OP_JMP      "jmp"
#define OP_RET      "ret"
#define OP_NEG      "neg"
#define OP_CALL     "call"
#define OP_SHL      "shll"

#define V_IMM_INT   1000
#define V_GLOBAL    1001
#define V_REG       1002
#define V_IND       1003
#define V_MEM_REG   1004
#define V_LOCAL     1005
#define V_MEM_IND   1006

#define RT_IARRALLOC "iarralloc"
#define RT_CARRALLOC "carralloc"
#define RT_ARRFREE "arrfree"

typedef struct _CGState CGState;

struct _CGState {
  int ident;
  int last_label;
  int curr_local;
  char *ret_label;
  char *filename;
  FILE *outfile;
  FILE *prefile;
  int ecx_used, edx_used, ebx_saved, esi_saved, edi_saved, ebx_used, esi_used, edi_used;
};

typedef struct _Value Value;

struct _Value {
  int tag;
  union {
    int ival;
    char* sval;
    char* reg;
    int ind;
  } u;
};

void gen_prog(char *filename, char *prefile);

void gen_maxstack(CGState *cgs, char *sym, char *size);

int get_temp(CGState *cgs);

void gen_saveregs(CGState *cgs, Value *ecx_saved, Value *edx_saved);

void gen_restoreregs(CGState *cgs, Value *ecx_saved, Value *edx_saved);

char *new_label(CGState *cgs);

void gen_label(CGState *cgs, char *label);

void free_reg(CGState* cgs, Value* v);

char* get_reg(CGState* cgs);

void gen_ident(CGState* cgs);

void gen_directive(CGState* cgs, char* dir, char* name, int size);

void gen_unop(CGState* cgs, char* opcode, char* operand);

void gen_binop(CGState* cgs, char* opcode, char* operand1, char* operand2);

char *gen_value(Value *v);

void gen_declrlist(CGState* cgs, DeclrListNode* dln);

void gen_global_var(CGState* cgs, char* name, Type* type);

void gen_function(CGState* cgs, Declr* d);

void gen_block(CGState* cgs, Block* b);

void gen_command(CGState* cgs, Command* c);

void gen_if(CGState* cgs, Command* c);

void gen_jumpiffalse(CGState *cgs, Exp *e, char *label);

void gen_jumpiftrue(CGState *cgs, Exp *e, char *label);

void gen_while(CGState* cgs, Command* c);

void gen_attr(CGState* cgs, Command* c);

void gen_return(CGState* cgs, Exp* ret);

Value* gen_var(CGState* cgs, Var* var, int addr);

void gen_declr(CGState* cgs, Declr* d);

Value* gen_exp(CGState* cgs, Exp* e);

Value* gen_int(CGState* cgs, Exp* e);

Value* gen_string(CGState* cgs, Exp* e);

Value* gen_exp_binop(CGState* cgs, Exp* e);

Value *gen_exp_neg(CGState *cgs, Exp *e);

Value *gen_exp_lneg(CGState *cgs, Exp *e);

Value *gen_funcall(CGState *cgs, Exp *e);

Value *gen_conv(CGState *cgs, Exp *e);

void gen_array_access(CGState *cgs, Type *type, ExpListNode *idx, Value *v, int dim);

#endif
