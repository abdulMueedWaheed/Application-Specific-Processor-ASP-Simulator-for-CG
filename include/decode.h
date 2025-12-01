#ifndef DECODE_H
#define DECODE_H

#include "isa.h"

typedef struct {
    int valid;

    Opcode op;

    int32_t rs1_val;
    int32_t rs2_val;

    int rd;            // destination register number
    int32_t imm;       // immediate value

    uint32_t pc;       // original PC (for branches)

} IDEXReg;

void id_stage(DecodedInst *dec, IDEXReg *idex);

#endif