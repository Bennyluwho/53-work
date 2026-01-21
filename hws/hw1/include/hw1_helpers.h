#ifndef HELPERS_1_H
#define HELPERS_1_H

#include "hw1.h"
#define true 1
#define false 0
#define bool _Bool



// Declare any helper functions here
void printString(char *s);
int myStrlen(char *s, char delim);
_Bool typeCheck(char *s, char *out);
_Bool uidCheck(char *s, uint32_t *out);
_Bool mnemonicCheck(char *s, char **out);
_Bool prettyCheck(const char *s, int *out);
#endif
