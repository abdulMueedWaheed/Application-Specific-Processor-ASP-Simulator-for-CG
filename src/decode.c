#include "../include/decode.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_label(const char *line) {
    if (!line) return 0;

    // skip leading whitespace
    while (isspace((unsigned char)*line)) line++;

    int len = strlen(line);
    if (len == 0) return 0;

    // if last non-space char is ':', it's a label
    const char *p = line + len - 1;
    while (p >= line && isspace((unsigned char)*p)) p--;

    return (*p == ':');
}

void detectLabels(const char* filename, LabelEntry label_table[], int *label_index) 
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("fopen");
        return;
    }

    char line[256];
    int pc = 0;

    while (fgets(line, sizeof(line), file)) {
        char original[256];
        strcpy(original, line);
        trim_inplace(original);

        if (strlen(original) == 0) 
            continue;

        // Case 1: label-only line: "LOOP:"
        if (is_label(original)) {
            original[strlen(original) - 1] = '\0';  // remove ':'

            strcpy(label_table[*label_index].name, original);
            label_table[*label_index].address = pc;
            (*label_index)++;

            // DO NOT increment PC for label-only line
            continue;
        }

        // Case 2: instruction or label+instruction on same line
        // We must detect label prefixes: "LOOP: ADD x1, x2, x3"
        char *colon = strchr(original, ':');
        if (colon) {
            *colon = '\0'; // split label and instruction
            trim_inplace(original);

            // store label
            strcpy(label_table[*label_index].name, original);
            label_table[*label_index].address = pc;
            (*label_index)++;

            // PC increments because instruction exists after label
            pc++;
            continue;
        }

        // Case 3: normal instruction
        pc++;
    }

    fclose(file);
}



