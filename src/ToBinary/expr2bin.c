#include "parse.h"
#include "token.h"
#include <ToBinary/expr2bin.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Expr* expr;
Buffer* buffer;
Result* VisitExpr(Expr* ex);
int c=0;
int startExpr=0;

uint32_t place=0;

int countNumber(int input);
uint8_t checkBase(char* value, uint8_t base);
uint64_t base2dec(char* str, int base);

void Write2Buffer(uint32_t data, int w) {
    int i=sizeof(uint16_t);
    if (w == 0) i--;
    for (; i>=0; i--) {
        uint8_t tmp = ((data>>i*8) & 0xff);
        memcpy(buffer->buffer+c, &tmp, 1);
        c++;
        buffer->size++;
    }
}

void CWrite2Buffer(uint8_t data) {
    memcpy(buffer->buffer+c, &data, 1);
    c++;
    buffer->size++;
}

uint8_t isFloat(char* c) {
    uint8_t res=0;
    for (int i=0;i<strlen(c);i++) {
        if (c[i] == '.') res=1; 
        else if (isdigit(c[i])) res=2;
    }
    return res;
}
char* substr(char* str, uint32_t start, uint32_t end);
uint8_t s_isdigit(char* s) {
    /*if (checkBase(s, 10) || checkBase(s, 16)||
    checkBase(s, 8) || checkBase(s, 2)) {
        return 1;
    }*/
    if (s[0] == 'h' || s[0] == 'b' || s[0] == 'o' || 
    checkBase(s, 10)) {
        return 1;
    }
    return 0;
}

uint8_t GetRegValue(enum Registers regs) {
    switch(regs) {
        case AX: case AL: case ES: return 0;
        case CX: case CL: case CS: return 1;
        case DX: case DL: case SS: return 2;
        case BX: case BL: case DS: return 3;
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

uint8_t isSegment(enum Registers regs) {
    switch(regs) {
        case SS: case ES: case DS: case CS: return 1;
        default: return 0; 
    }
}
enum Registers GetRegisterEnum(char* regs);

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
            imregs |= val < 256 ? val << 8 : (val & 0xff)<<8 | val>>8;
        } else {
            imregs <<= 8;
            if (val > 255) {
                printf("8-bit regs out of bound.\n");
                exit(0);
            }
            imregs |= val;
        }
        Write2Buffer(imregs, w);
    } else {
        if (isSegment(intr->regs)) {// Reg to Seg (segment can only go with 16, not 8)
            //10001100 | mod (2) 0 reg r/m (3)
            // We'll use mod= 11(2) for easy implementation
            uint8_t mod=3, rm=0;

            switch(GetRegisterEnum(value->string)) {
                case AL: case AX: rm=0; break;
                case CL: case CX: rm=1; break;
                case DL: case DX: rm=2; break;
                case BL: case BX: rm=3; break;
                case AH: case SP: rm=4; break;
                case CH: case BP: rm=5; break;
                case DH: case SI: rm=6; break;
                case BH: case DI: rm=7; break;
                default: break;
            }

            uint32_t imregs = (0x8E << 8) | mod << 6 | GetRegValue(intr->regs) << 3 | rm;
            Write2Buffer(imregs, 0);
        } else { //Seg 2 Reg | Reg 2 Reg

        }
    }
    return NULL;
}


Result* VisitLiteral(Literal* lit) {
    Result* tmp = realloc(NULL,2*sizeof(Expr));
    if (s_isdigit(lit->Literal)) {
        double d=0;
        char* sub=substr(lit->Literal, 
        1, strlen(lit->Literal));
        if (checkBase(lit->Literal, 10)) {
            d = atof(lit->Literal);
        } else if (lit->Literal[0] == 'b') {
            d = (float)base2dec(sub, 2);
        } else if (lit->Literal[0] == 'o') {
            d = (float)base2dec(sub, 8);
        } else if (lit->Literal[0] == 'h') {
            d = (float)base2dec(sub, 16);
        }
        tmp->numbers=d;
        tmp->number_defined=1;
    } else {
        if (isFloat(lit->Literal)==1) {
            char* sub=substr(lit->Literal, 
                    1, strlen(lit->Literal));
            double d = atof(sub);
            tmp->number_defined = 1;
            tmp->numbers=d;
        } else {
            tmp->string = malloc(2*strlen(lit->Literal));
            strncpy(tmp->string, lit->Literal, strlen(lit->Literal));
            tmp->str_defined=1;
        }
    }
    return tmp;
}

