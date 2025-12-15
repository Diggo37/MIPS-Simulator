#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "cpu.h"
#include "util.h"

using namespace std;

static Inst nop_inst = [] {
  Inst i;
  i.raw = "NOP";
  i.op = "NOP";
  i.opcode = OP_NOP;
  i.rd = i.rs = i.rt = 0;
  i.imm = 0;
  i.label = "";
  i.isNOP = true;
  return i;
}();

static const char* reg_name(int idx) {
  static const char* names[32] = {
      "$zero", "$at", "$v0", "$v1",  // 0-3
      "$a0",   "$a1", "$a2", "$a3",  // 4-7
      "$t0",   "$t1", "$t2", "$t3",  // 8-11
      "$t4",   "$t5", "$t6", "$t7",  // 12-15
      "$s0",   "$s1", "$s2", "$s3",  // 16-19
      "$s4",   "$s5", "$s6", "$s7",  // 20-23
      "$t8",   "$t9", "$k0", "$k1",  // 24-27
      "$gp",   "$sp", "$fp", "$ra"   // 28-31
  };
  return (idx >= 0 && idx < 32) ? names[idx] : "$r?";
}

CPU::CPU(const Program& prog, int mem_words, bool dbg)
    : P(prog), regs(32, 0), mem(mem_words, 0), pc(0), debug(dbg), cycle(0) {
  regs[0] = 0;
}

static int32_t load_word_from_vec(const vector<int32_t>& mem, int addr) {
  int index = addr / 4;
  if (index < 0 || index >= (int)mem.size()) return 0;
  return mem[index];
}
static void store_word_to_vec(vector<int32_t>& mem, int addr, int32_t val) {
  int index = addr / 4;
  if (index < 0 || index >= (int)mem.size()) return;
  mem[index] = val;
}

void CPU::print_regs() const {
  cout << "Registers:\n";
  for (int i = 0; i < 32; ++i) {
    cout << setw(6) << left << reg_name(i) << ": " << setw(11) << right
         << regs[i];

    if ((i + 1) % 4 != 0) {
      cout << " ";
    } else {
      cout << "\n";
    }
  }
}

void CPU::print_mem_used(bool nonZero) const {
  int maxAddr = -1;
  for (int i = 0; i < (int)mem.size(); i++) {
    if (mem[i] != 0) {
      maxAddr = i;
    }
  }

  if (nonZero) {
    cout << "Memory (non-zero words only):\n";
    for (int i = 0; i <= maxAddr; i++) {
      if (mem[i] != 0) {
        cout << setw(4) << i << ": " << mem[i] << "\n";
      }
    }
  } else {
    cout << "Memory (addresses 0 to " << maxAddr << "):\n";
    for (int i = 0; i <= maxAddr; i += 4) {
      cout << setw(4) << i << ":";
      for (int j = 0; j < 4; j++) {
        if (i + j <= maxAddr) {
          cout << " " << setw(11) << mem[i + j];
        }
      }
      cout << "\n";
    }
  }
}

