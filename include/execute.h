#ifndef EXECUTE_H
#define EXECUTE_H

#include "isa.h"
#include "cpu.h"


void execute_instruction(DecodedInst* input, ProgramCounter pc, LabelEntry label_table);
void add(int rs1_value, int rs2_value, int rd_value);

#endif