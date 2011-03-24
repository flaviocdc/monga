#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "y.tab.h"
#include "symtab.h"
#include "codegen.h"

extern DeclrListNode *ast;

char *format_string(char *s) {
  char *new_s, *new_s_corrected;
  int i, j;
  ALLOCS(new_s, strlen(s)*2 + 3);
  new_s[0] = '\"';
  for(i = 0, j = 1; i < strlen(s); i++, j++) {
    switch(s[i]) {
      case '\"': new_s[j] = '\\'; j++; new_s[j] = '\"'; break;
      case '\\': new_s[j] = '\\'; j++; new_s[j] = '\\'; break;
      case '\t': new_s[j] = '\\'; j++; new_s[j] = 't'; break;
      case '\n': new_s[j] = '\\'; j++; new_s[j] = 'n'; break;
      default: new_s[j] = s[i];
    }
  }
  new_s[j] = '\"';
  new_s[j + 1] = 0;
  ALLOCS(new_s_corrected, strlen(new_s) + 1);
  strcpy(new_s_corrected, new_s);
  free(new_s);
  return new_s_corrected;
}

void gen_prog(char *filename, char *prefile) {
  CGState *cgs;
  ALLOC(cgs, CGState);
  cgs->ident = 8;
  cgs->last_label = 0;
  cgs->curr_local = 0;
  cgs->ecx_used = 0;
  cgs->edx_used = 0;
  cgs->ebx_used = 0;
  cgs->esi_used = 0;
  cgs->edi_used = 0;
  cgs->ebx_saved = 0;
  cgs->esi_saved = 0;
  cgs->edi_saved = 0;
  cgs->outfile = fopen(filename, "w+");
  cgs->filename = filename;
  cgs->prefile = fopen(prefile, "w+");
  fprintf(cgs->prefile, "#!/bin/bash\n");
  gen_directive(cgs, DIR_FILE, format_string(filename), 0);
  gen_directive(cgs, DIR_TEXT, NULL, 0);
  gen_declrlist(cgs, ast);
  fclose(cgs->outfile);
  fclose(cgs->prefile);
}

void gen_maxstack(CGState *cgs, char *sym, char *size) {
  fprintf(cgs->prefile, "sed -i 's/%s/$%s/' %s\n", sym, size, cgs->filename);
}

int get_temp(CGState *cgs) {
  int loc;
  loc = cgs->curr_local;
  cgs->curr_local -= 4;
  return loc;
}

char *new_label(CGState *cgs) {
  char *label;
  ALLOCS(label, 10);
  cgs->last_label++;
  sprintf(label, ".LC%i", cgs->last_label);
  return label;
}

void gen_label(CGState *cgs, char *label) {
  fprintf(cgs->outfile, "%s:\n", label);
}

void free_reg(CGState *cgs, Value *v) {
  if(v->tag == V_REG || v->tag == V_MEM_REG) {
    if(v->u.reg == REG_ECX)
      cgs->ecx_used = 0;
    else if(v->u.reg == REG_EDX)
      cgs->edx_used = 0;
    else if(v->u.reg == REG_EBX)
      cgs->ebx_used = 0;
    else if(v->u.reg == REG_ESI)
      cgs->esi_used = 0;
    else if(v->u.reg == REG_EDI)
      cgs->edi_used = 0;
  }
}

int gen_save_reg(CGState *cgs, char *reg) {
  Value v;
  v.tag = V_IND;
  v.u.ind = get_temp(cgs);
  gen_binop(cgs, OP_MOV, reg, gen_value(&v));
  return v.u.ind;
}

char* get_reg(CGState* cgs) {
  if(!cgs->ecx_used) {
    cgs->ecx_used = 1;
    return REG_ECX;
  } else if(!cgs->edx_used) {
    cgs->edx_used = 1;
    return REG_EDX;
  } else if(!cgs->ebx_used) {
    if(!cgs->ebx_saved)
      cgs->ebx_saved = gen_save_reg(cgs, REG_EBX);
    cgs->ebx_used = 1;
    return REG_EBX;
  } else if(!cgs->esi_used) {
    if(!cgs->esi_saved)
      cgs->esi_saved = gen_save_reg(cgs, REG_ESI);
    cgs->esi_used = 1;
    return REG_ESI;
  } else if(!cgs->edi_used) {
    if(!cgs->edi_saved)
      cgs->edi_saved = gen_save_reg(cgs, REG_EDI);
    cgs->edi_used = 1;
    return REG_EDI;
  } else return REG_EAX;
}

void gen_ident(CGState* cgs) {
  int i;
  for(i = 0; i < cgs->ident; i++) fprintf(cgs->outfile, " ");
}

void gen_directive(CGState* cgs, char* dir, char* name, int size) {
  gen_ident(cgs);
  if(name && size)
    fprintf(cgs->outfile, "%s %s,%i\n", dir, name, size);
  else if(name)
    fprintf(cgs->outfile, "%s %s\n", dir, name);
  else
    fprintf(cgs->outfile, "%s\n", dir);
}

