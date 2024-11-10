/*
    ax <- 2 + 3 * (9 + 1)
    Literal: NUMBER |  STRING
    Unary: - | Expr
    Binary: Expr | binop | Expr
    Move: Register | <- | Expr
*/

#include "token.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <parse.h>
#include <string.h>

int currAST = 0;
int countAST = 0;
Token_t* tok;

// I swear to God  this code look exactly like yan dev code =(.
// I must find another way to fix this.
// Although this work, it look very cursed by just looking the function body...
// Usage: Return enum Registers from string.
enum Registers GetRegisterEnum(char* regs) {
    if (strncmp(regs, "AL", 2)==0
    || strncmp(regs, "al", 2)==0) {
        return AL;
    } else if (strncmp(regs, "CL", 2)==0
    || strncmp(regs, "cl", 2)==0) {
        return CL;
    } else if (strncmp(regs, "DL", 2)==0
    || strncmp(regs, "dl", 2)==0) {
        return DL;
    } else if (strncmp(regs, "BL", 2)==0
    || strncmp(regs, "bl", 2)==0) {
        return BL;
    }
    // H's Regs
    else if (strncmp(regs, "AH", 2)==0
    || strncmp(regs, "ah", 2)==0) {
        return AH;
    } else if (strncmp(regs, "CH", 2)==0
    || strncmp(regs, "ch", 2)==0) {
        return CH;
    } else if (strncmp(regs, "DH", 2)==0
    || strncmp(regs, "dh", 2)==0) {
        return DH;
    } else if (strncmp(regs, "BH", 2)==0
    || strncmp(regs, "bh", 2)==0) {
        return BH;
    }
    // X's regs
    else if (strncmp(regs, "AX", 2)==0
    || strncmp(regs, "ax", 2)==0) {
        return AX;
    } else if (strncmp(regs, "CX", 2)==0
    || strncmp(regs, "cx", 2)==0) {
        return CX;
    } else if (strncmp(regs, "DX", 2)==0||
    strncmp(regs, "dx", 2)==0) {
        return DX;
    } else if (strncmp(regs, "BX", 2)==0
    || strncmp(regs, "bx", 2)==0) {
        return BX;
    } 
    //P's registers
    else if (strncmp(regs, "SP", 2)==0
    || strncmp(regs, "sp", 2)==0) {
        return SP;
    } else if (strncmp(regs, "BP", 2)==0
    || strncmp(regs, "bp", 2)==0) {
        return BP;
    } else if (strncmp(regs, "SI", 2)==0
    || strncmp(regs, "si", 2)==0) {
        return SI;
    }     else if (strncmp(regs, "DI", 2)==0
    || strncmp(regs, "di", 2)==0) {
        return DI;
    }
    return Unknown;
}


int sizeofToken(Token_t* t) {   
    int count=0;
    while(t != NULL) {
        if (t->Key!=NULL)count++;
        t = t->next;
    }
    return count;
}

MoveInstr* CreateMoveInstr(enum Registers regs, struct 
ExprWithoutMoveInstr* data) {
    MoveInstr* expr = malloc(2*sizeof(MoveInstr));
    expr->regs = regs;
    expr->expr = data;
    return expr;
}

BinOp* CreateBinOp(ExprWithoutMoveInstr* left, TokenType type, ExprWithoutMoveInstr* right) {
    BinOp* op = malloc(2*sizeof(BinOp));
    op->left = left;
    op->t = type;
    op->right = right;
    return op;
}


Token_t* peekAST() {
    return tok;
}

uint8_t IsAtEndAST() {
    return peekAST()->t == _EOF_;
}

uint8_t checkAST(TokenType t) {
    if (IsAtEndAST()) return 0;
    //printf("%i, %i -> Ret: %i\n", peekAST()->t, t, peekAST()->t == t);
    return peekAST()->t == t;
}

Token_t* prevAST() {
    return tok->prev;
}

