#ifndef DECODE_H
#define DECODE_H

#include "cpu.h"
#include "isa.h"

int parse_register(const char *tok);
void instruction_parser(IFIDReg *ifid, DecodedInst *out);
int ctoi(const char *c);
void trim_inplace(char* s);
int32_t parse_immediate(const char* token);
int is_label(const char* line);
void detectLabels(const char* file, LabelEntry label_table[], int *label_index);


#endif