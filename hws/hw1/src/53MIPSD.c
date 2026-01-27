#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "hw1.h"


#define USAGE_MSG   "53MIPSD [-H] [-O ofile] [-I INFILE] [-N] IMAPFILE\n" \
                    "\n  -H    Prints the usage statement to STDOUT. All other arguments are ignored."  \
                    "\n  -O    OUTFILE Results are printed into the OUTFILE specified instead of STDOUT."  \
                    "\n  -I    INFILE Inputs are read from the INFILE specified instead of STDIN." \
                    "\n  -N    Prints the registers as numerical values, instead of human-readable names\n" \

char* humanRegisters = "$zero,$at,$v0,$v1,$a0,$a1,$a2,$a3,$t0,$t1,$t2,$t3,$t4,$t5,$t6,$t7,$s0,$s1,$s2,$s3,$s4,$s5,$s6,$s7,$t8,$t9,$k0,$k1,$gp,$sp,$fp,$ra";
char* numericRegisters ="$0,$1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$20,$21,$22,$23,$24,$25,$26,$27,$28,$29,$30,$31";

int main(int argc, char* argv[]) {

    char* OUTFILE = NULL; // The output file (remains NULL if -O option is not provided) 
    char* INFILE = NULL; // The input file (remains NULL if -I option is not provided)
    int NUMERIC = 0; // Numeric mode (remains false if -N option is not provided)
    
    
    opterr = 0;
    
    int c;

    
    
    while ((c = getopt(argc, argv, "HO:I:N")) >= 0) {
        switch (c)
        {
        case 'H':
            printf(USAGE_MSG);
            return EXIT_SUCCESS;
        case 'O':
            OUTFILE = optarg;
            break;
        case 'I':
            INFILE = optarg;
            break;
        case 'N':
            NUMERIC = 1;
            break;
        case '?':
            if (optopt == 'i' || optopt == 'o') {
                fprintf(stderr, "Option -%c requires an argument.\n\n" USAGE_MSG, optopt);
            }
        default:
            fprintf(stderr, USAGE_MSG);
            return EXIT_FAILURE;
        }
    }

    // validate that we only have 1 positional argument
    if (optind + 1 != argc) {
        fprintf(stderr, "Exactly one positional argument should be specified.\n\n" USAGE_MSG);
        return EXIT_FAILURE;
    }

    char* imapfile = argv[optind]; // The IMAPFILE specified

    // INFILE, ofile, numeric and imapfile are now usable by your program!
    // THE PRINT STATEMENTS BELOW ARE FOR DEBUGGING PURPOSES ONLY! Comment out from final implemenation
    // printf("INFILE: %s\n", INFILE);
    // printf("OUTFILE: %s\n", OUTFILE);
    // printf("NUMERIC: %d\n", NUMERIC);
    // printf("IMAPFILE: %s\n", imapfile);
    // THE PRINT STATEMENTS ABOVE ARE FOR DEBUGGING PURPOSES ONLY! Comment out from final implemenation

    // Create space for 1D array for the register names - still need to initialize!
    char** regNames  = calloc(32, sizeof(char*));     
    // INSERT YOUR IMPLEMENTATION HERE
    int exit_status = 0;

    FILE *in  = NULL;
    FILE *out = NULL;
    FILE *imap = NULL;

    list_t *imapList = NULL;

    char *buf = NULL;

    unsigned char *bytes = NULL;

    if (INFILE != NULL) {
        in = fopen(INFILE, "rb");
        if (in == NULL) {
            exit_status = 2;
            goto MAIN_DONE;
        }
    } else {
        in = stdin;
    }

    if (OUTFILE != NULL) {
        out = fopen(OUTFILE, "w");
        if (out == NULL) {
            exit_status = 2;
            goto MAIN_DONE;
        }
    } else {
        out = stdout;
    }

    imap = fopen(imapfile, "r");
    if (imap == NULL) {
        exit_status = 3;
        goto MAIN_DONE;
    }

    imapList = createMIPSinstrList(imap);
    if (imapList == NULL) {
        exit_status = 3;
        goto MAIN_DONE;
    }

    {
        char *src = (NUMERIC ? numericRegisters : humanRegisters);

        size_t len = 0;
        char *s = src;
        while (*s != '\0') {
            len++;
            s++;
        }

        buf = (char *)malloc(len + 1);
        if (buf == NULL) {
            exit_status = 1;
            goto MAIN_DONE;
        }

        char *d = buf;
        s = src;
        while (*s != '\0') {
            *d = *s;
            d++;
            s++;
        }
        *d = '\0';

        getSubstrings(buf, ',', regNames, 32);
    }

    bytes = (unsigned char *)malloc(4);
    if (bytes == NULL) {
        exit_status = 1;
        goto MAIN_DONE;
    }

    while (1) {
        size_t got = fread(bytes, 1, 4, in);

        if (got == 0) {
            break;
        }

        if (got != 4) {
            exit_status = 3;
            goto MAIN_DONE;
        }

        uint32_t instr =
              ((uint32_t)(*(bytes + 0))      )
            | ((uint32_t)(*(bytes + 1)) <<  8)
            | ((uint32_t)(*(bytes + 2)) << 16)
            | ((uint32_t)(*(bytes + 3)) << 24);

        MIPSfields f;
        parseMIPSfields(instr, &f);

        if (printInstr(&f, imapList, regNames, out) == 0) {
            exit_status = 3;
            goto MAIN_DONE;
        }
    }

    MAIN_DONE:
    if (bytes != NULL) free(bytes);

    if (imap != NULL) fclose(imap);

    if (imapList != NULL) {
        DestroyList(&imapList);
    }

    if (in != NULL && in != stdin) fclose(in);
    if (out != NULL && out != stdout) fclose(out);

    free(regNames);
    free(buf);

    return exit_status;
}
