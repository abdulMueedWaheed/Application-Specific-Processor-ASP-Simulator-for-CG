#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stddef.h>

#define MAX_IMEM 65536

// ---------- Instruction Memory ----------
typedef struct {
    char **lines;     // dynamic array of instruction text lines
    size_t size;      // number of instructions
} InstMem;

// ---------- IF State ----------
typedef struct {
    uint32_t pc;      // instruction index
} IFState;

// ---------- IF/ID Pipeline Register ----------
typedef struct {
    char *instr_text; // strdup'd text of the instruction
    uint32_t pc;      // original PC
    int valid;        // 1 = has instruction, 0 = bubble
} IFIDReg;

// ---------- Function Prototypes ----------
int load_imem(const char *filename, InstMem *im);
void free_imem(InstMem *im);

void init_ifid(IFIDReg *r);
void free_ifid(IFIDReg *r);

void set_pc(IFState *s, uint32_t pc_value);

// IF Stage function
void if_stage_fetch(IFState *s, InstMem *im, IFIDReg *ifid);

#endif
