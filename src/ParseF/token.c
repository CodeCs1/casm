#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <token.h>
#include <stdint.h>
#include <stdio.h>

static char* code;
const char* filename;



static const char* Keywords[] = {
    "loop", "dw", "hlt", "cli",
    "arch", "place"
};
static const char* Registers[] = {
    // upper case
    "AL","CL","DL","BL",
    "AH","CH","DH","BH",
    "AX","CX","DX", "BX", 
    "SP","BP","SI","DI",
    "EAX", "ECX", "EDX", 
    "EBX", "ESP", "EBP","ESI", 
    "EDI", "ES", "CS", "SS", "DS",
    //lower case
    "al","cl","dl","bl",
    "ah","ch","dh","bh",
    "ax","cx","dx", "bx", 
    "sp","bp","si","di",
    "eax", "ecx", "edx", 
    "ebx", "esp", "ebp","esi", 
    "edi", "es", "cs", "ss", "ds"
};


int line=2;
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
    printf("Error %i (%s:%i:%i): %s\n", err_no, filename,line,where, err_mess);
    printf("%i | %s\n", line, code);
    for (int i = 0;i<countNumber(line)+3+(where-1);i++) printf(" ");
    printf("^\n");
    exit_code=-1;
    exit(-1);
}

int curr,start;

char nextChar() {
    char c = code[curr];
    curr++;
    return c;
}

uint8_t IsAtEnd() {
    return curr == strlen(code);
}

uint8_t match(char exp) {
    if (IsAtEnd()) return 0;
    if (code[curr] != exp) return 0;
    curr++;
    return 1;
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
    AddToken(token, _EOF_, "EOF");
}

char peek() {
    if (IsAtEnd())return '\0';
    return code[curr];
}
char peekNext() {
    if (curr+1 >= strlen(code)) return '\0';
    return code[curr+1];
}

uint8_t isHex(char exp) {
    if (exp >= '0' && exp <='9' || exp >= 'A' && exp <= 'F' || exp >= 'a' && exp <= 'f') return 1;
    return 0;
}
uint8_t isBin(char exp) {
    return exp == '0' || exp == '1';
}
uint8_t isOctal(char exp) {
    return exp >='0' && exp <= '7';
}

uint64_t getHex(char exp) {
    switch(exp) {
        case 'a': case 'A': return 10;
        case 'b': case 'B': return 11;
        case 'c': case 'C': return 12;
        case 'd': case 'D': return 13;
        case 'e': case 'E': return 14;
        case 'f': case 'F': return 15;
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
    }
    return 0;
}

uint64_t base2dec(char* str, int base) {
    uint64_t digit = 1;
    uint64_t output=0;
    for (int i=strlen(str)-1; i >= 0; i--) {
        output += getHex(str[i])*digit;
        digit *=base;
    }
    return output;
}

char* substr(char* str, uint32_t start, uint32_t end) {
    char* t1 = &str[start];
    char* t2 = &str[end];
    char* substr = malloc(t2-t1+1);
    memcpy(substr, t1, t2-t1);
    return substr;
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
            case '=':
                if (match('=')) {
                    token = AddToken(token, EQUAL_EQUAL ,"EQ2");
                }
                else if (match('>')) {
                    token = AddToken(token, LAMBDA ,"LMDA");
                } else {
                    token = AddToken(token, EQUAL ,"EQ");
                }
                break;
            case '/':
                if (match('/')) {
                    while(peek() != '\n' && !IsAtEnd()) nextChar();
                } else {
                    token=AddToken(token, SLASH, "SL");
                }
                break;
            case '<':
                if (match('-')) {
                    token = AddToken(token, POINTER, "PTR");
                } 
                else if (match('=')) {
                    token = AddToken(token, LESS_EQUAL, "LEQ");
                } 
                else {
                    token = AddToken(token, LESS, "LS");
                }
                break;
            case '>':
                if (match('=')) {
                    token = AddToken(token, GREATER_EQUAL, "GEQ");
                } else {
                    token = AddToken(token, GREATER, "GR");
                }
                break;
            case '"':
                while(peek() != '"' && !IsAtEnd()) {
                    if (peek() == '\n') line++;
                    nextChar();
                }
                if (IsAtEnd()) {
                    ErrorReport(line, start+1, 69, "Unterminated Strings.");
                }
                nextChar();
                printf("STRING: %s\n", substr(code, start+1, curr-1));
                token = AddToken(token, STRING, substr(code, start+1, curr-1));
                break;
            case '&':
                nextChar();
                token = AddToken(token, STACK, substr(code, start, curr));
                nextChar(); // =)
                break;
            case ':':
                switch(nextChar()) {
                    case 'h': 
                        while(!IsAtEnd() && isHex(peek())) nextChar();
                        token = AddToken(token, NUMBER,substr(code, start+2, curr));
                        break; //Hex
                    case 'b':
                        while(!IsAtEnd() && isBin(peek())) nextChar();
                        token = AddToken(token, NUMBER,substr(code, start+2, curr));
                        break; //Binary
                    case 'o': 
                        while(!IsAtEnd() && isOctal(peek())) nextChar();
                        token = AddToken(token, NUMBER,substr(code, start+2, curr));
                        break; //octal
                }
                break;
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                break;
            default:
                if (isdigit(c)) {
                    while(isdigit(peek())) nextChar();
                    if (peek() == '.' && isdigit(peekNext())) {
                        nextChar();
                        while(isdigit(peek())) nextChar();
                    }
                    token=AddToken(token, NUMBER,  substr(code, start, curr));
                } else if (isalpha(c)) {
                    while(isalnum(peek())) nextChar();
                    char* t = substr(code,start,curr);
                    int i=0;
                    for (i=0;i<56;i++) {
                        if (i < 6) {
                            if (strcmp(t,Keywords[i])==0) {
                                token = AddToken(token, KEYWORDS, t);
                                break;
                            }
                        }
                        if (strcmp(t, Registers[i])==0) {
                            token = AddToken(token, REGISTERS, t);
                            break;
                        }
                    }
                    if (i >= 62) {
                        token = AddToken(token, IDENTIFIER, t);
                    }
                    
                } else {
                    ErrorReport(line, curr, 0, "Unexpect character.");
                }
                break;
        }
    }

    return token;
}

void IncreaseLine() {
    line++;
}

void TokenInit(char *sources, char* file) {
    code = sources;
    filename = file;
}