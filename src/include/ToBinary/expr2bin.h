#pragma once

#include "parse.h"
#include <stdint.h>
#include <stddef.h>

struct Result {
    double numbers;
    uint8_t number_defined;
    char* string;
    uint8_t str_defined;
    struct Result* next;
};

struct Buffer {
    size_t size;
    uint8_t* buffer;
};

typedef struct Result Result;
typedef struct Buffer Buffer;

void ConverterInit(Expr* exp);
Buffer* Convert2Bin();