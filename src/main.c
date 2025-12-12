#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../include/isa.h"
#include "../include/parse_instruction.h"

int32_t regs[32];

int main(int argc, char **argv)
{
    const char *filename = (argc > 1) ? argv[1] : "program.instr";

    LabelEntry labels[256];
    int label_count = 0;

    // === PASS 1: Collect labels ===
    detectLabels(filename, labels, &label_count);

    // === PASS 2: Build cleaned instruction memory ===
    InstMem im;
    if (build_imem(filename, &im, labels, label_count) != 0) {
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
    int cycle = 0;

    while (idle < 5) {
        printf("\n=== Cycle %d ===\n", cycle);

        // WB stage (writeback)
        wb_stage(&memwb);

        // MEM stage (memory access)
        mem_stage(&exmem, &memwb);

        // EX stage (execute)
        ex_stage(&idex, &exmem);

        // IF stage (fetch)
        if_stage(&pc, &im, &ifid);

        if (ifid.valid) { 
            printf("Fetched: [PC=%u] %s\n", ifid.pc, ifid.instr_text); 
            idle = 0; 
        } 
        else { 
            printf("Bubble (no instruction)\n");
            idle++;
        }

        // ID stage (decode)
        DecodedInst decoded;
        memset(&decoded, 0, sizeof(decoded));
        instruction_parser(&ifid, &decoded);
        id_stage(&decoded, &idex);

        if (decoded.valid) {
            printf("Decoded: pc=%u op=%d imm=%d rd=%d rs1=%d rs2=%d\n",
                   decoded.pc, decoded.op, decoded.imm,
                   decoded.rd, decoded.rs1, decoded.rs2);
        } else {
            printf("Decode: invalid or bubble\n");
        }

        cycle++;
    }

    printf("\n=== Final Register State ===\n");
    for (int i = 0; i < 32; i++) {
        if (regs[i] != 0) {
            printf("x%d = 0x%x (%d)\n", i, regs[i], regs[i]);
        }
    }

    free_ifid(&ifid);
    free_imem(&im);

    return 0;
}
