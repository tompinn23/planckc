#include "pk_debug.h"

#include "pk_object.h"
#include "pk_value.h"
#include "pk_vm.h"
#include "pk_opcode.h"

#include <stdio.h>

void pk_debug_print_val(pk_value* val, char* buf, int len) {
  if (!val) {
    snprintf(buf, len, "Invalid");
    return;
  }
  switch (val->type) {
  case VALUE_NIL:
    snprintf(buf, len, "nil");
    break;
  case VALUE_INT:
    snprintf(buf, len, "%ld", val->ival);
    break;
  case VALUE_DBL:
    snprintf(buf, len, "%f", val->dval);
    break;
  case VALUE_BOOL:
    snprintf(buf, len, "%s", val->bval ? "true" : "false");
    break;
  /*case VALUE_CFUNC:
    snprintf(buf, len, "<%p>", val->func);
    break;
  */
  case VALUE_OBJ:
    switch (val->object->type) {
    case OBJECT_FUNC: {
      // struct pk_func* fn = (struct pk_func*)val->obj;
      // snprintf(buf, len, "<func: %s>", fn->name != NULL ? fn->name->str : "script");
      break;
    }
      /*case OBJECT_UPVALUE:
        snprintf(buf, len, "upvalue");
        break;
      */
    }
    break;
  default:
    snprintf(buf, len, "Forgot to implement debugging for this type");
  }
}

const char* pk_debug_value_type(pk_value* val) {
  switch (val->type) {
  case VALUE_BOOL:
    return "bool";
  case VALUE_DBL:
    return "double";
  case VALUE_INT:
    return "int";
  case VALUE_NIL:
    return "nil";
  case VALUE_OBJ:
    switch (val->object->type) {
    case OBJECT_FUNC:
      return "object<func>";
      break;
    default:
      return "object<unknown>";
    }
  default:
    return "unknown";
  }
}

void pk_chunk_debug(pk_chunk* chunk) {
  for (int i = 0; i < chunk->size; i++) {
    uint32_t code = *(chunk->code + i);
    OpCode op = GET_OPCODE(code);
    switch (getOpMode(op)) { 
    case iABC:
      printf("[%s] A: %d B: %d C: %d sC: %d\n", getOpName(op), GETARG_A(code), GETARG_B(code),
             GETARG_C(code), GETARG_sC(code));
      break;
    case iAx:
      printf("[%s] Ax: %d\n", getOpName(op), GETARG_Ax(code));
      break;
    case iAsBx:
      printf("[%s] A: %d sBx: %d\n", getOpName(op), GETARG_A(code), GETARG_sBx(code));
      break;
    case iABx:
        break;
    }
  }
}