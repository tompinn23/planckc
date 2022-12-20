#pragma once

#include "pk_commons.h"
#include <limits.h>
#include <stdint.h>

/*===========================================================================
  We assume that instructions are unsigned 32-bit integers.
  All instructions have an opcode in the first 7 bits.
  Instructions can have the following formats:

        3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0
        1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
iABC          C(8)     |      B(8)     |k|     A(8)      |   Op(7)     |
iABx                Bx(17)               |     A(8)      |   Op(7)     |
iAsBx              sBx (signed)(17)      |     A(8)      |   Op(7)     |
iAx                           Ax(25)                     |   Op(7)     |
isJ                           sJ(25)                     |   Op(7)     |

  A signed argument is represented in excess K: the represented value is
  the written unsigned value minus K, where K is half the maximum for the
  corresponding unsigned argument.
===========================================================================*/

typedef uint32_t Instruction;

enum OpMode { iABC, iABx, iAsBx, iAx, isJ }; /* basic instruction formats */

/*
** size and position of opcode arguments.
*/
#define SIZE_C 8
#define SIZE_B 8
#define SIZE_Bx (SIZE_C + SIZE_B + 1)
#define SIZE_A 8
#define SIZE_Ax (SIZE_Bx + SIZE_A)
#define SIZE_sJ (SIZE_Bx + SIZE_A)

#define SIZE_OP 7

#define POS_OP 0

#define POS_A (POS_OP + SIZE_OP)
#define POS_k (POS_A + SIZE_A)
#define POS_B (POS_k + 1)
#define POS_C (POS_B + SIZE_B)

#define POS_Bx POS_k

#define POS_Ax POS_A

#define POS_sJ POS_A

/*
** limits for opcode arguments.
** we use (signed) 'int' to manipulate most arguments,
** so they must fit in ints.
*/

/* Check whether type 'int' has at least 'b' bits ('b' < 32) */
#define L_INTHASBITS(b) ((UINT_MAX >> ((b)-1)) >= 1)

#if L_INTHASBITS(SIZE_Bx)
#define MAXARG_Bx ((1 << SIZE_Bx) - 1)
#else
#define MAXARG_Bx INT_MAX
#endif

#define OFFSET_sBx (MAXARG_Bx >> 1) /* 'sBx' is signed */

#if L_INTHASBITS(SIZE_Ax)
#define MAXARG_Ax ((1 << SIZE_Ax) - 1)
#else
#define MAXARG_Ax INT_MAX
#endif

#if L_INTHASBITS(SIZE_sJ)
#define MAXARG_sJ ((1 << SIZE_sJ) - 1)
#else
#define MAXARG_sJ INT_MAX
#endif

#define OFFSET_sJ (MAXARG_sJ >> 1)

#define MAXARG_A ((1 << SIZE_A) - 1)
#define MAXARG_B ((1 << SIZE_B) - 1)
#define MAXARG_C ((1 << SIZE_C) - 1)
#define OFFSET_sC (MAXARG_C >> 1)

#define int2sC(i) ((i) + OFFSET_sC)
#define sC2int(i) ((i)-OFFSET_sC)

/* creates a mask with 'n' 1 bits at position 'p' */
#define MASK1(n, p) ((~((~(Instruction)0) << (n))) << (p))

/* creates a mask with 'n' 0 bits at position 'p' */
#define MASK0(n, p) (~MASK1(n, p))

/*
** the following macros help to manipulate instructions
*/

#define GET_OPCODE(i) (cast(OpCode, ((i) >> POS_OP) & MASK1(SIZE_OP, 0)))
#define SET_OPCODE(i, o)                                                                           \
  ((i) = (((i)&MASK0(SIZE_OP, POS_OP)) |                                                           \
          ((cast(Instruction, o) << POS_OP) & MASK1(SIZE_OP, POS_OP))))

#define checkopm(i, m) (getOpMode(GET_OPCODE(i)) == m)

#define getarg(i, pos, size) (cast_int(((i) >> (pos)) & MASK1(size, 0)))
#define setarg(i, v, pos, size)                                                                    \
  ((i) = (((i)&MASK0(size, pos)) | ((cast(Instruction, v) << pos) & MASK1(size, pos))))

