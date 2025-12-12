#include "../include/isa.h"
#include <stdio.h>
#include <string.h>

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
    exmem->alu_result = 0;

    // Execute instruction based on opcode
    switch (idex->op) {
        case OP_ADD:
            exmem->alu_result = idex->rs1_val + idex->rs2_val;
            break;

        case OP_ADDI:
            exmem->alu_result = idex->rs1_val + idex->imm;
            break;

        case OP_SUB:
            exmem->alu_result = idex->rs1_val - idex->rs2_val;
            break;

        case OP_SUBI:
            exmem->alu_result = idex->rs1_val - idex->imm;
            break;

        case OP_MUL:
            exmem->alu_result = idex->rs1_val * idex->rs2_val;
            break;

        case OP_LW:
            // Address calculation (imm is offset, rs1_val is base address)
            exmem->alu_result = idex->rs1_val + idex->imm;
            break;

        case OP_SW:
            // Address calculation (imm is offset, rs1_val is base address)
            exmem->alu_result = idex->rs1_val + idex->imm;
            break;

        case OP_BEQ:
            // Branch: compare rs1 and rs2, target is rs1_val + imm (PC-relative offset)
            if (idex->rs1_val == idex->rs2_val) {
                // Branch taken: target = PC + imm
                exmem->alu_result = idex->pc + idex->imm;
            } else {
                // Branch not taken
                exmem->alu_result = idex->pc + 1;  // Next instruction
            }
            break;

        case OP_NOP:
            exmem->alu_result = 0;
            break;

        // Graphics instructions: these are more complex and would need
        // framebuffer state. For now, treat them as no-ops
        case OP_DRAWPIX:
        case OP_DRAWSTEP:
        case OP_SETCLR:
        case OP_CLEARFB:
            exmem->alu_result = 0;
            break;

        case OP_INVALID:
        default:
            exmem->valid = 0;  // Bubble on invalid instruction
            break;
    }
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
