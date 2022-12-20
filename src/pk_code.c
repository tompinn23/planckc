#include "pk_code.h"

#include "pk_compile.h"
#include "pk_value.h"
#include "pk_vm.h"

#define hasjumps(e) ((e)->t != (e)->f)

bool pk_tonumeral(expdesc* e, pk_value* v) {
  switch (e->kind) {
  case EK_DOUBLE:
    if (v)
      v->ival = e->ival;
    return true;
  case EK_INTEGER:
    if (v)
      v->dval = e->dval;
    return true;
  default:
    return false;
  }
}

int pk_code(pk_funcstate* fs, Instruction op) {
  pk_function* f = fs->func;
  return pk_chunk_add(&f->chunk, op);
}

int pk_code_AsBx(pk_funcstate* fs, OpCode o, int a, int bc) {
  unsigned int b = bc + OFFSET_sBx;
  pk_assert(getOpMode(o) == iAsBx);
  pk_assert(a <= MAXARG_A && b <= MAXARG_Bx);
  return pk_code(fs, CREATE_ABx(o, a, b));
}

int pk_code_ABx(pk_funcstate* fs, OpCode o, int a, unsigned int bc) {
  pk_assert(getOpMode(o) == iABx);
  pk_assert(a <= MAXARG_A && bc == MAXARG_Bx);
  return pk_code(fs, CREATE_ABx(o, a, bc));
}

int pk_code_ABCk(pk_funcstate* fs, OpCode o, int a, int b, int c, int k) {
  pk_assert(getOpMode(o) == iABC);
  pk_assert(a <= MAXARG_A && b <= MAXARG_B && c <= MAXARG_C && (k & ~1) == 0);
  return pk_code(fs, CREATE_ABCk(o, a, b, c, k));
}

static void codeextraarg(pk_funcstate* fs, int k) {
  pk_assert(k <= MAXARG_Bx);
  return pk_code(fs, CREATE_Ax(OP_EXTRAARG, k));
}

int pk_code_k(pk_funcstate* fs, int reg, int k) {
  if (k <= MAXARG_Bx) {
    return pk_code_ABx(fs, OP_LOADK, reg, k);
  } else {
    int p = pk_code_ABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    return p;
  }
}

static int addK(pk_funcstate* fs, pk_value v) { pk_function* f = fs->func;
  return pk_constants_add(&f->constants, v);
}

static int pk_numberK(pk_funcstate* fs, double d) {
  pk_value v = pk_DValue(d);
  return addK(fs, v);
}

static int pk_intK(pk_funcstate* fs, long n) { pk_value v = pk_IValue(n);
  return addK(fs, v);
}

static int fitsBx(long i) { return (-OFFSET_sBx <= i && i <= MAXARG_Bx - OFFSET_sBx); }

void pk_code_double(pk_funcstate* fs, int reg, double val) {
  long iv;
  if (pk_value_dbl_to_int(val, &iv, F2Ieq) && fitsBx(iv)) {
    pk_code_AsBx(fs, OP_LOADF, reg, cast_int(iv));
  } else {
    pk_code_k(fs, reg, pk_numberK(fs, val));
  }
}

void pk_code_integer(pk_funcstate* fs, int reg, long val) {
  if (fitsBx(val)) {
    pk_code_AsBx(fs, OP_LOADI, reg, cast_int(val));
  } else {
    pk_code_k(fs, reg, pk_intK(fs, val));
  }
}

/* Ensure that the expression is somewhere not a variable, register etc. Make non transient*/
static void pk_discharge_vars(pk_funcstate* fs, expdesc* e) {
  // TODO: implement things like upvalue and table access.
  switch (e->kind) {
  default:
    break;
  }
}

static void freereg(pk_funcstate* fs, int reg) {
  if (reg >= pk_compile_nvarstack(fs)) {
      fs->freereg--;
    pk_assert(reg == fs->freereg);
  }
}