Result* VisitBinOP(BinOp* op) {
    Result* right = VisitExpr(op->right);
    Result* left = VisitExpr(op->left);
    Result* tmp = realloc(NULL,2*sizeof(Result));

    switch(op->t) {
        case MINUS:
            if (right->number_defined && left->number_defined) {
                tmp->number_defined=1;
                tmp->numbers = left->numbers-right->numbers;
                printf("%f - %f\n", left->numbers, right->numbers);
            }
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
        case PLUS:
            if (left->number_defined && right->number_defined) {
                tmp->number_defined=1;
                tmp->numbers=left->numbers + right->numbers;
                printf("T: %X + %X\n", (int)left->numbers, (int)right->numbers);
            }
            break;
        default:
            break;
    }

    return tmp;
}

uint8_t checkNumber(Result* res) {
    if (res) if (res->number_defined==1) return 1;
    exit(1);
}

Result* VisitUnary(Unary* un) {
    Result* expr = VisitExpr(un->expr);
    Result* tmp=malloc(2*sizeof(Result));
    switch(un->t) {
        case MINUS:
            checkNumber(expr);
            tmp->number_defined=1;
            tmp->numbers = -expr->numbers;
            return tmp;
        default: // BANG Doesn't exist.
            return expr;
    }
    return NULL;
}

Result* VisitKeyW(Keywords* key) {
    Result* args = VisitExpr(key->args);
    Result* res=NULL;
    if (strcmp(key->keywords, "hlt")==0) {
        CWrite2Buffer(0xF4);
    } else if (strcmp(key->keywords, "cli")==0) {
        CWrite2Buffer(0xFA);
    } else if (strcmp(key->keywords, "int")==0) {
        uint32_t intr=0xCD<<8;
        intr |= (uint16_t)args->numbers;
        Write2Buffer(intr, 0);
    } else if (strcmp(key->keywords, "loop")==0) {
        printf("%f\n", args->numbers);
        /*for (int i=0;i<(int)args->numbers;i++) {
            CWrite2Buffer(0);
        }*/
    } else if (strcmp(key->keywords, "place")==0) {
        if (!args) exit(1);
        if (!args->number_defined) {printf("should be number defined\n"); exit(0);}
        place = (uint32_t)args->numbers;
    } else if (strcmp(key->keywords, "begin")==0) {
        res = calloc(1,sizeof(Result));
        res->number_defined=1; res->numbers=place+startExpr;
    } else if (strcmp(key->keywords, "curr")==0) {
        res = calloc(1,sizeof(Result));
        res->number_defined=1; res->numbers=place+buffer->size;
    } else if (strcmp(key->keywords, "word")==0) {
        Write2Buffer(args->numbers, 0);
    } else if (strcmp(key->keywords, "byte")==0) {
        CWrite2Buffer(args->numbers);
    }
    return res;
}

Result* VisitGroup(Group* gr) {
    Result* r = VisitExpr(gr->expr);
    return r;
}


Result* AppendResult(Result* r, Result* r1) {
    if (!r) return r1;
    Result* rtmp = r;
    while(rtmp->next) rtmp = rtmp->next;
    rtmp->next = r1;
    return r;
}

Result* tmp;
Result* VisitExpr(Expr* ex) {
    if (!ex) return tmp;
    if (ex->moveinstr) tmp=VisitMoveIntr(ex->moveinstr);
    if (ex->binop) tmp=VisitBinOP(ex->binop);
    if (ex->unary) tmp=VisitUnary(ex->unary);
    if (ex->Literal) tmp=VisitLiteral(ex->Literal);
    if (ex->key) tmp=VisitKeyW(ex->key);
    if (ex->Group) tmp=VisitGroup(ex->Group);
    return VisitExpr(ex->next);
}

Buffer* Convert2Bin() {
    VisitExpr(expr);
    return buffer;
}


void ConverterInit(Expr* exp) {
    expr = exp;
    buffer=malloc(2*sizeof(Buffer));
    buffer->buffer=malloc(1024);
}