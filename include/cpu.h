#pragma once
#include <cstdint>
#include <vector>

#include "inst.h"

// Control signals structure
struct ControlSignals {
  bool RegWrite = false;
  bool MemRead = false;
  bool MemWrite = false;
  bool MemToReg = false;
  bool ALUSrc = false;
  bool Branch = false;
  bool Jump = false;
};

struct IF_ID {
  bool valid = false;
  int pc = 0;
  Inst inst;
};

struct ID_EX {
  bool valid = false;
  Inst inst;
  int pc = 0;
  int rs = 0, rt = 0, rd = 0, imm = 0;
  int32_t reg_rs_val = 0, reg_rt_val = 0;
  ControlSignals ctrl;
};

struct EX_MEM {
  bool valid = false;
  Inst inst;
  int pc = 0;
  int32_t aluResult = 0, writeData = 0;
  int writeReg = 0;
  ControlSignals ctrl;
};

struct MEM_WB {
  bool valid = false;
  Inst inst;
  int pc = 0;
  int32_t memData = 0, aluResult = 0;
  int writeReg = 0;
  ControlSignals ctrl;
};

struct CPU {
  Program P;
  std::vector<int32_t> regs;
  std::vector<int32_t> mem;
  int pc;
  bool debug;
  int cycle;

  IF_ID if_id;
  ID_EX id_ex;
  EX_MEM ex_mem;
  MEM_WB mem_wb;

  CPU(const Program &prog, int mem_words = 1024, bool dbg = false);
  void run();
  void print_regs() const;
  void print_mem_used(bool nonZero) const;
};
