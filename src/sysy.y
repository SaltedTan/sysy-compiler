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
%token CONST

%type <ast_val> FuncDef FuncType Block Stmt Number
%type <ast_val> Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <ast_val> UnaryOp MulOp AddOp RelOp EqOp
%type <ast_val> Decl ConstDecl ConstDefList ConstDef ConstInitVal
%type <ast_val> BlockItem BlockItemList LVal ConstExp
%type <ast_val> VarDecl VarDefList VarDef InitVal 

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
  : '{' BlockItemList '}' {
    $$ = $2;
  }
  ;

BlockItemList
  : /* empty */ {
    auto ast = new BlockAST();
    $$ = ast;
  }
  | BlockItemList BlockItem {
    auto ast = dynamic_cast<BlockAST*>($1);
    ast->items.push_back(unique_ptr<BaseAST>($2));
    $$ = ast;
  }
  ;

BlockItem
  : Decl {
    $$ = $1;
  }
  | Stmt {
    $$ = $1;
  }
  ;

Stmt
  : LVal '=' Exp ';' {
    auto ast = new AssignStmtAST();
    ast->lVal = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RETURN Exp ';' {
    auto ast = new ReturnStmtAST();
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
  | LVal {
    $$ = $1;
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

Decl
  : ConstDecl {
    $$ = $1;
  }
  | VarDecl {
    $$ = $1;
  }
  ;

// ConstDecl ::= "const" BType ConstDef {"," ConstDef} ";"
ConstDecl
  : CONST BType ConstDefList ';' {
    $$ = $3;
  }
  ;

ConstDefList
  : ConstDef {
    auto ast = new ConstDeclAST();
    ast->defs.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | ConstDefList ',' ConstDef {
    auto ast = dynamic_cast<ConstDeclAST*>($1);
    ast->defs.push_back(unique_ptr<BaseAST>($3));
    $$ = ast;
  }
  ;

BType
  : INT
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->constInitVal = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

ConstInitVal
  : ConstExp {
    $$ = $1;
  }
  ;

// VarDecl ::= BType VarDef {"," VarDef} ";"
VarDecl
  : BType VarDefList ';' {
    $$ = $2;
  }
  ;

VarDefList
  : VarDef {
    auto ast = new VarDeclAST();
    ast->defs.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | VarDefList ',' VarDef {
    auto ast = dynamic_cast<VarDeclAST*>($1);
    ast->defs.push_back(unique_ptr<BaseAST>($3));
    $$ = ast;
  }
  ;

VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->initVal = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

InitVal
  : Exp {
    $$ = $1;
  }
  ;

LVal
  : IDENT {
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;

ConstExp
  : Exp {
    $$ = $1;
  }
  ;

%%

void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