void gen_unop(CGState* cgs, char* opcode, char* operand) {
  gen_ident(cgs);
  fprintf(cgs->outfile, "%s %s\n", opcode, operand);
}

void gen_binop(CGState* cgs, char* opcode, char* operand1, char* operand2) {
  gen_ident(cgs);
  fprintf(cgs->outfile, "%s %s,%s\n", opcode, operand1, operand2);
}

char *gen_value(Value *v) {
  switch(v->tag) {
    case V_IMM_INT: {
      char *val;
      ALLOCS(val, 13);
      sprintf(val, "$%i", v->u.ival);
      return val;
    }
    case V_GLOBAL: return v->u.sval;
    case V_REG: return v->u.reg;
    case V_LOCAL:
    case V_MEM_IND:
    case V_IND: {
      char *val;
      ALLOCS(val, 10);
      sprintf(val, "%i(%%ebp)", v->u.ind);
      return val; 
    }
    case V_MEM_REG: {
      char *val;
      ALLOCS(val, 7);
      sprintf(val, "(%s)", v->u.reg);
      return val;
    }
  }
}

void gen_declrlist(CGState* cgs, DeclrListNode* dln) {
  while(dln) {
    gen_declr(cgs, dln->declr);
    dln = dln->next;
  }
}

void gen_global_var(CGState* cgs, char* name, Type* type) {
  if(!type->dimensions) {
    switch(type->type) {
      case TK_TINT: gen_directive(cgs, DIR_COMMON, name, SIZE_INT); break;
      default: break;
    }
  } else { 
    gen_directive(cgs, DIR_COMMON, name, SIZE_P);
  } 
}

void gen_restore_regs(CGState *cgs) {
  Value v;
  v.tag = V_IND;
  if(cgs->ebx_saved) {
    v.u.ind = cgs->ebx_saved;
    gen_binop(cgs, OP_MOV, REG_EBX, gen_value(&v));
  }
  if(cgs->esi_saved) {
    v.u.ind = cgs->esi_saved;
    gen_binop(cgs, OP_MOV, REG_ESI, gen_value(&v));
  }
  if(cgs->edi_saved) {
    v.u.ind = cgs->edi_saved;
    gen_binop(cgs, OP_MOV, REG_EDI, gen_value(&v));
  }
}

void gen_function(CGState* cgs, Declr* d) {
  if(d->u.func.block) {
    char buffer[10];
    char *max_stack;
    DeclrListNode *dln;
    int old_ident, offset;
    ALLOCS(max_stack, strlen(d->u.func.name) + 6);
    strcpy(max_stack, "$loc_");
    strcat(max_stack, d->u.func.name);
    dln = d->u.func.params;
    offset = 8;
    while(dln) {
      dln->declr->line = offset;
      offset += 4;
      dln = dln->next;
    }
    cgs->curr_local = -4;
    old_ident = cgs->ident;
    gen_directive(cgs, DIR_TEXT, NULL, 0);
    cgs->ident = 0;
    gen_directive(cgs, DIR_GLOBL, d->u.func.name, 0);
    cgs->ident = old_ident;
    cgs->ret_label = new_label(cgs);
    gen_label(cgs, d->u.func.name);
    gen_unop(cgs, OP_PUSH, REG_EBP);
    gen_binop(cgs, OP_MOV, REG_ESP, REG_EBP);
    gen_binop(cgs, OP_SUB, max_stack, REG_ESP);
    gen_block(cgs, d->u.func.block);
    gen_label(cgs, cgs->ret_label);
    gen_restore_regs(cgs);
    gen_binop(cgs, OP_MOV, REG_EBP, REG_ESP);
    gen_unop(cgs, OP_POP, REG_EBP);
    gen_unop(cgs, OP_RET, "");
    sprintf(buffer, "%i", (-cgs->curr_local)-4);
    gen_maxstack(cgs, max_stack, buffer);
  }
}

int gen_pushsizes(CGState *cgs, IntListNode *size, int count) {
  Value v;
  v.tag = V_IMM_INT;
  if(size->next)
    count = gen_pushsizes(cgs, size->next, count + 1);
  v.u.ival = size->n;
  gen_unop(cgs, OP_PUSH, gen_value(&v));
  return count;
}

void gen_array_alloc(CGState *cgs, Declr *d) {
  Value v;
  v.tag = V_IMM_INT;
  v.u.ival = gen_pushsizes(cgs, d->type->sizes, 1);
  gen_unop(cgs, OP_PUSH, gen_value(&v));
  v.u.ival = d->type->dimensions;
  gen_unop(cgs, OP_PUSH, gen_value(&v));
  switch(d->type->type) {
    case TK_TINT: gen_unop(cgs, OP_CALL, RT_IARRALLOC); break;
    case TK_TCHAR: gen_unop(cgs, OP_CALL, RT_CARRALLOC); break;
  }
  v.tag = V_IND;
  v.u.ind = d->line;
  gen_binop(cgs, OP_MOV, REG_EAX, gen_value(&v));
}

