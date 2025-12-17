#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../include/isa.h"
#include "../include/parse_instruction.h"
#include "../include/graphics.h"
#include "../include/trace.h"
#include "../include/single_cycle.h"

int32_t regs[32];
extern Framebuffer *global_fb;

typedef enum {
    MODE_PIPELINED,
    MODE_SINGLE_CYCLE,
    MODE_BOTH
} ExecutionMode;

void print_usage(const char *prog) {
    printf("Usage: %s [options] <program.instr>\n", prog);
    printf("\nOptions:\n");
    printf("  -p, --pipelined     Run pipelined (5-stage) model only\n");
    printf("  -s, --single        Run single-cycle model only\n");
    printf("  -c, --compare       Run both models and compare results\n");
    printf("\nDefault: pipelined model\n");
}

int main(int argc, char **argv)
{
    ExecutionMode mode = MODE_PIPELINED;
    const char *filename = "program.instr";
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--pipelined") == 0) {
            mode = MODE_PIPELINED;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--single") == 0) {
            mode = MODE_SINGLE_CYCLE;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compare") == 0) {
            mode = MODE_BOTH;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
        }
    }

    LabelEntry labels[256];
    int label_count = 0;

    // === Initialize Graphics ===
    global_fb = fb_init();
    if (!global_fb) {
        fprintf(stderr, "Failed to initialize framebuffer\n");
        return 1;
    }
    printf("Framebuffer initialized: %dx%d\n", FB_WIDTH, FB_HEIGHT);

    // === PASS 1: Collect labels ===
    detectLabels(filename, labels, &label_count);

    // === PASS 2: Build cleaned instruction memory ===
    InstMem im;
    if (build_imem(filename, &im, labels, label_count) != 0) {
        fb_free(global_fb);
        return 1;
    }
    printf("Loaded %zu instructions.\n\n", im.size);

    // === Execute based on mode ===
    
    if (mode == MODE_SINGLE_CYCLE || mode == MODE_BOTH) {
        printf("\n");
        printf("╔════════════════════════════════════════╗\n");
        printf("║     SINGLE-CYCLE EXECUTION MODEL       ║\n");
        printf("╚════════════════════════════════════════╝\n");
        
        SingleCycleResult *sc_result = single_cycle_execute(&im, labels, label_count, global_fb);
        
        printf("\nFinal Register State (Single-Cycle):\n");
        for (int i = 0; i < 32; i++) {
            if (sc_result->final_regs[i] != 0) {
                printf("  x%d = 0x%x (%d)\n", i, sc_result->final_regs[i], sc_result->final_regs[i]);
            }
        }
        
        single_cycle_free(sc_result);
    }
    
    if (mode == MODE_PIPELINED || mode == MODE_BOTH) {
        printf("\n");
        printf("╔════════════════════════════════════════╗\n");
        printf("║   5-STAGE PIPELINED EXECUTION MODEL    ║\n");
        printf("╚════════════════════════════════════════╝\n");
        
        // Initialize Trace Buffer
        TraceBuffer *trace = trace_init();
        if (!trace) {
            fprintf(stderr, "Failed to initialize trace buffer\n");
            fb_free(global_fb);
            return 1;
        }

        // Initialize registers
        memset(regs, 0, sizeof(regs));
        ProgramCounter pc = { .pc = 0 };

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
        
        printf("Starting pipeline simulation...\n\n");

        while (idle < 5 && cycle < 1000) {
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

            // Record stage snapshots
            if (ifid.valid) { 
                trace_stage_snapshot(&ct->if_stage, 1, ifid.instr_text, ifid.pc, 
                                   OP_INVALID, -1, -1, -1, 0, 0);
                idle = 0; 
            } 
            else { 
                trace_stage_snapshot(&ct->if_stage, 0, NULL, 0, OP_INVALID, -1, -1, -1, 0, 0);
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
            } else {
                trace_stage_snapshot(&ct->id_stage, 0, NULL, 0, OP_INVALID, -1, -1, -1, 0, 0);
            }

            // Record other stages
            if (idex.valid) {
                trace_stage_snapshot(&ct->ex_stage, 1, NULL, idex.pc, idex.op,
                                   idex.rd, -1, -1, idex.imm, 0);
            } else {
                trace_stage_snapshot(&ct->ex_stage, 0, NULL, 0, OP_INVALID, -1, -1, -1, 0, 0);
            }

            if (exmem.valid) {
                trace_stage_snapshot(&ct->mem_stage, 1, NULL, exmem.pc, exmem.op,
                                   exmem.rd, -1, -1, 0, exmem.alu_result);
            } else {
                trace_stage_snapshot(&ct->mem_stage, 0, NULL, 0, OP_INVALID, -1, -1, -1, 0, 0);
            }

            if (memwb.valid) {
                trace_stage_snapshot(&ct->wb_stage, 1, NULL, 0, OP_INVALID,
                                   memwb.rd, -1, -1, 0, memwb.write_data);
            } else {
                trace_stage_snapshot(&ct->wb_stage, 0, NULL, 0, OP_INVALID, -1, -1, -1, 0, 0);
            }

            memcpy(ct->regs, regs, sizeof(regs));

            cycle++;
        }

        printf("\nFinal Register State (Pipelined):\n");
        for (int i = 0; i < 32; i++) {
            if (regs[i] != 0) {
                printf("  x%d = 0x%x (%d)\n", i, regs[i], regs[i]);
            }
        }

        // Output trace
        printf("\n=== Generating Execution Trace ===\n");
        trace_dump_text(trace, "trace.txt");
        
        free_ifid(&ifid);
        trace_free(trace);
    }

    // === Graphics Output ===
    printf("\n=== Graphics Output ===\n");
    fb_dump_ppm(global_fb, "framebuffer.ppm");
    fb_dump_ascii(global_fb);

    // === Comparison ===
    if (mode == MODE_BOTH) {
        printf("\n");
        printf("╔════════════════════════════════════════╗\n");
        printf("║     EXECUTION MODEL COMPARISON        ║\n");
        printf("╚════════════════════════════════════════╝\n");
        printf("Single-Cycle: 1 cycle per instruction (deterministic)\n");
        printf("Pipelined:    Variable cycles (pipeline stalls due to hazards)\n");
        printf("See trace.txt for detailed cycle-by-cycle breakdown\n");
    }

    // Cleanup
    free_imem(&im);
    fb_free(global_fb);

    printf("\nSimulation completed successfully!\n");
    printf("  - Trace saved to: trace.txt\n");
    printf("  - Framebuffer saved to: framebuffer.ppm\n");

    return 0;
}
