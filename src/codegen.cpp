#include "codegen.hpp"
#include "koopa.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

void RiscVGenerator::Generate(const koopa_raw_program_t &program) {
  Visit(program.values);
  Visit(program.funcs);
}

void RiscVGenerator::Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    switch (slice.kind) {
    case KOOPA_RSIK_FUNCTION:
      Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
      break;
    case KOOPA_RSIK_BASIC_BLOCK:
      Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
      break;
    case KOOPA_RSIK_VALUE:
      Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
      break;
    default:
      assert(false);
    }
  }
}

void RiscVGenerator::Visit(const koopa_raw_function_t &func) {
  string func_name = func->name;
  func_name = func_name.substr(1); // Drops the '@'

  cout << "  .text" << endl;
  cout << "  .globl " << func_name << endl;
  cout << func_name << ":" << endl;

  Visit(func->bbs);
}

void RiscVGenerator::Visit(const koopa_raw_basic_block_t &bb) {
  Visit(bb->insts);
}

void RiscVGenerator::Visit(const koopa_raw_value_t &value) {
  const auto &kind = value->kind;

  switch (kind.tag) {
  case KOOPA_RVT_RETURN: {
    koopa_raw_value_t ret_value = kind.data.ret.value;

    if (ret_value->kind.tag == KOOPA_RVT_INTEGER) {
      cout << "  li\ta0, " << ret_value->kind.data.integer.value << endl;
    } else {
      string mapped_register = value_to_reg[ret_value];
      cout << "  mv\ta0, " << mapped_register << endl;
    }

    cout << "  ret" << endl;
    break;
  }
  case KOOPA_RVT_INTEGER: {
    string reg_name = "t" + to_string(reg_cnt++);
    value_to_reg[value] = reg_name;

    cout << "  li " << reg_name << ", " << kind.data.integer.value << endl;
    break;
  }
  case KOOPA_RVT_BINARY: {
    koopa_raw_value_t lhs = kind.data.binary.lhs;
    koopa_raw_value_t rhs = kind.data.binary.rhs;

    string left_reg;
    if (lhs->kind.tag == KOOPA_RVT_INTEGER) {
      int val = lhs->kind.data.integer.value;
      if (val == 0) {
        left_reg = "x0";
      } else {
        left_reg = "t" + to_string(reg_cnt++);
        cout << "  li " << left_reg << ", " << val << endl;
      }
    } else {
      left_reg = value_to_reg[lhs];
    }

    string right_reg;
    if (rhs->kind.tag == KOOPA_RVT_INTEGER) {
      int val = rhs->kind.data.integer.value;
      if (val == 0) {
        right_reg = "x0";
      } else {
        right_reg = "t" + to_string(reg_cnt++);
        cout << "  li " << right_reg << ", " << val << endl;
      }
    } else {
      right_reg = value_to_reg[rhs];
    }

    string dest_reg = "t" + to_string(reg_cnt++);
    value_to_reg[value] = dest_reg;

    switch (kind.data.binary.op) {
    case KOOPA_RBO_EQ:
      cout << " xor " << dest_reg << ", " << left_reg << ", " << right_reg
           << endl;
      cout << " seqz " << dest_reg << ", " << dest_reg << endl;
      break;
    case KOOPA_RBO_MUL:
      cout << " mul " << dest_reg << ", " << left_reg << ", " << right_reg
           << endl;
      break;
    case KOOPA_RBO_DIV:
      cout << " div " << dest_reg << ", " << left_reg << ", " << right_reg
           << endl;
      break;
    case KOOPA_RBO_MOD:
      cout << " rem " << dest_reg << ", " << left_reg << ", " << right_reg
           << endl;
      break;
    case KOOPA_RBO_ADD:
      cout << " add " << dest_reg << ", " << left_reg << ", " << right_reg
           << endl;
      break;
    case KOOPA_RBO_SUB:
      cout << " sub " << dest_reg << ", " << left_reg << ", " << right_reg
           << endl;
      break;
    default:
      assert(false);
    }
    break;
  }
  default:
    assert(false);
  }
}
