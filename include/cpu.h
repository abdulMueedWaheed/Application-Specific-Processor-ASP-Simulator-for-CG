#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stddef.h>

#define MAX_IMEM 65536 // For 32 bit instructions: 65536 * 32 =  2MB RAM

// ---------- Instruction Memory ----------
typedef struct {
    char **lines;     // dynamic array of instruction text lines
    size_t size;      // number of instructions
} InstMem;

// ---------- IF State ----------
typedef struct {
    uint32_t pc;      // instruction index
} ProgramCounter;

// ---------- IF/ID Pipeline Register ----------
typedef struct {
    char *instr_text; // strdup'd text of the instruction
    uint32_t pc;      // original PC
    int valid;        // 1 = has instruction, 0 = bubble
} IFIDReg;

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

// ---------- Function Prototypes ----------
void free_imem(InstMem *im);
int build_imem(const char *filename,
                     InstMem *im,
                     LabelEntry labels[],
                     int label_count);

void init_ifid(IFIDReg *r);
void free_ifid(IFIDReg *r);

void set_pc(ProgramCounter *s, uint32_t pc_value);

// IF Stage function
void if_stage_fetch(ProgramCounter *s, InstMem *im, IFIDReg *ifid);

#endif
