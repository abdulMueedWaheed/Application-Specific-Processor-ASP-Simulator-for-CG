#include "../include/execution.h"
#include "../include/graphics.h"
#include "../include/isa.h"
#include "../include/parse_instruction.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int32_t regs[32];
extern Framebuffer *global_fb;

void print_usage(const char *prog) {
  printf("Usage: %s [options] <program.instr>\n", prog);
  printf("\nOptions:\n");
  printf("  -p, --pipelined     Run pipelined (5-stage) model\n");
  printf("  -s, --single        Run single-cycle model\n");
  printf(
      "  -o, --output FILE   Output PPM filename (default: framebuffer.ppm)\n");
  printf("\nDefault: pipelined model\n");
}

int main(int argc, char **argv) {
  int mode = -1;
  const char *filename = "program.instr";
  const char *output_file = "framebuffer.ppm";

  if (argc > 1) {
    filename = argv[1];
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

  // === Execute program ===
  ExecutionResult *exec_result = NULL;

  // Default: Run BOTH
  printf("\n===========================================\n");
  printf(">>> Running SINGLE-CYCLE Mode <<<\n");
  printf("===========================================\n");
  ExecutionResult *res1 =
      execute_program(EXEC_MODE_SINGLE_CYCLE, &im, labels, label_count,
                      global_fb, "trace_single.txt");
  execution_free(res1);

  printf("\n===========================================\n");
  printf(">>> Running PIPELINED Mode <<<\n");
  printf("===========================================\n");
  exec_result = execute_program(EXEC_MODE_PIPELINED, &im, labels, label_count,
                                global_fb, "trace_pipe.txt");

  // If exec_result is NULL (should not happen), handle it.
  if (!exec_result)
    return 1;

  printf("\nFinal Register State (from last run):\n");
  for (int i = 0; i < 32; i++) {
    if (exec_result->final_regs[i] != 0) {
      printf("  x%d = 0x%x (%d)\n", i, exec_result->final_regs[i],
             exec_result->final_regs[i]);
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
