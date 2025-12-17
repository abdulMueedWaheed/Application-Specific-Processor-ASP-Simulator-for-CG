#include <stdlib.h>
#include <string.h>
#include "../include/isa.h"

// IF/ID initialization
void init_ifid(IFIDreg *r) {
    r->instr_text = NULL;
    r->pc = 0;
    r->valid = 0;
}

// IF/ID cleanup
void free_ifid(IFIDreg *r) {
    if (r->instr_text)
        free(r->instr_text);
    r->instr_text = NULL;
    r->valid = 0;
}

// ID/EX initialization
void init_idex(IDEXreg *r) {
    r->valid = 0;
    r->op = OP_NOP;
    r->rs1_val = 0;
    r->rs2_val = 0;
    r->rd = -1;
    r->imm = 0;
    r->pc = 0;
}

// EX/MEM initialization
void init_exmem(EXMEMreg *r) {
    r->valid = 0;
    r->op = OP_NOP;
    r->alu_result = 0;
    r->rs2_val = 0;
    r->rd = -1;
    r->pc = 0;
}

// MEM/WB initialization
void init_memwb(MEMWBreg *r) {
    r->valid = 0;
    r->write_data = 0;
    r->rd = -1;
    r->is_memory = 0;
}

void if_stage(ProgramCounter *s, InstMem *im, IFIDreg *ifid)
{
    // Clear previous contents
    if (ifid->instr_text) {
        free(ifid->instr_text);
        ifid->instr_text = NULL;
    }
    ifid->valid = 0;

    // If PC out of bounds → bubble
    if (s->pc >= im->size) {
        return;
    }

    ifid->instr_text = strdup(im->lines[s->pc]);
    ifid->pc = s->pc;
    ifid->valid = 1;

    // Move PC forward
    s->pc++;
}
