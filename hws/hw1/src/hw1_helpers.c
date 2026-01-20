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

int myStrlen(char *s, char delim) {
    int count = 0;

    while(*s != delim) {
        s++;
        count++;
    }

    return count + 1;
}

bool typeCheck(char *s, char *out) {
	if(*s == 'r' || *s == 'i' || *s == 'j') {
		*out = *s;
		return true;
	}
	return false;
}

bool isHexChar(char c) {
    if (c >= '0' && c <= '9') return true;
    if (c >= 'a' && c <= 'f') return true;
    if (c >= 'A' && c <= 'F') return true;
    return false;
}

static uint32_t hexDigitVal(char c) {
    if (c >= '0' && c <= '9') return (uint32_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (uint32_t)(c - 'a' + 10);
    return (uint32_t)(c - 'A' + 10); // assumes caller validated A-F
}

bool uidCheck(char *s, uint32_t *out) {
	if (s== NULL || out == NULL) return false;

	char *p = s;
	uint32_t value = 0;

	if (*p == ' ' || *p == '\n' || *p == '\0') return false;

	while (*p != '\0' && *p != '\n' && *p != ' ' && *p != '\t') {
        if(!isHexChar(*p)) return false;

		value = (value << 4) | hexDigitVal(*p);
        p++;
    }

	*out = value;
	return true;
}


bool mnemonicCheck(char *s, char **out) {
	if (s== NULL || out == NULL) return false;

	*out = s;

	if (*s == ' ' || *s == '\n' || *s == '\0' || *s == '\t') return false;
    if (!(*s >= 'a' && *s <= 'z')) return false;

	while (*s != ' ' && *s != '\n' && *s != '\0' && *s != '\t') {
        if (!(*s >= 'a' && *s <= 'z')) return false;
        s++;
    }

	return true;
}

bool prettyCheck(const char *s, int *out){
	if (s == NULL || out == NULL) return false;

    if (*s < '0' || *s > '9') return false;

    int value = 0;

    while (*s >= '0' && *s <= '9') {
        value = value * 10 + (*s - '0');
        s++;
    }

    if (*s != ' ' && *s != '\n' && *s != '\0') return false;

    if (value < 0 || value > 10) return false;

    *out = value;
    return true;
}