#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

int strcmp(const char *s1,const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char* trim(char* str, int len);

#endif // _STRING_H