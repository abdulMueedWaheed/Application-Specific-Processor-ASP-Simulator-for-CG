#include "../include/executor.h"
#include "../include/graphics.h"
#include "../include/isa.h"
#include <stdio.h>
#include <string.h>

// External global framebuffer
Framebuffer *global_fb = NULL;

// Simple data memory (simulated)
#define DATA_MEM_SIZE 4096
static int32_t data_memory[DATA_MEM_SIZE];

// ========== EXECUTE STAGE ==========
void ex_stage(IDEXreg *idex, EXIOreg *exio, IOMEMreg *iomem_fwd,
              MEMWBreg *memwb_fwd) {
  // Initialize output as bubble
  exio->valid = 0;

  if (!idex->valid) {
    return; // Pass through bubble
  }

  exio->valid = 1;
  exio->op = idex->op;
  exio->rd = idex->rd;
  exio->pc = idex->pc;

  // Initial values from ID (RegFile read)
  int32_t current_rs1_val = idex->rs1_val;
  int32_t current_rs2_val = idex->rs2_val;

  // --- FORWARDING LOGIC ---
  // Priority: IOMEM (youngest/most recent) > MEMWB (older)

  // Forwarding for RS1
  if (idex->rs1_idx != 0) { // Don't forward r0
    if (iomem_fwd->valid && iomem_fwd->rd != 0 &&
        iomem_fwd->rd == idex->rs1_idx) {
      current_rs1_val = iomem_fwd->alu_result;
    } else if (memwb_fwd->valid && memwb_fwd->rd != 0 &&
               memwb_fwd->rd == idex->rs1_idx) {
      current_rs1_val = memwb_fwd->write_data;
    }
  }

  // Forwarding for RS2
  if (idex->rs2_idx != 0) {
    if (iomem_fwd->valid && iomem_fwd->rd != 0 &&
        iomem_fwd->rd == idex->rs2_idx) {
      current_rs2_val = iomem_fwd->alu_result;
    } else if (memwb_fwd->valid && memwb_fwd->rd != 0 &&
               memwb_fwd->rd == idex->rs2_idx) {
      current_rs2_val = memwb_fwd->write_data;
    }
  }

  exio->rs1_val = current_rs1_val;
  exio->rs2_val = current_rs2_val;
  exio->imm = idex->imm;

  // Use unified executor with NULL framebuffer
  // Graphics ops will be effectively NOPs here but valid ops
  ExecResult exec_result = execute_inst(
      idex->op, idex->rd, -1, -1, // Registers already read in ID stage
      idex->imm, idex->pc, current_rs1_val, current_rs2_val,
      regs, // Global register file
      NULL, // NO FRAMEBUFFER IN EX STAGE
      data_memory, DATA_MEM_SIZE);

  exio->alu_result = exec_result.alu_result;
  exio->branch_taken = (exec_result.is_branch && exec_result.branch_taken);
  exio->target_pc = exec_result.next_pc;
}

// ========== I/O STAGE ==========
void io_stage(EXIOreg *exio, IOMEMreg *iomem) {
  // Initialize output as bubble
  iomem->valid = 0;

  if (!exio->valid) {
    return;
  }

  iomem->valid = 1;
  iomem->op = exio->op;
  iomem->rd = exio->rd;
  iomem->pc = exio->pc;
  iomem->rs2_val = exio->rs2_val;
  iomem->alu_result = exio->alu_result;

  // Execute only graphics instructions
  if (exio->op == OP_DRAWPIX || exio->op == OP_DRAWSTEP ||
      exio->op == OP_SETCLR || exio->op == OP_CLEARFB ||
      exio->op == OP_MOVETO || exio->op == OP_LINETO) {

    execute_inst(exio->op, exio->rd, -1, -1, exio->imm, exio->pc, exio->rs1_val,
                 exio->rs2_val, regs,
                 global_fb,   // ACCESS FRAMEBUFFER HERE
                 data_memory, // Should not touch memory but passed anyway
                 DATA_MEM_SIZE);
  }
}

// ========== MEMORY STAGE ==========
void mem_stage(IOMEMreg *iomem, MEMWBreg *memwb) {
  // Initialize output as bubble
  memwb->valid = 0;

  if (!iomem->valid) {
    return; // Pass through bubble
  }

  memwb->valid = 1;
  memwb->rd = iomem->rd;
  memwb->is_memory = 0; // Default: ALU result
  memwb->write_data = iomem->alu_result;

  // Handle memory operations
  switch (iomem->op) {
  case OP_LW: {
    // Load from memory
    uint32_t addr = iomem->alu_result;
    if (addr < DATA_MEM_SIZE) {
      memwb->write_data = data_memory[addr];
      memwb->is_memory = 1;
    } else {
      printf("Memory access violation: LW at address 0x%x\n", addr);
    }
    break;
  }

  case OP_SW: {
    // Store to memory
    uint32_t addr = iomem->alu_result;
    if (addr < DATA_MEM_SIZE) {
      data_memory[addr] = iomem->rs2_val;
      memwb->valid = 1;
      memwb->rd = -1; // No writeback register for store
    } else {
      printf("Memory access violation: SW at address 0x%x\n", addr);
    }
    break;
  }

  // Graphics operations don't need memory stage
  case OP_DRAWPIX:
  case OP_DRAWSTEP:
  case OP_SETCLR:
  case OP_CLEARFB:
  case OP_MOVETO:
  case OP_LINETO:
    memwb->rd = -1; // No register writeback
    break;

  default:
    // All other operations: ALU result is passed through
    memwb->write_data = iomem->alu_result;
    break;
  }
}

// ========== WRITEBACK STAGE ==========
void wb_stage(MEMWBreg *memwb) {
  if (!memwb->valid) {
    return; // Bubble: nothing to write back
  }

  // Write back to register file
  if (memwb->rd >= 0 && memwb->rd < 32) {
    regs[memwb->rd] = memwb->write_data;
    printf("WB: Wrote 0x%x to register x%d\n", memwb->write_data, memwb->rd);
  }
}

// ========== LEGACY ALU OPERATIONS (kept for reference) ==========
int add(int a, int b) { return regs[a] + regs[b]; }

int addi(int a, int b) { return regs[a] + b; }

int sub(int a, int b) { return regs[a] - regs[b]; }

int mul(int a, int b) { return regs[a] * regs[b]; }
