#ifndef EXECUTION_H
#define EXECUTION_H

#include "graphics.h"
#include "isa.h"
#include "parse_instruction.h"
#include <stdint.h>

typedef enum {
  EXEC_MODE_SINGLE_CYCLE = 0,
  EXEC_MODE_PIPELINED = 1
} ExecutionMode;

typedef struct {
  uint32_t cycle_count;
  uint32_t total_instructions;
  int32_t final_regs[32];
  ExecutionMode mode;
} ExecutionResult;

ExecutionResult *execute_program(ExecutionMode mode, InstMem *im,
                                 LabelEntry labels[], int label_count,
                                 Framebuffer *fb, const char *trace_filename);
void execution_free(ExecutionResult *result);

#endif
