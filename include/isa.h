#ifndef ISA_H
#define ISA_H

#include "cpu.h"

typedef enum {
    OP_ADD,
    OP_ADDI,
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
    char name[64];
    int address;
} LabelEntry;


typedef struct {
    Opcode op;
    int rd, 
    rs1, rs2; 
    int32_t imm; 
    uint32_t pc; 
    int valid; /* control flags */
} DecodedInst;

typedef struct {
    char *instr_text; // strdup'd text of the instruction
    uint32_t pc;      // original PC
    int valid;        // 1 = has instruction, 0 = bubble
} IFIDreg;

typedef struct {
    int valid;

    Opcode op;

    int32_t rs1_val;
    int32_t rs2_val;

    int rd;            // destination register number
    int32_t imm;       // immediate value

    uint32_t pc;       // original PC (for branches)

} IDEXreg;

void free_imem(InstMem *im);
int build_imem(const char *filename,
    InstMem *im,
    LabelEntry labels[],
    int label_count);


void if_stage(ProgramCounter *pc, InstMem *im, IFIDreg *ifid);
    

void id_stage(DecodedInst *dec, IDEXreg *idex);
void init_ifid(IFIDreg *r);
void free_ifid(IFIDreg *r);

void execute_instruction(DecodedInst* input, ProgramCounter pc, LabelEntry label_table);
void add(int rs1_value, int rs2_value, int rd_value);



#endif