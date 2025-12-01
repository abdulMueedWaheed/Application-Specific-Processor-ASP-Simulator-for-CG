#ifndef MEMORY_H
#define MEMORY_H

#include "parse_instruction.h"


void if_stage(ProgramCounter *pc, InstMem *im, IFIDreg *ifid);
void free_imem(InstMem *im);
int build_imem(const char *filename,
                     InstMem *im,
                     LabelEntry labels[],
                     int label_count);


#endif