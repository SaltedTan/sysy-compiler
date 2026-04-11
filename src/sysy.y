%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"

int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

%parse-param { std::unique_ptr<BaseAST> &ast }

%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST
%token LE GE EQ NE AND OR

%type <ast_val> FuncDef FuncType Block Stmt Number Exp PrimaryExp UnaryExp UnaryOp MulExp MulOp AddExp AddOp RelExp RelOp EqExp EqOp LAndExp LOrExp

%%

CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = std::move(comp_unit);
  }
  ;

FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type_name = "int";
    $$ = ast;
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast = new BlockAST();
    ast->stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    $$ = $1;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    $$ = $2;
  }
  | Number {
    $$ = $1;
  }
  ;

Number
  : INT_CONST {
    auto ast = new NumberAST();
    ast->int_const = $1;
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    $$ = $1;
  }
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST();
    ast->op = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

UnaryOp
  : '+' {
    auto ast = new OpAST();
    ast->op = "+";
    $$ = ast;
  }
  | '-' {
    auto ast = new OpAST();
    ast->op = "-";
    $$ = ast;
  }
  | '!' {
    auto ast = new OpAST();
    ast->op = "!";
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp {
    $$ = $1;
  }
  | MulExp MulOp UnaryExp {
    auto ast = new BinaryExpAST();
    ast->lhs = unique_ptr<BaseAST>($1);
    ast->op = unique_ptr<BaseAST>($2);
    ast->rhs = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

MulOp
  : '*' {
    auto ast = new OpAST();
    ast->op = "*";
    $$ = ast;
  }
  | '/' {
    auto ast = new OpAST();
    ast->op = "/";
    $$ = ast;
  }
  | '%' {
    auto ast = new OpAST();
    ast->op = "%";
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    $$ = $1;
  }
  | AddExp AddOp MulExp {
    auto ast = new BinaryExpAST();
    ast->lhs = unique_ptr<BaseAST>($1);
    ast->op = unique_ptr<BaseAST>($2);
    ast->rhs = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

AddOp
  : '+' {
    auto ast = new OpAST();
    ast->op = "+";
    $$ = ast;
  }
  | '-' {
    auto ast = new OpAST();
    ast->op = "-";
    $$ = ast;
  }
  ;

RelExp
  : AddExp {
    $$ = $1;
  }
  | RelExp RelOp AddExp {
    auto ast = new BinaryExpAST();
    ast->lhs = unique_ptr<BaseAST>($1);
    ast->op = unique_ptr<BaseAST>($2);
    ast->rhs = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

RelOp
  : '<' {
    auto ast = new OpAST();
    ast->op = "<";
    $$ = ast;
  }
  | '>' {
    auto ast = new OpAST();
    ast->op = ">";
    $$ = ast;
  }
  | LE {
    auto ast = new OpAST();
    ast->op = "<=";
    $$ = ast;
  }
  | GE {
    auto ast = new OpAST();
    ast->op = ">=";
    $$ = ast;
  }
  ;

EqExp
  : RelExp {
    $$ = $1;
  }
  | EqExp EqOp RelExp {
    auto ast = new BinaryExpAST();
    ast->lhs = unique_ptr<BaseAST>($1);
    ast->op = unique_ptr<BaseAST>($2);
    ast->rhs = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

EqOp
  : EQ {
    auto ast = new OpAST();
    ast->op = "==";
    $$ = ast;
  }
  | NE {
    auto ast = new OpAST();
    ast->op = "!=";
    $$ = ast;
  }
  ;

LAndExp
  : EqExp {
    $$ = $1;
  }
  | LAndExp AND EqExp {
    auto ast = new LogicalExpAST();
    ast->lhs = unique_ptr<BaseAST>($1);
    ast->op = "and";
    ast->rhs = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
    $$ = $1;
  }
  | LOrExp OR LAndExp {
    auto ast = new LogicalExpAST();
    ast->lhs = unique_ptr<BaseAST>($1);
    ast->op = "or";
    ast->rhs = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

%%

void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
