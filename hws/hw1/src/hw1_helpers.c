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

char **makeSubstringsArray(const char *line, int size, char delim, char **out_buf) {
    if (line == NULL || out_buf == NULL || size < 1) return NULL;

    size_t len = myStrlen((char *)line, '\n');
    char *buf = (char*)malloc((len + 1) * sizeof(char));
    if(!buf) return NULL;

    const char *src = line;
    char *dst = buf;
    while (*src != '\0' && *src != '\n') {
        *dst++ = *src++;
    }
    *dst = '\0';

    char **tokens = (char **)malloc((size_t)size * sizeof(char*));
    if(!tokens) {
        free(buf);
        return NULL;
    }

    if(getSubstrings(buf, delim, tokens, size) != size) {
        free(tokens);
        free(buf);
        return NULL;
    }

    *out_buf = buf;
    return tokens;

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