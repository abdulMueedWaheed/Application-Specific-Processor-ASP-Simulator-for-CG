# ASP Simulator Capabilities

## Overview
This custom Application-Specific Processor (ASP) simulator demonstrates advanced computer architecture concepts, including a 6-stage pipeline, hardware forwarding, and a dedicated graphics processing unit.

## Key Features

### 1. Dual Execution Modes
The simulator can execute programs in two modes, visible when running the default simulation:
*   **Single-Cycle Model**: Instructions execute in one atomic step. Useful for functional verification.
*   **Pipelined Model**: A classic 6-stage pipeline (IF, ID, EX, IO, MEM, WB) with hazard detection.

### 2. Hazard Resolution
*   **Hardware Forwarding**: Solves Data Hazards (RAW) without stalling, by forwarding results from IO and MEM stages back to Execution.
*   **Stalling**: Handles control hazards and load-use cases automatically.

### 3. Visualization Tools
*   **Trace Generation**: Produces detailed logs (`trace_single.txt` and `trace_pipe.txt`) showing register values and pipeline states per cycle.
*   **Graphics Output**: Supports a framebuffer for visual applications. Output is saved as `framebuffer.ppm`.

## Improved ISA
The Instruction Set Architecture has been expanded to support complex graphics algorithms:

| Opcode | Description | Usage |
| :--- | :--- | :--- |
| `ADD` / `ADDI` | Addition | `ADD rd, rs1, rs2` |
| `SUB` / `SUBI` | Subtraction | `SUB rd, rs1, rs2` |
| `MUL` / `DIV` | Arithmetic | `MUL rd, rs1, rs2` |
| `BLT` / `BEQ` | Conditional Branching | `BLT rs1, rs2, label` (Less Than) |
| `DRAWPIX` | Graphics | `DRAWPIX x, y` |
| `SETCLR` | Color Control | `SETCLR 0xRRGGBB` |

## Capabilities Demo (`capabilities.instr`)
A demonstration program is included to showcase these features:

1.  **Line Drawing**: Uses loops and pixel plotting to draw lines.
2.  **3D Cube (Perspective)**: Uses `DIV` instructions to perform perspective projection ($x' = x/z$), transforming 3D coordinates into 2D screen space.
3.  **Voronoi Tessellation**: Uses `MUL` (squared distance), `ADD`, and `BLT` (inequality check) to calculate nearest neighbors dynamically for each pixel.

## Running the Demo
Simply run:
```bash
./sim capabilities.instr
```
This will execute both modes and produce `framebuffer.ppm` showing the graphics result.
