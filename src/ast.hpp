#pragma once

#include <iostream>
#include <memory>
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

  void Dump() const override {
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }";
  }
};

// FunDef ::= FuncType IDENT "(" ")" Block
class FuncDefAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump() const override {
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    block->Dump();
    std::cout << " }";
  }
};

// FuncType ::= "int"
class FuncTypeAST : public BaseAST {
public:
  std::string type_name;

  void Dump() const override {
    std::cout << "FuncTypeAST { ";
    std::cout << type_name;
    std::cout << " }";
  }
};

// Block ::= "{" Stmt "}"
class BlockAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override {
    std::cout << "BlockAST { ";
    stmt->Dump();
    std::cout << " }";
  }
};

// Stmt ::= "return" Number ";"
class StmtAST : public BaseAST {
public:
  std::unique_ptr<BaseAST> number;

  void Dump() const override {
    std::cout << "StmtAST { ";
    number->Dump();
    std::cout << " }";
  }
};

// Number ::= INT_CONST
class NumberAST : public BaseAST {
public:
  int int_const;

  void Dump() const override { std::cout << int_const; }
};
