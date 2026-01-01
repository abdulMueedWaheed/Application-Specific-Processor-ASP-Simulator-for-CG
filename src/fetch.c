#include "../include/isa.h"
#include <stdlib.h>
#include <string.h>

void if_stage(ProgramCounter *s, InstMem *im, IFIDreg *ifid) {
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
