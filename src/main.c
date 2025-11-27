#include <stdio.h>
#include "../include/cpu.h"

int main(int argc, char **argv)
{
    const char *filename = (argc > 1) ? argv[1] : "program.asm";

    InstMem im;
    IFState ifstate = { .pc = 0 };
    IFIDReg ifid;

    init_ifid(&ifid);

    if (load_imem(filename, &im) != 0) {
        return 1;
    }

    printf("Loaded %zu instructions.\n", im.size);

    int idle = 0;
    int cycle = 0;

    while (idle < 2) {
        printf("\n=== Cycle %d ===\n", cycle);

        if_stage_fetch(&ifstate, &im, &ifid);

        if (ifid.valid) {
            printf("Fetched: [PC=%u] %s\n", ifid.pc, ifid.instr_text);
            idle = 0;
        } else {
            printf("Bubble (no instruction)\n");
            idle++;
        }

        cycle++;
    }

    free_ifid(&ifid);
    free_imem(&im);

    return 0;
}
