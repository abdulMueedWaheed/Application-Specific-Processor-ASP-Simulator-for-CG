#ifndef ISA_H
#define ISA_H

#include <stdint.h>
#include <string.h>
#include "cpu.h"
#include <stdio.h>

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DRAWPIX,
    OP_DRAWSTEP,
    OP_SETCLR,
    OP_CLEARFB,
    OP_LW,
    OP_SW,
    OP_BEQ,
    OP_NOP,
    OP_INVALID
} Opcode;

typedef struct {
    Opcode op;
    int rd, 
    rs1, rs2; 
    int32_t imm; 
    uint32_t pc; 
    int valid; /* control flags */
} DecodedInst;

typedef struct {
    char name[64];
    int address;
} LabelEntry;


#endif