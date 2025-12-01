#ifndef DECODE_H
#define DECODE_H

#include "cpu.h"
#include "isa.h"

typedef struct {
    char *instr_text; // strdup'd text of the instruction
    uint32_t pc;      // original PC
    int valid;        // 1 = has instruction, 0 = bubble
} IFIDreg;

// ---------- Function Prototypes ----------

void if_stage_fetch(ProgramCounter *pc,
                          InstMem *im,
                          IFIDreg *ifid);

void init_ifid(IFIDreg *r);
void free_ifid(IFIDreg *r);

int parse_register(const char *tok);
void instruction_parser(IFIDreg *ifid, DecodedInst *out);
int ctoi(const char *c);
void trim_inplace(char* s);
int32_t parse_immediate(const char* token);
int is_label(const char* line);
void detectLabels(const char* file, LabelEntry label_table[], int *label_index);


#endif