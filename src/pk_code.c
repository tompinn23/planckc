#include "pk_code.h"

#include "pk_value.h"
#include "pk_vm.h"

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

/* Ensure that the expression is somewhere not a variable, register etc. Make non transient*/
static void pk_discharge_vars(pk_function* fs, expdesc* e) {
  // TODO: implement things like upvalue and table access.
  switch (e->kind) {
  default:
    break;
  }
}

void pk_code_infix(pk_function* fs, int op, expdesc* e) {
  pk_discharge_vars(fs, e);
  switch (op) {
  case OPR_ADD:
  case OPR_MINUS:
  case OPR_MULT:
  case OPR_DIV:
    if (!pk_tonumeral(e, NULL)) {
    }
  }
}
