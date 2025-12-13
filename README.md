# Application-Specific Processor (ASP) Simulator

## Phase 1 & 2 Implementation Status

### Phase 1: Single-Cycle Model ✅

**Custom ISA** (14 instructions):
- Arithmetic: ADD, ADDI, SUB, SUBI, MUL
- Memory: LW, SW
- Control: BEQ, NOP
- Graphics: DRAWPIX, DRAWSTEP, SETCLR, CLEARFB

**32-Bit Instruction Format**:
```
[opcode(6)] [rd(5)] [rs1(5)] [rs2(5)] [imm(11)]
```

**Single-Cycle Execution**: IF → ID → EX → MEM → WB in one cycle
- Register File: 32 x 32-bit
- Instruction Memory: 65536 x 32-bit  
- Data Memory: 4096 x 32-bit
- Framebuffer: 256x256 ARGB pixels

---

### Phase 2: 5-Stage Pipelined Model ✅

**Pipeline Stages**:
1. **IF** (Instruction Fetch): Fetch from IMEM
2. **ID** (Instruction Decode): Parse instruction, read registers
3. **EX** (Execute): ALU operations, address calc
4. **MEM** (Memory): Load/Store operations
5. **WB** (Write Back): Write results to registers

**Pipeline Registers**:
- IF/ID: Instruction + PC
- ID/EX: Decoded operands + immediates
- EX/MEM: ALU result + rs2_val for stores
- MEM/WB: Data to write back + destination register

**Hazard Handling**:
- **Data Hazards**: NOP spacing in test programs (explicit stalls)
- **Control Hazards**: BEQ resolved in EX, pipeline flush if taken
- *Future*: Forwarding paths, branch prediction

---

### Graphics Subsystem ✅

- **Framebuffer**: 256×256, 32-bit ARGB
- **Operations**: CLEARFB, SETCLR, DRAWPIX, DRAWSTEP (Bresenham lines)
- **Output**: PPM image + ASCII preview

---

### Instruction Encoding

| Opcode | Op | Format | Description |
|--------|-----|--------|-------------|
| 0  | ADD      | rd, rs1, rs2  | Arithmetic |
| 1  | ADDI     | rd, rs1, imm  | Immediate |
| 2  | SUB      | rd, rs1, rs2  | Arithmetic |
| 3  | SUBI     | rd, rs1, imm  | Immediate |
| 4  | MUL      | rd, rs1, rs2  | Arithmetic |
| 5  | DRAWPIX  | rs1, rs2      | Graphics |
| 6  | DRAWSTEP | dx, dy        | Graphics |
| 7  | SETCLR   | imm           | Graphics |
| 8  | CLEARFB  | —             | Graphics |
| 9  | LW       | rd, imm(rs1)  | Load |
| 10 | SW       | rs2, imm(rs1) | Store |
| 11 | BEQ      | rs1, rs2, imm | Branch |
| 12 | NOP      | —             | No-op |
| 13 | INVALID  | —             | Reserved |

---

### Example Program

```asm
CLEARFB                 # Clear framebuffer
SETCLR 0xFF0000         # Set color red
ADDI x1, x0, 50         # x1 = 50
ADDI x2, x0, 50         # x2 = 50
NOP
NOP
DRAWPIX x1, x2          # Draw pixel at (50,50)
SETCLR 0x00FF00         # Set color green
ADDI x3, x0, 75
ADDI x4, x0, 75
NOP
NOP
DRAWPIX x3, x4          # Draw pixel at (75,75)
NOP
```

### Output

```
Cycle-by-cycle trace showing:
- Fetched instruction
- Decoded operation
- EX/MEM/WB stage results
- Register state at end

PPM image: framebuffer.ppm (viewable with any image viewer)
```

---

### Build & Run

```bash
make clean && make
./sim program.instr
```

---

### Files

```
include/     - Headers (ISA, graphics, CPU structs)
src/         - Implementation
program.instr - Test assembly program
framebuffer.ppm - Generated image output
```

