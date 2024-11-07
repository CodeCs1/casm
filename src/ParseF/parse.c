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

//Print move instruction (for debuging purpose).
void PrintMoveInstr(MoveInstr* instr) {
    char regs[3];
    switch(instr->regs) {
        case AX:
            strncpy(regs, "AX", 2);
            break;
        case AL:
            strncpy(regs, "AL", 2);
            break;
        case CL:
            strncpy(regs, "CL", 2);
            break;
        case DL:
            strncpy(regs, "DL", 2);
            break;
        case BL:
            strncpy(regs, "BL", 2);
            break;
        case AH:
            strncpy(regs, "AH", 2);
            break;
        case CH:
            strncpy(regs, "CH", 2);
            break;
        case DH:
            strncpy(regs, "DH", 2);
            break;
        case BH:
            strncpy(regs, "BH", 2);
            break;
        case CX:
            strncpy(regs, "CX", 2);
            break;
        case DX:
            strncpy(regs, "DX", 2);
            break;
        case BX:
            strncpy(regs, "BX", 2);
            break;
        case SP:
            strncpy(regs, "SP", 2);
            break;
        case BP:
            strncpy(regs, "BP", 2);
            break;
        case SI:
            strncpy(regs, "SI", 2);
            break;
        case DI:
            strncpy(regs, "DI", 2);
            break;
        case EAX:
            strncpy(regs, "EAX", 3);
            break;
        case ECX:
            strncpy(regs, "ECX", 3);
            break;
        case EDX:
            strncpy(regs, "EDX", 3);
            break;
        case EBX:
            strncpy(regs, "EBX", 3);
            break;
        case ESP:
            strncpy(regs, "ESP", 3);
            break;
        case EBP:
            strncpy(regs, "EBP", 3);
            break;
        case ESI:
            strncpy(regs, "ESI", 3);
            break;
        case EDI:
            strncpy(regs, "EDI", 3);
            break;
        case ES:
            strncpy(regs, "ES", 2);
            break;
        case CS:
            strncpy(regs, "CS", 2);
            break;
        case SS:
            strncpy(regs, "SS", 2);
            break;
        case DS:
            strncpy(regs, "DS", 2);
            break;
        }

    printf("Registers Appended: %s\n", regs);
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


Token_t peekAST() {
    while(tok->Key == NULL) {
        tok = tok->next;
    }
    return tok[currAST];
}

uint8_t IsAtEndAST() {
    return peekAST().t == _EOF_;
}

uint8_t checkAST(TokenType t) {
    if (IsAtEndAST()) return 0;
    printf("%i, %i\n", peekAST().t, t);
    return peekAST().t == t;
}

Token_t prevAST() {
    return tok[currAST-1];
}

Token_t nextAST() {
    while(tok->Key == NULL) {
        tok = tok->next;
    }
    if (!IsAtEndAST()) {
        currAST++; 
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
        expr->Literal = CreateLiteral(prevAST().Key);
        return expr;
    }
    if (matchAST(t2, 1)) {
        expr->Literal = CreateLiteral(peekAST().Key);
        return expr;
    }
    expr = expr->next;
    return expr;
}

Expr* unary() {
    TokenType t[1] = {MINUS};
    if (matchAST(t, 1)) {
        Token_t op = prevAST();
        Expr* right = unary();
        return (Expr*)CreateUnary(op.t, (ExprWithoutMoveInstr*)right);
    }
    return primary();
}

Expr* factor() {
    ExprWithoutMoveInstr* expr = (ExprWithoutMoveInstr*)unary();

    TokenType t[] = {SLASH,STAR};
    while(matchAST(t, 2)) {
        Token_t op = prevAST();
        ExprWithoutMoveInstr* right = (ExprWithoutMoveInstr*)unary();
        expr->binop = CreateBinOp(expr, op.t, right);
    }
    return (Expr*)expr;
}

Expr* term() {
    ExprWithoutMoveInstr* expr = (ExprWithoutMoveInstr*)factor();
    TokenType t[] = {MINUS, PLUS};
    while(matchAST(t, 2)) {
        Token_t op = prevAST();
        ExprWithoutMoveInstr* right = (ExprWithoutMoveInstr*)factor();
        expr->binop = CreateBinOp(expr, op.t, right);
    }
    return (Expr*)expr;
}


Expr* comparison() {
    ExprWithoutMoveInstr* expr = (ExprWithoutMoveInstr*)term();
    TokenType t[] = {GREATER, GREATER_EQUAL, LESS, LESS_EQUAL};
    while(matchAST(t, 4)) {
        Token_t op = prevAST();
        ExprWithoutMoveInstr* right = (ExprWithoutMoveInstr*)term();
        expr->binop = CreateBinOp(expr,op.t,right);
    }
    return (Expr*)expr;
}

Expr* equality() {
    Expr* expr = comparison();
    TokenType t[1] = {EQUAL_EQUAL};
    while(matchAST(t,1)) {
        Token_t op = prevAST();
        ExprWithoutMoveInstr* right=(ExprWithoutMoveInstr*)comparison();
        expr->binop = CreateBinOp((ExprWithoutMoveInstr*)expr, op.t, right);
    }
    return expr;
}

Expr* movepointer() {
    Expr* expr = equality();
    TokenType t[] = {POINTER};
    printf("Here: %i, %i\n", POINTER, matchAST(t,1));
    while(matchAST(t,1)) {
        Expr* right  = equality();
        expr->moveinstr = CreateMoveInstr(AX, (ExprWithoutMoveInstr*)expr);
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