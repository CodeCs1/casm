#include <stdlib.h>
#include <string.h>
#include <token.h>
#include <stdint.h>
#include <stdio.h>

static char* code;
const char* filename;

int countNumber(int input) {
    int res = 0;
    while(input != 0) {
        input /= 10;
        res++;
    }
    return res;
}


void ErrorReport(uint32_t line, uint32_t where,
uint32_t err_no, char* err_mess) {
    printf("Error %i (%s:%i): %s\n", err_no, filename,where, err_mess);
    printf("%i | %s\n", line, code);
    for (int i = 0;i<countNumber(line)+3+(where-1);i++) printf(" ");
    printf("^\n");
    exit_code=-1;
    
}

int curr,start;

char nextChar() {
    char c = code[curr];
    curr++;
    return c;
}

int count=0;
Token_t* AddToken(Token_t* t, TokenType type, char* key) {
    Token_t* tmp=malloc(2*sizeof(Token_t));
    tmp->Key = malloc(strlen(key));
    strncpy(tmp->Key, key, strlen(key));
    tmp->t = type;
    tmp->next = NULL;
    if (t == NULL) {return tmp;}
    Token_t* tmp2 = t;
    while(tmp2->next != NULL) {
        tmp2=tmp2->next;
    }
    tmp2->next = tmp;
    return t;
}

void AddEndEOF(Token_t* token) {
    AddToken(token, _EOF_, " ");
}

Token_t* Scan() {
    curr=0;start=0;
    Token_t* token = malloc(2*sizeof(Token_t));
    while (curr < strlen(code)) {
        start = curr;
        char c = nextChar();
        switch(c) {
            case ')':
                token=AddToken(token, RIGHT_PAREN, "RP");
                break;
            case '(':
                token=AddToken(token, LEFT_PAREN, "LP");
                break;
            case '[':
                token=AddToken(token, LEFT_BLOCK, "LB1");
                break;
            case ']':
                token=AddToken(token, RIGHT_BLOCK, "RB1");
                break;
            case '{':
                token=AddToken(token, LEFT_BRACE, "LB2");
                break;
            case '}':
                token=AddToken(token, RIGHT_BRACE, "RB2");
                break;
            case '+':
                token = AddToken(token, PLUS, "PLS");
                break;
            case '-':
                token = AddToken(token, MINUS, "MIN");
                break;
            case '*':
                token = AddToken(token, STAR, "STAR");
                break;
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                break;
            default:
                ErrorReport(line, curr, 0, "Unexpect character.");
                break;
        }
    }

    return token;
}


void TokenInit(char *sources, char* file) {
    code = sources;
    filename = file;
}