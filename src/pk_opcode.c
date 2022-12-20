#include "pk_opcode.h"

const uint8_t pk_opmodes[NUM_OPCODES] = {
    [OP_MOVE] = opmode(iABC),    [OP_RETURN] = opmode(iABC), [OP_LOADI] = opmode(iAsBx),
    [OP_LOADF] = opmode(iAsBx),  [OP_LOADK] = opmode(iABx),  [OP_LOADKX] = opmode(iABx),
    [OP_EXTRAARG] = opmode(iAx), [OP_ADDI] = opmode(iABC)};

const char* pk_opnames[NUM_OPCODES] = {
    [OP_MOVE] = "OP_MOVE",         [OP_RETURN] = "OP_RETURN", [OP_LOADI] = "OP_LOADI",
    [OP_LOADF] = "OP_LOADF",       [OP_LOADK] = "OP_LOADK",   [OP_LOADKX] = "OP_LOADKX",
                                       [OP_EXTRAARG] = "OP_EXTRAARG", [OP_ADDI] = "OP_ADDI",
                                       [OP_ADDK]     = "OP_ADDK",
                                       [OP_SUBK]     = "OP_SUBK",
                                       [OP_MULTK]    = "OP_MULTK"};