void instruction_parser(IFIDreg *ifid, DecodedInst *out){
    memset(out, 0, sizeof(DecodedInst));
    out->pc = ifid->pc;
    out->valid = 0;

    if (!ifid->valid || !ifid->instr_text) {
        return;
    }

    char buffer[256];
    strncpy(buffer, ifid->instr_text, sizeof(buffer));
    buffer[sizeof(buffer)-1] = '\0';
    trim_inplace(buffer);

    if (strlen(buffer) == 0) {
        return;
    }

    char *delimiters = " ,\t";
    char *saveptr = NULL;
    char *token = strtok_r(buffer, delimiters, &saveptr);

    while(token != NULL){
        if (is_label(token)) {
            continue;
        }

        else if (strcmp(token, "ADD") == 0) {
            /* ADD rd, rs1, rs2 */
            char *rd = strtok_r(NULL, delimiters, &saveptr);
            char *rs1 = strtok_r(NULL, delimiters, &saveptr);
            char *rs2 = strtok_r(NULL, delimiters, &saveptr);

            out->rd  = parse_register(rd);
            out->rs1 = parse_register(rs1);
            out->rs2 = parse_register(rs2);
            out->op = OP_ADD;
            out->imm = 0;
            out->valid = (out->rd >= 0 && out->rs1 >= 0 && out->rs2 >= 0);
            return;
        }

        else if (strcmp(token, "ADDI") == 0) {
            /* ADD rd, rs1, rs2 */
            char *rd = strtok_r(NULL, delimiters, &saveptr);
            char *rs1 = strtok_r(NULL, delimiters, &saveptr);
            char* imm = strtok_r(NULL, delimiters, &saveptr);

            out->rd  = parse_register(rd);
            out->rs1 = parse_register(rs1);
            out->rs2 = -1;
            out->op = OP_ADD;
            out->imm = parse_immediate(imm);
            out->valid = (out->rd >= 0 && out->rs1 >= 0 && out->rs2 >= 0);
            return;
        }

        else if (strcmp(token, "MUL") == 0) {
            char *rd = strtok_r(NULL, delimiters, &saveptr);
            char *rs1 = strtok_r(NULL, delimiters, &saveptr);
            char *rs2 = strtok_r(NULL, delimiters, &saveptr);

            out->rd  = parse_register(rd);
            out->rs1 = parse_register(rs1);
            out->rs2 = parse_register(rs2);
            out->op = OP_MUL;
            out->imm = 0;
            out->valid = (out->rd >= 0 && out->rs1 >= 0 && out->rs2 >= 0);
            return;
        }

        if (strcmp(token, "NOP") == 0) {
            out->op = OP_NOP;
            out->valid = 1;
            return;
        }

        if (strcmp(token, "SUB") == 0) {
            /* SUB rd, rs1, rs2 */
            char *rd = strtok_r(NULL, delimiters, &saveptr);
            char *rs1 = strtok_r(NULL, delimiters, &saveptr);
            char *rs2 = strtok_r(NULL, delimiters, &saveptr);

            out->rd  = parse_register(rd);
            out->rs1 = parse_register(rs1);
            out->rs2 = parse_register(rs2);
            out->op = OP_SUB;
            out->imm = 0;
            out->valid = (out->rd >= 0 && out->rs1 >= 0 && out->rs2 >= 0);
            return;
        }

        if (strcmp(token, "DRAWPIX") == 0) {
            /* DRAWPIX rs_x, rs_y   (both registers) */
            char *rx = strtok_r(NULL, delimiters, &saveptr);
            char *ry = strtok_r(NULL, delimiters, &saveptr);

            out->rs1 = parse_register(rx);   /* rs1 holds x */
            out->rs2 = parse_register(ry);   /* rs2 holds y */
            out->op = OP_DRAWPIX;
            out->rd = -1;
            out->imm = 0;
            out->valid = (out->rs1 >= 0 && out->rs2 >= 0);
            return;
        }

        if (strcmp(token, "DRAWSTEP") == 0) {
            /* DRAWSTEP rd, rs1, rs2  - custom multi-value step */
            char *rd = strtok_r(NULL, delimiters, &saveptr);
            char *rs1 = strtok_r(NULL, delimiters, &saveptr);
            char *rs2 = strtok_r(NULL, delimiters, &saveptr);

            out->rd  = parse_register(rd);
            out->rs1 = parse_register(rs1);
            out->rs2 = parse_register(rs2);
            out->op = OP_DRAWSTEP;
            out->imm = 0;
            out->valid = (out->rd >= 0 && out->rs1 >= 0 && out->rs2 >= 0);
            return;
        }

        if (strcmp(token, "SETCLR") == 0) {
            /* SETCLR imm  (e.g., SETCLR 0xFF00) -> write to COLOR_REG later in WB */
            char *imm = strtok_r(NULL, delimiters, &saveptr);
            out->imm = parse_immediate(imm);
            out->op = OP_SETCLR;
            out->rd = -1;
            out->rs1 = -1;
            out->rs2 = -1;
            out->valid = 1;
            return;
        }

        if (strcmp(token, "CLEARFB") == 0) {
            /* CLEARFB has no operands */
            out->op = OP_CLEARFB;
            out->valid = 1;
            return;
        }

        if (strcmp(token, "LW") == 0) {
            /* LW rd, imm(rs1)  e.g., LW x5, 8(x3)  OR LW x5, 12(x31) */
            char *rd = strtok_r(NULL, delimiters, &saveptr);
            char *rest = strtok_r(NULL, "\n", &saveptr); /* get rest of line for imm(rs) */
            if (!rest) rest = strtok_r(NULL, delimiters, &saveptr);

            /* rest can be like "8(x3)" or "8(x3)," or "0x10(x2)".
            Extract immediate and register inside parentheses. */
            if (rest) {
                /* find '(' */
                char *lpar = strchr(rest, '(');
                char *rpar = strchr(rest, ')');
                if (lpar && rpar && rpar > lpar) {
                    *lpar = '\0';
                    /* rest now has imm string */
                    char immtok[64];
                    strncpy(immtok, rest, sizeof(immtok));
                    immtok[sizeof(immtok)-1] = '\0';
                    char *regtok = lpar + 1;
                    /* strip trailing comma etc. */
                    out->imm = parse_immediate(immtok);
                    out->rs1 = parse_register(regtok);
                } else {
                    /* If not of form imm(reg), try two-token form: imm reg */
                    char *imm2 = strtok_r(rest, delimiters, &saveptr);
                    char *r2 = strtok_r(NULL, delimiters, &saveptr);
                    out->imm = parse_immediate(imm2);
                    out->rs1 = parse_register(r2);
                }
            }
            out->rd = parse_register(rd);
            out->op = OP_LW;
            out->valid = (out->rd >= 0 && out->rs1 >= 0);
            return;
        }

        if (strcmp(token, "SW") == 0) {
            /* SW rs2, imm(rs1)  e.g., SW x5, 8(x3) */
            char *rs2tok = strtok_r(NULL, delimiters, &saveptr);
            char *rest = strtok_r(NULL, "\n", &saveptr);
            if (!rest) rest = strtok_r(NULL, delimiters, &saveptr);

            if (rest) {
                char *lpar = strchr(rest, '(');
                char *rpar = strchr(rest, ')');
                if (lpar && rpar && rpar > lpar) {
                    *lpar = '\0';
                    char immtok[64];
                    strncpy(immtok, rest, sizeof(immtok));
                    immtok[sizeof(immtok)-1] = '\0';
                    char *regtok = lpar + 1;
                    out->imm = parse_immediate(immtok);
                    out->rs1 = parse_register(regtok);
                } else {
                    char *imm2 = strtok_r(rest, delimiters, &saveptr);
                    char *r2 = strtok_r(NULL, delimiters, &saveptr);
                    out->imm = parse_immediate(imm2);
                    out->rs1 = parse_register(r2);
                }
            }
            out->rs2 = parse_register(rs2tok);
            out->op = OP_SW;
            out->valid = (out->rs2 >= 0 && out->rs1 >= 0);
            return;
        }

        if (strcmp(token, "BEQ") == 0) {
            /* BEQ rs1, rs2, imm_or_label (we only parse numeric immediates here) */
            char *rs1t = strtok_r(NULL, delimiters, &saveptr);
            char *rs2t = strtok_r(NULL, delimiters, &saveptr);
            char *offt = strtok_r(NULL, delimiters, &saveptr);

            out->rs1 = parse_register(rs1t);
            out->rs2 = parse_register(rs2t);
            out->imm = parse_immediate(offt); /* assembler should produce numeric PC offset in instructions */
            out->op = OP_BEQ;
            out->valid = (out->rs1 >= 0 && out->rs2 >= 0);
            return;
        }

        else {
            out->op = OP_INVALID;
            out->valid = 0;
            return;
        }
    }
}

