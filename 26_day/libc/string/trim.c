#include <string.h>

char* trim(char* str, int len) {
    if (*str == '\0') {
        return str;
    }
    char* end = str + len - 1;

    // Remove leading whitespace
    while (*str == ' ') {
        str++;
    }

    // If the string is now empty, return
    if (*str == '\0') {
        return str;
    }

    // Remove trailing whitespace
    while (end > str && *end == ' ') {
        end--;
    }

    // Null-terminate the trimmed string
    *(end + 1) = '\0';

    return str;
}