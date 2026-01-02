#ifndef ISA_H
#define ISA_H

#include "cpu.h"
#include <stdint.h>

typedef enum {
  OP_ADD,
  OP_ADDI,
  OP_SUB,
  OP_SUBI,
  OP_MUL,
  OP_DIV,
  OP_DRAWPIX,
  OP_DRAWSTEP,
  OP_SETCLR,
  OP_CLEARFB,
  OP_LW,
  OP_SW,
  OP_BEQ,
  OP_BLT,
  OP_SIN,
  OP_COS,
  OP_MOVETO,
  OP_LINETO,
  OP_NOP,
  OP_INVALID
} Opcode;

typedef struct {
  char name[64];
  int address;
} LabelEntry;

// ========== 32-BIT INSTRUCTION ENCODING ==========
// Format: [opcode(6)] [rd(5)] [rs1(5)] [rs2(5)] [imm(11)]
// Opcode encoding: 0-13 for valid ops, 14 for invalid

typedef struct {
  uint32_t encoded;
  uint32_t pc;
  int valid;
} EncodedInst;

#define OPCODE_SHIFT 26
#define OPCODE_MASK 0x3F

#define RD_SHIFT 21
#define RS1_SHIFT 16
#define RS2_SHIFT 11
#define REG_MASK 0x1F

#define IMM_MASK 0x7FF
#define IMM_SIGN_BIT 0x400

static inline uint32_t encode_instruction(Opcode op, int rd, int rs1, int rs2,
                                          int32_t imm) {
  uint32_t instr = 0;

  instr |= ((op & OPCODE_MASK) << OPCODE_SHIFT);

  if (rd >= 0)
    instr |= ((rd & REG_MASK) << RD_SHIFT);
  if (rs1 >= 0)
    instr |= ((rs1 & REG_MASK) << RS1_SHIFT);
  if (rs2 >= 0)
    instr |= ((rs2 & REG_MASK) << RS2_SHIFT);

  instr |= (imm & IMM_MASK);

  return instr;
}

// Function to extract opcode from 32-bit instruction
static inline Opcode decode_opcode(uint32_t instr) {
  return (Opcode)((instr >> OPCODE_SHIFT) & OPCODE_MASK);
}

// Function to extract rd from 32-bit instruction
static inline int decode_rd(uint32_t instr) {
  return (instr >> RD_SHIFT) & REG_MASK;
}

// Function to extract rs1 from 32-bit instruction
static inline int decode_rs1(uint32_t instr) {
  return (instr >> RS1_SHIFT) & REG_MASK;
}

// Function to extract rs2 from 32-bit instruction
static inline int decode_rs2(uint32_t instr) {
  return (instr >> RS2_SHIFT) & REG_MASK;
}

// Function to extract and sign-extend immediate from 32-bit instruction
static inline int32_t decode_imm(uint32_t instr) {
  int32_t imm = instr & IMM_MASK;
  // Sign extend if bit 10 is set
  if (imm & IMM_SIGN_BIT) {
    imm |= 0xFFFFF800; // sign extend to 32 bits
  }
  return imm;
}

// ========== PIPELINE REGISTERS ==========

typedef struct {
  char *instr_text; // strdup'd text of the instruction
  uint32_t pc;      // original PC
  int valid;        // 1 = has instruction, 0 = bubble
} IFIDreg;

typedef struct {
  Opcode op;
  int32_t rs1_val;
  int32_t rs2_val;
  int rs1_idx;
  int rs2_idx;
  int rd;      // destination register number
  int32_t imm; // immediate value
  uint32_t pc; // original PC (for branches)
  int valid;   // 1 = valid, 0 = bubble
} IDEXreg;

typedef struct {
  Opcode op;
  int32_t alu_result; // result from ALU
  int32_t rs2_val;    // for store operations
  int rd;             // destination register
  int32_t imm;        // immediate value (needed for SETCLR)
  int32_t rs1_val;    // needed for graphics coords

  uint32_t pc; // PC for branch prediction/debugging
  int valid;   // 1 = valid, 0 = bubble

  // Branch support
  int branch_taken;
  uint32_t target_pc;
} EXIOreg;

typedef struct {
  Opcode op;
  int32_t alu_result; // result from ALU
  int32_t rs2_val;    // for store operations
  int rd;             // destination register
  uint32_t pc;        // PC for branch prediction/debugging
  int valid;          // 1 = valid, 0 = bubble
} IOMEMreg;

typedef struct {
  int32_t write_data; // data to write back to register
  int rd;             // destination register
  int is_memory;      // 1 if loading from memory, 0 if ALU result
  int valid;          // 1 = valid, 0 = bubble
} MEMWBreg;

// ========== DECODED INSTRUCTION (for legacy support) ==========
typedef struct {
  Opcode op;
  int rd, rs1, rs2;
  int32_t imm;
  uint32_t pc;
  int valid;
} DecodedInst;

// ========== FUNCTION DECLARATIONS ==========
void free_imem(InstMem *im);
int build_imem(const char *filename, InstMem *im, LabelEntry labels[],
               int label_count);

// Pipeline stages
void if_stage(ProgramCounter *pc, InstMem *im, IFIDreg *ifid);
void id_stage(DecodedInst *dec, IDEXreg *idex);
void ex_stage(IDEXreg *idex, EXIOreg *exio, IOMEMreg *iomem_fwd,
              MEMWBreg *memwb_fwd);
void io_stage(EXIOreg *exio, IOMEMreg *iomem);
void mem_stage(IOMEMreg *iomem, MEMWBreg *memwb);
void wb_stage(MEMWBreg *memwb);

// Register initialization/cleanup
void init_ifid(IFIDreg *r);
void free_ifid(IFIDreg *r);
void init_idex(IDEXreg *r);
void init_exio(EXIOreg *r);
void init_iomem(IOMEMreg *r);
void init_memwb(MEMWBreg *r);

// Legacy instruction execution (can be refactored later)
int execute_instruction(IDEXreg *input, ProgramCounter pc,
                        LabelEntry label_table);
int add(int a, int b);
int addi(int a, int b);
int sub(int a, int b);
int mul(int a, int b);

#endif
