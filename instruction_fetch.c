#include <stdbool.h>
#define _GNU_SOURCE   // for getline
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_IMEM 65536

// Instruction memory: array of text lines (assembly instructions)
typedef struct {
    char **lines;    // dynamically allocated array of char*
    size_t size;     // number of loaded instructions
} InstMem;

// IF state with PC (instruction-indexed)
typedef struct {
    int pc;          // instruction index (0 .. size-1)
} IFState;

typedef struct {
    char *instr_text;   // strdup'd string (NULL if invalid)
    int pc;             // PC of this instruction
    bool valid;         // false = empty/invalid, true = valid
} IFIDReg;


// Initialize IF/ID register
void init_ifid(IFIDReg *r);


// Free IF/ID register contents
void free_ifid(IFIDReg *r);


/* 
    Set PC (used by branch resolution logic later)
    pc_value is an instruction index (not byte address)
*/
void set_pc(IFState *s, uint32_t pc_value);


/* 
    Load instruction memory from text file. Lines starting with '#' or empty lines are ignored.
    Returns 0 on success, -1 on failure.
*/
int load_imem(const char *filename, InstMem *im);

/* Free InstMem */
void free_imem(InstMem *im);


void fetch(IFState *s, InstMem *im, IFIDReg *ifid);


int main(int argc, char** argv) {
    const char *filename = "p.instr";
    if (argc > 1) filename = argv[1];

    InstMem im = { .lines = NULL, .size = 0 };
    IFState ifstate = { .pc = 0 };
    IFIDReg ifid;
    init_ifid(&ifid);

    if (load_imem(filename, &im) != 0) {
        fprintf(stderr, "Failed to load instruction memory from %s\n", filename);
        return 1;
    }
    printf("Loaded %zu instructions from %s\n", im.size, filename);
    
    // We'll simulate cycles until fetch produces no instruction for 3 consecutive cycles
    int idle_cycles = 0;
    unsigned long cycle = 0;

    while (idle_cycles < 3) {
        printf("\n=== Cycle %lu ===\n", cycle);

        // IF stage: fetch the next instruction into IF/ID
        fetch(&ifstate, &im, &ifid);

        // Print IF/ID state (what ID would see next cycle)
        if (ifid.valid) {
            printf("IF/ID.valid = 1, PC = %u, instr = \"%s\"\n", ifid.pc, ifid.instr_text);
            idle_cycles = 0;
        } else {
            printf("IF/ID.valid = 0 (bubble)\n");
            idle_cycles++;
        }

        // (In a real pipeline the ID stage would run here next cycle and might call set_pc()
        //  to change ifstate.pc for branches/jumps; we just demonstrate fetch + PC.)
        // Example (comment): how an ID stage could change PC:
        // if (/* branch taken */) set_pc(&ifstate, target_index);

        cycle++;
    }

    printf("\nFetch finished. Final PC = %u after %lu cycles\n", ifstate.pc, cycle);

    free_ifid(&ifid);
    free_imem(&im);


    return 0;
}

void init_ifid(IFIDReg* r){
    r->instr_text = NULL;
    r->pc = 0;
    r->valid = 0;
}


void free_ifid(IFIDReg *r) {
    if (r->instr_text) free(r->instr_text);
    r->instr_text = NULL;
    r->valid = 0;
}

void set_pc(IFState *s, uint32_t pc_value) {
    s->pc = pc_value;
}

int load_imem(const char *filename, InstMem *im) {
    FILE* f = fopen(filename, "r");
    if(!f){
        perror("fopen");
        return -1;
    }

    im->lines = calloc(MAX_IMEM, sizeof(char*));
    if (!im->lines) {
        perror("calloc");
        fclose(f);
        return -1;
    }
    im->size = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len /* Evil */, f)) != -1) {
        // Trim leading whitespace
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;

        // Skip empty lines and comments
        if (*s == '\n' || *s == '\0' || *s == '#') continue;

        // Remove trailing newline
        char *nl = strchr(s, '\n');
        if (nl) *nl = '\0';

        // strdup the cleaned line and store
        im->lines[im->size] = strdup(s);
        if (!im->lines[im->size]) {
            perror("strdup");
            free(line);
            fclose(f);
            return -1;
        }

        im->size++;
        if (im->size >= MAX_IMEM) {
            fprintf(stderr, "Warning: hit MAX_IMEM (%d), stopping load\n", MAX_IMEM);
            break;
        }
    }
    
    free(line);
    fclose(f);
    return 0;
}

/* Free InstMem */
void free_imem(InstMem *im) {
    if (!im || !im->lines) return;
    for (size_t i = 0; i < im->size; ++i) {
        free(im->lines[i]);
    }
    free(im->lines);
    im->lines = NULL;
    im->size = 0;
}

void fetch(IFState *s, InstMem *im, IFIDReg *ifid) {
    // clear previous ifid content
    if (ifid->instr_text) {
        free(ifid->instr_text);
        ifid->instr_text = NULL;
    }
    ifid->valid = 0;

    if (s->pc >= im->size) {
        // No instruction to fetch: pipeline receives bubble (invalid)
        return;
    }

    // duplicate the instruction text into IF/ID
    ifid->instr_text = strdup(im->lines[s->pc]);
    ifid->pc = s->pc;
    ifid->valid = 1;

    // advance PC to next instruction (word/instruction indexed)
    s->pc += 1;
}