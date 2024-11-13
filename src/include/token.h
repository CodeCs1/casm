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
    AND,OR,XOR,NOT, /*Bit-wise operation*/
    IDENTIFIER, REGISTERS, STACK,
    _EOF_, _EOL_
}TokenType;

static int exit_code;


struct Token_t {
    struct Token_t* next;
    struct Token_t* prev;
    TokenType t;
    char* Key;
};

typedef struct Token_t Token_t;

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

//Initialize Tokenizer
void TokenInit(char* sources, char* file);
// Scan sources
Token_t* Scan();
void AddEndEOF(Token_t* token);
void IncreaseLine();