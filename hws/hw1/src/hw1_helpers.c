// Define any helper functions here
#include "hw1_helpers.h"
#include "hw1.h"
void printString(char *s) {
	while(*s != '\0') {
		printf("%c", *s);
		s++;
	}
	printf("\n");
}

bool typeCheck(char *s) {
	if(*s == 'r' || *s == 'i' || *s == 'j') return true;
	return false;
}

bool isHexChar(char c) {
    if (c >= '0' && c <= '9') return true;
    if (c >= 'a' && c <= 'f') return true;
    if (c >= 'A' && c <= 'F') return true;
    return false;
}

bool uidCheck(char *s) {
	char *p = s;
	while (*p != ' ') {
		printf("%c\n", *p);
        if(!isHexChar(*p)) return false;
        p++;
    }
	return true;
}