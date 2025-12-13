# CPU Simulator Architecture

## Overview

This is a dual-model CPU simulator implementing both **single-cycle** and **5-stage pipelined** execution of a custom 14-instruction ISA with graphics support.

## System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                         CPU Simulator                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │              Instruction Memory (IMEM)                   │   │
│  │              65536 x 32-bit Instructions                 │   │
│  └────────────────────────┬─────────────────────────────────┘   │
│                           │                                      │
│                    ┌──────▼──────┐                              │
│                    │  Program    │                              │
│                    │  Counter    │                              │
│                    └──────┬──────┘                              │
│                           │                                      │
│  ┌────────────────────────▼──────────────────────────────────┐  │
│  │              Register File (x0-x31)                       │  │
│  │              32 x 32-bit General Purpose                  │  │
│  │              x0 = Always 0 (RISC-V)                       │  │
│  └────────────────────────┬──────────────────────────────────┘  │
│                           │                                      │
│  ┌────────────────────────▼──────────────────────────────────┐  │
│  │              Data Memory (Dmem)                           │  │
│  │              4096 x 32-bit Words                          │  │
│  │              For LW/SW operations                         │  │
│  └────────────────────────┬──────────────────────────────────┘  │
│                           │                                      │
│  ┌────────────────────────▼──────────────────────────────────┐  │
│  │              Framebuffer                                  │  │
│  │              256x256 x 32-bit (ARGB)                      │  │
│  │              Graphics output buffer                       │  │
│  └──────────────────────────────────────────────────────────┘  │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
```

## 5-Stage Pipeline Architecture

```
┌──────────────────────────────────────────────────────────────┐
│                     Fetch Stage (IF)                         │
│  - Load instruction from IMEM[PC]                            │
│  - Increment PC (or jump if branch taken)                    │
├──────────────────────────────────────────────────────────────┤
│ IF/ID Pipeline Register                                      │
│ ├─ instr_text: Instruction as text string                   │
│ ├─ pc: Program Counter value                                │
│ └─ valid: Instruction valid bit                             │
└──────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────────┐
│                    Decode Stage (ID)                         │
│  - Parse instruction text (mnemonic + operands)             │
│  - Extract register indices (rs1, rs2, rd)                  │
│  - Extract immediate value                                  │
│  - Read operand values: reg[rs1], reg[rs2]                  │
├──────────────────────────────────────────────────────────────┤
│ ID/EX Pipeline Register                                      │
│ ├─ op: Opcode                                                │
│ ├─ rs1_val, rs2_val: Operand values                         │
│ ├─ rd: Destination register                                 │
│ ├─ imm: Immediate value (sign-extended)                     │
│ ├─ pc: Program Counter                                      │
│ └─ valid: Valid bit                                         │
└──────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────────┐
│                    Execute Stage (EX)                        │
│  - ALU Operations (ADD, SUB, MUL, etc.)                      │
│  - Address Calculation (LW, SW)                             │
│  - Branch Comparison (BEQ)                                   │
│  - Graphics Operations (DRAWPIX, SETCLR, etc.)             │
│                                                              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  ALU (Arithmetic Logic Unit)                        │   │
│  │  Inputs: rs1_val, rs2_val, imm, opcode            │   │
│  │  Output: alu_result                                 │   │
│  └─────────────────────────────────────────────────────┘   │
├──────────────────────────────────────────────────────────────┤
│ EX/MEM Pipeline Register                                     │
│ ├─ op: Opcode                                                │
│ ├─ alu_result: ALU computation result                        │
│ ├─ rs2_val: Register value for store operations            │
│ ├─ rd: Destination register                                 │
│ ├─ pc: Program Counter                                      │
│ └─ valid: Valid bit                                         │
└──────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────────┐
│                    Memory Stage (MEM)                        │
│  - Load: Read from data_memory[address]                     │
│  - Store: Write rs2_val to data_memory[address]             │
│  - Handle memory access violations                          │
│  - Framebuffer graphics operations                          │
├──────────────────────────────────────────────────────────────┤
│ MEM/WB Pipeline Register                                     │
│ ├─ write_data: Data to write back to registers             │
│ ├─ rd: Destination register                                 │
│ ├─ is_memory: Flag (load vs ALU result)                    │
│ └─ valid: Valid bit                                         │
└──────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌──────────────────────────────────────────────────────────────┐
│                 Write Back Stage (WB)                        │
│  - Write write_data to register[rd]                         │
│  - Handle special register x0 (always 0)                    │
│  - Update register file                                      │
└──────────────────────────────────────────────────────────────┘
```

## Instruction Encoding Format

```
Bit:    31 30 29 28 27 26 | 25 24 23 22 21 | 20 19 18 17 16 | 15 14 13 12 11 | 10  9  8  7  6  5  4  3  2  1  0
Field:  └────opcode(6)────┤ └─rd(5)─┤ └─rs1(5)─┤ └─rs2(5)─┤ └─────imm(11)─────┘

Examples:
- ADD x1, x2, x3:   op=0, rd=1, rs1=2, rs2=3, imm=0
- ADDI x1, x2, 100: op=1, rd=1, rs1=2, rs2=0, imm=100
- LW x1, 4(x2):     op=9, rd=1, rs1=2, imm=4
```

## Hazard Detection & Handling

### Data Hazards

**Type**: Register dependency (Read-After-Write)
```
Cycle N:   ADDI x1, x0, 50   (rd=x1)
Cycle N+1:                  (ID stage: read x1)
         → WB not complete yet!
         → x1 still contains old value (or 0)
