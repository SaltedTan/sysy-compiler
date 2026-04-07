#pragma once

#include <iostream>
#include <memory>
#include <ostream>
#include <string>

class BaseAST {
public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
};

// CompUnit ::= FuncDef
class CompUnitAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_def;

  void Dump() const override { func_def->Dump(); }
};

// FunDef ::= FuncType IDENT "(" ")" Block
class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump() const override {
    std::cout << "fun @" << ident << "(): ";
    func_type->Dump();
    std::cout << " {" << std::endl;
    std::cout << "%entry:" << std::endl;
    block->Dump();
    std::cout << "}" << std::endl;
  }
};

// FuncType ::= "int"
class FuncTypeAST : public BaseAST {
public:
  std::string type_name;

  void Dump() const override { std::cout << "i32"; }
};

// Block ::= "{" Stmt "}"
class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override { stmt->Dump(); }
};

// Stmt ::= "return" Number ";"
class StmtAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> number;

  void Dump() const override {
    std::cout << "  ret ";
    number->Dump();
    std::cout << std::endl;
  }
};

// Number ::= INT_CONST
class NumberAST : public BaseAST {
public:
  int int_const;

  void Dump() const override { std::cout << int_const; }
};
