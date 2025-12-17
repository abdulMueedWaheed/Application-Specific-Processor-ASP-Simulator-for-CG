#ifndef HAZARD_H
#define HAZARD_H

#include "isa.h"

typedef enum {
    HAZARD_NONE,
    HAZARD_RAW,
    HAZARD_LOAD_USE,
    HAZARD_BRANCH,
} HazardType;

typedef struct {
    HazardType type;
    int cycles_to_wait;
    int forward_from_stage;
    int rd_source;
} HazardInfo;

typedef struct {
    int enabled;
    int rd;
    int32_t value;
    int from_stage;
} ForwardingPath;

HazardInfo detect_data_hazard(IDEXreg *idex, EXMEMreg *exmem, MEMWBreg *memwb);
HazardInfo detect_load_use_hazard(IDEXreg *idex, EXMEMreg *exmem);
HazardInfo detect_control_hazard(IDEXreg *idex, int branch_taken, uint32_t target_pc);

int can_forward_from_ex(IDEXreg *idex, EXMEMreg *exmem, int operand_num);
int can_forward_from_mem(IDEXreg *idex, MEMWBreg *memwb, int operand_num);
int can_forward_from_wb(IDEXreg *idex, MEMWBreg *memwb, int operand_num);

int32_t get_forwarded_value(int source_stage, int rd, int32_t ex_result, 
                            int32_t mem_result, int32_t wb_result);

const char* hazard_type_str(HazardType type);

#endif
