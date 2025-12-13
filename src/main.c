#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../include/isa.h"
#include "../include/parse_instruction.h"
#include "../include/graphics.h"
#include "../include/trace.h"

int32_t regs[32];
extern Framebuffer *global_fb;

int main(int argc, char **argv)
{
    const char *filename = (argc > 1) ? argv[1] : "program.instr";

    LabelEntry labels[256];
    int label_count = 0;

    // === Initialize Trace Buffer ===
    TraceBuffer *trace = trace_init();
    if (!trace) {
        fprintf(stderr, "Failed to initialize trace buffer\n");
        return 1;
    }

    // === Initialize Graphics ===
    global_fb = fb_init();
    if (!global_fb) {
        fprintf(stderr, "Failed to initialize framebuffer\n");
        trace_free(trace);
        return 1;
    }
    printf("Framebuffer initialized: %dx%d\n", FB_WIDTH, FB_HEIGHT);

    // === PASS 1: Collect labels ===
    detectLabels(filename, labels, &label_count);

    // === PASS 2: Build cleaned instruction memory ===
    InstMem im;
    if (build_imem(filename, &im, labels, label_count) != 0) {
        fb_free(global_fb);
        trace_free(trace);
        return 1;
    }
    printf("Loaded %zu instructions.\n", im.size);

    // === Initialize CPU state ===
    ProgramCounter pc = { .pc = 0 };
    
    // Initialize all registers to 0
    for (int i = 0; i < 32; i++) {
        regs[i] = 0;
    }

    // Initialize pipeline registers
    IFIDreg ifid;
    IDEXreg idex;
    EXMEMreg exmem;
    MEMWBreg memwb;

    init_ifid(&ifid);
    init_idex(&idex);
    init_exmem(&exmem);
    init_memwb(&memwb);

    // === Pipeline simulation ===
    int idle = 0;
    uint32_t cycle = 0;
    
    printf("\n=== Starting Pipeline Simulation ===\n\n");

    while (idle < 5 && cycle < 1000) {  // Prevent infinite loops
        printf("=== Cycle %u ===\n", cycle);

        // Record pipeline state before execution
        trace_record_cycle(trace, cycle);
        CycleTrace *ct = &trace->cycles[cycle];

        // WB stage (writeback)
        wb_stage(&memwb);

        // MEM stage (memory access)
        mem_stage(&exmem, &memwb);

        // EX stage (execute)
        ex_stage(&idex, &exmem);

        // IF stage (fetch)
        if_stage(&pc, &im, &ifid);

        // Record stage snapshots for tracing
        if (ifid.valid) { 
            trace_stage_snapshot(&ct->if_stage, 1, ifid.instr_text, ifid.pc, 
                               OP_INVALID, -1, -1, -1, 0, 0);
            printf("Fetched: [PC=%u] %s\n", ifid.pc, ifid.instr_text); 
            idle = 0; 
        } 
        else { 
            trace_stage_snapshot(&ct->if_stage, 0, NULL, 0, OP_INVALID, -1, -1, -1, 0, 0);
            printf("Bubble (no instruction)\n");
            idle++;
        }

        // ID stage (decode)
        DecodedInst decoded;
        memset(&decoded, 0, sizeof(decoded));
        instruction_parser(&ifid, &decoded);
        id_stage(&decoded, &idex);

        if (decoded.valid) {
            trace_stage_snapshot(&ct->id_stage, 1, NULL, decoded.pc, decoded.op,
                               decoded.rd, decoded.rs1, decoded.rs2, decoded.imm, 0);
            printf("Decoded: pc=%u op=%d imm=%d rd=%d rs1=%d rs2=%d\n",
                   decoded.pc, decoded.op, decoded.imm,
                   decoded.rd, decoded.rs1, decoded.rs2);
        } else {
            trace_stage_snapshot(&ct->id_stage, 0, NULL, 0, OP_INVALID, -1, -1, -1, 0, 0);
            printf("Decode: invalid or bubble\n");
        }

        // Record EX stage
        if (idex.valid) {
            trace_stage_snapshot(&ct->ex_stage, 1, NULL, idex.pc, idex.op,
                               idex.rd, -1, -1, idex.imm, 0);
        } else {
            trace_stage_snapshot(&ct->ex_stage, 0, NULL, 0, OP_INVALID, -1, -1, -1, 0, 0);
        }

        // Record MEM stage
        if (exmem.valid) {
            trace_stage_snapshot(&ct->mem_stage, 1, NULL, exmem.pc, exmem.op,
                               exmem.rd, -1, -1, 0, exmem.alu_result);
        } else {
            trace_stage_snapshot(&ct->mem_stage, 0, NULL, 0, OP_INVALID, -1, -1, -1, 0, 0);
        }

        // Record WB stage
        if (memwb.valid) {
            trace_stage_snapshot(&ct->wb_stage, 1, NULL, 0, OP_INVALID,
                               memwb.rd, -1, -1, 0, memwb.write_data);
        } else {
            trace_stage_snapshot(&ct->wb_stage, 0, NULL, 0, OP_INVALID, -1, -1, -1, 0, 0);
        }

        // Copy register state after this cycle
        memcpy(ct->regs, regs, sizeof(regs));

        printf("\n");
        cycle++;
    }

    printf("\n=== Final Register State ===\n");
    for (int i = 0; i < 32; i++) {
        if (regs[i] != 0) {
            printf("x%d = 0x%x (%d)\n", i, regs[i], regs[i]);
        }
    }

    // === Output Trace ===
    printf("\n=== Generating Execution Trace ===\n");
    trace_dump_text(trace, "trace.txt");
    trace_dump_console(trace);

    // === Dump Framebuffer ===
    printf("\n=== Graphics Output ===\n");
    fb_dump_ppm(global_fb, "framebuffer.ppm");
    fb_dump_ascii(global_fb);

    // Cleanup
    free_ifid(&ifid);
    free_imem(&im);
    fb_free(global_fb);
    trace_free(trace);

    printf("\nSimulation completed successfully!\n");
    printf("  - Trace saved to: trace.txt\n");
    printf("  - Framebuffer saved to: framebuffer.ppm\n");

    return 0;
}
