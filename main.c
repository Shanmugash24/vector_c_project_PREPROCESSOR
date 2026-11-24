#include "preprocessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *program)
{
    fprintf(stderr, "Usage: %s <input.c>\n", program);
}

int main(int argc, char *argv[])
{
    Preprocessor pp = {0};
    char output_name[1024];
    size_t len;
    FILE *out;

    if (argc != 2) { usage(argv[0]); return EXIT_FAILURE; }
    len = strlen(argv[1]);
    if (len >= sizeof(output_name) - 3) {
        fprintf(stderr, "Input filename is too long.\n");
        return EXIT_FAILURE;
    }
    strcpy(output_name, argv[1]);
    char *dot = strrchr(output_name, '.');
    char *slash = strrchr(output_name, '/');
    if (dot && (!slash || dot > slash)) strcpy(dot, ".i");
    else strcat(output_name, ".i");

    out = fopen(output_name, "w");
    if (!out) { perror(output_name); return EXIT_FAILURE; }

    if (!process_file(&pp, argv[1], out, 0)) {
        fclose(out);
        remove(output_name);
        return EXIT_FAILURE;
    }
    fclose(out);
    printf("Preprocessing completed successfully.\nOutput file: %s\n", output_name);
    return EXIT_SUCCESS;
}
