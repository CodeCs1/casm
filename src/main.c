#include <stddef.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <argp.h>

//user define
#include <token.h>

const char* argp_program_version="1.0-beta";

struct args_t {
    char* argv[4];
    uint8_t mode;
    uint8_t verbose;
    uint8_t arch;
    char* inf;
    char* outf;
};

static struct argp_option options[] = {
    {"input", 'i', "FILE", 0, "Input file"},
    {"exetype", 'e', "type", 0, "Executable type"},
    {"output", 'o', "FILE", 0, "Output file"},
    {"Verbose", 'v', 0, 0, "Verbose the output"},
    {"arch", 'a', "ARCH", 0, "Set the architecture for the program"},
    {0}
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    struct args_t* t = state->input;
    switch(key) {
        case 'e':
            if (atoi(arg) == 1 || strcmp(arg, "windows")==0)
                t->mode = 1;
            else if (atoi(arg) == 2 || strcmp(arg, "linux")==0)
                t->mode = 2;
            else
                { printf("The exe type option must be 'windows' (1) or 'linux' (2)\n"); exit(1);}
            break;
        case 'o':
            t->outf = arg;
            break;
        case 'i':
            t->inf = arg;
            break;
        case 'v':
            t->verbose = 1;
            break;
        case 'a':
            t->arch=1; // default is 16 bit
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 5) {
                printf("Too many arguments.\n");
                argp_usage(state);
            }
            t->argv[state->arg_num] = arg;
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static char doc[] =
  "CASM - An assembly compiler.";

static char args_doc[] = "INPUT OUTPUT";

static struct argp argp = {options, parse_opt, args_doc ,doc};

double time_spent;

void exit_func() {
    printf("Program exit with status: %i\n", exit_code);
    printf("Program was run in %f secounds\n", time_spent);
}


int countAppend=0;

void AppendToken_Pointer2Pointer(Token_t* input, Token_t* output, size_t sz) {
    for (int i=0;i<sz;i++) {
        output[countAppend] = input[i];
        countAppend++;
    }
}   

int main(int argc, char** argv) {
    //setting up command args
    atexit(exit_func);
    clock_t begin = clock();
    struct args_t t;
    t.mode = 0;
    t.outf = "-";
    t.inf = malloc(2*sizeof(uint64_t));
    argp_parse(&argp, argc, argv, 0, 0, &t);

    if (strcmp(t.inf, "\0")==0) {
        printf("Input file is empty.\n");
        exit_code=1;
        return 1;
    }

    FILE* f=fopen(t.inf, "r+");
    if (f == NULL) {
        printf("File %s can't be opened or not exist!\n", t.inf);
        return -1;
    }

    fseek(f,0,SEEK_END);
    size_t sz = ftell(f);
    fseek(f,0,SEEK_SET);
    char* s = malloc(sz);
    char signature[6];
    fgets(signature, 6, f);
    if (strcmp(signature, "$casm") != 0) {
        printf("CASM Signature should be on top the very first line\n");
        exit_code=-1;
        return -1;
    }
    fseek(f,1,SEEK_CUR);
    Token_t* tok= malloc(sz);
    while(fgets(s, sz, f)) {
        TokenInit(s, t.inf);
        tok = Scan();
        tok = tok->next;
        line++;
    }
    while(tok != NULL) {
        printf("Token Type -> %i\n", tok->t);
        tok = tok->next;
    }

    clock_t end = clock();
    time_spent = (double)(end-begin)/CLOCKS_PER_SEC;

    free(tok);

    return 0;
}