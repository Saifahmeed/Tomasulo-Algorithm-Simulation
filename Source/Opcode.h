#ifndef OPCODE_H
#define OPCODE_H

#include <string>

enum class Opcode {
    ADD,
    ADDI,
    MUL,
    LOAD,
    STORE,
    BEQ,
    CALL,
    RET,
    NAND,
    NotKnown
};

std::string StringofOp(Opcode opcode);

#endif
