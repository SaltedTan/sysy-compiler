#pragma once

#include <iostream>
#include <memory>
#include <ostream>
#include <string>

inline std::string next_ir_reg() {
  static int ir_reg_cnt = 0;
  return "%" + std::to_string(ir_reg_cnt++);
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
  std::unique_ptr<BaseAST> unaryOp;
  std::unique_ptr<BaseAST> unaryExp;

  std::string Dump() const override {
    std::string op = unaryOp->Dump();
    std::string inner_val = unaryExp->Dump();

    if (op == "+") {
      return inner_val;
    } else if (op == "-") {
      std::string new_reg = next_ir_reg();
      std::cout << "  " << new_reg << " = sub 0, " << inner_val << std::endl;
      return new_reg;
    } else if (op == "!") {
      std::string new_reg = next_ir_reg();
      std::cout << "  " << new_reg << " = eq " << inner_val << ", 0"
                << std::endl;
      return new_reg;
    }
    return "";
  }
};

// UnaryOp ::= "+" | "-" | "!"
class UnaryOpAST : public BaseAST {
public:
  std::string op;

  std::string Dump() const override { return op; }
};

// MulExp ::= UnaryExp | MulExp MulOp UnaryExp
class MulExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> lhs;
  std::unique_ptr<BaseAST> mulOp;
  std::unique_ptr<BaseAST> rhs;

  std::string Dump() const override {
    std::string lhs_val = lhs->Dump();
    std::string op = mulOp->Dump();
    std::string rhs_val = rhs->Dump();

    std::string new_reg = next_ir_reg();
    if (op == "*") {
      std::cout << "  " << new_reg << " = mul " << lhs_val << ", " << rhs_val
                << std::endl;
      return new_reg;
    } else if (op == "/") {
      std::cout << "  " << new_reg << " = div " << lhs_val << ", " << rhs_val
                << std::endl;
      return new_reg;
    } else if (op == "%") {
      std::cout << "  " << new_reg << " = mod " << lhs_val << ", " << rhs_val
                << std::endl;
      return new_reg;
    }
    return "";
  }
};

// MulOp ::= "*" | "/" | "%"
class MulOpAST : public BaseAST {
public:
  std::string op;

  std::string Dump() const override { return op; }
};

// AddExp ::= MulExp | AddExp AddOp MulExp
class AddExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> lhs;
  std::unique_ptr<BaseAST> addOp;
  std::unique_ptr<BaseAST> rhs;

  std::string Dump() const override {
    std::string lhs_val = lhs->Dump();
    std::string op = addOp->Dump();
    std::string rhs_val = rhs->Dump();

    std::string new_reg = next_ir_reg();
    if (op == "+") {
      std::cout << "  " << new_reg << " = add " << lhs_val << ", " << rhs_val
                << std::endl;
      return new_reg;
    } else if (op == "-") {
      std::cout << "  " << new_reg << " = sub " << lhs_val << ", " << rhs_val
                << std::endl;
      return new_reg;
    }
    return "";
  }
};

// AddOp ::= "+" | "-"
class AddOpAST : public BaseAST {
public:
  std::string op;

  std::string Dump() const override { return op; }
};

// RelExp ::= AddExp | RelExp RelOp AddExp
class RelExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> lhs;
  std::unique_ptr<BaseAST> relOp;
  std::unique_ptr<BaseAST> rhs;

  std::string Dump() const override {
    std::string lhs_val = lhs->Dump();
    std::string op = relOp->Dump();
    std::string rhs_val = rhs->Dump();

    std::string new_reg = next_ir_reg();
    if (op == "<") {
      std::cout << "  " << new_reg << " = lt " << lhs_val << ", " << rhs_val
                << std::endl;
      return new_reg;
    } else if (op == ">") {
      std::cout << "  " << new_reg << " = gt " << lhs_val << ", " << rhs_val
                << std::endl;
      return new_reg;
    } else if (op == "<=") {
      std::cout << "  " << new_reg << " = le " << lhs_val << ", " << rhs_val
                << std::endl;
      return new_reg;
    } else if (op == ">=") {
      std::cout << "  " << new_reg << " = ge " << lhs_val << ", " << rhs_val
                << std::endl;
      return new_reg;
    }
    return "";
  }
};

// RelOp ::= "<" | ">" | "<=" | ">="
class RelOpAST : public BaseAST {
public:
  std::string op;

  std::string Dump() const override { return op; }
};

// EqExp ::= RelExp | EqExp EqOp RelExp
class EqExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> lhs;
  std::unique_ptr<BaseAST> eqOp;
  std::unique_ptr<BaseAST> rhs;

  std::string Dump() const override {
    std::string lhs_val = lhs->Dump();
    std::string op = eqOp->Dump();
    std::string rhs_val = rhs->Dump();

    std::string new_reg = next_ir_reg();
    if (op == "==") {
      std::cout << "  " << new_reg << " = eq " << lhs_val << ", " << rhs_val
                << std::endl;
      return new_reg;
    } else if (op == "!=") {
      std::cout << "  " << new_reg << " = ne " << lhs_val << ", " << rhs_val
                << std::endl;
      return new_reg;
    }
    return "";
  }
};

// EqOp ::= "==" | "!="
class EqOpAST : public BaseAST {
public:
  std::string op;

  std::string Dump() const override { return op; }
};

// LAndExp ::= EqExp | LAndExp "&&" EqExp
class LAndExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> lhs;
  std::unique_ptr<BaseAST> rhs;

  std::string Dump() const override {
    std::string lhs_val = lhs->Dump();
    std::string rhs_val = rhs->Dump();

    std::string lhs_reg = next_ir_reg();
    std::string rhs_reg = next_ir_reg();
    std::string dest_reg = next_ir_reg();

    std::cout << "  " << lhs_reg << " = ne " << lhs_val << ", 0" << std::endl;
    std::cout << "  " << rhs_reg << " = ne " << rhs_val << ", 0" << std::endl;
    std::cout << "  " << dest_reg << " = and " << lhs_reg << ", " << rhs_reg
              << std::endl;
    return dest_reg;
  }
};

// LOrExp ::= LAndExp | LOrExp "||" LAndExp
class LOrExpAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> lhs;
  std::unique_ptr<BaseAST> rhs;

  std::string Dump() const override {
    std::string lhs_val = lhs->Dump();
    std::string rhs_val = rhs->Dump();

    std::string lhs_reg = next_ir_reg();
    std::string rhs_reg = next_ir_reg();
    std::string dest_reg = next_ir_reg();

    std::cout << "  " << lhs_reg << " = ne " << lhs_val << ", 0" << std::endl;
    std::cout << "  " << rhs_reg << " = ne " << rhs_val << ", 0" << std::endl;
    std::cout << "  " << dest_reg << " = or " << lhs_reg << ", " << rhs_reg
              << std::endl;
    return dest_reg;
  }
};