void gen_array_free(CGState *cgs, Declr *d) {
  Value v;
  v.tag = V_IMM_INT;
  v.u.ival = gen_pushsizes(cgs, d->type->sizes, 1);
  gen_unop(cgs, OP_PUSH, gen_value(&v));
  v.u.ival = d->type->dimensions;
  v.tag = V_IND;
  v.u.ind = d->line;
  gen_unop(cgs, OP_PUSH, gen_value(&v));
  gen_unop(cgs, OP_CALL, RT_ARRFREE);
}

void gen_block(CGState* cgs, Block* b) {
  DeclrListNode *dln;
  CommListNode* cln;
  dln = b->declrs;
  while(dln) {
    dln->declr->line = get_temp(cgs);
    if(dln->declr->type->dimensions && dln->declr->type->sizes->n)
      gen_array_alloc(cgs, dln->declr);
    dln = dln->next;
  }
  cln = b->comms;
  while(cln) {
    gen_command(cgs, cln->comm);
    cln = cln->next;
  }
  dln = b->declrs;
  while(dln) {
    if(dln->declr->type->dimensions && dln->declr->type->sizes->n)
      gen_array_free(cgs, dln->declr);
    dln = dln->next;
  }
}

void gen_command(CGState* cgs, Command* c) {
  if(!c) return;
  switch(c->tag) {
    case COMMAND_IF: {
      gen_if(cgs, c);
      break;
    }
    case COMMAND_WHILE: {
      gen_while(cgs, c);
      break;
    }
    case COMMAND_ATTR: {
      gen_attr(cgs, c);
      break;
    }
    case COMMAND_RET: {
      gen_return(cgs, c->u.ret);
      break;
    }
    case COMMAND_FUNCALL: {
      free_reg(cgs, gen_funcall(cgs, c->u.funcall));
      break;
    }
    case COMMAND_BLOCK: {
      gen_block(cgs, c->u.block);
      break;
    }
  }
}

void gen_if(CGState* cgs, Command* c) {
  char *else_label, *end_label;
  else_label = new_label(cgs);
  end_label = new_label(cgs);
  gen_jumpiffalse(cgs, c->u.cif.exp, else_label);
  gen_command(cgs, c->u.cif.comm);
  if(c->u.cif.celse) gen_unop(cgs, OP_JMP, end_label);
  gen_label(cgs, else_label);
  gen_command(cgs, c->u.cif.celse);
  gen_label(cgs, end_label);
}

char *jump_instruction_1(int op) {
  switch(op) {
    case TK_EQ: return OP_JNE;
    case TK_LEQ: return OP_JL;
    case TK_GEQ: return OP_JG;
    case TK_NEQ: return OP_JE;
    case '<':  return OP_JLE;
    case '>': return OP_JGE;
  }
}

char *jump_instruction_2(int op) {
  switch(op) {
    case TK_EQ: return OP_JNE;
    case TK_LEQ: return OP_JG;
    case TK_GEQ: return OP_JL;
    case TK_NEQ: return OP_JE;
    case '<':  return OP_JGE;
    case '>': return OP_JLE;
  }
}

char *jump_instruction_3(int op) {
  switch(op) {
    case TK_EQ: return OP_JE;
    case TK_LEQ: return OP_JGE;
    case TK_GEQ: return OP_JLE;
    case TK_NEQ: return OP_JNE;
    case '<':  return OP_JG;
    case '>': return OP_JL;
  }
}

char *jump_instruction_4(int op) {
  switch(op) {
    case TK_EQ: return OP_JE;
    case TK_LEQ: return OP_JLE;
    case TK_GEQ: return OP_JGE;
    case TK_NEQ: return OP_JNE;
    case '<':  return OP_JL;
    case '>': return OP_JG;
  }
}

char *select_binop(int op) {
  switch(op) {
    case '+': return OP_ADD;
    case '-': return OP_SUB;
    case '*': return OP_IMUL;
  }
}

int test_op(int op, int v1, int v2) {
  switch(op) {
    case TK_EQ: return v1 == v2;
    case TK_LEQ: return v1 <= v2;
    case TK_GEQ: return v1 >= v2;
    case TK_NEQ: return v1 != v2;
    case '<': return v1 < v2;
    case '>': return v1 > v2;
  }
}

