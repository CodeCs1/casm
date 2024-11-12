#include "parse.h"
#include <ToBinary/expr2bin.h>
#include <ctype.h>
#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Expr* expr;
uint8_t* buffer;
Expr* VisitExpr(Expr* ex);
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

Expr* VisitMoveIntr(MoveInstr* intr) {
    Expr* value = VisitExpr(intr->expr);
    if (value->Literal) {
        uint8_t t = s_isdigit(value->Literal->Literal);
        if (t) { // Immediate to Register
            uint8_t w=is16Bit(intr->regs);
            if (w == 0xff) { printf("Unknown regs"); exit(1); }
            //imregs |= w<<3;
            uint8_t r = GetRegValue(intr->regs);
            if (r == 0xff) { printf("Unknown regs"); exit(1); }
            uint32_t imregs=(11<<4 | w<<3 | r);
            uint16_t val=0;
            if (checkBase(value->Literal->Literal, 10)) {
                val = atoi(value->Literal->Literal);
            } else if (checkBase(value->Literal->Literal, 2)) {
                val = (uint16_t)base2dec(value->Literal->Literal, 2);
            } else if (checkBase(value->Literal->Literal, 8)) {
                val = (uint16_t)base2dec(value->Literal->Literal, 8);
            } else if (checkBase(value->Literal->Literal, 16)) {
                val = (uint16_t)base2dec(value->Literal->Literal, 16);
            }
            if (w) {
                imregs <<= 16;
                if (val > 65535) {
                    printf("16-bit regs out of bound.\n");
                    exit(0);
                }
                imregs |= val < 256 ? val << 8 : val>>8;
            }
            else { 
                imregs <<= 8;
                if (val > 255) {
                    printf("8-bit regs out of bound.\n");
                    exit(0);
                }
                imregs |= val;
            }
            Write2Buffer(imregs,w);
        } else {
            // Segment Register to Register/Memory: 10001100 | mod 0 reg r/m
        }
    }
    return (Expr*)intr;
}
Expr* VisitLiteral(Literal* lit) {
    Expr* tmp = realloc(NULL,2*sizeof(Expr));
    tmp->Literal=lit;
    return tmp;
}

Expr* VisitBinOP(BinOp* op) {
    Expr* ex = VisitExpr(op->right);
}

Expr* VisitUnary(Unary* un) {

}

Expr* VisitKeyW(Keywords* key) {
    if (strcmp(key->keywords, "hlt")==0) {
        CWrite2Buffer(0xF4);
    } else if (strcmp(key->keywords, "cli")==0) {
        CWrite2Buffer(0xFA);
    }
    return NULL;
}

Expr* tmp;
Expr* VisitExpr(Expr* ex) {
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