#include "linkedlist.h"
#include "hw1_helpers.h"
#include "hw1.h"

// Part 1 Functions
int getSubstrings(char *str,  char delim, char ** array, int maxSize) {
    if (str == NULL || array == NULL || maxSize < 1) return -1;
    if (delim == '\0' || *str == '\0') return 0;
    
    int count = 0;
    char *p = str;

    while (count < maxSize && *p != '\0') {
        *array = p;
        array++;
        count++;

        while (*p != '\0' && *p != delim) {
            p++;
        }
        if (*p == delim) {
            *p = '\0';
            p++;
        }
    }

    return count;
}

void parseMIPSfields(const uint32_t instruction, MIPSfields* f) {
    f->opcode        = (instruction >> 26) & 0x3f;
    f->rs            = (instruction >> 21) & 0x1f;
    f->rt            = (instruction >> 16) & 0x1f;
    f->rd            = (instruction >> 11) & 0x1f;
    f->shamt         = (instruction >> 6) & 0x1f;
    f->func          = (instruction)      & 0x3f;
    f->immediate16   = instruction & 0xffff;
    f->immediate26   = (instruction) & 0x3ffffff;
    if (f->opcode == 0) f->uid = f->func;
    else f->uid = f->opcode << 26;
}

MIPSinstr* loadInstrFormat(char* line) {
     char *p = line;

    while (*p == ' ' || *p == '\t' || *p == '\n') p++;

    char type;
    if (!typeCheck(p, &type)) return NULL;

    while (*p != ' ' && *p != '\n' && *p != '\0') p++;
    while (*p == ' ' || *p == '\t' || *p == '\n') p++;

    uint32_t uid;
    if (!uidCheck(p, &uid)) return NULL;

    while (*p != ' ' && *p != '\n' && *p != '\0') p++;
    while (*p == ' ' || *p == '\t' || *p == '\n') p++;

    char *mnemonic;
    if (!mnemonicCheck(p, &mnemonic)) return NULL;

    while (*p != ' ' && *p != '\n' && *p != '\0') p++;
    while (*p == ' ' || *p == '\t' || *p == '\n') p++;

    int pretty;
    if (!prettyCheck(p, &pretty)) return NULL;

    MIPSinstr *instr = malloc(sizeof(MIPSinstr));
    if (instr == NULL) {
        return NULL;
    }

    instr->type = type;
    instr->uid = uid;
    instr->mnemonic = mnemonic;
    instr->pretty = pretty;
    instr->usagecnt = 0;

    return instr;
}

// Part 2 Functions
int MIPSinstr_uidComparator(const void* s1, const void* s2) {
    MIPSinstr *x = (MIPSinstr *)s1;
    MIPSinstr *y = (MIPSinstr *)s2;

    if (x->uid < y->uid) return -1;
    if (x->uid > y->uid) return 1;

    return 0;
}

void MIPSinstr_Printer(void* data, void* fp) {
    if (data == NULL || fp == NULL) return;

    MIPSinstr *instr = (MIPSinstr *)data;
    FILE *out = (FILE *)fp;

    fprintf(out,
            "%c\t%u\t%u\t%u\t%s\n",
            instr->type,
            instr->uid,
            instr->pretty,
            instr->usagecnt,
            instr->mnemonic);
}

void MIPSinstr_Deleter(void* data) {
    if (data == NULL) return;

    MIPSinstr *instr = (MIPSinstr *)data;

    if (instr->mnemonic != NULL) {
        free(instr->mnemonic);
    }

    free(instr);
}

node_t* FindInList(list_t* list, void* token) {

    return (node_t*) 0xDEADBEEF;
}

void DestroyList(list_t** list)  {

}


// Part 3 Functions
list_t* createMIPSinstrList(FILE* IMAPFILE) {

    return (list_t*) 0xDEADBEEF;
}

int printInstr(MIPSfields* instr, list_t* MIPSinstrList, char** regNames, FILE* OUTFILE) {

    return 0xDEADBEEF;
}


// Extra Credit Functions
void MIPSinstr_removeZeros(list_t* list) {

}

int MIPSinstr_usagecntComparator(const void* s1, const void* s2) {

    return 0xDEADBEEF;
}

void MIPSinstr_statPrinter(void* data, void* fp) {

}

void sortLinkedList(list_t* list) {

}