void gen_jumpiffalse(CGState *cgs, Exp *e, char *label) {
  switch(e->tag) {
    case EXP_LNEG: gen_jumpiftrue(cgs, e->u.exp, label); break;
    case EXP_BINOP: {
      switch(e->u.binop.op) {
        case TK_AND: {
          gen_jumpiffalse(cgs, e->u.binop.e1, label);
          gen_jumpiffalse(cgs, e->u.binop.e2, label);
          break;
        }
        case TK_OR: {
          char *fall_through;
          fall_through = new_label(cgs);
          gen_jumpiftrue(cgs, e->u.binop.e1, fall_through);
          gen_jumpiffalse(cgs, e->u.binop.e2, label);
          gen_label(cgs, fall_through);
          break;
        }
        case TK_EQ:
        case TK_LEQ:
        case TK_GEQ:
        case TK_NEQ:
        case '<':
        case '>': {
          Value *v1, *v2;
          v1 = gen_exp(cgs, e->u.binop.e1);
          v2 = gen_exp(cgs, e->u.binop.e2);
          if(v1->tag == V_IMM_INT && v2->tag == V_IMM_INT) {
            if(!test_op(e->u.binop.op, v1->u.ival, v2->u.ival))
              gen_unop(cgs, OP_JMP, label);
          } else if(v2->tag == V_IMM_INT || v1->tag == V_REG) {
            gen_binop(cgs, OP_CMP, gen_value(v2), gen_value(v1));
            gen_unop(cgs, jump_instruction_2(e->u.binop.op), label);
          } else if(v1->tag == V_IMM_INT || v2->tag == V_REG) {
            gen_binop(cgs, OP_CMP, gen_value(v1), gen_value(v2));
            gen_unop(cgs, jump_instruction_1(e->u.binop.op), label);
          } else {
            gen_binop(cgs, OP_MOV, gen_value(v1), REG_EAX);
            gen_binop(cgs, OP_CMP, gen_value(v2), REG_EAX);
            gen_unop(cgs, jump_instruction_2(e->u.binop.op), label);
          }
          free_reg(cgs, v1);
          free_reg(cgs, v2);
          break;
        }
        default: {
          Value *v;
          v = gen_exp(cgs, e);
          switch(v->tag) {
            case V_IMM_INT: if(!v->u.ival) gen_unop(cgs, OP_JMP, label); break;
            default: {
              gen_binop(cgs, OP_CMP, "$0", gen_value(v));
              gen_unop(cgs, OP_JE, label);
              free_reg(cgs, v);
            }
          }
        }
      }
      break;
    }
    default: {
      Value *v;
      v = gen_exp(cgs, e);
      switch(v->tag) {
        case V_IMM_INT: if(!v->u.ival) gen_unop(cgs, OP_JMP, label); break;
        default: {
          gen_binop(cgs, OP_CMP, "$0", gen_value(v));
          gen_unop(cgs, OP_JE, label);
          free_reg(cgs, v);
        }
      }
    }
  }
}

void gen_jumpiftrue(CGState *cgs, Exp *e, char *label) {
  switch(e->tag) {
    case EXP_LNEG: gen_jumpiffalse(cgs, e->u.exp, label); break;
    case EXP_BINOP: {
      switch(e->u.binop.op) {
        case TK_AND: {
          char *fall_through;
          fall_through = new_label(cgs);
          gen_jumpiffalse(cgs, e->u.binop.e1, fall_through);
          gen_jumpiftrue(cgs, e->u.binop.e2, label);
          gen_label(cgs, fall_through);
          break;
        }
        case TK_OR: {
          gen_jumpiftrue(cgs, e->u.binop.e1, label);
          gen_jumpiftrue(cgs, e->u.binop.e2, label);
          break;
        }
        case TK_EQ:
        case TK_LEQ:
        case TK_GEQ:
        case TK_NEQ:
        case '<':
        case '>': {
          Value *v1, *v2;
          v1 = gen_exp(cgs, e->u.binop.e1);
          v2 = gen_exp(cgs, e->u.binop.e2);
          if(v1->tag == V_IMM_INT && v2->tag == V_IMM_INT) {
            if(test_op(e->u.binop.op, v1->u.ival, v2->u.ival))
              gen_unop(cgs, OP_JMP, label);
          } else if(v2->tag == V_IMM_INT || v1->tag == V_REG) {
            gen_binop(cgs, OP_CMP, gen_value(v2), gen_value(v1));
            gen_unop(cgs, jump_instruction_4(e->u.binop.op), label);
          } else if(v1->tag == V_IMM_INT || v2->tag == V_REG) {
            gen_binop(cgs, OP_CMP, gen_value(v1), gen_value(v2));
            gen_unop(cgs, jump_instruction_3(e->u.binop.op), label);
          } else {
            gen_binop(cgs, OP_MOV, gen_value(v1), REG_EAX);
            gen_binop(cgs, OP_CMP, gen_value(v2), REG_EAX);
            gen_unop(cgs, jump_instruction_4(e->u.binop.op), label);
          }
          free_reg(cgs, v1);
          free_reg(cgs, v2);
          break;
        }
        default: {
          Value *v;
          v = gen_exp(cgs, e);
          switch(v->tag) {
            case V_IMM_INT: if(v->u.ival) gen_unop(cgs, OP_JMP, label); break;
            default: {
              gen_binop(cgs, OP_CMP, "$0", gen_value(v));
              gen_unop(cgs, OP_JNE, label);
              free_reg(cgs,v);
            }
          }
        }
      }
      break;
    }
    default: {
      Value *v;
      v = gen_exp(cgs, e);
      switch(v->tag) {
        case V_IMM_INT: if(v->u.ival) gen_unop(cgs, OP_JMP, label); break;
        default: {
          gen_binop(cgs, OP_CMP, "$0", gen_value(v));
          gen_unop(cgs, OP_JNE, label);
          free_reg(cgs, v);
        }
      }
    }
  }
}