#define GETARG_A(i) getarg(i, POS_A, SIZE_A)
#define SETARG_A(i, v) setarg(i, v, POS_A, SIZE_A)

#define GETARG_B(i) check_exp(checkopm(i, iABC), getarg(i, POS_B, SIZE_B))
#define GETARG_sB(i) sC2int(GETARG_B(i))
#define SETARG_B(i, v) setarg(i, v, POS_B, SIZE_B)

#define GETARG_C(i) check_exp(checkopm(i, iABC), getarg(i, POS_C, SIZE_C))
#define GETARG_sC(i) sC2int(GETARG_C(i))
#define SETARG_C(i, v) setarg(i, v, POS_C, SIZE_C)

#define TESTARG_k(i) check_exp(checkopm(i, iABC), (cast_int(((i) & (1u << POS_k)))))
#define GETARG_k(i) check_exp(checkopm(i, iABC), getarg(i, POS_k, 1))
#define SETARG_k(i, v) setarg(i, v, POS_k, 1)

#define GETARG_Bx(i) check_exp(checkopm(i, iABx), getarg(i, POS_Bx, SIZE_Bx))
#define SETARG_Bx(i, v) setarg(i, v, POS_Bx, SIZE_Bx)

#define GETARG_Ax(i) check_exp(checkopm(i, iAx), getarg(i, POS_Ax, SIZE_Ax))
#define SETARG_Ax(i, v) setarg(i, v, POS_Ax, SIZE_Ax)

#define GETARG_sBx(i) check_exp(checkopm(i, iAsBx), getarg(i, POS_Bx, SIZE_Bx) - OFFSET_sBx)
#define SETARG_sBx(i, b) SETARG_Bx((i), cast_uint((b) + OFFSET_sBx))

#define GETARG_sJ(i) check_exp(checkopm(i, isJ), getarg(i, POS_sJ, SIZE_sJ) - OFFSET_sJ)
#define SETARG_sJ(i, j) setarg(i, cast_uint((j) + OFFSET_sJ), POS_sJ, SIZE_sJ)

#define CREATE_ABCk(o, a, b, c, k)                                                                 \
  ((cast(Instruction, o) << POS_OP) | (cast(Instruction, a) << POS_A) |                            \
   (cast(Instruction, b) << POS_B) | (cast(Instruction, c) << POS_C) |                             \
   (cast(Instruction, k) << POS_k))

#define CREATE_ABx(o, a, bc)                                                                       \
  ((cast(Instruction, o) << POS_OP) | (cast(Instruction, a) << POS_A) |                            \
   (cast(Instruction, bc) << POS_Bx))

#define CREATE_Ax(o, a) ((cast(Instruction, o) << POS_OP) | (cast(Instruction, a) << POS_Ax))

#define CREATE_sJ(o, j, k)                                                                         \
  ((cast(Instruction, o) << POS_OP) | (cast(Instruction, j) << POS_sJ) |                           \
   (cast(Instruction, k) << POS_k))

#if !defined(MAXINDEXRK) /* (for debugging only) */
#define MAXINDEXRK MAXARG_B
#endif

/*
** invalid register that fits in 8 bits
*/
#define NO_REG MAXARG_A

/*
   R[x] = register x
   K[x] = constant x
*/

typedef enum {
  OP_MOVE,   // AB R[A] = R[B]
  OP_RETURN, // return R{A}
  OP_LOADI,  // AsBx R[A] = sBx
  OP_LOADF,  // AsBx R[A] = sBx
  OP_LOADK,  // ABx R[A] = K[Bx]
  OP_LOADKX,
  OP_EXTRAARG,
  
  OP_ADDI,

  OP_ADDK,
  OP_SUBK,
  OP_MULTK,
  OP_DIVK,

  OP_ADD,
  OP_SUB,
  OP_MULT,
  OP_DIV,
  NUM_OPCODES,
} OpCode;

extern const uint8_t pk_opmodes[NUM_OPCODES];
extern const char* pk_opnames[NUM_OPCODES];

#define opmode(m) ((m))
#define getOpMode(m) (cast(enum OpMode, pk_opmodes[m] & 7))
#define getOpName(m) pk_opnames[m]
