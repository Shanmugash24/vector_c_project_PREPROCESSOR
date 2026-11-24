#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <stddef.h>
#include <stdio.h>

#define MAX_MACROS 256
#define MAX_NAME 128
#define MAX_VALUE 2048
#define MAX_INCLUDE_DEPTH 32

typedef struct {
    char name[MAX_NAME];
    char params[32][MAX_NAME];
    int param_count;
    int function_like;
    char value[MAX_VALUE];
} Macro;

typedef struct {
    Macro items[MAX_MACROS];
    int count;
    char include_dirs[16][512];
    int include_dir_count;
} Preprocessor;

void remove_comments(const char *input, char *output, int *in_block_comment);
int handle_define(Preprocessor *pp, const char *line);
void handle_undef(Preprocessor *pp, const char *line);
void expand_macros(Preprocessor *pp, const char *input, char *output);
int process_file(Preprocessor *pp, const char *input_path, FILE *out, int depth);

#endif
