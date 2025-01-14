#pragma once

#include "token.h"


enum Registers {
    AL,CL,DL,BL,AH,CH,DH,BH,/* 8 bits regs*/
    AX,CX,DX,BX,SP,BP,SI,DI,/* 16 bits regs*/
    EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI, /* 32 bits regs */
    ES,CS,SS,DS, /*Segment*/
    Unknown
};

struct BinOp {
    struct Expr* left;
    TokenType t;
    struct Expr* right;
};

struct MoveInstr {
    enum Registers regs;
    struct Expr* expr;
};

struct Group {
    struct Expr* expr;
};

struct Expr {
    //Type
    struct BinOp* binop; // Binary Operator
    struct MoveInstr* moveinstr; // move instr
    struct Literal* Literal;
    struct Unary* unary;
    struct Keywords* key;
    struct VarDeclare* var;
    struct Group* Group;
    //End Type
    struct Expr* next;
    struct Expr* last;
};

struct Unary {
    TokenType t;
    struct Expr* expr;
};

struct Literal {
    char* Literal;
};

struct Keywords {
    char* keywords;
    struct Expr* args; // push cx
};

struct VarDeclare {
    char* name;
    struct Expr* express;
};

typedef struct BinOp BinOp;
typedef struct Expr Expr;
typedef struct MoveInstr MoveInstr;
typedef struct ExprWithoutMoveInstr ExprWithoutMoveInstr;
typedef struct Unary Unary;
typedef struct Literal Literal;
typedef struct Keywords Keywords;
typedef struct VarDeclare VarDeclare;
typedef struct Group Group;

// parse tokenized token into AST ??
void InitparseAST(Token_t*);
Expr* parseAST();
void AddEOL(Token_t*);