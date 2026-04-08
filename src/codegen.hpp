#pragma once

#include "koopa.h"
#include <string>
#include <unordered_map>

class RiscVGenerator {
private:
  int reg_cnt = 0;
  std::unordered_map<koopa_raw_value_t, std::string> value_to_reg;

public:
  void Generate(const koopa_raw_program_t &program);

private:
  void Visit(const koopa_raw_slice_t &slice);
  void Visit(const koopa_raw_function_t &func);
  void Visit(const koopa_raw_basic_block_t &bb);
  void Visit(const koopa_raw_value_t &value);
};
