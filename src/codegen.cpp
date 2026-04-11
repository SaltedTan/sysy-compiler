#include "codegen.hpp"
#include "koopa.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

const string REG_NAMES[] = {"t2", "t3", "t4", "t5", "t6", "a1",
                            "a2", "a3", "a4", "a5", "a6", "a7"};

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
  case KOOPA_RVT_RETURN:
    VisitReturn(kind.data.ret);
    break;

  case KOOPA_RVT_INTEGER: {
    string dest_reg = REG_NAMES[reg_cnt % 12];
    reg_cnt++;
    value_to_reg[value] = dest_reg;

    cout << "  li " << dest_reg << ", " << kind.data.integer.value << endl;
    break;
  }

  case KOOPA_RVT_BINARY: {
    string dest_reg = REG_NAMES[reg_cnt % 12];
    reg_cnt++;
    value_to_reg[value] = dest_reg;

    VisitBinary(kind.data.binary, dest_reg);
    break;
  }
  default:
    assert(false);
  }
}

void RiscVGenerator::VisitReturn(const koopa_raw_return_t &ret) {
  auto ret_value = ret.value;
  if (ret_value->kind.tag == KOOPA_RVT_INTEGER) {
    cout << "  li\ta0, " << ret_value->kind.data.integer.value << endl;
  } else {
    string mapped_register = value_to_reg[ret_value];
    cout << "  mv\ta0, " << mapped_register << endl;
  }

  cout << "  ret" << endl;
}

void RiscVGenerator::VisitBinary(const koopa_raw_binary_t &binary,
                                 const std::string &dest_reg) {
  string left_reg = FetchOperand(binary.lhs, "t0");
  string right_reg = FetchOperand(binary.rhs, "t1");

  switch (binary.op) {
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
  case KOOPA_RBO_LT:
    cout << " slt " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    break;
  case KOOPA_RBO_GT:
    cout << " sgt " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    break;
  case KOOPA_RBO_LE:
    cout << " sgt " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    cout << " seqz " << dest_reg << ", " << dest_reg << endl;
    break;
  case KOOPA_RBO_GE:
    cout << " slt " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    cout << " seqz " << dest_reg << ", " << dest_reg << endl;
    break;
  case KOOPA_RBO_NOT_EQ:
    cout << " xor " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    cout << " snez " << dest_reg << ", " << dest_reg << endl;
    break;
  case KOOPA_RBO_AND:
    cout << " and " << dest_reg << ", " << left_reg << ", " << right_reg
         << endl;
    break;
  case KOOPA_RBO_OR:
    cout << " or " << dest_reg << ", " << left_reg << ", " << right_reg << endl;
    break;
  default:
    assert(false);
  }
}

string RiscVGenerator::FetchOperand(const koopa_raw_value_t &operand,
                                    const string &temp_reg) {
  if (operand->kind.tag == KOOPA_RVT_INTEGER) {
    int val = operand->kind.data.integer.value;
    if (val == 0)
      return "x0";

    cout << "  li " << temp_reg << ", " << val << endl;
    return temp_reg;
  }
  return value_to_reg[operand];
}
