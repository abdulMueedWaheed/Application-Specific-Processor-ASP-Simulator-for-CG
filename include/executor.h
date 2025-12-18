#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "isa.h"
#include "graphics.h"
#include <stdint.h>

/**
 * Unified instruction executor shared by both single-cycle and pipelined models
 * Eliminates code duplication and ensures consistent execution semantics
 */

// Execution result structure
typedef struct {
    int32_t alu_result;       // ALU output or address calculation
    int32_t mem_data;         // Data read from memory (LW)
    int32_t next_pc;          // Next PC (for branches)
    int mem_write_addr;       // Address for SW operation (-1 = no write)
    int mem_read_addr;        // Address for LW operation (-1 = no read)
    int is_memory_op;         // 1 if this is LW/SW, 0 otherwise
    int is_branch;            // 1 if branch instruction
    int branch_taken;         // 1 if branch was taken
} ExecResult;

/**
 * Execute a single instruction completely
 * Used by both single-cycle and pipelined models
 * 
 * @param op            Opcode to execute
 * @param rd            Destination register (-1 = none)
 * @param rs1           Source register 1 (-1 = zero)
 * @param rs2           Source register 2 (-1 = zero)
 * @param imm           Immediate value
 * @param pc            Current program counter
 * @param rs1_val       Value from rs1 (already read)
 * @param rs2_val       Value from rs2 (already read)
 * @param regs          Register file (32 x 32-bit)
 * @param fb            Framebuffer for graphics ops
 * @param data_mem      Data memory for LW/SW
 * @param data_mem_size Size of data memory
 * @return ExecResult   Execution results (ALU output, addresses, etc.)
 */
ExecResult execute_inst(
    Opcode op,
    int rd, int rs1, int rs2,
    int32_t imm,
    uint32_t pc,
    int32_t rs1_val, int32_t rs2_val,
    int32_t *regs,
    Framebuffer *fb,
    int32_t *data_mem,
    size_t data_mem_size
);

/**
 * Write result back to register file
 * Handles invalid register numbers and x0 (always zero)
 */
static inline void writeback_register(int32_t *regs, int rd, int32_t value) {
    if (rd > 0 && rd < 32) {
        regs[rd] = value;
    }
}

/**
 * Read from register file
 * Returns 0 for invalid register numbers
 */
static inline int32_t read_register(int32_t *regs, int rs) {
    if (rs >= 0 && rs < 32) {
        return regs[rs];
    }
    return 0;
}

#endif // EXECUTOR_H
