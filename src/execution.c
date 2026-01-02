#include "../include/execution.h"
#include "../include/executor.h"
#include "../include/parse_instruction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_MEM_SIZE 4096
static int32_t data_memory[DATA_MEM_SIZE];

// ============================================================================
// TRACING UTILITIES
// ============================================================================

static FILE *trace_file = NULL;

static void open_trace(const char *filename) {
  if (!trace_file && filename)
    trace_file = fopen(filename, "w");
}

static void close_trace() {
  if (trace_file) {
    fclose(trace_file);
    trace_file = NULL;
  }
}

static void trace_reg_file(uint32_t cycle, uint32_t pc, int32_t *regs) {
  if (!trace_file)
    return;
  fprintf(trace_file, "Cycle %u (PC=%u): ", cycle, pc);
  for (int i = 0; i < 32; i++) {
    if (regs[i] != 0)
      fprintf(trace_file, "x%d=%d ", i, regs[i]);
  }
  fprintf(trace_file, "\n");
}

static void trace_pipeline_state(uint32_t cycle, IFIDreg *ifid, IDEXreg *idex,
                                 EXIOreg *exio, IOMEMreg *iomem,
                                 MEMWBreg *memwb) {
  if (!trace_file)
    return;
  fprintf(trace_file, "Cycle %u:\n", cycle);
  if (ifid->valid)
    fprintf(trace_file, "  IF/ID: PC=%u\n", ifid->pc);
  else
    fprintf(trace_file, "  IF/ID: Bubble\n");

  if (idex->valid)
    fprintf(trace_file, "  ID/EX: Op=%d RD=%d RS1=%d RS2=%d\n", idex->op,
            idex->rd, idex->rs1_idx, idex->rs2_idx);
  else
    fprintf(trace_file, "  ID/EX: Bubble\n");

  if (exio->valid)
    fprintf(trace_file, "  EX/IO: Op=%d RD=%d Res=%d\n", exio->op, exio->rd,
            exio->alu_result);
  else
    fprintf(trace_file, "  EX/IO: Bubble\n");

  if (iomem->valid)
    fprintf(trace_file, "  IO/MEM: Op=%d RD=%d Res=%d\n", iomem->op, iomem->rd,
            iomem->alu_result);
  else
    fprintf(trace_file, "  IO/MEM: Bubble\n");

  if (memwb->valid)
    fprintf(trace_file, "  MEM/WB: RD=%d WData=%d\n", memwb->rd,
            memwb->write_data);
  else
    fprintf(trace_file, "  MEM/WB: Bubble\n");

  fprintf(trace_file, "--------------------------------\n");
}

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

      // Execute instruction (Unified Handling)
      int32_t rs1_val = read_register(regs, decoded.rs1);
      int32_t rs2_val = read_register(regs, decoded.rs2);

      ExecResult res = execute_inst(
          decoded.op, decoded.rd, decoded.rs1, decoded.rs2, decoded.imm, pc,
          rs1_val, rs2_val, regs, fb, data_memory, DATA_MEM_SIZE);

      // Update PC based on result
      if (res.is_branch && res.branch_taken) {
        pc = res.next_pc;
        printf("  Branch TAKEN to PC=%u\n", pc);
      } else {
        pc = res.next_pc; // Usually pc + 1
      }
    } else {
      printf("  INVALID instruction\n");
      pc++;
    }

    // Trace
    trace_reg_file(cycle, pc, regs);

    printf("\n");
    cycle++;

    if (cycle > 1000000) {
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

  if (trace_file) {
    fprintf(trace_file, "\n=== SIMULATION SUMMARY ===\n");
    fprintf(trace_file, "Mode: SINGLE-CYCLE\n");
    fprintf(trace_file, "Total Cycles: %u\n", cycle);
    fprintf(trace_file, "Total Instructions: %lu\n", im->size);
    fprintf(trace_file, "CPI: 1.0\n");
  }

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
  while (idle < 6 && cycle < 1000000) {
    // Execute stages in reverse order (so latest results propagate)
    wb_stage(&memwb);
    mem_stage(&iomem, &memwb);
    io_stage(&exio, &iomem);

    // Use unified executor in EX stage
    // Note: Logic inside ex_stage is now handling the execution
    ex_stage(&idex, &exio, &iomem, &memwb);

    // --- PIPELINE CONTROL: BRANCH FLUSH ---
    // If a branch was taken in EX stage, we must flush IF/ID and ID/EX
    // and update PC to the target.
    if (exio.valid && exio.branch_taken) {
      printf("[Branch] Taken at PC=%u -> Target=%u. Flushing pipeline.\n",
             exio.pc, exio.target_pc);

      // Update PC
      pc.pc = exio.target_pc;

      // Flush younger stages
      init_ifid(&ifid);
      init_idex(&idex);

      // We must also ensure we don't re-fetch from the old PC or decode bad
      // data The updated PC will be used in next fetch.
    }

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

    // Trace
    trace_pipeline_state(cycle, &ifid, &idex, &exio, &iomem, &memwb);
    trace_reg_file(cycle, pc.pc, regs); // Added register dump as requested

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

  if (trace_file) {
    fprintf(trace_file, "\n=== SIMULATION SUMMARY ===\n");
    fprintf(trace_file, "Mode: PIPELINED (6-Stage)\n");
    fprintf(trace_file, "Total Cycles: %u\n", cycle);
    fprintf(trace_file, "Total Instructions: %lu\n", im->size);
    fprintf(trace_file, "CPI: %.2f\n", cpi);
  }

  free_ifid(&ifid);

  return result;
}

// ============================================================================
// UNIFIED EXECUTION INTERFACE
// ============================================================================

ExecutionResult *execute_program(ExecutionMode mode, InstMem *im,
                                 LabelEntry labels[], int label_count,
                                 Framebuffer *fb, const char *trace_filename) {
  open_trace(trace_filename);
  ExecutionResult *res = NULL;
  if (mode == EXEC_MODE_SINGLE_CYCLE) {
    res = execute_single_cycle(im, labels, label_count, fb);
  } else if (mode == EXEC_MODE_PIPELINED) {
    res = execute_pipelined(im, labels, label_count, fb);
  }
  close_trace();
  return res;
}

void execution_free(ExecutionResult *result) {
  if (result)
    free(result);
}
