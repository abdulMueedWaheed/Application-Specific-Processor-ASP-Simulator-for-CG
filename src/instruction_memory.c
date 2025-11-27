#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/cpu.h"

int load_imem(const char *filename, InstMem *im)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }

    im->lines = calloc(MAX_IMEM, sizeof(char*));
    im->size = 0;

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, f) != -1) {

        // trim whitespace
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;

        // skip empty or comment lines
        if (*s == '\0' || *s == '\n' || *s == '#') continue;

        // remove newline
        char *nl = strchr(s, '\n');
        if (nl) *nl = '\0';

        im->lines[im->size] = strdup(s);
        im->size++;
    }

    free(line);
    fclose(f);
    return 0;
}

void free_imem(InstMem *im)
{
    for (size_t i = 0; i < im->size; ++i)
        free(im->lines[i]);
    free(im->lines);
}