void gen_while(CGState* cgs, Command* c) {
  char* loop_label;
  char* test_label;
  loop_label = new_label(cgs);
  test_label = new_label(cgs);
  gen_unop(cgs, OP_JMP, test_label);
  gen_label(cgs, loop_label);
  gen_command(cgs, c->u.cwhile.comm);
  gen_label(cgs, test_label);
  gen_jumpiftrue(cgs, c->u.cwhile.exp, loop_label);
}

void gen_attr(CGState* cgs, Command* c) {
  Value* lvalue;
  Value* rvalue;
  char *op_mov;
  if(c->u.attr.lvalue->type->type == TK_TCHAR && c->u.attr.lvalue->idxs)
    op_mov = OP_MOVB;
  else
    op_mov = OP_MOV;
  rvalue = gen_exp(cgs, c->u.attr.rvalue);
  lvalue = gen_var(cgs, c->u.attr.lvalue, 1);
  if(lvalue->tag == V_MEM_IND) {
    lvalue->tag = V_IND;
    gen_binop(cgs, OP_MOV, gen_value(lvalue), REG_EAX);
    lvalue->tag = V_MEM_REG;
    lvalue->u.reg = REG_EAX;
  }
  if(lvalue->tag == V_REG || rvalue->tag == V_REG) {
    gen_binop(cgs, op_mov, gen_value(rvalue), gen_value(lvalue));
  } else {
    Value v_temp;
    free_reg(cgs, rvalue);
    v_temp.tag = V_REG;
    v_temp.u.reg = get_reg(cgs);
    gen_binop(cgs, OP_MOV, gen_value(rvalue), gen_value(&v_temp));
    gen_binop(cgs, op_mov, gen_value(&v_temp), gen_value(lvalue));
    free_reg(cgs, &v_temp);
  }
  free_reg(cgs, lvalue);
  free_reg(cgs, rvalue);
}

void gen_return(CGState* cgs, Exp* ret) {
  Value* v;
  v = gen_exp(cgs, ret);
  if(v->tag != V_REG || v->u.reg != REG_EAX) {
    gen_binop(cgs, OP_MOV, gen_value(v), REG_EAX);
  }
  free_reg(cgs, v);
  gen_unop(cgs, OP_JMP, cgs->ret_label);
}

Value* gen_var(CGState* cgs, Var* var, int addr) {
  Value* val;
  ALLOC(val, Value);
  if(var->var->line == GLOBAL_VAR) {
    val->tag = V_GLOBAL;
    val->u.sval = var->var->u.name;
  } else {
    val->tag = V_LOCAL;
    val->u.ind = var->var->line;
  }
  if(var->idxs) {
    gen_array_access(cgs, var->var->type, var->idxs, val, 1);
  }
  if(var->idxs && !addr && var->type->type == TK_TCHAR && var->type->dimensions == 0) {
    char *reg = get_reg(cgs);
    if(val->tag == V_MEM_IND) {
      gen_binop(cgs, OP_MOV, gen_value(val), reg);
      val->tag = V_MEM_REG;
      val->u.reg = reg;
    }
    gen_binop(cgs, OP_MOVB, gen_value(val), reg);
    gen_binop(cgs, OP_AND, "$255", reg);
    if(reg != REG_EAX) {
      val->tag = V_REG;
      val->u.reg = reg;
    } else {
      val->tag = V_IND;
      val->u.ind = get_temp(cgs);
      gen_binop(cgs, OP_MOV, reg, gen_value(val));
    }
  } else if(!addr && val->tag == V_MEM_IND) {
    val->tag = V_IND;
    gen_binop(cgs, OP_MOV, gen_value(val), REG_EAX);
    gen_binop(cgs, OP_MOV, REG_MEM_EAX, REG_EAX);
    val->u.ind = get_temp(cgs);
    gen_binop(cgs, OP_MOV, REG_EAX, gen_value(val));
  }
  return val;
}

int item_size(Type *type, int dim) {
  if(dim < type->dimensions) {
    return SIZE_P;
  } else {
    switch(type->type) {
      case TK_TINT: return SIZE_INT;
      case TK_TCHAR: return SIZE_CHAR;
    }
  }
}

