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
    struct ExprWithoutMoveInstr* left;
    TokenType t;
    struct ExprWithoutMoveInstr* right;
};

struct MoveInstr {
    enum Registers regs;
    struct ExprWithoutMoveInstr* expr;
};

//Same as Expr struct, but without Move Instruction
// Used for binop only
struct ExprWithoutMoveInstr {
    //Type
    struct BinOp* binop; // Binary Operator
    //End Type
    struct ExprWithoutMoveInstr* next;
    struct ExprWithoutMoveInstr* last;
    struct Literal* Literal;
};


struct Expr {
    //Type
    struct BinOp* binop; // Binary Operator
    struct MoveInstr* moveinstr; // move instr
    struct Literal* Literal;
    //End Type
    struct Expr* next;
    struct Expr* last;
};

struct Unary {
    TokenType t;
    struct ExprWithoutMoveInstr* expr;
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