#include "codegen.hpp"
#include "koopa.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

void Visit(const koopa_raw_program_t &program) {
  Visit(program.values);
  Visit(program.funcs);
}

void Visit(const koopa_raw_slice_t &slice) {
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

void Visit(const koopa_raw_function_t &func) {
  string func_name = func->name;
  func_name = func_name.substr(1); // Drops the '@'

  cout << "  .text" << endl;
  cout << "  .globl " << func_name << endl;
  cout << func_name << ":" << endl;

  Visit(func->bbs);
}

void Visit(const koopa_raw_basic_block_t &bb) { Visit(bb->insts); }

void Visit(const koopa_raw_value_t &value) {
  const auto &kind = value->kind;
  switch (kind.tag) {
  case KOOPA_RVT_RETURN:
    Visit(kind.data.ret.value);
    cout << "  ret" << endl;
    break;
  case KOOPA_RVT_INTEGER:
    cout << "  li a0, " << kind.data.integer.value << endl;
    break;
  default:
    assert(false);
  }
}
