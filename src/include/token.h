#pragma once

typedef enum {
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BLOCK,
    RIGHT_BLOCK,
    LEFT_BRACE,
    RIGHT_BRACE,
    LESS, LESS_EQUAL,
    GREATER, GREATER_EQUAL,
    EQUAL, EQUAL_EQUAL,
    POINTER /*<-*/, LAMBDA /*=>*/,
    PLUS, MINUS, STAR, SLASH,
    NUMBER, STRING, KEYWORDS,
    IDENTIFIER, REGISTERS, STACK,
    _EOF_
}TokenType;

static int exit_code;


struct Token_t {
    struct Token_t* next;
    struct Token_t* prev;
    TokenType t;
    char* Key;
};

typedef struct Token_t Token_t;

//Initialize Tokenizer
void TokenInit(char* sources, char* file);
// Scan sources
Token_t* Scan();
void AddEndEOF(Token_t* token);
void IncreaseLine();