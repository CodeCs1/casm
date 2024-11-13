#pragma once

#include "parse.h"
#include <stdint.h>


struct Result {
    double numbers;
    uint8_t number_defined;
    char* string;
    uint8_t str_defined;
};

typedef struct Result Result;

void ConverterInit(Expr* exp);
uint8_t* Convert2Bin();