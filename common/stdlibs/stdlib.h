#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


int abs(int x);
long labs(long x);

int rand(void);
void srand(unsigned int seed);

int atoi(const char *str);
long atol(const char *str);

char *itoa(int value, char *str, int base);
char *utoa(unsigned int value, char *str, int base);
char *ltoa(long value, char *str, int base);

#endif // _STDLIB_H