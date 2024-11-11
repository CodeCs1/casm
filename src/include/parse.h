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

struct Expr {
    //Type
    struct BinOp* binop; // Binary Operator
    struct MoveInstr* moveinstr; // move instr
    struct Literal* Literal;
    struct Unary* unary;
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



typedef struct BinOp BinOp;
typedef struct Expr Expr;
typedef struct MoveInstr MoveInstr;
typedef struct ExprWithoutMoveInstr ExprWithoutMoveInstr;
typedef struct Unary Unary;
typedef struct Literal Literal;

// parse tokenized token into AST ??
void InitparseAST(Token_t*);
Expr* parseAST();