int ctoi(const char *c) {
    int a;
    a= *c - '0';
    return a;
}

int parse_register(const char* token){
    // Accept forms like: x5, r5, R5
    if (token[0] == 'x' || token[0] == 'X' ||
        token[0] == 'r' || token[0] == 'R') {
        return ctoi(token + 1);
    }
    return -1; // invalid
}

void trim_inplace(char* s){
    if (!s) return;
    /* trim leading */
    char *p = s;
    while (isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);

    /* trim trailing */
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) { *end = '\0'; end--; }
}

int32_t parse_immediate(const char *tok) {
    if (!tok) return 0;
    /* make local copy and trim trailing comma/newline */
    char buf[64];
    size_t i = 0;
    while (*tok && i < sizeof(buf)-1) {
        if (*tok == ',' || *tok == '\n') break;
        buf[i++] = *tok++;
    }
    buf[i] = '\0';

    if (i == 0) return 0;
    char *endptr = NULL;
    long val = 0;
    if (strncmp(buf, "0x", 2) == 0 || strncmp(buf, "-0x", 3) == 0) {
        val = strtol(buf, &endptr, 0); /* base 0 recognizes 0x */
    } else {
        val = strtol(buf, &endptr, 0); /* base 0 accepts decimal and 0x */
    }
    if (endptr == buf) return 0;
    return (int32_t)val;
}