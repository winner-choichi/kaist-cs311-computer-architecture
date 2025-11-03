# KAIST CS311: Computer Architecture Projects (2025 Spring)

This repository contains four major projects completed for CS311 (Computer Architecture) course at KAIST in Spring 2025.

## Course Information
- **Institution**: KAIST (Korea Advanced Institute of Science and Technology)
- **Course**: CS311 - Computer Architecture
- **Semester**: 2025 Spring
- **Language**: C/C++

---

## Project Overview

### Project 1: MIPS Assembler
**Due Date**: March 27, 2025

#### Description
Implemented a MIPS Instruction Set Architecture (ISA) assembler that translates MIPS assembly language code into binary files.

#### Key Features
- **Supported Instructions**: 22 MIPS instructions including:
  - R-type: ADD, AND, JR, NOR, OR, SLT, SLL, SRL, SUB
  - I-type: ADDI, ANDI, BEQ, BNE, LUI, LW, ORI, SLTI, SW
  - J-type: J, JAL
  - Pseudo-instruction: LA (load address)

- **Directive Support**:
  - `.text`: Instructions stored in user text segment (starting from 0x400000)
  - `.data`: Data stored in data segment (starting from 0x10000000)
  - `.word`: Stores 32-bit quantities in consecutive memory words

- **Implementation Highlights**:
  - Label handling for jump/branch targets and static data section
  - Sign extension for immediate and offset fields
  - LA pseudo-instruction expansion to LUI/ORI
  - Support for decimal and hexadecimal immediate values
  - Two-pass assembly process for label resolution

- **Output Format**: Custom ASCII binary format with text and data section sizes

#### Technical Details
- Binary output format with section size headers
- Memory layout: Text at 0x400000, Data at 0x10000000
- Implemented in C++

---

### Project 2: MIPS Simulator
**Due Date**: April 13, 2025

#### Description
Built a functional simulator for MIPS instruction set that loads MIPS binaries and executes instructions, simulating register and memory state changes.

#### Key Features
- **Simulated Components**:
  - 32 General Purpose Registers (R0-R31)
  - Program Counter (PC)
  - Simulated memory (text, data segments)

- **Instruction Execution**:
  - Fetch-decode-execute cycle
  - Same 22 instructions as Project 1
  - Sign extension for signed operations
  - Proper handling of branch and jump instructions

- **Memory Model**:
  - Text section: 0x400000
  - Data section: 0x10000000
  - 4KB word loads and stores only

- **Simulator Options**:
  - `-m addr1:addr2`: Print memory content range
  - `-d`: Print register file for each instruction
  - `-n num_instr`: Number of instructions to simulate

#### Implementation Details
- Implemented in `parse.c` and `run.c`
- Accurate simulation of MIPS ISA behavior
- Register R0 always remains 0
- No delay slot for JAL instruction (R31 = PC + 4)

---

### Project 3: MIPS Pipelined Simulator
**Due Date**: May 13, 2025

#### Description
Extended the MIPS simulator to implement a 5-stage pipeline with hazard detection and handling mechanisms.

#### Key Features
- **5-Stage Pipeline**:
  1. **IF (Instruction Fetch)**: Fetch instruction from memory
  2. **ID (Instruction Decode)**: Decode instruction and read registers
  3. **EX (Execute)**: Execute ALU operations
  4. **MEM (Memory)**: Access memory for loads/stores
  5. **WB (Write Back)**: Write results to register file

- **Hazard Handling**:
  - **Data Forwarding**:
    - EX/MEM-to-EX forwarding
    - MEM/WB-to-EX forwarding
    - MEM/WB-to-MEM forwarding
  - **Load-Use Stall**: 1 cycle stall for load dependency
  - **Branch Prediction**: Static "not taken" prediction
    - 3 cycle penalty on misprediction
    - Branch resolution at EX stage, flushing at MEM stage

- **Pipeline Registers**:
  - IF/ID: Instruction, NPC
  - ID/EX: NPC, REG1, REG2, IMM, control signals
  - EX/MEM: ALU_OUT, BR_TARGET, control signals
  - MEM/WB: ALU_OUT, MEM_OUT, control signals

- **Additional Features**:
  - Perfect LRU for pipeline register management
  - Dual-ported memory (simultaneous IF and MEM access)
  - Register file with split cycle access (WB first half, ID second half)

#### Technical Highlights
- `-p` option: Print PC of instructions in each pipeline stage
- Accurate cycle counting
- Implemented in modified `run.c` and `util.c`

---

### Project 4: MMU Simulator
**Due Date**: May 30, 2025

#### Description
Implemented a trace-based Memory Management Unit (MMU) simulator with TLB, page table walker, and page fault handler.

#### Key Features
- **TLB (Translation Lookaside Buffer)**:
  - Configurable capacity: 16-128 entries (powers of 2)
  - Configurable associativity: 1, 2, 4, 8-way, or fully associative
  - Perfect LRU replacement policy
  - Dirty bit tracking for write operations