static void freeregs(pk_funcstate* fs, int r1, int r2) {
  if (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  } else {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}

/*
** Free register used by expression 'e' (if any)
*/
static void freeexp(pk_funcstate* fs, expdesc* e) {
  if (e->kind == EK_NON_RELOC)
    freereg(fs, e->info);
}

/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
static void freeexps(pk_funcstate* fs, expdesc* e1, expdesc* e2) {
  int r1 = (e1->kind == EK_NON_RELOC) ? e1->info : -1;
  int r2 = (e2->kind == EK_NON_RELOC) ? e2->info : -1;
  freeregs(fs, r1, r2);
}

/* Ensure expression is in register reg. making the expression non relocatable.*/
static void discharge2reg(pk_funcstate* fs, expdesc* e, int reg) {
  pk_discharge_vars(fs, e);
  switch (e->kind) {
  case EK_DOUBLE: {
    pk_code_double(fs, reg, e->dval);
    break;
  }
  case EK_INTEGER: {
    pk_code_integer(fs, reg, e->ival);
    break;
  }
  }
}

static void exp2reg(pk_funcstate* fs, expdesc* e, int reg) {
  discharge2reg(fs, e, reg);
  e->f = e->t = NO_JUMP;
  e->info     = reg;
  e->kind     = EK_NON_RELOC;
}

int pk_code_checkstack(pk_funcstate* fs, int amt) {
  int newstack = fs->freereg + amt;
  if (newstack > fs->func->maxstacksize) {
    if (newstack > MAXREGS) {
      fs->func->maxstacksize = cast_byte(newstack);
    }
  }
}

int pk_code_reserveregs(pk_funcstate* fs, int amt) {
  pk_code_checkstack(fs, amt);
  fs->freereg += amt;
}

int pk_code_exp2next(pk_funcstate* fs, expdesc* e) {
  pk_discharge_vars(fs, e);
  freeexp(fs, e);
  pk_code_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}

int pk_code_exp2any(pk_funcstate* fs, expdesc* e) {
  pk_discharge_vars(fs, e);
  pk_code_exp2next(fs, e);
  return e->info;
}

void pk_code_infix(pk_parser* p, int op, expdesc* e) {
  pk_discharge_vars(p->fs, e);
  switch (op) {
  case OPR_ADD:
  case OPR_MINUS:
  case OPR_MULT:
  case OPR_DIV:
    if (!pk_tonumeral(e, NULL)) {
      pk_code_exp2any(p->fs, e);
    }
  }
}

static void swap_exps(expdesc* e1, expdesc* e2) {
  expdesc temp = *e1;
  *e1          = *e2;
  *e2          = temp;
}

static int fitsC(long i) { return ((unsigned long)(i)) + OFFSET_sC <= cast_uint(MAXARG_C); }

int pk_isKint(expdesc* e) { return (e->kind == EK_INTEGER && !hasjumps(e)); }

static int isSCint(expdesc* e) { return pk_isKint(e) && fitsC(e->ival); }

static void finish_bin_expval(pk_funcstate* fs, OpCode op, expdesc* e1, expdesc* e2, int v2,
                              int flip) {
  int v1 = pk_code_exp2any(fs, e1);
  int pc = pk_code_ABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->info = pc;
  e1->kind = EK_RELOC;
}

static int finish_bin_expneg(pk_funcstate* fs, OpCode op, expdesc* e1, expdesc* e2) {
  if (!pk_isKint(e2))
    return 0;
  else {
    long i2 = e2->ival;
    if (!(fitsC(i2) && fitsC(-i2))) {
      return 0;
    } else {
      int v2 = cast_int(i2);
      finish_bin_expval(fs, op, e1, e2, int2sC(-v2), 0);
      return 1;
    }
  }
}

static void code_bin_expval(pk_funcstate* fs, OpCode op, expdesc* e1, expdesc* e2, int flip) {
  int v2 = pk_code_exp2any(fs, e2);
  pk_assert(OP_ADD <= op && op <= OP_DIV);
  finish_bin_expval(fs, op, e1, e2, v2, flip);
}



static void code_bini(pk_funcstate* fs, OpCode op, expdesc* e1, expdesc* e2, int flip) {
  int v2 = int2sC(cast_int(e2->ival));
  pk_assert(e2->kind == EK_INTEGER);
  finish_bin_expval(fs, op, e1, e2, v2, flip);
}

static int pk_exp2K(pk_funcstate* fs, expdesc* e) {
  if (!hasjumps(e)) {
    int info;
    switch (e->kind) { 
    case EK_DOUBLE:
      info = pk_numberK(fs, e->dval);
      break;
    case EK_INTEGER:
      info = pk_intK(fs, e->ival);
      break;
    case EK_CONST:
      info = e->info;
      break;
    case EK_STRING: //TODO: implement strings
    default:
      return 0;
    }
    if (info <= MAXINDEXRK) {
        e->kind = EK_CONST;
      e->info = info;
        return 1;
    }
  }
  return 0;
}


static void code_arithmetic(pk_funcstate* fs, int opr, expdesc* e1, expdesc* e2, int flip) {
  if (pk_tonumeral(e2, NULL) && pk_exp2K(fs, e2)) {
    int v2 = e2->info;
    OpCode op = cast(OpCode, opr + OP_ADDK);
    finish_bin_expval(fs, op, e1, e2, v2, flip);
  } else {
      OpCode op = cast(OpCode, opr + OP_ADD);
    if (flip) {
      swap_exps(e1, e2);
    }
    code_bin_expval(fs, op, e1, e2, flip);
  }
}

static void code_commutative(pk_funcstate* fs, int op, expdesc* e1, expdesc* e2) {
  int flip = 0;
  if (pk_tonumeral(e1, NULL)) {
    swap_exps(e1, e2);
    flip = 1;
  }
  if (op == OPR_ADD && isSCint(e2)) {
    code_bini(fs, cast(OpCode, OP_ADDI), e1, e2, flip);
  } else {
    code_arithmetic(fs, op, e1, e2, flip);
  }
}

void pk_code_postfix(pk_parser* p, int op, expdesc* e1, expdesc* e2) {
  pk_discharge_vars(p->fs, e2);
  switch (op) {
  case OPR_ADD:
  case OPR_MULT: {
    code_commutative(p->fs, op, e1, e2);
    break;
  }
  case OPR_MINUS: {
    if (finish_bin_expneg(p->fs, OP_ADDI, e1, e2))
        break;
  }
  }
}