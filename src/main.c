#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../include/isa.h"
#include "../include/parse_instruction.h"
#include "../include/graphics.h"
#include "../include/execution.h"

int32_t regs[32];
extern Framebuffer *global_fb;

void print_usage(const char *prog) {
    printf("Usage: %s [options] <program.instr>\n", prog);
    printf("\nOptions:\n");
    printf("  -p, --pipelined     Run pipelined (5-stage) model\n");
    printf("  -s, --single        Run single-cycle model\n");
    printf("  -o, --output FILE   Output PPM filename (default: framebuffer.ppm)\n");
    printf("\nDefault: pipelined model\n");
}

int main(int argc, char **argv)
{
    ExecutionMode mode = EXEC_MODE_PIPELINED;
    const char *filename = "program.instr";
    const char *output_file = "framebuffer.ppm";
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--pipelined") == 0) {
            mode = EXEC_MODE_PIPELINED;
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--single") == 0) {
            mode = EXEC_MODE_SINGLE_CYCLE;
        }
        else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            }
        }
        else if (argv[i][0] != '-') {
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

    // === Execute program in chosen mode ===
    ExecutionResult *exec_result = execute_program(mode, &im, labels, label_count, global_fb);

    printf("\nFinal Register State:\n");
    for (int i = 0; i < 32; i++) {
        if (exec_result->final_regs[i] != 0) {
            printf("  x%d = 0x%x (%d)\n", i, exec_result->final_regs[i], exec_result->final_regs[i]);
        }
    }

    // === Graphics Output ===
    printf("\n=== Graphics Output ===\n");
    fb_dump_ppm(global_fb, output_file);
    fb_dump_ascii(global_fb);

    // Cleanup
    execution_free(exec_result);
    free_imem(&im);
    fb_free(global_fb);

    printf("\nSimulation completed successfully!\n");
    printf("  - Framebuffer saved to: %s\n", output_file);

    return 0;
}
