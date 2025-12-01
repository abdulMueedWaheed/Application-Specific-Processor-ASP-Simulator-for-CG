#include "../include/isa.h"

void id_stage(DecodedInst *dec, IDEXreg *idex) {
    if (!dec->valid) {
        // Pass a bubble into ID/EX
        idex->valid = 0;
        return;
    }

    idex->valid = 1;
    idex->op    = dec->op;

    // Read operand values from register file
    idex->rs1_val = regs[dec->rs1];
    idex->rs2_val = regs[dec->rs2];

    idex->rd  = dec->rd;
    idex->imm = dec->imm;
    idex->pc  = dec->pc;
}
