#include "../include/hazard.h"
#include <stdio.h>
#include <string.h>

// Detect data hazards - simplified version
HazardInfo detect_data_hazard(IDEXreg *idex, EXMEMreg *exmem, MEMWBreg *memwb) {
    HazardInfo hz = {HAZARD_NONE, 0, -1, -1};
    if (!idex || !idex->valid) return hz;
    return hz;  // Simplified: no data hazard detection
}

// Detect load-use hazards
HazardInfo detect_load_use_hazard(IDEXreg *idex, EXMEMreg *exmem) {
    HazardInfo hz = {HAZARD_NONE, 0, -1, -1};
    if (!idex || !idex->valid || !exmem || !exmem->valid) return hz;
    if (exmem->op != OP_LW) return hz;
    return hz;  // Simplified: no load-use detection
}

// Detect control hazards
HazardInfo detect_control_hazard(IDEXreg *idex, int branch_taken, uint32_t target_pc) {
    HazardInfo hz = {HAZARD_NONE, 0, -1, -1};
    if (!idex || idex->op != OP_BEQ) return hz;
    return hz;  // Simplified: no branch hazard detection
}

// Forwarding checks
int can_forward_from_ex(IDEXreg *idex __attribute__((unused)), 
                       EXMEMreg *exmem, int operand_num __attribute__((unused))) {
    if (!exmem || !exmem->valid) return 0;
    if (exmem->rd < 0 || exmem->rd >= 32) return 0;
    return 1;
}

int can_forward_from_mem(IDEXreg *idex __attribute__((unused)), 
                        MEMWBreg *memwb, int operand_num __attribute__((unused))) {
    if (!memwb || !memwb->valid) return 0;
    if (memwb->rd < 0 || memwb->rd >= 32) return 0;
    return 1;
}

int can_forward_from_wb(IDEXreg *idex __attribute__((unused)), 
                       MEMWBreg *memwb, int operand_num __attribute__((unused))) {
    if (!memwb || !memwb->valid) return 0;
    if (memwb->rd < 0 || memwb->rd >= 32) return 0;
    return 1;
}

int32_t get_forwarded_value(int source_stage, int rd __attribute__((unused)), 
                            int32_t ex_result, int32_t mem_result, int32_t wb_result) {
    if (source_stage == 1) return ex_result;
    if (source_stage == 2) return mem_result;
    if (source_stage == 3) return wb_result;
    return 0;
}

const char* hazard_type_str(HazardType type) {
    switch (type) {
        case HAZARD_NONE: return "NONE";
        case HAZARD_RAW: return "RAW";
        case HAZARD_LOAD_USE: return "LOAD-USE";
        case HAZARD_BRANCH: return "BRANCH";
        default: return "UNKNOWN";
    }
}
