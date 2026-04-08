#include "ast.hpp"
#include "codegen.hpp"
#include "koopa.h"
#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <streambuf>
#include <string>

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
  // compiler mode input -o output
  assert(argc == 5);
  string mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  yyin = fopen(input, "r");
  assert(yyin);

  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);

  // Capture the AST Dump output into a stringstream
  ostringstream oss;
  streambuf *old_cout_buf = cout.rdbuf(oss.rdbuf());
  ast->Dump();
  cout.rdbuf(old_cout_buf);

  string ir_str = oss.str();

  if (mode == "-koopa") {
    freopen(output, "w", stdout);
    cout << ir_str;
    return 0;
  }

  koopa_program_t program;
  koopa_error_code_t lib_ret =
      koopa_parse_from_string(ir_str.c_str(), &program);
  assert(lib_ret == KOOPA_EC_SUCCESS);

  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  koopa_delete_program(program);

  freopen(output, "w", stdout);

  RiscVGenerator generator;
  generator.Generate(raw);

  koopa_delete_raw_program_builder(builder);
  return 0;
}
