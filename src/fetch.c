#include <stdlib.h>
#include <string.h>
#include "../include/parse_instruction.h"

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