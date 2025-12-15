# MIPS-Simulator

## Overview
This project is a simulator for a pipelined CPU processor.
It parses assembly programs, executes them through a simulated 5-stage pipeline (IF, ID, EX, MEM, WB), and prints out the final state of the registers and memory

The simulator is categorized in different folders called src(cpp files), include(header files), and examples(asm files), with the main and makefile outside of these folders.


## Project Structure 
- main.cpp -> The entry point. Loads assembly, calls parser function, creates the CPU, runs the simulation, and prints the results
- parser.h/parser.cpp -> Converts raw assembly text into structured instructions (Inst) and builds the Program object with labels
- cpu.h/cpu.cpp -> Defines and implements the CPU class, pipeline registers, control signals, and the run() function.
- util.h/util.cpp -> Defines and implements helper functions for string trimming, token splitting, register name conversion, immediate parsing, memory access, and formatted output.
- inst.h -> Defines the Instruction structure and opcode enumeration.

## Compilation
To build the simulator with g++:

g++ -std=c++17 -Wall -O2 main.cpp cpu.cpp parser.cpp util.cpp -o <run_file_name>

or use the provided makefile and build with:

make -j$(nproc)

Run the simulator like this:

./<run_file_name> <assembly_file_name>.asm

- add a debug argument to print out each cycle to see how each instruction goes through each stage
- add --nonzero flag to only print out the final state of memory with non-zero values
