#pragma once
#include <string>
#include <unordered_map>
#include <vector>

enum Opcode {
  OP_ADD,
  OP_ADDI,
  OP_SUB,
  OP_MUL,
  OP_AND,
  OP_OR,
  OP_SLL,
  OP_SRL,
  OP_LW,
  OP_SW,
  OP_BEQ,
  OP_J,
  OP_NOP
};

struct Inst {
  std::string raw;
  std::string op;
  Opcode opcode;
  int rd, rs, rt;
  int imm;
  std::string label;
  bool isNOP;
  Inst();
};

struct Program {
  std::vector<Inst> insts;
  std::unordered_map<std::string, int> label_to_idx;
};
