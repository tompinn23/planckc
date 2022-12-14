#include "pk_debug.h"

#include "pk_object.h"
#include "pk_value.h"

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
    snprintf(buf, len, "%ld", val->l);
    break;
  case VALUE_DBL:
    snprintf(buf, len, "%f", val->d);
    break;
  case VALUE_BOOL:
    snprintf(buf, len, "%s", val->b ? "true" : "false");
    break;
  /*case VALUE_CFUNC:
    snprintf(buf, len, "<%p>", val->func);
    break;
  */
  case VALUE_OBJ:
    switch (val->o->type) {
    case OBJECT_FUNC: {
      //struct pk_func* fn = (struct pk_func*)val->obj;
      //snprintf(buf, len, "<func: %s>", fn->name != NULL ? fn->name->str : "script");
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
    switch(val->o->type) {
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