Token_t* nextAST() {
    if (!IsAtEndAST()) {
        return tok->next;
    }
    return prevAST();
}

uint8_t matchAST(TokenType t[], size_t sz) {
    for (int i=0;i<sz;i++) {
        if (checkAST(t[i])) {
            nextAST();
            return 1;
        }
    }
    return 0;
}

Unary* CreateUnary(TokenType op, ExprWithoutMoveInstr* right) {
    Unary* un = malloc(2*sizeof(Unary));
    un->expr = right;
    un->t = op;
    return un;
}

Literal* CreateLiteral(char* value) {
    Literal* lit = malloc(2*sizeof(Literal));
    lit->Literal = value;
    return lit;
}

void Error() {
    printf("Error detected.\n");
    exit_code=-1;
    exit(-1);
}

Expr* primary() {
    Expr* expr=malloc(2*sizeof(Expr));
    TokenType t[] = {NUMBER, STRING};
    TokenType t2[] = {REGISTERS};
    if (matchAST(t,2)) {
        expr->Literal = CreateLiteral(prevAST()->Key);
        return expr;
    }
    if (matchAST(t2, 1)) {
        expr->Literal = CreateLiteral(prevAST()->Key);
        return expr;
    }
    expr = expr->next;
    return expr;
}

Expr* unary() {
    TokenType t[1] = {MINUS};
    if (matchAST(t, 1)) {
        Token_t* op = prevAST();
        Expr* right = unary();
        return (Expr*)CreateUnary(op->t, (ExprWithoutMoveInstr*)right);
    }
    return primary();
}

Expr* factor() {
    ExprWithoutMoveInstr* expr = (ExprWithoutMoveInstr*)unary();

    TokenType t[] = {SLASH,STAR};
    while(matchAST(t, 2)) {
        Token_t* op = prevAST();
        ExprWithoutMoveInstr* right = (ExprWithoutMoveInstr*)unary();
        expr->binop = CreateBinOp(expr, op->t, right);
    }
    return (Expr*)expr;
}

Expr* term() {
    ExprWithoutMoveInstr* expr = (ExprWithoutMoveInstr*)factor();
    TokenType t[] = {MINUS, PLUS};
    while(matchAST(t, 2)) {
        Token_t* op = prevAST();
        ExprWithoutMoveInstr* right = (ExprWithoutMoveInstr*)factor();
        expr->binop = CreateBinOp(expr, op->t, right);
    }
    return (Expr*)expr;
}


Expr* comparison() {
    ExprWithoutMoveInstr* expr = (ExprWithoutMoveInstr*)term();
    TokenType t[] = {GREATER, GREATER_EQUAL, LESS, LESS_EQUAL};
    while(matchAST(t, 4)) {
        Token_t* op = prevAST();
        ExprWithoutMoveInstr* right = (ExprWithoutMoveInstr*)term();
        expr->binop = CreateBinOp(expr,op->t,right);
    }
    return (Expr*)expr;
}

Expr* equality() {
    Expr* expr = comparison();
    TokenType t[1] = {EQUAL_EQUAL};
    while(matchAST(t,1)) {
        Token_t* op = prevAST();
        ExprWithoutMoveInstr* right=(ExprWithoutMoveInstr*)comparison();
        expr->binop = CreateBinOp((ExprWithoutMoveInstr*)expr, op->t, right);
    }
    return expr;
}

Expr* movepointer() {
    Expr* expr = equality();
    TokenType t[] = {POINTER};
    //printf("Here: %i, %i\n", POINTER, matchAST(t,1));
    while(matchAST(t,1)) {
        Expr* right  = equality();
        expr->moveinstr = CreateMoveInstr(GetRegisterEnum(expr->Literal->Literal), (ExprWithoutMoveInstr*)expr);
    }
    return expr;
}

Expr* parseAST() {
    return movepointer();
}

void InitparseAST(Token_t* token) {
    tok = token;
    countAST = sizeofToken(token);
}