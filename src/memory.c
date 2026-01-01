#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/isa.h"

// Helper: trim whitespace in-place
static void trim(char *s) {
  char *p = s;
  while (isspace((unsigned char)*p))
    p++;
  memmove(s, p, strlen(p) + 1);

  int len = strlen(s);
  while (len > 0 && isspace((unsigned char)s[len - 1])) {
    s[len - 1] = '\0';
    len--;
  }
}

// Helper: check if operand is numeric or hex
static int is_numeric(const char *s) {
  if (!s || !*s)
    return 0;

  if (isdigit(*s) || *s == '-' || *s == '+')
    return 1;

  if (strlen(s) > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    return 1;

  return 0;
}

int lookup_label(const char *name, LabelEntry table[], int count) {
  for (int i = 0; i < count; i++) {
    if (strcmp(table[i].name, name) == 0)
      return table[i].address;
  }
  return -1; // not found
}

int build_imem(const char *filename, InstMem *im, LabelEntry labels[],
               int label_count) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    perror("fopen");
    return -1;
  }

  im->lines = calloc(MAX_IMEM, sizeof(char *));
  im->size = 0;

  char line_raw[256];
  int pc = 0; // instruction index

  while (fgets(line_raw, sizeof(line_raw), file)) {

    char line[256];
    strcpy(line, line_raw);
    trim(line);

    if (line[0] == '\0' || line[0] == '#')
      continue;

    // Check for label-only line: "LOOP:"
    if (line[strlen(line) - 1] == ':' && strchr(line, ' ') == NULL) {
      // pure label line → skip (PC doesn’t increment)
      continue;
    }

    // Check for label followed by instruction: "LOOP: ADD x1 x2 x3"
    char *colon = strchr(line, ':');
    if (colon) {
      // Split at colon
      *colon = '\0';
      char labelname[128];
      strcpy(labelname, line);
      trim(labelname);

      // instruction begins after colon
      char *instr = colon + 1;
      trim(instr);

      // Insert only instruction into IMEM
      line[0] = '\0';
      strcpy(line, instr);
    }

    // Now `line` should contain a pure instruction
    // Need to check if BEQ has label operand
    char clean[256];
    strcpy(clean, line);

    char *tok = strtok(clean, " ,\t");
    if (!tok)
      continue;

    // uppercase mnemonic
    for (char *p = tok; *p; ++p)
      *p = toupper(*p);

    if (strcmp(tok, "BEQ") == 0) {
      char *rs1 = strtok(NULL, " ,\t");
      char *rs2 = strtok(NULL, " ,\t");
      char *imm = strtok(NULL, " ,\t");

      if (!rs1 || !rs2 || !imm) {
        // malformed instruction
        im->lines[im->size++] = strdup(line);
        pc++;
        continue;
      }

      // CASE 1: immediate is numeric → leave instruction unchanged
      if (is_numeric(imm)) {
        im->lines[im->size++] = strdup(line);
        pc++;
        continue;
      }

      // CASE 2: immediate is label → convert to PC-relative offset
      int target = lookup_label(imm, labels, label_count);
      if (target < 0) {
        printf("ERROR: Undefined label '%s'\n", imm);
        fclose(file);
        return -1;
      }

      int offset = target - pc;

      // Build new instruction string
      char final[256];
      snprintf(final, sizeof(final), "BEQ %s, %s, %d", rs1, rs2, offset);

      im->lines[im->size++] = strdup(final);
      pc++;
      continue;
    }

    // Normal instruction — store as-is
    im->lines[im->size++] = strdup(line);
    pc++;
  }

  fclose(file);
  return 0;
}

void free_imem(InstMem *im) {
  for (size_t i = 0; i < im->size; ++i)
    free(im->lines[i]);
  free(im->lines);
}

// ========== PIPELINE REGISTER INITIALIZATION ==========

void init_ifid(IFIDreg *r) {
  if (!r)
    return;
  r->instr_text = NULL;
  r->pc = 0;
  r->valid = 0;
}

void free_ifid(IFIDreg *r) {
  // Only free if we strdup'd something.
  // Usually the simulator might just point to IMEM strings or strdup them.
  // If strdup'd in fetch, free here.
  // For safety in this sim, let's assume fetch does strdup or we just clear
  // pointer.
  if (r && r->instr_text) {
    // free(r->instr_text); // deciding not to free to avoid double-free if just
    // ptr copy
  }
}

void init_idex(IDEXreg *r) {
  if (!r)
    return;
  memset(r, 0, sizeof(IDEXreg));
  r->valid = 0;
}

void init_exio(EXIOreg *r) {
  if (!r)
    return;
  memset(r, 0, sizeof(EXIOreg));
  r->valid = 0;
}

void init_iomem(IOMEMreg *r) {
  if (!r)
    return;
  memset(r, 0, sizeof(IOMEMreg));
  r->valid = 0;
}

void init_memwb(MEMWBreg *r) {
  if (!r)
    return;
  memset(r, 0, sizeof(MEMWBreg));
  r->valid = 0;
}