void CPU::run() {
  // local pipeline regs
  IF_ID if_id;
  ID_EX id_ex;
  EX_MEM ex_mem;
  MEM_WB mem_wb;
  while (true) {
    cycle++;
    // WB
    if (mem_wb.valid && mem_wb.ctrl.RegWrite && mem_wb.writeReg != 0) {
      int32_t val = mem_wb.ctrl.MemToReg ? mem_wb.memData : mem_wb.aluResult;
      if (mem_wb.writeReg != 0) {
        regs[mem_wb.writeReg] = val;
      }
    }
    // MEM
    MEM_WB next_mem_wb;
    next_mem_wb.valid = false;
    if (ex_mem.valid) {
      next_mem_wb.valid = true;
      next_mem_wb.inst = ex_mem.inst;
      next_mem_wb.pc = ex_mem.pc;
      next_mem_wb.aluResult = ex_mem.aluResult;
      next_mem_wb.writeReg = ex_mem.writeReg;
      next_mem_wb.ctrl = ex_mem.ctrl;
      if (ex_mem.ctrl.MemRead) {
        next_mem_wb.memData = load_word_from_vec(mem, ex_mem.aluResult);
      } else if (ex_mem.ctrl.MemWrite) {
        store_word_to_vec(mem, ex_mem.aluResult, ex_mem.writeData);
      }
    }
    // EX
    EX_MEM next_ex_mem;
    next_ex_mem.valid = false;
    bool redirect_pc = false;
    int new_pc = pc;
    if (id_ex.valid) {
      next_ex_mem.valid = true;
      next_ex_mem.inst = id_ex.inst;
      next_ex_mem.pc = id_ex.pc;
      next_ex_mem.ctrl = id_ex.ctrl;
      int32_t opA = id_ex.reg_rs_val;
      int32_t opB = id_ex.reg_rt_val;
      if (ex_mem.valid && ex_mem.ctrl.RegWrite && ex_mem.writeReg != 0) {
        if (ex_mem.writeReg == id_ex.rs) {
          opA = ex_mem.aluResult;
        }
        if (ex_mem.writeReg == id_ex.rt) {
          opB = ex_mem.aluResult;
        }
      }
      if (mem_wb.valid && mem_wb.ctrl.RegWrite && mem_wb.writeReg != 0) {
        int32_t val = mem_wb.ctrl.MemToReg ? mem_wb.memData : mem_wb.aluResult;
        if (mem_wb.writeReg == id_ex.rs) {
          opA = val;
        }
        if (mem_wb.writeReg == id_ex.rt) {
          opB = val;
        }
      }
      int32_t aluB = id_ex.ctrl.ALUSrc ? id_ex.imm : opB;
      int32_t aluOut = 0;

      switch (id_ex.inst.opcode) {
        case OP_ADD:
        case OP_ADDI:
          aluOut = opA + aluB;
          break;
        case OP_SUB:
        case OP_BEQ:
          aluOut = opA - aluB;
          break;
        case OP_MUL:
          aluOut = opA * opB;
          break;
        case OP_AND:
          aluOut = opA & opB;
          break;
        case OP_OR:
          aluOut = opA | opB;
          break;
        case OP_SLL:
          aluOut = (uint32_t)opB << (id_ex.imm & 31);
          break;
        case OP_SRL:
          aluOut = (uint32_t)opB >> (id_ex.imm & 31);
          break;
        case OP_LW:
        case OP_SW:
          aluOut = opA + id_ex.imm;
          break;
        default:
          break;
      }

      next_ex_mem.aluResult = aluOut;
      next_ex_mem.writeData = opB;

      if (id_ex.inst.opcode == OP_ADD || id_ex.inst.opcode == OP_SUB ||
          id_ex.inst.opcode == OP_MUL || id_ex.inst.opcode == OP_AND ||
          id_ex.inst.opcode == OP_OR || id_ex.inst.opcode == OP_SLL ||
          id_ex.inst.opcode == OP_SRL) {
        next_ex_mem.writeReg = id_ex.rd;
      } else if (id_ex.inst.opcode == OP_ADDI || id_ex.inst.opcode == OP_LW) {
        next_ex_mem.writeReg = id_ex.inst.rt;
      } else {
        next_ex_mem.writeReg = 0;
      }

      // Branch resolution
      if (id_ex.inst.opcode == OP_BEQ && aluOut == 0) {
        auto it = P.label_to_idx.find(id_ex.inst.label);
        if (it == P.label_to_idx.end()) {
          throw std::runtime_error("Unknown label");
        }
        new_pc = it->second;
        redirect_pc = true;
        next_ex_mem.ctrl.RegWrite = false;
      }
    }
    // ID
    ID_EX next_id_ex;
    next_id_ex.valid = false;
    if (if_id.valid) {
      auto I = if_id.inst;
      next_id_ex.valid = true;
      next_id_ex.inst = I;
      next_id_ex.pc = if_id.pc;
      next_id_ex.rs = I.rs;
      next_id_ex.rt = I.rt;
      next_id_ex.rd = I.rd;
      next_id_ex.imm = I.imm;
      next_id_ex.reg_rs_val = regs[next_id_ex.rs];
      next_id_ex.reg_rt_val = regs[next_id_ex.rt];
      // control signals
      next_id_ex.ctrl = {};

      switch (I.opcode) {
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_AND:
        case OP_OR:
        case OP_SLL:
        case OP_SRL:
          next_id_ex.ctrl.RegWrite = true;
          break;
        case OP_ADDI:
          next_id_ex.ctrl.RegWrite = true;
          next_id_ex.ctrl.ALUSrc = true;
          break;
        case OP_LW:
          next_id_ex.ctrl.RegWrite = true;
          next_id_ex.ctrl.ALUSrc = true;
          next_id_ex.ctrl.MemRead = true;
          next_id_ex.ctrl.MemToReg = true;
          break;
        case OP_SW:
          next_id_ex.ctrl.ALUSrc = true;
          next_id_ex.ctrl.MemWrite = true;
          break;
        case OP_BEQ:
          next_id_ex.ctrl.Branch = true;
          break;
        case OP_J: {
          auto it = P.label_to_idx.find(I.label);
          if (it == P.label_to_idx.end())
            throw std::runtime_error("Unknown label: " + I.label);
          new_pc = it->second;
          redirect_pc = true;
          // Bubble the decode stage
          next_id_ex.inst = nop_inst;
          break;
        }
        case OP_NOP:
          next_id_ex.valid = false;
          break;
        default:
          break;
      }

      // load-use hazard detection (simple): if id_ex is load and its rt matches
      // sources, stall
      if (id_ex.valid && id_ex.ctrl.MemRead) {
        int loadRt = id_ex.rt;
        bool uses_rs = (next_id_ex.rs == loadRt);
        bool uses_rt =
            (next_id_ex.rt == loadRt) && (next_id_ex.inst.opcode != OP_LW);
        if (uses_rs || uses_rt) {
          next_id_ex.inst = nop_inst;
        }
      }
    }
    // IF
    IF_ID next_if_id;
    next_if_id.valid = false;
    if (redirect_pc) {
      // Branch or jump redirect
      pc = new_pc;
      next_if_id.valid = true;
      next_if_id.inst = nop_inst;  // flush
    } else {
      if (pc >= 0 && pc < (int)P.insts.size()) {
        next_if_id.valid = true;
        next_if_id.pc = pc;
        next_if_id.inst = P.insts[pc];
        pc++;
      }
    }

    // === Commit pipeline registers ===
    mem_wb = next_mem_wb;
    ex_mem = next_ex_mem;
    id_ex = next_id_ex;
    if_id = next_if_id;

    // debug print
    if (debug) {
      auto print_stage = [&](const Inst& I, bool valid) {
        if (!valid) {
          return string("EMPTY");
        }
        if (I.isNOP) {
          return string("NOP");
        }
        return I.raw;
      };
      cout << "Cycle " << cycle << "\n";
      cout << "IF/ID: " << print_stage(if_id.inst, if_id.valid) << "\n";
      cout << "ID/EX: " << print_stage(id_ex.inst, id_ex.valid) << "\n";
      cout << "EX/MEM: " << print_stage(ex_mem.inst, ex_mem.valid) << "\n";
      cout << "MEM/WB: " << print_stage(mem_wb.inst, mem_wb.valid) << "\n";
      print_regs();
      cout << "-------------------------\n";
    }

    // termination: pipeline empty and PC out of range
    bool any_valid = if_id.valid || id_ex.valid || ex_mem.valid || mem_wb.valid;
    if (!any_valid && (pc < 0 || pc >= (int)P.insts.size())) break;
    if (cycle > 1000000) {
      cerr << "Cycle limit\n";
      break;
    }
  }
}