void gen_array_access(CGState *cgs, Type *type, ExpListNode *idx, Value *v, int dim) {
  Value *v_exp;
  char *reg;
  int size;
  size = item_size(type, dim);
  v_exp = gen_exp(cgs, idx->exp);
  if(v->tag == V_REG || v->tag == V_MEM_REG)
    reg = v->u.reg;
  else
    reg = get_reg(cgs);
  if(v->tag == V_MEM_IND) v->tag = V_IND;
  if(v->tag == V_MEM_REG) v->tag = V_REG;
  gen_binop(cgs, OP_MOV, gen_value(v), reg);
  v->tag = V_REG;
  v->u.reg = reg;
  switch(v_exp->tag) {
    case V_IMM_INT: {
      v_exp->u.ival *= size;
      gen_binop(cgs, OP_ADD, gen_value(v_exp), reg);
      break; 
    }
    case V_IND:
    case V_REG: {
      if(size != SIZE_CHAR)
        gen_binop(cgs, OP_SHL, "$2", gen_value(v_exp));
      gen_binop(cgs, OP_ADD, gen_value(v_exp), reg);
      free_reg(cgs, v_exp);
      break;
    }
    case V_MEM_REG:
    case V_LOCAL:
    case V_GLOBAL: {
      if(size != SIZE_CHAR) {
        Value *temp;
        ALLOC(temp, Value);
        free_reg(cgs, v_exp);
        temp->tag = V_REG;
        temp->u.reg = get_reg(cgs);
        if(temp->u.reg == reg) {
          v->tag = V_IND;
          v->u.ind = get_temp(cgs);
          gen_binop(cgs, OP_MOV, REG_EAX, gen_value(v));
        }
        gen_binop(cgs, OP_MOV, gen_value(v_exp), gen_value(temp));
        gen_binop(cgs, OP_SHL, "$2", gen_value(temp));
        gen_binop(cgs, OP_ADD, gen_value(temp), gen_value(v));
        if(temp->u.reg == reg) {
          gen_binop(cgs, OP_MOV, gen_value(v), REG_EAX);
          v->tag = V_REG;
          v->u.reg = REG_EAX;
        }
        free_reg(cgs, temp);
      } else {
        gen_binop(cgs, OP_ADD, gen_value(v_exp), reg);
      }
      free_reg(cgs, v_exp);
      break;
    }
  }
  if(reg == REG_EAX) {
    v->tag = V_IND;
    v->u.ind = get_temp(cgs);
    gen_binop(cgs, OP_MOV, REG_EAX, gen_value(v));
    v->tag = V_MEM_IND;
  } else {
    v->tag = V_MEM_REG;
    v->u.reg = reg;
  }
  if(idx->next) gen_array_access(cgs, type, idx->next, v, dim + 1);
}

void gen_declr(CGState* cgs, Declr* d) {
  switch(d->tag) {
    case DECLR_VAR: {
      d->line = GLOBAL_VAR;
      gen_global_var(cgs, d->u.name, d->type);
      break;
    }
    case DECLR_FUNC: {
      gen_function(cgs, d);
      break;
    }
  }
}

Value* gen_exp(CGState* cgs, Exp* e) {
  switch(e->tag) {
    case EXP_INT: return gen_int(cgs, e);
    case EXP_FLOAT: break;
    case EXP_STRING: return gen_string(cgs, e);
    case EXP_VAR: return gen_var(cgs, e->u.var, 0);
    case EXP_BINOP: return gen_exp_binop(cgs, e);
    case EXP_NEG: return gen_exp_neg(cgs, e->u.exp);
    case EXP_LNEG: return gen_exp_lneg(cgs, e->u.exp);
    case EXP_FUNCALL: return gen_funcall(cgs, e);
    case EXP_CONV: return gen_conv(cgs, e);
  }
}

Value* gen_int(CGState* cgs, Exp* e) {
  Value* v;
  ALLOC(v, Value);
  v->tag = V_IMM_INT;
  v->u.ival = e->u.ival;
  return v;
}

Value* gen_string(CGState* cgs, Exp* e) {
  Value* v;
  char* str_label;
  ALLOC(v, Value);
  v->tag = V_GLOBAL;
  gen_directive(cgs, DIR_DATA, NULL, 0);
  gen_label(cgs, str_label = new_label(cgs));
  gen_directive(cgs, DIR_STRING, format_string(e->u.sval), 0);
  gen_directive(cgs, DIR_TEXT, NULL, 0);
  ALLOCS(v->u.sval, strlen(str_label) + 2);
  strcpy(v->u.sval, "$");
  strcat(v->u.sval, str_label);
  return v;
}

int do_binop(int op, int v1, int v2) {
  switch(op) {
    case '+': return v1 + v2;
    case '-': return v1 - v2;
    case '*': return v1 * v2;
  }
}

