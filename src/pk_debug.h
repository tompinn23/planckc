#pragma once

typedef struct pk_value pk_value;

void pk_debug_print_val(pk_value *val, char *buf, int len);
const char *pk_debug_value_type(pk_value *val);
