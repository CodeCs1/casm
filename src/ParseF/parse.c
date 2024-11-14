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
    } else if (strncmp(regs, "DI", 2)==0
    || strncmp(regs, "di", 2)==0) {
        return DI;
    } 
    // S's segment
    else if (strncmp(regs, "ES", 2)==0
    || strncmp(regs, "es", 2)==0) {
        return ES;
    } else if (strncmp(regs, "SS", 2)==0
    || strncmp(regs, "ss", 2)==0) {
        return SS;
    } else if (strncmp(regs, "DS", 2)==0
    || strncmp(regs, "ds", 2)==0) {
        return DS;
    } else if (strncmp(regs, "CS", 2)==0
    || strncmp(regs, "cs", 2)==0) {
        return CS;
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

MoveInstr* CreateMoveInstr(enum Registers regs,
Expr* data) {
    MoveInstr* expr = malloc(2*sizeof(MoveInstr));
    expr->regs = regs;
    expr->expr = data;
    return expr;
}

Expr* CreateBinOp(Expr* left, TokenType type, Expr* right) {
    BinOp* op = malloc(2*sizeof(BinOp));
    Expr* exp = malloc(2*sizeof(Expr));
    op->left = left;
    op->t = type;
    op->right = right;
    exp->binop=op;
    return exp;
}


Token_t* peekAST() {
    return tok;
}

uint8_t IsAtEndAST() {
    return peekAST()->t == _EOF_;
}

uint8_t checkAST(TokenType t) {
    if (IsAtEndAST()) return 0;
    return peekAST()->t == t;
}

Token_t* prevAST() {
    if (!tok->prev) {
        return tok;
    }
    return tok->prev;
}

Token_t* nextAST() {
    if (!IsAtEndAST()) {
        if (tok->next) tok = tok->next;
        return tok;
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

Unary* CreateUnary(TokenType op, Expr* right) {
    Unary* un = malloc(2*sizeof(Unary));
    un->expr = right;
    un->t = op;
    return un;
}

Literal* CreateLiteral(char* value) {
    Literal* lit = malloc(2*sizeof(Literal));
    //lit->Literal = value;
    lit->Literal=malloc(2*strlen(value));
    strncpy(lit->Literal, value, strlen(value));
    return lit;
}

Keywords* CreateKey(char* key, Expr* args) {
    Keywords* keyw = malloc(2*sizeof(Keywords));
    //keyw->keywords = key;
    keyw->keywords=malloc(2*strlen(key));
    keyw->args = args;
    strncpy(keyw->keywords, key, strlen(key));
    return keyw;
}

void Error() {
    printf("Error detected.\n");
    exit_code=-1;
    exit(-1);
}

Expr* bitOP();

Expr* primary() {
    Expr* expr=malloc(2*sizeof(Expr));
    TokenType t[] = {NUMBER, STRING,REGISTERS, STACK};
    TokenType t2[] = {KEYWORDS};

    if (matchAST(t,4)) {
        expr->Literal = CreateLiteral(prevAST()->Key);
        return expr;
    }
    if (matchAST(t2, 1)) {
        char* key = prevAST()->Key;
        Expr* tmp = bitOP();
        expr->key = CreateKey(key, tmp);
        return expr;
    }
    return expr;
}

Expr* unary() {
    TokenType t[1] = {MINUS};
    if (matchAST(t, 1)) {
        Token_t* op = prevAST();
        Expr* right = unary();
        Expr* tmp = malloc(2*sizeof(Expr));
        tmp->unary = CreateUnary(op->t, right);
        return tmp;
    }
    return primary();
}

Expr* factor() {
    Expr* expr=unary();
    TokenType t[] = {SLASH,STAR};
    while(matchAST(t, 2)) {
        Token_t* op = prevAST();
        Expr* right = unary();
        expr = CreateBinOp(expr, op->t, right);
    }
    return expr;
}

Expr* term() {
    Expr* expr=factor();
    TokenType t[] = {MINUS, PLUS};
    while(matchAST(t, 2)) {
        Token_t* op = prevAST();
        Expr* right = factor();
        expr = CreateBinOp(expr, op->t, right);
    }
    return expr;
}


Expr* comparison() {
    //Expr* ETerm = term();
    Expr* expr=term();
    TokenType t[] = {GREATER, GREATER_EQUAL, LESS, LESS_EQUAL};
    while(matchAST(t, 4)) {
        Token_t* op = prevAST();
        Expr* right = term();
        expr = CreateBinOp(expr,op->t,right);
    }
    return expr;
}

Expr* equality() {
    //Expr* ECompare = comparison();
    Expr* expr=comparison();
    TokenType t[1] = {EQUAL_EQUAL};
    while(matchAST(t,1)) {
        Token_t* op = prevAST();
        Expr* right = comparison();
        expr = CreateBinOp(expr, op->t, right);
    }
    return expr;
}

Expr* bitOP() {
    Expr* expr=equality();
    TokenType t[4] = {AND, OR, XOR, NOT};
    while(matchAST(t, 4)) {
        Token_t* op = prevAST();
        Expr* right = equality();
        expr = CreateBinOp(expr, op->t, right);
    }
    return expr;
}

Expr* movepointer() {
    Expr* expr = bitOP(); // we'll use register instead.
    TokenType t[] = {POINTER};
    while(matchAST(t,1)) {
        Expr* right  = bitOP();
        expr->moveinstr = CreateMoveInstr(
            GetRegisterEnum(expr->Literal->Literal),
             right);
    }
    return expr;
}

Expr* AppendParse(Expr* expr, Expr* output) {
    if (!expr) { expr = output; return expr; }
    Expr* ex=expr;
    while(ex->next) ex = ex->next;
    ex->next = output;
    return expr;
}
Expr* express() {
    return movepointer();
}

Expr* parseAST() {
    Expr* ex = NULL;
    while(!IsAtEndAST()) {
        if (peekAST()->t == _EOL_) {
            nextAST();
            continue;
        }
        ex = AppendParse(ex, express());
    }
    return ex;
}

void InitparseAST(Token_t* token) {
    tok = token;
}