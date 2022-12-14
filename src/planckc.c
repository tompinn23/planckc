#include "pk_opcode.h"
#include "pk_vm.h"
#include <stdio.h>

void print_byte_as_bits(char val) {
  for (int i = 7; 0 <= i; i--) {
    printf("%c", (val & (1 << i)) ? '1' : '0');
  }
}

void print_bits(char * ty, char * val, unsigned char * bytes, size_t num_bytes) {
  printf("(%*s) %*s = [ ", 15, ty, 16, val);
  for (size_t i = 0; i < num_bytes; i++) {
    print_byte_as_bits(bytes[i]);
    printf(" ");
  }
  printf("]\n");
}

#define SHOW(T,V) do { T x = V; print_bits(#T, #V, (unsigned char*) &x, sizeof(x)); } while(0)

void print_op(uint32_t op) {
  printf("A: %d B: %d C: %d k: %d\n", GETARG_A(op), TESTARG_k(op) ? GETARG_sB(op) : GETARG_B(op), GETARG_C(op), GETARG_k(op));
}

int main(int argc, char** argv) {
  Instruction op = CREATE_ABCk(OP_MOVE, 1, 4, 0, 0);
  SHOW(Instruction, op);
  op = CREATE_ABCk(OP_MOVE, 1, 16, 4, 0);
  SHOW(Instruction, op);
  print_op(op);
  op = CREATE_ABCk(OP_MOVE, 1, 16, 4, 1);
  SHOW(Instruction, op);
  print_op(op);
  pk_vm* vm = pk_vm_new();
  pk_chunk chunk;
  pk_chunk_init(&chunk);
  op = 0;
  SET_OPCODE(op, OP_LOADI);
  SETARG_sBx(op, -18406);
  pk_chunk_add(&chunk, op);
  pk_chunk_add(&chunk, CREATE_ABCk(OP_RETURN, 0, 0, 0, 0));
  pk_vm_do_chunk(vm, &chunk);
}
