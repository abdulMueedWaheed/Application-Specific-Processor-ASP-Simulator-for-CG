#include "../include/executor.h"
#include <stdio.h>

/**
 * Unified instruction executor
 * Consolidates execution logic for both single-cycle and pipelined models
 * Eliminates duplication and ensures consistent semantics
 */

ExecResult execute_inst(
    Opcode op,
    int rd, int rs1 __attribute__((unused)), int rs2 __attribute__((unused)),
    int32_t imm,
    uint32_t pc,
    int32_t rs1_val, int32_t rs2_val,
    int32_t *regs,
    Framebuffer *fb,
    int32_t *data_mem,
    size_t data_mem_size
) {
    ExecResult result = {
        .alu_result = 0,
        .mem_data = 0,
        .next_pc = pc + 1,
        .mem_write_addr = -1,
        .mem_read_addr = -1,
        .is_memory_op = 0,
        .is_branch = 0,
        .branch_taken = 0
    };

    switch (op) {
        // ========== ARITHMETIC OPERATIONS ==========
        case OP_ADD:
            result.alu_result = rs1_val + rs2_val;
            writeback_register(regs, rd, result.alu_result);
            break;

        case OP_ADDI:
            result.alu_result = rs1_val + imm;
            writeback_register(regs, rd, result.alu_result);
            break;

        case OP_SUB:
            result.alu_result = rs1_val - rs2_val;
            writeback_register(regs, rd, result.alu_result);
            break;

        case OP_SUBI:
            result.alu_result = rs1_val - imm;
            writeback_register(regs, rd, result.alu_result);
            break;

        case OP_MUL:
            result.alu_result = rs1_val * rs2_val;
            writeback_register(regs, rd, result.alu_result);
            break;

        // ========== MEMORY OPERATIONS ==========
        case OP_LW: {
            // Load word: address = rs1_val + imm
            uint32_t addr = rs1_val + imm;
            result.is_memory_op = 1;
            result.mem_read_addr = addr;
            
            if (addr < (uint32_t)data_mem_size) {
                result.mem_data = data_mem[addr];
                result.alu_result = result.mem_data;  // For single-cycle
                writeback_register(regs, rd, result.mem_data);
            } else {
                fprintf(stderr, "Memory access violation: LW at address 0x%x\n", addr);
            }
            break;
        }

        case OP_SW: {
            // Store word: address = rs1_val + imm, data = rs2_val
            uint32_t addr = rs1_val + imm;
            result.is_memory_op = 1;
            result.mem_write_addr = addr;
            
            if (addr < (uint32_t)data_mem_size) {
                data_mem[addr] = rs2_val;
                result.alu_result = addr;  // Return address for verification
            } else {
                fprintf(stderr, "Memory access violation: SW at address 0x%x\n", addr);
            }
            break;
        }

        // ========== CONTROL FLOW ==========
        case OP_BEQ: {
            // Branch if equal: target = pc + imm (if rs1_val == rs2_val)
            result.is_branch = 1;
            
            if (rs1_val == rs2_val) {
                result.branch_taken = 1;
                result.next_pc = pc + imm;
            } else {
                result.branch_taken = 0;
                result.next_pc = pc + 1;
            }
            result.alu_result = result.next_pc;
            break;
        }

        // ========== GRAPHICS OPERATIONS ==========
        case OP_DRAWPIX: {
            // Draw pixel at (rs1_val, rs2_val) with current color
            if (fb) {
                int x = rs1_val & 0xFFFF;
                int y = rs2_val & 0xFFFF;
                fb_draw_pixel(fb, x, y);
            }
            result.alu_result = 0;
            break;
        }

        case OP_DRAWSTEP: {
            // Draw step offset by (rs1_val, rs2_val)
            if (fb) {
                int dx = rs1_val;
                int dy = rs2_val;
                fb_draw_step(fb, dx, dy);
            }
            result.alu_result = 0;
            break;
        }

        case OP_SETCLR: {
            // Set color to immediate value (24-bit RGB)
            if (fb) {
                uint32_t color = (0xFF << 24) | (imm & 0xFFFFFF);
                fb_set_color(fb, color);
            }
            result.alu_result = 0;
            break;
        }

        case OP_CLEARFB: {
            // Clear framebuffer
            if (fb) {
                fb_clear(fb);
            }
            result.alu_result = 0;
            break;
        }

        // ========== SPECIAL INSTRUCTIONS ==========
        case OP_NOP:
            result.alu_result = 0;
            break;

        case OP_INVALID:
        default:
            result.alu_result = 0;
            break;
    }

    return result;
}
