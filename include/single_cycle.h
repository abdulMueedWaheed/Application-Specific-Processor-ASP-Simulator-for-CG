#ifndef SINGLE_CYCLE_H
#define SINGLE_CYCLE_H

#include <stdint.h>
#include "isa.h"
#include "graphics.h"

typedef struct {
    uint32_t cycle_count;
    uint32_t total_instructions;
    int32_t final_regs[32];
} SingleCycleResult;

SingleCycleResult* single_cycle_execute(InstMem *im, LabelEntry labels[], 
                                        int label_count, Framebuffer *fb);
void single_cycle_free(SingleCycleResult *result);

#endif
