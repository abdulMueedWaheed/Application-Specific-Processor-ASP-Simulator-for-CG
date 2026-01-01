#include "../include/execution.h"
#include "../include/executor.h"
#include "../include/parse_instruction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_MEM_SIZE 4096
static int32_t data_memory[DATA_MEM_SIZE];

// ============================================================================
// SINGLE-CYCLE EXECUTION MODE
// ============================================================================

static ExecutionResult *
execute_single_cycle(InstMem *im, LabelEntry labels[] __attribute__((unused)),
                     int label_count __attribute__((unused)), Framebuffer *fb) {
  ExecutionResult *result = (ExecutionResult *)malloc(sizeof(ExecutionResult));
  if (!result)
    return NULL;

  int32_t regs[32];
  memset(regs, 0, sizeof(regs));

  uint32_t pc = 0;
  uint32_t cycle = 0;

  printf("\n=== SINGLE-CYCLE EXECUTION MODEL ===\n");
  printf("Each instruction completes in exactly 1 cycle\n\n");

  while (pc < im->size) {
    printf("Cycle %u: PC=%u\n", cycle, pc);

    // Parse instruction
    DecodedInst decoded;
    memset(&decoded, 0, sizeof(decoded));

    IFIDreg temp_ifid;
    temp_ifid.instr_text = im->lines[pc];
    temp_ifid.pc = pc;
    temp_ifid.valid = 1;

    instruction_parser(&temp_ifid, &decoded);

    if (decoded.valid) {
      printf("  Instr: %s\n", im->lines[pc]);
      printf("  Op=%s rd=%d rs1=%d rs2=%d imm=%d\n",
             (decoded.op < 14) ? "OP" : "INVALID", decoded.rd, decoded.rs1,
             decoded.rs2, decoded.imm);

      // Handle branch
      if (decoded.op == OP_BEQ) {
        int32_t rs1_val = read_register(regs, decoded.rs1);
        int32_t rs2_val = read_register(regs, decoded.rs2);

        if (rs1_val == rs2_val) {
          pc = (pc + decoded.imm) & 0xFFFFFFFF;
          printf("  Branch TAKEN to PC=%u\n", pc);
        } else {
          pc++;
          printf("  Branch NOT taken, PC=%u\n", pc);
        }
      } else {
        // Execute instruction
        int32_t rs1_val = read_register(regs, decoded.rs1);
        int32_t rs2_val = read_register(regs, decoded.rs2);

        execute_inst(decoded.op, decoded.rd, decoded.rs1, decoded.rs2,
                     decoded.imm, pc, rs1_val, rs2_val, regs, fb, data_memory,
                     DATA_MEM_SIZE);

        pc++;
      }
    } else {
      printf("  INVALID instruction\n");
      pc++;
    }

    printf("\n");
    cycle++;

    if (cycle > 10000) {
      printf("ERROR: Infinite loop detected\n");
      break;
    }
  }

  result->cycle_count = cycle;
  result->total_instructions = im->size;
  result->mode = EXEC_MODE_SINGLE_CYCLE;
  memcpy(result->final_regs, regs, sizeof(regs));

  printf("=== SINGLE-CYCLE RESULTS ===\n");
  printf("Total cycles: %u\n", cycle);
  printf("Total instructions: %lu\n", im->size);
  printf("CPI (Cycles Per Instruction): 1.0\n\n");

  return result;
}

// ============================================================================
// PIPELINED EXECUTION MODE
// ============================================================================

static ExecutionResult *
execute_pipelined(InstMem *im, LabelEntry labels[] __attribute__((unused)),
                  int label_count __attribute__((unused)), Framebuffer *fb) {
  ExecutionResult *result = (ExecutionResult *)malloc(sizeof(ExecutionResult));
  if (!result)
    return NULL;

  int32_t regs[32];
  memset(regs, 0, sizeof(regs));

  // Initialize pipeline registers
  IFIDreg ifid;
  IDEXreg idex;
  EXIOreg exio;
  IOMEMreg iomem;
  MEMWBreg memwb;

  init_ifid(&ifid);
  init_idex(&idex);
  init_exio(&exio);
  init_iomem(&iomem);
  init_memwb(&memwb);

  ProgramCounter pc = {.pc = 0};
  uint32_t cycle = 0;

  printf("\n=== PIPELINED EXECUTION MODEL ===\n");
  printf("6-stage pipeline: IF → ID → EX → IO → MEM → WB\n\n");
  printf("Starting pipeline simulation...\n\n");

  int idle = 0;
  while (idle < 6 && cycle < 1000) {
    // Execute stages in reverse order (so latest results propagate)
    wb_stage(&memwb);
    mem_stage(&iomem, &memwb);
    io_stage(&exio, &iomem);

    // Use unified executor in EX stage
    // Note: Logic inside ex_stage is now handling the execution
    ex_stage(&idex, &exio);

    // Decode and ID stage
    DecodedInst decoded;
    memset(&decoded, 0, sizeof(decoded));
    instruction_parser(&ifid, &decoded);
    id_stage(&decoded, &idex);

    // Fetch stage
    if_stage(&pc, im, &ifid);

    if (ifid.valid) {
      idle = 0;
    } else {
      idle++;
    }

    cycle++;
  }

  result->cycle_count = cycle;
  result->total_instructions = im->size;
  result->mode = EXEC_MODE_PIPELINED;
  memcpy(result->final_regs, regs, sizeof(regs));

  printf("\n=== PIPELINED RESULTS ===\n");
  printf("Total cycles: %u\n", cycle);
  printf("Total instructions: %lu\n", im->size);
  double cpi = (im->size > 0) ? (double)cycle / im->size : 0;
  printf("CPI (Cycles Per Instruction): %.2f\n\n", cpi);

  free_ifid(&ifid);

  return result;
}

// ============================================================================
// UNIFIED EXECUTION INTERFACE
// ============================================================================

ExecutionResult *execute_program(ExecutionMode mode, InstMem *im,
                                 LabelEntry labels[], int label_count,
                                 Framebuffer *fb) {
  if (mode == EXEC_MODE_SINGLE_CYCLE) {
    return execute_single_cycle(im, labels, label_count, fb);
  } else if (mode == EXEC_MODE_PIPELINED) {
    return execute_pipelined(im, labels, label_count, fb);
  }
  return NULL;
}

void execution_free(ExecutionResult *result) {
  if (result)
    free(result);
}
