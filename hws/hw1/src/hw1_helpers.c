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

int isLower(const char *s) {
	if (s == NULL || *s == '\0') return 0;
	const char *p = s;
	while(*p) {
		if (*p < 'a' || *p > 'z') return 0;
		p++;
	}
	return 1;
}

int isHex(const char *s){
	if (s == NULL || *s == '\0') return 0;
	int len = 0;
	const char *p = s;
	while(*p) {
		char c = *p;

        int is_digit = (c >= '0' && c <= '9');
        int is_lower = (c >= 'a' && c <= 'f');
        int is_upper = (c >= 'A' && c <= 'F');

        if (!(is_digit || is_lower || is_upper))
            return 0;

        len++;
        if (len > 8) return 0;

        p++;
	}
	return 1;
}

char* copyLineHeap(const char *line) {
	const char *p = line;
	size_t n = 0;
	while(*p++) n++;

	char *buf = malloc(n + 1);
	if (!buf) return NULL;

	const char *src = line;
    char *dst = buf;
    while ((*dst++ = *src++) != '\0') { }
    return buf;
}