Value* gen_exp_binop(CGState* cgs, Exp* e) {
  Value *v;
  switch(e->u.binop.op) {
    case TK_AND: {
      char *false_label;
      char *end_label;
      false_label = new_label(cgs);
      end_label = new_label(cgs);
      gen_jumpiffalse(cgs, e, false_label);
      ALLOC(v, Value);
      v->tag = V_REG;
      v->u.reg = get_reg(cgs);
      gen_binop(cgs, OP_MOV, "$1", gen_value(v));
      gen_unop(cgs, OP_JMP, end_label);
      gen_label(cgs, false_label);
      gen_binop(cgs, OP_MOV, "$0", gen_value(v));
      gen_label(cgs, end_label);
      if(v->u.reg == REG_EAX) {
        v->tag = V_IND;
        v->u.ind = get_temp(cgs);
        gen_binop(cgs, OP_MOV, REG_EAX, gen_value(v));
      }
      return v;
    }
    case TK_OR:
    case TK_EQ:
    case TK_LEQ:
    case TK_GEQ:
    case TK_NEQ:
    case '<':
    case '>': {
      char *true_label;
      char *end_label;
      true_label = new_label(cgs);
      end_label = new_label(cgs);
      gen_jumpiftrue(cgs, e, true_label);
      ALLOC(v, Value);
      v->tag = V_REG;
      v->u.reg = get_reg(cgs);
      gen_binop(cgs, OP_MOV, "$0", gen_value(v));
      gen_unop(cgs, OP_JMP, end_label);
      gen_label(cgs, true_label);
      gen_binop(cgs, OP_MOV, "$1", gen_value(v));
      gen_label(cgs, end_label);
      if(v->u.reg == REG_EAX) {
        v->tag = V_IND;
        v->u.ind = get_temp(cgs);
        gen_binop(cgs, OP_MOV, REG_EAX, gen_value(v));
      }
      return v;
    }
    case '+':
    case '-':
    case '*': {
      Value *v1, *v2;
      v1 = gen_exp(cgs, e->u.binop.e1);
      v2 = gen_exp(cgs, e->u.binop.e2);
      if(v1->tag == V_IMM_INT && v2->tag == V_IMM_INT) {
        ALLOC(v, Value);
        v->tag = V_IMM_INT;
        v->u.ival = do_binop(e->u.binop.op, v1->u.ival, v2->u.ival);
        return v;
      } else if(v1->tag == V_REG) {
        gen_binop(cgs, select_binop(e->u.binop.op), gen_value(v2), gen_value(v1)); 
        free_reg(cgs, v2);
        return v1;
      } else if(v2->tag == V_REG) {
        gen_binop(cgs, select_binop(e->u.binop.op), gen_value(v1), gen_value(v2));
        if(e->u.binop.op == '-')
          gen_unop(cgs, OP_NEG, gen_value(v2));
        free_reg(cgs, v1);
        return v2;
      } else {
        ALLOC(v, Value);
        free_reg(cgs, v1);
        v->tag = V_REG;
        v->u.reg = get_reg(cgs);
        gen_binop(cgs, OP_MOV, gen_value(v1), gen_value(v));
        gen_binop(cgs, select_binop(e->u.binop.op), gen_value(v2), gen_value(v));
        if(v->u.reg == REG_EAX) {
          v->tag = V_IND;
          v->u.ind = get_temp(cgs);
          gen_binop(cgs, OP_MOV, REG_EAX, gen_value(v));
        }
        free_reg(cgs, v2);
        return v;
      }
    }
    case '/': {
      Value *v1, *v2;
      v1 = gen_exp(cgs, e->u.binop.e1);
      v2 = gen_exp(cgs, e->u.binop.e2);
      if(v1->tag == V_IMM_INT && v2->tag == V_IMM_INT) {
        ALLOC(v, Value);
        v->tag = V_IMM_INT;
        v->u.ival = (int)(v1->u.ival)/(v2->u.ival);
        return v;
      } else {
        gen_binop(cgs, OP_MOV, gen_value(v2), REG_EAX);
        free_reg(cgs, v2);
        if(v1->tag == V_IMM_INT) {
          char *reg;
          reg = get_reg(cgs);
          gen_binop(cgs, OP_MOV, gen_value(v1), reg);
          v1->tag = V_REG;
          v1->u.reg = reg;
        } else if(v1->tag == V_REG && v1->u.reg == REG_EDX) {
          char *reg = get_reg(cgs);
          gen_binop(cgs, OP_MOV, REG_EDX, reg);
          free_reg(cgs, v1);
          v1->u.reg = reg;
        }
        gen_unop(cgs, OP_IDIV, gen_value(v1));
        free_reg(cgs, v1);
        ALLOC(v, Value);
        v->tag = V_REG;
        v->u.reg = get_reg(cgs);
        if(v->u.reg == REG_EAX) {
          v->tag = V_IND;
          v->u.ind = get_temp(cgs);
        }
        gen_binop(cgs, OP_MOV, REG_EAX, gen_value(v));
        return v;
      }
    }
  }
}

