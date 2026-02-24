#ifndef _STRING_H
#define _STRING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void *memset(void *dest, int value, size_t count);
void *memcpy(void *dest, const void *src, size_t count);
int memcmp(const void *ptr1, const void *ptr2, size_t count);

char *strcpy(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char *str);

char *strncpy(char *dest, const char *src, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncat(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);

char *strrchr(const char *str, int character);


#endif // _STRING_H