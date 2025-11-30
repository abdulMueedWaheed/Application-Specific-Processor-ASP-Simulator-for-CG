#include <stdlib.h>
#include <string.h>
#include "../include/cpu.h"

// IF/ID initialization
void init_ifid(IFIDReg *r) {
    r->instr_text = NULL;
    r->pc = 0;
    r->valid = 0;
}

// IF/ID cleanup
void free_ifid(IFIDReg *r) {
    if (r->instr_text)
        free(r->instr_text);
    r->instr_text = NULL;
    r->valid = 0;
}

// Set PC (needed for branches)
void set_pc(ProgramCounter *s, uint32_t pc_value) {
    s->pc = pc_value;
}

void if_stage_fetch(ProgramCounter *s, InstMem *im, IFIDReg *ifid)
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