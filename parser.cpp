#include <cctype>
#include <fstream>
#include <stdexcept>

#include "parser.h"
#include "util.h"

Inst::Inst() : opcode(OP_NOP), rd(0), rs(0), rt(0), imm(0), isNOP(true) {}

Program parse_asm(const std::vector<std::string> &lines) {
  Program P;
  std::vector<std::pair<std::string, std::string>> raw;
  for (auto ln : lines) {
    size_t cpos = ln.find('#');
    if (cpos != std::string::npos) {
      ln = ln.substr(0, cpos);
    }

    ln = trim(ln);

    if (ln.empty()) continue;
    if (ln.back() == ':') {
      raw.push_back({trim(ln.substr(0, ln.size() - 1)), ""});
    } else {
      size_t col = ln.find(':');
      if (col != std::string::npos) {
        raw.push_back({trim(ln.substr(0, col)), trim(ln.substr(col + 1))});
      } else {
        raw.push_back({"", ln});
      }
    }
  }
  int pc = 0;
  for (auto &p : raw) {
    if (!p.first.empty()) {
      P.label_to_idx[p.first] = pc;
    }
    if (!p.second.empty()) {
      pc++;
    }
  }
  for (auto &p : raw) {
    if (p.second.empty()) continue;
    std::string line = p.second;
    for (char &c : line) {
      if (c == ',') {
        c = ' ';
      }
    }
    auto toks = split_ws(line);
    if (toks.empty()) continue;

    Inst I;
    I.raw = line;
    I.isNOP = false;
    I.op = toks[0];
    for (auto &c : I.op) {
      c = toupper(c);
    }
    if (I.op == "NOP") {
      I.opcode = OP_NOP;
      I.isNOP = true;
      P.insts.push_back(I);
      continue;
    }
    if (I.op == "ADD" || I.op == "SUB" || I.op == "MUL" || I.op == "AND" ||
        I.op == "OR" || I.op == "SLL" || I.op == "SRL") {
      if (I.op == "ADD") {
        I.opcode = OP_ADD;
      } else if (I.op == "SUB") {
        I.opcode = OP_SUB;
      } else if (I.op == "MUL") {
        I.opcode = OP_MUL;
      } else if (I.op == "AND") {
        I.opcode = OP_AND;
      } else if (I.op == "OR") {
        I.opcode = OP_OR;
      } else if (I.op == "SLL") {
        I.opcode = OP_SLL;
      } else if (I.op == "SRL") {
        I.opcode = OP_SRL;
      }
      if (toks.size() < 4) {
        throw std::runtime_error("Bad R-type: " + line);
      }
      I.rd = reg_index(toks[1]);
      I.rs = reg_index(toks[2]);
      I.rt = reg_index(toks[3]);
      if ((I.op == "SLL" || I.op == "SRL") && isdigit(toks[2][0])) {
        I.rd = reg_index(toks[1]);
        I.rs = 0;
        I.rt = reg_index(toks[3]);
        I.imm = parse_imm(toks[2]);
      }
      P.insts.push_back(I);
      continue;
    }
    if (I.op == "ADDI") {
      I.opcode = OP_ADDI;
      I.rt = reg_index(toks[1]);
      I.rs = reg_index(toks[2]);
      I.imm = parse_imm(toks[3]);
      P.insts.push_back(I);
      continue;
    }
    if (I.op == "LW" || I.op == "SW") {
      I.opcode = (I.op == "LW") ? OP_LW : OP_SW;
      I.rt = reg_index(toks[1]);
      std::string offbase = toks[2];
      size_t lpar = offbase.find('('), rpar = offbase.find(')');
      if (lpar == std::string::npos || rpar == std::string::npos) {
        throw std::runtime_error("Bad mem operand: " + offbase);
      }
      I.imm = parse_imm(offbase.substr(0, lpar));
      I.rs = reg_index(offbase.substr(lpar + 1, rpar - lpar - 1));
      P.insts.push_back(I);
      continue;
    }
    if (I.op == "BEQ") {
      I.opcode = OP_BEQ;
      I.rs = reg_index(toks[1]);
      I.rt = reg_index(toks[2]);
      I.label = toks[3];
      P.insts.push_back(I);
      continue;
    }
    if (I.op == "J") {
      I.opcode = OP_J;
      I.label = toks[1];
      P.insts.push_back(I);
      continue;
    }
    throw std::runtime_error("Unknown op: " + I.op + " line: " + line);
  }
  return P;
}
