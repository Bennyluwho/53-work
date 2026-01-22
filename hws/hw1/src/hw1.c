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
    if(line == NULL) return NULL;

    char* toSubstring = malloc(myStrlen(line, '\n')*sizeof(char));
    char* toSubstringSeek = toSubstring;
    while (*line != '\0' && *line != '\n') {
    *toSubstringSeek++ = *line++;
    }
    *toSubstringSeek = '\0';


    int size = 4;
    char **substrings = malloc(4*sizeof(char *));
    if (!substrings) { 
        free(toSubstring); 
        return NULL; 
    }

    if (getSubstrings(toSubstring, ' ', substrings, 4) != 4) {
        free(substrings);
        free(toSubstring);
        return NULL;
    }

    char type;
    if (!typeCheck(*substrings, &type)) {
        free(substrings);
        free(toSubstring);
        return NULL;
    }

    uint32_t uid;
    if (!uidCheck(*(substrings + 1), &uid)) {
        free(substrings);
        free(toSubstring);
        return NULL;
    }

    char *mnemonicStart;
    if (!mnemonicCheck(*(substrings + 2), &mnemonicStart)) {
        free(substrings);
        free(toSubstring);
        return NULL;
    }

    int pretty;
    if (!prettyCheck(*(substrings + 3), &pretty)) {
        free(substrings);
        free(toSubstring);
        return NULL;
    }

    MIPSinstr *instr = malloc(sizeof(MIPSinstr));
    if (!instr) {
        free(substrings);
        free(toSubstring);
        return NULL;
    }

    int mlen = myStrlen(mnemonicStart, '\0');
    instr->mnemonic = malloc(mlen + 1);
    if (!instr->mnemonic) {
        free(instr);
        free(substrings);
        free(toSubstring);
        return NULL;
    }

    char *md = instr->mnemonic;
    char *ms = mnemonicStart;
    while ((*md++ = *ms++)) { }

    instr->type = type;
    instr->uid = uid;
    instr->pretty = pretty;
    instr->usagecnt = 0;

    free(substrings);
    free(toSubstring);
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
    if (list == NULL || token == NULL) return NULL;

    node_t *p = list->head;
    while(p != NULL) {
        if(MIPSinstr_uidComparator(p->data, token) == 0) return p;
        p = p->next;
    }

    return NULL;
}

void DestroyList(list_t** list)  {
    if (list == NULL || *list == NULL) return;

    node_t *p = (*list)->head;
    while (p != NULL) {
        node_t *next = p->next;
        if ((*list)->deleter != NULL) {
            (*list)->deleter(p->data);
        }
        free(p);
        p = next;
    }
    free(*list);
    *list = NULL;
}

// Part 3 Functions
list_t* createMIPSinstrList(FILE* IMAPFILE) {
    if(IMAPFILE == NULL) return NULL;
    
    list_t *list = CreateList(&MIPSinstr_uidComparator, &MIPSinstr_Printer, &MIPSinstr_Deleter);
    
    char *line = NULL;
    size_t cap = 0;

    while (getline(&line, &cap, IMAPFILE) != -1) {
        MIPSinstr *instr = loadInstrFormat(line);
        if(instr == NULL || FindInList(list, instr)) {
            free(line);
            DestroyList(&list);
            return NULL;
        }
        InsertAtHead(list, instr);
    }



    free(line);
    return list;
}

int printInstr(MIPSfields* instr, list_t* MIPSinstrList, char** regNames, FILE* OUTFILE) {
    
    //creates a temp MIPSinstr to search the linked list
    MIPSinstr key;
    key.uid = instr->uid;

    //after the MIPSinstr uid matches with a node in the LL point to it so we can manipulate its data
    //ensure its found by using the FindInList fuction (NULL == not found)
    node_t *p = FindInList(MIPSinstrList, &key);
    if(p == NULL) return 0;

    //now process the regnames using the getSubstrings() function. As hinted in the hw doc.
    //should index the reg values from *(subs) eg.(subs+1 == subs[1])***
    // char *buf = NULL;
    // char **subs = makeSubstringsArray(*regNames, 32, ',', &buf);
    // // if(!subs) return 0;

    // //BIG BOI BUG FIX!!
    // fprintf(OUTFILE, "%s\n", *(regNames+instr->rd));
    // fprintf(OUTFILE, "i am slow????");
    // printf("%s\n", *(regNames+instr->rd));

    // free(subs);
    // free(buf);
    // return 6767;

    //use switch/cases to manipulate the MIPSinstr node as well as print in its specified pretty format
    //  printf("%d\n",((MIPSinstr *)p->data)->pretty);
    // printf("%d\n",((MIPSinstr *)p->data)->usagecnt);

    //base format for the cases below**
    //fprintf(OUTFILE, "%s %s\n", ((MIPSinstr *)p->data)->mnemonic);
    //TODO: fix this string array!!!
    switch(((MIPSinstr *)p->data)->pretty) {
        case 0:
            fprintf(OUTFILE, "%s %s\n", ((MIPSinstr *)p->data)->mnemonic,*(regNames+instr->rd));
            break;
        case 1:
            fprintf(OUTFILE, "%s %s, %s\n", ((MIPSinstr *)p->data)->mnemonic,*(regNames+instr->rs), *(regNames+instr->rt));
            break;
        case 2:
            fprintf(OUTFILE,"%s %s, %s, 0x%x\n", ((MIPSinstr *)p->data)->mnemonic, *(regNames+instr->rt), *(regNames+instr->rs), instr->immediate16);
            break;
        case 3:
            fprintf(OUTFILE, "%s %s, %s, %s\n", ((MIPSinstr *)p->data)->mnemonic, *(regNames+instr->rd), *(regNames+instr->rs), *(regNames+instr->rt));
            break;
        case 4:
            fprintf(OUTFILE, "%s %s, 0x%x(%s)\n", ((MIPSinstr *)p->data)->mnemonic, *(regNames+instr->rt), instr->immediate16, *(regNames+instr->rs));
            break;
        case 5:
            fprintf(OUTFILE, "%s\n", ((MIPSinstr *)p->data)->mnemonic);
            break;
        case 6:
            fprintf(OUTFILE, "%s 0x%x\n", ((MIPSinstr *)p->data)->mnemonic, instr->immediate26);
            break;
        case 7:
            fprintf(OUTFILE, "%s %s, 0x%x\n", ((MIPSinstr *)p->data)->mnemonic, *(regNames+instr->rs), instr->immediate16);
            break;
        case 8:
            fprintf(OUTFILE, "%s %s, %s, 0x%x\n", ((MIPSinstr *)p->data)->mnemonic, *(regNames+instr->rd), *(regNames+instr->rs), instr->shamt);
            break;
        case 9:
            fprintf(OUTFILE, "%s %s, %s, 0x%x\n", ((MIPSinstr *)p->data)->mnemonic, *(regNames+instr->rs), *(regNames+instr->rt), instr->immediate16);
            break;
        case 10:
            fprintf(OUTFILE, "%s %s, 0x%x\n", ((MIPSinstr *)p->data)->mnemonic, *(regNames+instr->rt), instr->immediate16);
            break;
    }


    ((MIPSinstr *)p->data)->usagecnt++;
    // MIPSinstr_Printer(p->data, stdout);

    // free(subs);
    // free(buf);
    return 1;
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

