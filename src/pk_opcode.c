#include "pk_opcode.h"

const uint8_t pk_opmodes[NUM_OPCODES] = {
    [OP_MOVE]   = opmode(iABC),
    [OP_RETURN] = opmode(iABC),
    [OP_LOADI]  = opmode(iAsBx),
    [OP_LOADK]  = opmode(iABx),
};

const char* pk_opnames[NUM_OPCODES] = {[OP_MOVE]   = "OP_MOVE",
                                       [OP_RETURN] = "OP_RETURN",
                                       [OP_LOADI]  = "OP_LOADI",
                                       [OP_LOADK]  = "OP_LOADK"};
