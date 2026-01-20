#ifndef HELPERS_1_H
#define HELPERS_1_H

#include <stdbool.h>
#include <stdint.h>


// Declare any helper functions here
void printString(char *s);
bool typeCheck(char *s, char *out);
bool uidCheck(char *s, uint32_t *out);
bool mnemonicCheck(char *s, char **out);
bool prettyCheck(const char *s, int *out);
#endif
