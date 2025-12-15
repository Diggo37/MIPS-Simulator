#pragma once
#include <string>
#include <vector>

std::string trim(const std::string &s);
std::vector<std::string> split_ws(const std::string &s);
bool starts_with(const std::string &s, const std::string &p);
int reg_index(const std::string &r);
int parse_imm(const std::string &s);
