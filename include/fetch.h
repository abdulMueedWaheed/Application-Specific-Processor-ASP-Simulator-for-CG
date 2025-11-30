#ifndef MEMORY_H
#define MEMORY_H

#include "cpu.h"
#include "isa.h"

void free_imem(InstMem *im);
int build_imem(const char *filename,
                     InstMem *im,
                     LabelEntry labels[],
                     int label_count);

#endif