#include "../include/trace.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Initialize trace buffer
TraceBuffer* trace_init(void) {
    TraceBuffer *tb = (TraceBuffer*)malloc(sizeof(TraceBuffer));
    if (!tb) return NULL;
    
    tb->cycle_count = 0;
    tb->max_cycles = MAX_CYCLES;
    memset(tb->cycles, 0, sizeof(tb->cycles));
    
    return tb;
}

// Free trace buffer
void trace_free(TraceBuffer *tb) {
    if (tb) free(tb);
}

// Record a snapshot of a pipeline stage
void trace_stage_snapshot(StageSnapshot *snap, int valid, const char *instr_text, 
                         uint32_t pc, Opcode op, int rd, int rs1, int rs2, 
                         int32_t imm, int32_t result) {
    snap->valid = valid;
    if (instr_text) {
        strncpy(snap->instr_text, instr_text, MAX_TRACE_INSTR_LEN - 1);
        snap->instr_text[MAX_TRACE_INSTR_LEN - 1] = '\0';
    } else {
        snap->instr_text[0] = '\0';
    }
    snap->pc = pc;
    snap->op = op;
    snap->rd = rd;
    snap->rs1 = rs1;
    snap->rs2 = rs2;
    snap->imm = imm;
    snap->result = result;
}

// Record cycle data
void trace_record_cycle(TraceBuffer *tb, uint32_t cycle) {
    if (!tb || cycle >= tb->max_cycles) return;
    
    CycleTrace *ct = &tb->cycles[cycle];
    ct->cycle = cycle;
    
    // Register state will be filled externally
    tb->cycle_count = cycle + 1;
}

// Opcode to string
static const char* opcode_to_str(Opcode op) {
    switch (op) {
        case OP_ADD: return "ADD";
        case OP_ADDI: return "ADDI";
        case OP_SUB: return "SUB";
        case OP_SUBI: return "SUBI";
        case OP_MUL: return "MUL";
        case OP_DRAWPIX: return "DRAWPIX";
        case OP_DRAWSTEP: return "DRAWSTEP";
        case OP_SETCLR: return "SETCLR";
        case OP_CLEARFB: return "CLEARFB";
        case OP_LW: return "LW";
        case OP_SW: return "SW";
        case OP_BEQ: return "BEQ";
        case OP_NOP: return "NOP";
        default: return "INVALID";
    }
}

// Format stage snapshot as string
static void format_stage(char *buf, size_t len, StageSnapshot *snap) {
    if (!snap->valid) {
        snprintf(buf, len, "BUBBLE");
    } else if (snap->instr_text[0] != '\0') {
        snprintf(buf, len, "%s (PC=%u)", snap->instr_text, snap->pc);
    } else {
        snprintf(buf, len, "%s rd=%d rs1=%d rs2=%d imm=%d (PC=%u)", 
                 opcode_to_str(snap->op), snap->rd, snap->rs1, snap->rs2, 
                 snap->imm, snap->pc);
    }
}

// Dump trace to console (formatted)
void trace_dump_console(TraceBuffer *tb) {
    if (!tb) return;
    
    printf("\n");
    printf("===============================================\n");
    printf("         CYCLE-BY-CYCLE EXECUTION TRACE\n");
    printf("===============================================\n");
    printf("\n");
    
    for (uint32_t i = 0; i < tb->cycle_count && i < tb->max_cycles; i++) {
        CycleTrace *ct = &tb->cycles[i];
        
        printf("--- Cycle %u ---\n", ct->cycle);
        
        // Print each stage
        char buf[256];
        
        format_stage(buf, sizeof(buf), &ct->if_stage);
        printf("  IF: %s\n", buf);
        
        format_stage(buf, sizeof(buf), &ct->id_stage);
        printf("  ID: %s\n", buf);
        
        format_stage(buf, sizeof(buf), &ct->ex_stage);
        printf("  EX: %s\n", buf);
        
        format_stage(buf, sizeof(buf), &ct->mem_stage);
        printf("  MEM: %s\n", buf);
        
        format_stage(buf, sizeof(buf), &ct->wb_stage);
        printf("  WB: %s\n", buf);
        
        printf("\n");
    }
    
    printf("===============================================\n");
    printf("Total Cycles: %u\n", tb->cycle_count);
    printf("===============================================\n\n");
}

// Dump trace to file (formatted)
void trace_dump_text(TraceBuffer *tb, const char *filename) {
    if (!tb || !filename) return;
    
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return;
    }
    
    fprintf(f, "===============================================\n");
    fprintf(f, "         CYCLE-BY-CYCLE EXECUTION TRACE\n");
    fprintf(f, "===============================================\n\n");
    
    for (uint32_t i = 0; i < tb->cycle_count && i < tb->max_cycles; i++) {
        CycleTrace *ct = &tb->cycles[i];
        
        fprintf(f, "--- Cycle %u ---\n", ct->cycle);
        
        // Print each stage
        char buf[256];
        
        format_stage(buf, sizeof(buf), &ct->if_stage);
        fprintf(f, "  IF: %s\n", buf);
        
        format_stage(buf, sizeof(buf), &ct->id_stage);
        fprintf(f, "  ID: %s\n", buf);
        
        format_stage(buf, sizeof(buf), &ct->ex_stage);
        fprintf(f, "  EX: %s\n", buf);
        
        format_stage(buf, sizeof(buf), &ct->mem_stage);
        fprintf(f, "  MEM: %s\n", buf);
        
        format_stage(buf, sizeof(buf), &ct->wb_stage);
        fprintf(f, "  WB: %s\n", buf);
        
        fprintf(f, "\n");
    }
    
    fprintf(f, "===============================================\n");
    fprintf(f, "Total Cycles: %u\n", tb->cycle_count);
    fprintf(f, "===============================================\n");
    
    fclose(f);
    printf("Trace dumped to %s\n", filename);
}
