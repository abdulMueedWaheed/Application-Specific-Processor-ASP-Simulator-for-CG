#ifndef TRACE_H
#define TRACE_H

#include <stdint.h>
#include "isa.h"

#define MAX_CYCLES 10000
#define MAX_TRACE_INSTR_LEN 128

// Stage state at a particular cycle
typedef struct {
    int valid;
    char instr_text[MAX_TRACE_INSTR_LEN];
    uint32_t pc;
    Opcode op;
    int rd, rs1, rs2;
    int32_t imm;
    int32_t result;  // for EX/MEM stages
} StageSnapshot;

// Per-cycle trace entry
typedef struct {
    uint32_t cycle;
    
    // Pipeline stages
    StageSnapshot if_stage;
    StageSnapshot id_stage;
    StageSnapshot ex_stage;
    StageSnapshot mem_stage;
    StageSnapshot wb_stage;
    
    // Register state after this cycle
    int32_t regs[32];
    
} CycleTrace;

// Global trace buffer
typedef struct {
    CycleTrace cycles[MAX_CYCLES];
    uint32_t cycle_count;
    uint32_t max_cycles;
} TraceBuffer;

// Function declarations
TraceBuffer* trace_init(void);
void trace_free(TraceBuffer *tb);
void trace_record_cycle(TraceBuffer *tb, uint32_t cycle);
void trace_stage_snapshot(StageSnapshot *snap, int valid, const char *instr_text, uint32_t pc, Opcode op, int rd, int rs1, int rs2, int32_t imm, int32_t result);
void trace_dump_text(TraceBuffer *tb, const char *filename);
void trace_dump_console(TraceBuffer *tb);

#endif