- **Two-Level Page Table**:
  - Page size: 4KB (0x1000 bytes)
  - Level 1: 10-bit index (1024 entries)
  - Level 2: 10-bit index (1024 entries)
  - Page offset: 12 bits

- **Page Table Entry (PTE) Format** (32 bits):
  - Bits 31-12: Physical Page Number (PPN)
  - Bit 1: Dirty bit (Level 2 PTE only)
  - Bit 0: Valid bit

- **MMU Components**:
  1. **TLB Access**:
     - Hit: Update LRU, set dirty bit if write
     - Miss: Invoke page table walker

  2. **Page Table Walker**:
     - Traverse two-level page table
     - Handle invalid PTEs with page fault
     - Update TLB on successful translation

  3. **Page Fault Handler**:
     - Allocate new page table nodes (Level 1)
     - Allocate new physical pages (Level 2)
     - Update PTEs with valid and dirty bits

#### Simulator Usage
```bash
./cs311mmu -c entries:associativity [-x] <trace_file>
```
- `-c`: TLB configuration
- `-x`: Dump TLB and page table contents

#### Statistics Reported
- Total memory accesses
- Read/Write access counts
- TLB hits/misses
- Page table walks
- Page faults

#### Implementation Details
- Implemented in `main.c` (C++)
- Uses emulated memory for page tables only
- Physical pages are allocated virtually (addresses only)

---

## Build and Run

### General Build Instructions
Each project directory contains a `Makefile`:
```bash
cd project<N>
make
```

### Testing
Each project includes sample test cases:
```bash
make test
```

### Example Usage

**Project 1 (Assembler)**:
```bash
./runfile input.s
```

**Project 2 (Simulator)**:
```bash
./cs311sim -n 100 input.o
./cs311sim -m 0x10000000:0x10000100 -d -n 50 input.o
```

**Project 3 (Pipelined Simulator)**:
```bash
./cs311sim -n 100 -p input.o
```

**Project 4 (MMU Simulator)**:
```bash
./cs311mmu -c 32:4 sample_input/gcc
./cs311mmu -c 64:8 -x sample_input/swim
```

---

## Project Structure

```
.
├── project1/           # MIPS Assembler
│   ├── main.cpp        # Main assembler implementation
│   ├── Makefile
│   ├── sample_input/
│   └── sample_output/
│
├── project2/           # MIPS Simulator
│   ├── cs311.c         # Main driver
│   ├── parse.c         # Binary parser
│   ├── run.c           # Instruction execution
│   ├── util.c/h        # Utility functions
│   ├── sample_input/
│   └── sample_output/
│
├── project3/           # MIPS Pipelined Simulator
│   ├── cs311.c
│   ├── parse.c
│   ├── run.c           # Pipeline implementation
│   ├── util.c/h        # Pipeline register management
│   ├── sample_input/
│   └── sample_output/
│
└── project4/           # MMU Simulator
    └── project4/
        ├── main.c      # TLB, page walker, fault handler
        ├── util.c/h    # Memory management utilities
        ├── sample_input/
        └── sample_output/
```

---

## Key Learning Outcomes

### 1. **Assembly and Machine Code**
- Understanding MIPS instruction formats (R, I, J types)
- Binary encoding of instructions
- Label resolution and address calculation

### 2. **Computer Architecture Fundamentals**
- Instruction fetch-decode-execute cycle
- Register file operations
- Memory hierarchy and addressing

### 3. **Pipeline Design**
- Pipeline stages and registers
- Data hazard detection and forwarding
- Control hazard handling
- Pipeline stalls and flushing

### 4. **Memory Management**
- Virtual to physical address translation
- TLB design and operation
- Multi-level page tables
- Page fault handling
- LRU replacement policy

---

## Technical Specifications

### MIPS Instruction Set Supported
- **Arithmetic**: ADD, ADDI, SUB
- **Logical**: AND, ANDI, OR, ORI, NOR
- **Comparison**: SLT, SLTI (signed operations)
- **Shift**: SLL, SRL
- **Memory**: LW, SW (4-byte word only)
- **Branch**: BEQ, BNE
- **Jump**: J, JAL, JR
- **Special**: LUI, LA (pseudo)

### Memory Layout
- **Reserved**: 0x00000000 - 0x003FFFFF
- **Text Segment**: 0x00400000 - 0x0FFFFFFF
- **Data Segment**: 0x10000000 - 0x7FFFFFFF
- **Stack Segment**: 0x7FFFFFFF - 0xFFFFFFFF

---

## Notes

- All projects were implemented in C/C++
- Projects were tested on KAIST's Linux server environment
- Submission via GitLab with tagged commits
- Emphasis on exact output matching for grading
- Anti-plagiarism checks enforced

---

## Academic Integrity

These projects were completed individually for academic credit at KAIST. The code is shared for portfolio purposes only. If you are currently taking CS311, please adhere to your institution's academic integrity policies.

---

## License

This repository is for educational and portfolio purposes. Please respect academic integrity guidelines if you are currently enrolled in a similar course.