Value *gen_exp_neg(CGState *cgs, Exp *e) {
  Value *v = gen_exp(cgs, e);
  if(v->tag == V_IMM_INT) {
    v->u.ival = -v->u.ival;
    return v;
  } else if(v->tag == V_REG) {
    gen_unop(cgs, OP_NEG, gen_value(v));
    return v;
  } else {
    char *reg;
    free_reg(cgs, v);
    reg = get_reg(cgs);
    gen_binop(cgs, OP_MOV, gen_value(v), reg);
    gen_unop(cgs, OP_NEG, reg);
    if(reg == REG_EAX) {
      v->tag = V_IND;
      v->u.ind = get_temp(cgs);
      gen_binop(cgs, OP_MOV, REG_EAX, gen_value(v));
    } else {
      v->tag = V_REG;
      v->u.reg = reg;
    }
    return v;
  }
}

Value *gen_exp_lneg(CGState *cgs, Exp *e) {
  Value *v;
  char *true_label;
  char *end_label;
  true_label = new_label(cgs);
  end_label = new_label(cgs);
  gen_jumpiffalse(cgs, e, true_label);
  ALLOC(v, Value);
  v->tag = V_REG;
  v->u.reg = get_reg(cgs);
  gen_binop(cgs, OP_MOV, "$0", gen_value(v));
  gen_unop(cgs, OP_JMP, end_label);
  gen_label(cgs, true_label);
  gen_binop(cgs, OP_MOV, "$1", gen_value(v));
  gen_label(cgs, end_label);
  if(v->u.reg == REG_EAX) {
    v->tag = V_IND;
    v->u.ind = get_temp(cgs);
    gen_binop(cgs, OP_MOV, REG_EAX, gen_value(v));
  }
  return v;
}

int gen_paramlist(CGState *cgs, ExpListNode *eln, int nparams) {
  Value *v;
  if(!eln) return 0;
  if(eln->next)
    nparams = gen_paramlist(cgs, eln->next, nparams + 4);
  v = gen_exp(cgs, eln->exp);
  gen_unop(cgs, OP_PUSH, gen_value(v));
  free_reg(cgs, v);
  return nparams;
}

Value *gen_funcall(CGState *cgs, Exp *e) {
  Value v_params;
  Value *v;
  Value ecx_saved, edx_saved;
  ecx_saved.tag = V_IMM_INT;
  edx_saved.tag = V_IMM_INT;
  v_params.tag = V_IMM_INT;
  gen_saveregs(cgs, &ecx_saved, &edx_saved);
  v_params.u.ival = gen_paramlist(cgs, e->u.funcall.expl, 4);
  gen_unop(cgs, OP_CALL, e->u.funcall.name);
  ALLOC(v, Value);
  v->tag = V_REG;
  v->u.reg = get_reg(cgs);
  if(v->u.reg == REG_EAX) {
    v->tag = V_IND;
    v->u.ind = get_temp(cgs);
  }
  gen_binop(cgs, OP_MOV, REG_EAX, gen_value(v));
  gen_binop(cgs, OP_ADD, gen_value(&v_params), REG_ESP);
  gen_restoreregs(cgs, &ecx_saved, &edx_saved);
  return v;
}

void gen_saveregs(CGState *cgs, Value *ecx_saved, Value *edx_saved) {
  if(cgs->ecx_used) {
    ecx_saved->tag = V_IND;
    ecx_saved->u.ind = get_temp(cgs);
    gen_binop(cgs, OP_MOV, REG_ECX, gen_value(ecx_saved));
  }
  if(cgs->edx_used) {
    edx_saved->tag = V_IND;
    edx_saved->u.ind = get_temp(cgs);
    gen_binop(cgs, OP_MOV, REG_EDX, gen_value(edx_saved));
  }
}

void gen_restoreregs(CGState *cgs, Value *ecx_saved, Value *edx_saved) {
  if(edx_saved->tag == V_IND) {
    gen_binop(cgs, OP_MOV, gen_value(edx_saved), REG_EDX);
  }
  if(ecx_saved->tag == V_IND) {
    gen_binop(cgs, OP_MOV, gen_value(ecx_saved), REG_ECX);
  }
}

Value *gen_conv(CGState *cgs, Exp *e) {
  Value *v;
  v = gen_exp(cgs, e->u.conv.exp);
  switch(e->u.conv.type) {
    case TK_TCHAR: {
      switch(v->tag) {
        case V_IMM_INT: v->u.ival = ((char)v->u.ival); return v;
        case V_IND:
        case V_REG: gen_binop(cgs, OP_AND, "$255", gen_value(v)); return v;
        case V_MEM_REG:
        case V_GLOBAL:
        case V_LOCAL: {
          char *reg;
          free_reg(cgs, v);
          reg = get_reg(cgs);
          gen_binop(cgs, OP_MOV, gen_value(v), reg);
          gen_binop(cgs, OP_AND, "$255", reg);
          if(reg == REG_EAX) {
            v->tag = V_IND;
            v->u.ind = get_temp(cgs);
            gen_binop(cgs, OP_MOV, REG_EAX, gen_value(v));
          } else {
            v->tag = V_REG;
            v->u.reg = reg;
          }
          return v;
        }
      }
    }
    case TK_TINT: {
      return v;
    }
  }
}

