#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <token.h>
#include <stdint.h>
#include <stdio.h>

static char* code;
const char* filename;
char* substr(char* str, uint32_t start, uint32_t end);

static const char* Keywords[] = {
    "loop", "word", "hlt", "cli",
    "arch", "place", "int", "curr", "begin",
    "byte"
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
    printf("%i | %s", line, code[strlen(code)-1] == '\n' ? code : strncat(code, "\n", 2));
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


Token_t* CreateToken(TokenType type, char* key) {
    Token_t* res = malloc(2*sizeof(Token_t));
    res->Key = malloc(2*strlen(key));
    strncpy(res->Key, key, strlen(key));
    res->t = type;
    res->next = NULL;
    res->prev = NULL;
    return res;
}

Token_t* AddToken(Token_t* t, TokenType type, char* key) {
    Token_t* tmp=CreateToken(type, key);
    if (!t) { t=tmp; return tmp; }
    Token_t* tmp2 = t;
    while(tmp2->next != NULL) tmp2 = tmp2->next;
    tmp2->next = tmp;
    tmp->prev = tmp2;
    return t;
}

void AddEndEOF(Token_t* token) {
    AddToken(token, _EOF_, "EOF");
}
void AddEOL(Token_t* token) {
    AddToken(token, _EOL_, "EOL");
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


uint8_t checkBase(char* value, uint8_t base) {
    if (base > 16) return 0;
    if (base <= 10) {
        for (int i=0;i<strlen(value);i++) {
            if (!(value[i] >= '0' &&
            value[i] < ('0' + base))) return 0; 
        }
    } else {
        for (int i=0;i<strlen(value);i++) {
            if (!((value[i] >='0' &&
            value[i] < ('0'+base)) ||
            (value[i] >= 'A' &&
            value[i] < ('A'+base-10)))) return 0;
        }
    }
    return 1;
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
    //Token_t* token = malloc(2*sizeof(Token_t));
    Token_t* token = NULL; // I don't know how this works =|
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
                token = AddToken(token, STRING, substr(code, start+1, curr-1));
                break;
            case '&':
                token = AddToken(token, AND, "AND");
                break;
            case '^':
                token = AddToken(token, XOR, "XOR");
                break;
            case '|':
                token = AddToken(token, OR, "OR");
                break;
            case '@':
                nextChar();
                token = AddToken(token, STACK, substr(code, start, curr));
                nextChar(); // =)
                break;
            case ':':
                switch(nextChar()) {
                    case 'h': 
                        while(!IsAtEnd() && isHex(peek())) nextChar();
                        token = AddToken(token, NUMBER,substr(code, start+1, curr));
                        break; //Hex
                    case 'b':
                        while(!IsAtEnd() && isBin(peek())) nextChar();
                        token = AddToken(token, NUMBER,substr(code, start+1, curr));
                        break; //Binary
                    case 'o': 
                        while(!IsAtEnd() && isOctal(peek())) nextChar();
                        token = AddToken(token, NUMBER,substr(code, start+1, curr));
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
                        if (i < 10) {
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
                    if (i >= 66) {
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