```

**Solution 1 - Forwarding** (Not yet implemented):
```
Cycle N+1: Forward result from EX/MEM or MEM/WB
          to ID/EX ALU inputs (bypass register file)
```

**Solution 2 - Stalling** (Currently used):
```
Add NOP instructions to space out dependent instructions
Allow WB stage to complete before dependent instruction enters ID
```

**Current Implementation**: Test programs manually spaced

### Control Hazards

**Type**: Branch instruction misprediction
```
Cycle N:   BEQ x1, x2, offset
         (Fetches next instruction at PC+1)
         (Branch resolved in EX stage)
         
If branch TAKEN:
  - PC should jump to PC + offset
  - Instructions fetched at PC+1 are wrong
  - Flush IF/ID registers (set valid=0)
```

**Solution - Static Prediction**:
```
Always assume not-taken
Prefetch from PC+1
If actual != predicted: Flush pipeline
```

## Cycle Progression Example

### Program
```
ADDI x1, x0, 50     # Cycle 0 in IF
ADDI x2, x0, 60     # Cycle 0 waiting in fetch
ADD x3, x1, x2      # 
NOP
```

### Timeline (5-Stage Pipeline)

```
     Cycle 0   Cycle 1   Cycle 2   Cycle 3   Cycle 4   Cycle 5
Instr 1
ADDI  x1, x0, 50
      IF    →   ID    →    EX    →   MEM    →   WB

Instr 2
ADDI  x2, x0, 60
                 IF    →    ID    →   EX     →   MEM    →   WB

Instr 3
ADD   x3, x1, x2
                         IF    →    ID    →   EX     →   MEM    →   WB

Instr 4
NOP
                                  IF    →    ID    →   EX     →   MEM    →   WB
```

**Cycle Count**: 8 cycles for 4 instructions
(Startup: 4 cycles, then 1 instr/cycle sustain)

## Data Structures

### Pipeline Registers (C structs)

```c
typedef struct {
    char *instr_text;
    uint32_t pc;
    int valid;
} IFIDreg;

typedef struct {
    Opcode op;
    int32_t rs1_val, rs2_val;
    int rd;
    int32_t imm;
    uint32_t pc;
    int valid;
} IDEXreg;

typedef struct {
    Opcode op;
    int32_t alu_result;
    int32_t rs2_val;
    int rd;
    uint32_t pc;
    int valid;
} EXMEMreg;

typedef struct {
    int32_t write_data;
    int rd;
    int is_memory;
    int valid;
} MEMWBreg;
```

## File Organization

```
Project Root
├── include/
│   ├── cpu.h                 # CPU structs
│   ├── isa.h                 # ISA + encoding/decoding
│   ├── graphics.h            # Framebuffer interface
│   ├── fetch.h               # (Legacy)
│   └── parse_instruction.h   # Parser interface
│
├── src/
│   ├── main.c                # Main simulator loop
│   ├── fetch.c               # IF stage + register init
│   ├── deode.c               # ID stage
│   ├── execute.c             # EX, MEM, WB stages
│   ├── graphics.c            # Graphics implementation
│   ├── memory.c              # Label resolution, IMEM build
│   └── parse_instruction.c   # Assembly parser
│
├── program.instr             # Test program (assembly)
├── Makefile                  # Build config
├── README.md                 # Project overview
├── REQUIREMENTS_STATUS.md    # Phase 1/2 checklist
└── ARCHITECTURE.md           # This file
```

## Performance Characteristics

### Cycle Count Analysis

**Single-Cycle Model**:
- Time = N cycles (1 instruction per cycle)
- No startup penalty
- High clock period (critical path through all 5 stages)

**5-Stage Pipeline**:
- Time = 4 + N cycles (with N = instruction count)
- Startup penalty: 4 cycles (fill pipeline)
- Sustain rate: 1 instruction per cycle
- Lower clock period (each stage ~1/5 of single-cycle delay)
- Speedup = N / (4 + N) → 1.0 as N → ∞

**With Data Hazards (stalls)**:
- Time = 4 + N + stall_count cycles
- Stall count depends on register dependencies
- Forwarding reduces stalls significantly

### Example Metrics

```
Program: 10 instructions
Single-Cycle: 10 cycles
Pipeline (no stalls): 14 cycles
Pipeline (2 stalls): 16 cycles

Clock Period:
Single-Cycle: ~1000 ps (example)
Pipeline: ~200 ps (5x faster due to logic simplification per stage)

Actual Execution Time:
Single-Cycle: 10 * 1000 = 10,000 ps
Pipeline: 14 * 200 = 2,800 ps → 3.6x speedup!
```

## Extension Points

1. **Forwarding Logic** → Reduce stalls
2. **Branch Prediction** → Handle control hazards better
3. **Cache Subsystem** → Add L1 I-cache, D-cache
4. **Performance Counters** → Track metrics
5. **Out-of-Order Execution** → Higher IPC
6. **SIMD Graphics** → Vector operations

---

*End of Architecture Document*
