#include "../include/isa.h"
#include <stdio.h>


void execute_instruction(DecodedInst* input, ProgramCounter pc, LabelEntry label_table){
    if (input->valid) {
        printf("Invalid instruction...", stderr);
        return;
    }

    switch (input->op) {
    
    case OP_ADD:
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