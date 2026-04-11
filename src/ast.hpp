#pragma once

#include <iostream>
#include <memory>
#include <ostream>
#include <string>

inline std::string next_ir_reg() {
  static int ir_reg_cnt = 0;
  return "%" + std::to_string(ir_reg_cnt++);
}

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
};

// CompUnit ::= FuncDef
class CompUnitAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_def;

  std::string Dump() const override {
    func_def->Dump();
    return "";
  }
};

// FunDef ::= FuncType IDENT "(" ")" Block
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

// FuncType ::= "int"
class FuncTypeAST : public BaseAST {
public:
  std::string type_name;

  std::string Dump() const override {
    std::cout << "i32";
    return "";
  }
};

// Block ::= "{" Stmt "}"
class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;

  std::string Dump() const override {
    stmt->Dump();
    return "";
  }
};

// Stmt ::= "return" Exp ";"
class StmtAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> exp;

  std::string Dump() const override {
    std::string ret_val = exp->Dump();
    std::cout << "  ret " << ret_val << std::endl;
    return "";
  }
};

// Number ::= INT_CONST
class NumberAST : public BaseAST {
public:
  int int_const;

  std::string Dump() const override { return std::to_string(int_const); }
};

// UnaryExp ::= PrimaryExp | UnaryOp UnaryExp
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
};

class OpAST : public BaseAST {
public:
  std::string op;
  std::string Dump() const override { return op; }
};
