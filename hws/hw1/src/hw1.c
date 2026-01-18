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
    return 0;
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
    if (line == NULL) return NULL;

    // 1) copy so getSubstrings can safely write '\0'
    char *buf = copyLineHeap(line);
    if (!buf) return NULL;

    char *parts[4];
    int n = getSubstrings(buf, ' ', parts, 4);

    if (n != 4) { free(buf); return NULL; }

    char *type_s = *(parts + 0);
    char *uid_s  = *(parts + 1);
    char *mnem_s = *(parts + 2);
    char *pret_s = *(parts + 3);

    if (!type_s || *type_s == '\0' || *(type_s + 1) != '\0') { free(buf); return NULL; }
    char type = *type_s;
    if (!(type == 'r' || type == 'i' || type == 'j')) { free(buf); return NULL; }

    if (!isHex(uid_s)) { free(buf); return NULL; }
    char *end = NULL;
    unsigned long uid_ul = strtoul(uid_s, &end, 16);
    if (end == uid_s || *end != '\0' || uid_ul > 0xFFFFFFFFUL) { free(buf); return NULL; }

    if (!isLower(mnem_s)) { free(buf); return NULL; }

    end = NULL;
    long pretty_l = strtol(pret_s, &end, 10);
    if (end == pret_s || *end != '\0' || pretty_l < 0 || pretty_l > 10) { free(buf); return NULL; }

    MIPSinstr *instr = malloc(sizeof *instr);
    if (!instr) { free(buf); return NULL; }

    const char *p = mnem_s;
    size_t mlen = 0;
    while (*p++) mlen++;

    instr->mnemonic = malloc(mlen + 1);
    if (!instr->mnemonic) { free(instr); free(buf); return NULL; }

    const char *ms = mnem_s;
    char *md = instr->mnemonic;
    while ((*md++ = *ms++) != '\0') { }

    instr->type = type;
    instr->uid = (uint32_t)uid_ul;
    instr->pretty = (uint8_t)pretty_l;
    instr->usagecnt = 0; // always 0 as you said

    free(buf);
    return instr;
}

// Part 2 Functions
int MIPSinstr_uidComparator(const void* s1, const void* s2) {

    return 0xDEADBEEF;
}

void MIPSinstr_Printer(void* data, void* fp) {

}

void MIPSinstr_Deleter(void* data) {

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

