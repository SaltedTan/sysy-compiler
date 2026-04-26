#pragma once

#include <cassert>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

inline bool is_block_terminated = false;

inline std::string next_ir_reg() {
  static int ir_reg_cnt = 0;
  return "%" + std::to_string(ir_reg_cnt++);
}

inline std::string next_var_name(const std::string &name) {
  static int var_cnt = 0;
  return "@" + name + "_" + std::to_string(var_cnt++);
}

struct Symbol {
  bool is_const;
  int val;
  std::string ir_id;
};

class Environment {
private:
  std::vector<std::unordered_map<std::string, Symbol>> scope_stack;

public:
  Environment() { scope_stack.push_back({}); }

  void insert(const std::string &name, Symbol sym) {
    scope_stack.back()[name] = sym;
  }

  Symbol lookup(const std::string &name) const {
    for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it) {
      auto found = it->find(name);
      if (found != it->end()) {
        return found->second;
      }
    }

    std::cerr << "Error: Undefined variable '" << name << "'" << std::endl;
    exit(1);
  }

  void enter_scope() { scope_stack.push_back({}); }

  void exit_scope() { scope_stack.pop_back(); }
};

inline Environment symbol_table;

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
    is_block_terminated = false;
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
    symbol_table.enter_scope();
    for (const auto &item : items) {
      if (item != nullptr) {
        item->Dump();
        if (is_block_terminated)
          break;
      }
    }
    symbol_table.exit_scope();
    return "";
  }
};

class LValAST : public BaseAST {
public:
  std::string ident;

  std::string Dump() const override {
    if (symbol_table.lookup(ident).is_const) {
      return std::to_string(symbol_table.lookup(ident).val);
    }

    std::string dest_reg = next_ir_reg();
    std::string ir_id = symbol_table.lookup(ident).ir_id;
    std::cout << "  " << dest_reg << " = load " << ir_id << std::endl;
    return dest_reg;
  }

  int Evaluate() const override { return symbol_table.lookup(ident).val; }
};

class StmtAST : public BaseAST {};

class ReturnStmtAST : public StmtAST {
public:
  std::unique_ptr<BaseAST> exp;

  std::string Dump() const override {
    if (exp) {
      std::string ret_val = exp->Dump();
      std::cout << "  ret " << ret_val << std::endl;
    } else {
      std::cout << "  ret" << std::endl;
    }
    is_block_terminated = true;
    return "";
  }
};

class AssignStmtAST : public StmtAST {
public:
  std::unique_ptr<BaseAST> lVal;
  std::unique_ptr<BaseAST> exp;

  std::string Dump() const override {
    std::string rhs_val = exp->Dump();
    auto lval_ast = dynamic_cast<LValAST *>(lVal.get());
    std::string var_name = lval_ast->ident;
    std::string ir_id = symbol_table.lookup(var_name).ir_id;

    std::cout << "  store " << rhs_val << ", " << ir_id << std::endl;
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
    static int logical_cnt = 0;
    int current_cnt = logical_cnt++;

    std::string res_alloc = "@logical_res_" + std::to_string(current_cnt);
    std::string true_block = "@logical_true_" + std::to_string(current_cnt);
    std::string false_block = "@logical_false_" + std::to_string(current_cnt);
    std::string end_block = "@logical_end_" + std::to_string(current_cnt);

    std::cout << "  " << res_alloc << " = alloc i32" << std::endl;

    std::string lhs_val = lhs->Dump();

    std::cout << "  br " << lhs_val << ", " << true_block << ", " << false_block
              << std::endl;

    if (op == "or") {
      // --- OR: Short-circuit on TRUE ---
      std::cout << std::endl << true_block << ":" << std::endl;
      is_block_terminated = false;
      std::cout << "  store 1, " << res_alloc << std::endl;
      std::cout << "  jump " << end_block << std::endl;

      // --- OR: Evaluate RHS on FALSE ---
      std::cout << std::endl << false_block << ":" << std::endl;
      is_block_terminated = false;
      std::string rhs_val = rhs->Dump();
      std::string rhs_bool = next_ir_reg();
      std::cout << "  " << rhs_bool << " = ne " << rhs_val << ", 0"
                << std::endl;
      std::cout << "  store " << rhs_bool << ", " << res_alloc << std::endl;
      std::cout << "  jump " << end_block << std::endl;

    } else if (op == "and") {
      // --- AND: Evaluate RHS on TRUE ---
      std::cout << std::endl << true_block << ":" << std::endl;
      is_block_terminated = false;
      std::string rhs_val = rhs->Dump();
      std::string rhs_bool = next_ir_reg();
      std::cout << "  " << rhs_bool << " = ne " << rhs_val << ", 0"
                << std::endl;
      std::cout << "  store " << rhs_bool << ", " << res_alloc << std::endl;
      std::cout << "  jump " << end_block << std::endl;

      // --- AND: Short-circuit on FALSE ---
      std::cout << std::endl << false_block << ":" << std::endl;
      is_block_terminated = false;
      std::cout << "  store 0, " << res_alloc << std::endl;
      std::cout << "  jump " << end_block << std::endl;
    }

    std::cout << std::endl << end_block << ":" << std::endl;
    is_block_terminated = false;

    std::string final_res = next_ir_reg();
    std::cout << "  " << final_res << " = load " << res_alloc << std::endl;

    return final_res;
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

class DeclAST : public BaseAST {
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
    symbol_table.insert(ident, {true, val, ""});
    return "";
  }
};

class VarDefAST : public BaseAST {
public:
  std::string ident;
  std::unique_ptr<BaseAST> initVal;

  std::string Dump() const override {
    std::string ir_id = next_var_name(ident);
    std::cout << "  " << ir_id << " = alloc i32" << std::endl;
    symbol_table.insert(ident, {false, 0, ir_id});

    if (initVal != nullptr) {
      std::string init_val_str = initVal->Dump();
      std::cout << "  store " << init_val_str << ", " << ir_id << std::endl;
    }
    return "";
  }
};

class IfStmtAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> cond;
  std::unique_ptr<BaseAST> then_branch;
  std::unique_ptr<BaseAST> else_branch; // Might be nullptr

  std::string Dump() const override {
    static int if_cnt = 0;
    int current_if = if_cnt++;

    std::string then_block = "%then_" + std::to_string(current_if);
    std::string else_block = "%else_" + std::to_string(current_if);
    std::string end_block = "%end_" + std::to_string(current_if);

    std::string cond_reg = cond->Dump();

    if (else_branch) {
      std::cout << "  br " << cond_reg << ", " << then_block << ", "
                << else_block << std::endl;
    } else {
      std::cout << "  br " << cond_reg << ", " << then_block << ", "
                << end_block << std::endl;
    }

    std::cout << std::endl << then_block << ":" << std::endl;
    is_block_terminated = false;
    if (then_branch)
      then_branch->Dump();
    if (!is_block_terminated) {
      std::cout << "  jump " << end_block << std::endl;
    }

    if (else_branch) {
      std::cout << std::endl << else_block << ":" << std::endl;
      is_block_terminated = false;
      else_branch->Dump();
      if (!is_block_terminated) {
        std::cout << "  jump " << end_block << std::endl;
      }
    }

    std::cout << std::endl << end_block << ":" << std::endl;
    is_block_terminated = false;

    return "";
  }
};
