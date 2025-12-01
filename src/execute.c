#include "../include/execute.h"
#include <stdio.h>

int32_t registers[32];

void execute_instruction(DecodedInst* input, ProgramCounter pc, LabelEntry label_table){
    if (input->valid) {
        printf("Invalid instruction...", stderr);
        return;
    }

    switch (input->op) {
    
    case OP_ADD:
        add(input->rs1,
            input->rs2,
            input->rd,
            input->imm);
    break;
    
    case OP_ADDI:
      break;
    case OP_SUB:
    case OP_MUL:
    case OP_DRAWPIX:
    case OP_DRAWSTEP:
    case OP_SETCLR:
    case OP_CLEARFB:
    case OP_LW:
    case OP_SW:
    case OP_BEQ:
    case OP_NOP:
    case OP_INVALID:
    break;
    }
}

void add(int rs1_value, int rs2_value, int rd_value, int imm) {
    int a = rs1_value, b = rs2_value, c = rd_value;
    registers[c] = registers[a] + registers[b];
}