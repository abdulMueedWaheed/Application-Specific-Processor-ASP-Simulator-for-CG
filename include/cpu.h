#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stddef.h>

#define MAX_IMEM 65536 // For 32 bit instructions: 65536 * 32 =  2MB RAM
extern int32_t regs[32];

// ---------- Instruction Memory ----------
typedef struct {
    char **lines;     // dynamic array of instruction text lines
    size_t size;      // number of instructions
} InstMem;

typedef struct {
    uint32_t pc;      // instruction index
} ProgramCounter;

#endif
