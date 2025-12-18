#include "../include/isa.h"
#include "../include/graphics.h"
#include "../include/executor.h"
#include <stdio.h>
#include <string.h>

// External global framebuffer
Framebuffer *global_fb = NULL;

// Simple data memory (simulated)
#define DATA_MEM_SIZE 4096
static int32_t data_memory[DATA_MEM_SIZE];

// ========== EXECUTE STAGE ==========
void ex_stage(IDEXreg *idex, EXMEMreg *exmem) {
    // Initialize output as bubble
    exmem->valid = 0;
    
    if (!idex->valid) {
        return;  // Pass through bubble
    }

    exmem->valid = 1;
    exmem->op = idex->op;
    exmem->rd = idex->rd;
    exmem->pc = idex->pc;
    exmem->rs2_val = idex->rs2_val;  // needed for SW

    // Use unified executor
    ExecResult exec_result = execute_inst(
        idex->op,
        idex->rd, -1, -1,  // Registers already read in ID stage
        idex->imm,
        idex->pc,
        idex->rs1_val, idex->rs2_val,
        regs,  // Global register file
        global_fb,
        data_memory,
        DATA_MEM_SIZE
    );

    exmem->alu_result = exec_result.alu_result;
}

// ========== MEMORY STAGE ==========
void mem_stage(EXMEMreg *exmem, MEMWBreg *memwb) {
    // Initialize output as bubble
    memwb->valid = 0;
    
    if (!exmem->valid) {
        return;  // Pass through bubble
    }

    memwb->valid = 1;
    memwb->rd = exmem->rd;
    memwb->is_memory = 0;  // Default: ALU result
    memwb->write_data = exmem->alu_result;

    // Handle memory operations
    switch (exmem->op) {
        case OP_LW: {
            // Load from memory
            uint32_t addr = exmem->alu_result;
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
            uint32_t addr = exmem->alu_result;
            if (addr < DATA_MEM_SIZE) {
                data_memory[addr] = exmem->rs2_val;
                memwb->valid = 1;
                memwb->rd = -1;  // No writeback register for store
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
            memwb->rd = -1;  // No register writeback
            break;

        default:
            // All other operations: ALU result is passed through
            memwb->write_data = exmem->alu_result;
            break;
    }
}

// ========== WRITEBACK STAGE ==========
void wb_stage(MEMWBreg *memwb) {
    if (!memwb->valid) {
        return;  // Bubble: nothing to write back
    }

    // Write back to register file
    if (memwb->rd >= 0 && memwb->rd < 32) {
        regs[memwb->rd] = memwb->write_data;
        printf("WB: Wrote 0x%x to register x%d\n", memwb->write_data, memwb->rd);
    }
}

// ========== LEGACY ALU OPERATIONS (kept for reference) ==========
int add(int a, int b) {
    return regs[a] + regs[b];
}

int addi(int a, int b) {
    return regs[a] + b;
}

int sub(int a, int b) {
    return regs[a] - regs[b];
}

int mul(int a, int b) {
    return regs[a] * regs[b];
}
