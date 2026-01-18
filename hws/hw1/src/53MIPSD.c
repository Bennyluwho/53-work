#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "hw1.h"


#define USAGE_MSG   "53MIPSD [-H] [-O ofile] [-I INFILE] [-N] IMAPFILE\n" \
                    "\n  -H    Prints the usage statement to STDOUT. All other arguments are ignored."  \
                    "\n  -O    OUTFILE Results are printed into the OUTFILE specified instead of STDOUT."  \
                    "\n  -I    INFILE Inputs are read from the INFILE specified instead of STDIN." \
                    "\n  -N    Prints the registers as numerical values, instead of human-readable names\n" \

char* humanRegisters = "$zero,$at,$v0,$v1,$a0,$a1,$a2,$a3,$t0,$t1,$t2,$t3,$t4,$t5,$t6,$t7,$s0,$s1,$s2,$s3,$s4,$s5,$s6,$s7,$t8,$t9,$k0,$k1,$gp,$sp,$fp,$ra";

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
    printf("INFILE: %s\n", INFILE);
    printf("OUTFILE: %s\n", OUTFILE);
    printf("NUMERIC: %d\n", NUMERIC);
    printf("IMAPFILE: %s\n", imapfile);
    // THE PRINT STATEMENTS ABOVE ARE FOR DEBUGGING PURPOSES ONLY! Comment out from final implemenation

    // Create space for 1D array for the register names - still need to initialize!
    char** regNames  = calloc(32, sizeof(char*));     
    // INSERT YOUR IMPLEMENTATION HERE




    free(regNames);  // free the memory we allocated because we are awesomely efficient programmers! 
    
    return EXIT_SUCCESS;
}
