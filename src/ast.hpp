#pragma once

#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

inline std::string next_ir_reg() {
  static int ir_reg_cnt = 0;
  return "%" + std::to_string(ir_reg_cnt++);
}

struct Symbol {
  bool is_const;
  int val;
  std::string ir_id;
};

inline std::unordered_map<std::string, Symbol> symbol_table;

inline std::string to_koopa_op(const std::string &op) {
  if (op == "+")
    return "add";
  if (op == "-")
    return "sub";
  if (op == "*")
    return "mul";
  if (op == "/")
    return "div";
  if (op == "%")
    return "mod";
  if (op == "<")
    return "lt";
  if (op == ">")
    return "gt";
  if (op == "<=")
    return "le";
  if (op == ">=")
    return "ge";
  if (op == "==")
    return "eq";
  if (op == "!=")
    return "ne";
  return "";
}

class BaseAST {
public:
  virtual ~BaseAST() = default;
  virtual std::string Dump() const = 0;
  virtual int Evaluate() const { return 0; }
};

class CompUnitAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_def;

  std::string Dump() const override {
    func_def->Dump();
    return "";
  }
};

class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  std::string Dump() const override {
    std::cout << "fun @" << ident << "(): ";
    func_type->Dump();
    std::cout << " {" << std::endl;
    std::cout << "%entry:" << std::endl;
    block->Dump();
    std::cout << "}" << std::endl;
    return "";
  }
};

class FuncTypeAST : public BaseAST {
public:
  std::string type_name;

  std::string Dump() const override {
    std::cout << "i32";
    return "";
  }
};

class BlockAST : public BaseAST {
public:
  std::vector<std::unique_ptr<BaseAST>> items;

  std::string Dump() const override {
    for (const auto &item : items) {
      item->Dump();
    }
    return "";
  }
};

class LValAST : public BaseAST {
public:
  std::string ident;

  std::string Dump() const override {
    if (symbol_table[ident].is_const) {
      return std::to_string(symbol_table[ident].val);
    }

    std::string dest_reg = next_ir_reg();
    std::string ir_id = symbol_table[ident].ir_id;
    std::cout << "  " << dest_reg << " = load " << ir_id << std::endl;
    return dest_reg;
  }

  int Evaluate() const override { return symbol_table[ident].val; }
};

class StmtAST : public BaseAST {
public:
  bool is_return = false;
  std::unique_ptr<BaseAST> lVal;
  std::unique_ptr<BaseAST> exp;

  std::string Dump() const override {
    if (is_return) {
      std::string ret_val = exp->Dump();
      std::cout << "  ret " << ret_val << std::endl;
    } else {
      std::string rhs_val = exp->Dump();
      auto lval_ast = dynamic_cast<LValAST *>(lVal.get());
      std::string var_name = lval_ast->ident;
      std::string ir_id = symbol_table[var_name].ir_id;
      std::cout << "  store " << rhs_val << ", " << ir_id << std::endl;
    }
    return "";
  }
};

class NumberAST : public BaseAST {
public:
  int int_const;

  std::string Dump() const override { return std::to_string(int_const); }

  int Evaluate() const override { return int_const; }
};

class UnaryExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> op;
  std::unique_ptr<BaseAST> exp;

  std::string Dump() const override {
    std::string op_str = op->Dump();
    std::string inner_val = exp->Dump();

    if (op_str == "+") {
      return inner_val;
    } else if (op_str == "-") {
      std::string new_reg = next_ir_reg();
      std::cout << "  " << new_reg << " = sub 0, " << inner_val << std::endl;
      return new_reg;
    } else if (op_str == "!") {
      std::string new_reg = next_ir_reg();
      std::cout << "  " << new_reg << " = eq " << inner_val << ", 0"
                << std::endl;
      return new_reg;
    }
    return "";
  }

  int Evaluate() const override {
    int inner_val = exp->Evaluate();
    std::string op_str = op->Dump();
    if (op_str == "+")
      return inner_val;
    if (op_str == "-")
      return -inner_val;
    if (op_str == "!")
      return inner_val == 0;
    return 0;
  }
};

class BinaryExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> lhs;
  std::unique_ptr<BaseAST> op;
  std::unique_ptr<BaseAST> rhs;

  std::string Dump() const override {
    std::string lhs_val = lhs->Dump();
    std::string op_str = op->Dump();
    std::string rhs_val = rhs->Dump();

    std::string new_reg = next_ir_reg();

    std::cout << "  " << new_reg << " = " << to_koopa_op(op_str) << " "
              << lhs_val << ", " << rhs_val << std::endl;

    return new_reg;
  }
  int Evaluate() const override {
    int lhs_val = lhs->Evaluate();
    std::string op_str = op->Dump();
    int rhs_val = rhs->Evaluate();
    if (op_str == "+")
      return lhs_val + rhs_val;
    if (op_str == "-")
      return lhs_val - rhs_val;
    if (op_str == "*")
      return lhs_val * rhs_val;
    if (op_str == "/")
      return lhs_val / rhs_val;
    if (op_str == "%")
      return lhs_val % rhs_val;
    if (op_str == "<")
      return lhs_val < rhs_val;
    if (op_str == ">")
      return lhs_val > rhs_val;
    if (op_str == "<=")
      return lhs_val <= rhs_val;
    if (op_str == ">=")
      return lhs_val >= rhs_val;
    if (op_str == "==")
      return lhs_val == rhs_val;
    if (op_str == "!=")
      return lhs_val != rhs_val;
    return 0;
  }
};

class LogicalExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> lhs;
  std::string op;
  std::unique_ptr<BaseAST> rhs;

  std::string Dump() const override {
    std::string lhs_val = lhs->Dump();
    std::string rhs_val = rhs->Dump();

    std::string lhs_reg = next_ir_reg();
    std::string rhs_reg = next_ir_reg();
    std::string dest_reg = next_ir_reg();

    std::cout << "  " << lhs_reg << " = ne " << lhs_val << ", 0" << std::endl;
    std::cout << "  " << rhs_reg << " = ne " << rhs_val << ", 0" << std::endl;
    std::cout << "  " << dest_reg << " = " << op << " " << lhs_reg << ", "
              << rhs_reg << std::endl;
    return dest_reg;
  }

  int Evaluate() const override {
    int lhs_val = lhs->Evaluate();
    int rhs_val = rhs->Evaluate();
    if (op == "and")
      return lhs_val && rhs_val;
    if (op == "or")
      return lhs_val || rhs_val;
    return 0;
  }
};

class OpAST : public BaseAST {
public:
  std::string op;
  std::string Dump() const override { return op; }
};

class ConstDeclAST : public BaseAST {
public:
  std::vector<std::unique_ptr<BaseAST>> defs;

  std::string Dump() const override {
    for (const auto &def : defs) {
      def->Dump();
    }
    return "";
  }
};

class ConstDefAST : public BaseAST {
public:
  std::string ident;
  std::unique_ptr<BaseAST> constInitVal;

  std::string Dump() const override {
    int val = constInitVal->Evaluate();
    symbol_table[ident] = {true, val, ""};
    return "";
  }
};

class VarDeclAST : public BaseAST {
public:
  std::vector<std::unique_ptr<BaseAST>> defs;

  std::string Dump() const override {
    for (const auto &def : defs) {
      def->Dump();
    }
    return "";
  }
};

class VarDefAST : public BaseAST {
public:
  std::string ident;
  std::unique_ptr<BaseAST> initVal;

  std::string Dump() const override {
    std::string ident_str = "@" + ident;

    std::cout << "  " << ident_str << " = alloc i32" << std::endl;
    symbol_table[ident] = {false, 0, ident_str};

    if (initVal != nullptr) {
      std::string init_val_str = initVal->Dump();
      std::cout << "  store " << init_val_str << ", " << ident_str << std::endl;
    }
    return "";
  }
};
