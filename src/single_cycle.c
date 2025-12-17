#include "../include/single_cycle.h"

#include "../include/parse_instruction.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Data memory for single-cycle
#define DATA_MEM_SIZE 4096
static int32_t data_memory[DATA_MEM_SIZE];

// Execute a single instruction in one cycle (all 5 stages)
static void execute_instruction_single(Opcode op, int rd, int rs1, int rs2, 
                                       int32_t imm, uint32_t pc,
                                       int32_t *regs, Framebuffer *fb) {
    int32_t rs1_val = (rs1 >= 0) ? regs[rs1] : 0;
    int32_t rs2_val = (rs2 >= 0) ? regs[rs2] : 0;
    int32_t result = 0;
    
    switch (op) {
        case OP_ADD:
            result = rs1_val + rs2_val;
            if (rd >= 0) regs[rd] = result;
            break;
            
        case OP_ADDI:
            result = rs1_val + imm;
            if (rd >= 0) regs[rd] = result;
            break;
            
        case OP_SUB:
            result = rs1_val - rs2_val;
            if (rd >= 0) regs[rd] = result;
            break;
            
        case OP_SUBI:
            result = rs1_val - imm;
            if (rd >= 0) regs[rd] = result;
            break;
            
        case OP_MUL:
            result = rs1_val * rs2_val;
            if (rd >= 0) regs[rd] = result;
            break;
            
        case OP_LW: {
            uint32_t addr = rs1_val + imm;
            if (addr < DATA_MEM_SIZE) {
                result = data_memory[addr];
                if (rd >= 0) regs[rd] = result;
            }
            break;
        }
        
        case OP_SW: {
            uint32_t addr = rs1_val + imm;
            if (addr < DATA_MEM_SIZE) {
                data_memory[addr] = rs2_val;
            }
            break;
        }
        
        case OP_BEQ:
            // Branch handled by PC update in main loop
            break;
            
        case OP_DRAWPIX:
            if (fb) {
                int x = rs1_val & 0xFFFF;
                int y = rs2_val & 0xFFFF;
                fb_draw_pixel(fb, x, y);
            }
            break;
            
        case OP_SETCLR:
            if (fb) {
                uint32_t color = (0xFF << 24) | (imm & 0xFFFFFF);
                fb_set_color(fb, color);
            }
            break;
            
        case OP_CLEARFB:
            if (fb) fb_clear(fb);
            break;
            
        case OP_NOP:
        default:
            break;
    }
}

// Single-cycle execution model
SingleCycleResult* single_cycle_execute(InstMem *im, LabelEntry labels[],
                                        int label_count, Framebuffer *fb) {
    SingleCycleResult *result = (SingleCycleResult*)malloc(sizeof(SingleCycleResult));
    if (!result) return NULL;
    
    int32_t regs[32];
    memset(regs, 0, sizeof(regs));
    
    uint32_t pc = 0;
    uint32_t cycle = 0;
    
    printf("\n=== SINGLE-CYCLE EXECUTION MODEL ===\n");
    printf("Each instruction completes in exactly 1 cycle\n\n");
    
    while (pc < im->size) {
        printf("Cycle %u: PC=%u\n", cycle, pc);
        
        // Fetch & Decode
        char buffer[256];
        strncpy(buffer, im->lines[pc], sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
        
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
                   (decoded.op < 14) ? "OP" : "INVALID",
                   decoded.rd, decoded.rs1, decoded.rs2, decoded.imm);
            
            // Handle branch (PC update before execution)
            if (decoded.op == OP_BEQ) {
                int32_t rs1_val = (decoded.rs1 >= 0) ? regs[decoded.rs1] : 0;
                int32_t rs2_val = (decoded.rs2 >= 0) ? regs[decoded.rs2] : 0;
                
                if (rs1_val == rs2_val) {
                    pc = (pc + decoded.imm) & 0xFFFFFFFF;
                    printf("  Branch TAKEN to PC=%u\n", pc);
                } else {
                    pc++;
                    printf("  Branch NOT taken, PC=%u\n", pc);
                }
            } else {
                // Execute instruction (all stages in one cycle)
                execute_instruction_single(decoded.op, decoded.rd, decoded.rs1, 
                                         decoded.rs2, decoded.imm, pc, regs, fb);
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
    memcpy(result->final_regs, regs, sizeof(regs));
    
    printf("=== SINGLE-CYCLE RESULTS ===\n");
    printf("Total cycles: %u\n", cycle);
    printf("Total instructions: %u\n", im->size);
    printf("CPI (Cycles Per Instruction): 1.0\n\n");
    
    return result;
}

void single_cycle_free(SingleCycleResult *result) {
    if (result) free(result);
}
