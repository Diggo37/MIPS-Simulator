#include <cctype>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "util.h"

std::string trim(const std::string &s) {
  size_t a = s.find_first_not_of(" \t\r\n");
  if (a == std::string::npos) return "";
  size_t b = s.find_last_not_of(" \t\r\n");
  return s.substr(a, b - a + 1);
}

std::vector<std::string> split_ws(const std::string &s) {
  std::vector<std::string> out;
  std::istringstream iss(s);
  std::string cur;
  while (iss >> cur) {
    out.push_back(cur);
  }
  return out;
}

bool starts_with(const std::string &s, const std::string &p) {
  return s.size() >= p.size() && s.substr(0, p.size()) == p;
}

int reg_index(const std::string &r) {
  static const std::unordered_map<std::string, int> reg_map = {
      {"$zero", 0}, {"$at", 1},  {"$v0", 2},  {"$v1", 3},  {"$a0", 4},
      {"$a1", 5},   {"$a2", 6},  {"$a3", 7},  {"$t0", 8},  {"$t1", 9},
      {"$t2", 10},  {"$t3", 11}, {"$t4", 12}, {"$t5", 13}, {"$t6", 14},
      {"$t7", 15},  {"$s0", 16}, {"$s1", 17}, {"$s2", 18}, {"$s3", 19},
      {"$s4", 20},  {"$s5", 21}, {"$s6", 22}, {"$s7", 23}, {"$t8", 24},
      {"$t9", 25},  {"$k0", 26}, {"$k1", 27}, {"$gp", 28}, {"$sp", 29},
      {"$fp", 30},  {"$ra", 31}};

  std::string key = r;
  if (!key.empty() && key[0] != '$') key = "$" + key;

  auto it = reg_map.find(key);
  if (it != reg_map.end()) return it->second;

  // Numeric registers like $0, $5, etc.
  if (key.size() > 1 && key[0] == '$') {
    try {
      int v = std::stoi(key.substr(1));
      if (v >= 0 && v < 32) return v;
    } catch (...) {
    }
  }

  throw std::runtime_error("Unknown register: " + r);
}

int parse_imm(const std::string &s) {
  if (s.size() > 1 && (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))) {
    return (int)strtol(s.c_str(), nullptr, 16);
  }
  if (s.size() > 1 && s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) {
    return (int)strtol(s.c_str() + 2, nullptr, 2);
  }
  return std::stoi(s);
}
