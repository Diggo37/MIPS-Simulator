#pragma once
#include <string>
#include <vector>

#include "inst.h"

Program parse_asm(const std::vector<std::string>& lines);
