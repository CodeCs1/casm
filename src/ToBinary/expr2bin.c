#include "parse.h"
#include "token.h"
#include <ToBinary/expr2bin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Expr* expr;
uint8_t* buffer;
Result* VisitExpr(Expr* ex);
int c=0;

int countNumber(int input);
uint8_t checkBase(char* value, uint8_t base);
uint64_t base2dec(char* str, int base);

void Write2Buffer(uint32_t data, int w) {
    int i=sizeof(uint16_t);
    if (w == 0) i--;
    for (; i>=0; i--) {
        uint8_t tmp = ((data>>i*8) & 0xff);
        memcpy(buffer+c, &tmp, 1);
        c++;
    }
}

void CWrite2Buffer(uint8_t data) {
    memcpy(buffer+c, &data, 1);
    c++;
}
uint8_t s_isdigit(char* s) {
    if (checkBase(s, 10) || checkBase(s, 16)||
    checkBase(s, 8) || checkBase(s, 2)) {
        return 1;
    }
    return 0;
}

uint8_t GetRegValue(enum Registers regs) {
    switch(regs) {
        case AX: case AL: return 0;
        case CX: case CL: return 1;
        case DX: case DL: return 2;
        case BX: case BL: return 3;
        case SP: case AH: return 4;
        case BP: case CH: return 5;
        case SI: case DH: return 6;
        case DI: case BH: return 7;
        default: return 0xff;
    }
}
uint8_t is16Bit(enum Registers regs) {
    switch (regs) {
        case AX: case EAX:
        case CX: case ECX:
        case DX: case EDX:
        case BX: case EBX:
        case SP: case ESP:
        case BP: case EBP:
        case SI: case ESI:
        case DI: case EDI: 
            return 1;
        case AL: case CL:
        case DL: case BL:
        case AH: case CH:
        case DH: case BH:
            return 0;
        default: return 0xff;
    }
}
Result* VisitMoveIntr(MoveInstr* intr) {
    Result* value = VisitExpr(intr->expr);
    if (value->number_defined) {
        uint8_t w = is16Bit(intr->regs);
        if (w == 0xff) { printf("Unknown regs"); exit(1); }
        uint8_t r = GetRegValue(intr->regs);
        if (r == 0xff) { printf("Unknown regs"); exit(1); }
        uint32_t imregs=(11<<4 | w<<3 | r);
        uint16_t val=value->numbers;
        if (w) {
            imregs <<= 16;
            if (val > 65535) {
                printf("16-bit regs out of bound.\n");
                exit(0);
            }
            imregs |= val < 256 ? val << 8 : val>>8;
        } else {
            imregs <<= 8;
            if (val > 255) {
                printf("8-bit regs out of bound.\n");
                exit(0);
            }
            imregs |= val;
        }
        Write2Buffer(imregs, w);
    }
    return NULL;
}
Result* VisitLiteral(Literal* lit) {
    Result* tmp = realloc(NULL,2*sizeof(Expr));
    //printf("%s\n", lit->Literal);
    if (s_isdigit(lit->Literal)) {
        //tmp->numbers = atof(lit->Literal);
        double d=0;
        if (checkBase(lit->Literal, 10)) {
            d = atof(lit->Literal);
        } else if (checkBase(lit->Literal, 2)) {
            d = (float)base2dec(lit->Literal, 2);
        } else if (checkBase(lit->Literal, 8)) {
            d = (float)base2dec(lit->Literal, 8);
        } else if (checkBase(lit->Literal, 16)) {
            d = (float)base2dec(lit->Literal, 16);
        }
        tmp->numbers=d;
        tmp->number_defined=1;
    } else {
        tmp->string = malloc(2*strlen(lit->Literal));
        strncpy(tmp->string, lit->Literal, strlen(lit->Literal));
        tmp->str_defined=1;
    }
    return tmp;
}

Result* VisitBinOP(BinOp* op) {
    Result* right = VisitExpr(op->right);
    Result* left = VisitExpr(op->left);
    Result* tmp = realloc(NULL,2*sizeof(Expr));

    switch(op->t) {
        case MINUS:
            tmp->number_defined=1;
            tmp->numbers = left->numbers-right->numbers;
            break;
        case SLASH:
            tmp->number_defined=1;
            if (right->numbers==0) {
                printf("ERROR: Division by 0\n");
                exit_code=1;
                exit(1);
            }
            tmp->numbers = left->numbers/right->numbers;
            break;
        case PLUS: // we only deal with number, not string nor register yet
            tmp->number_defined=1;
            tmp->numbers=left->numbers + right->numbers;
            break;
    }

    return tmp;
}

uint8_t checkNumber(Result* res) {
    if (res->number_defined==1) return 1;
    exit(1);
}

Result* VisitUnary(Unary* un) {
    Result* expr = VisitExpr(un->expr);
    Result* tmp=malloc(2*sizeof(Result));
    switch(un->t) {
        case MINUS:
            checkNumber(expr);
            tmp->numbers = -expr->numbers;
            return tmp;
        default: // BANG Doesn't exist.
            return expr;
    }
    return NULL;
}

Result* VisitKeyW(Keywords* key) {
    if (strcmp(key->keywords, "hlt")==0) {
        CWrite2Buffer(0xF4);
    } else if (strcmp(key->keywords, "cli")==0) {
        CWrite2Buffer(0xFA);
    }
    return NULL;
}

Result* tmp;
Result* VisitExpr(Expr* ex) {
    if (!ex) return tmp;
    if (ex->moveinstr) tmp=VisitMoveIntr(ex->moveinstr);
    if (ex->binop) tmp=VisitBinOP(ex->binop);
    if (ex->unary) tmp=VisitUnary(ex->unary);
    if (ex->Literal) tmp=VisitLiteral(ex->Literal);
    if (ex->key) tmp=VisitKeyW(ex->key);
    return VisitExpr(ex->next);
}

uint8_t* Convert2Bin() {
    VisitExpr(expr);
    return buffer;
}


void ConverterInit(Expr* exp) {
    expr = exp;
    buffer=malloc(1024);
}