#include "stdlib.h"

static unsigned long next_rand = 1;

int abs(int x) {
    return (x < 0) ? -x : x;
}

long labs(long x) {
    return (x < 0) ? -x : x;
}

int rand(void) {
    next_rand = next_rand * 1103515245UL + 12345UL;
    return (int)((next_rand / 65536UL) % 32768UL);
}

void srand(unsigned int seed) {
    next_rand = seed;
}

int atoi(const char *str) {
    int sign = 1;
    int result = 0;

    /* Skip whitespaces */
    while (*str == ' ' || *str == '\t' || *str == '\n' ||
           *str == '\r' || *str == '\v' || *str == '\f') {
        str++;
    }

    /* Handle sign */
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}

long atol(const char *str) {
    long sign = 1;
    long result = 0;

    /* Skip whitespaces */
    while (*str == ' ' || *str == '\t' || *str == '\n' ||
           *str == '\r' || *str == '\v' || *str == '\f') {
        str++;
    }

    /* Handle sign */
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }

    return result * sign;
}

static void reverse_str(char *start, char *end) {
    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

char *itoa(int value, char *str, int base) {
    if (base < 2 || base > 36) {
        str[0] = '\0';
        return str;
    }

    int sign = (value < 0) ? -1 : 1;
    unsigned int num = (sign < 0) ? (unsigned int)(-value) : (unsigned int)value;
    int i = 0;

    do {
        unsigned int digit = num % base;
        str[i++] = (digit > 9) ? (digit - 10 + 'a') : (digit + '0');
        num /= base;
    } while (num > 0);

    if (sign < 0) {
        str[i++] = '-';
    }

    str[i] = '\0';
    reverse_str(str, str + i - 1);
    return str;
}

char *utoa(unsigned int value, char *str, int base) {
    if (base < 2 || base > 36) {
        str[0] = '\0';
        return str;
    }

    int i = 0;
    do {
        unsigned int digit = value % base;
        str[i++] = (digit > 9) ? (digit - 10 + 'a') : (digit + '0');
        value /= base;
    } while (value > 0);

    str[i] = '\0';
    reverse_str(str, str + i - 1);
    return str;
}

char *ltoa(long value, char *str, int base) {
    if (base < 2 || base > 36) {
        str[0] = '\0';
        return str;
    }

    long sign = (value < 0) ? -1 : 1;
    unsigned long num = (sign < 0) ? (unsigned long)(-value) : (unsigned long)value;
    int i = 0;

    do {
        unsigned long digit = num % base;
        str[i++] = (digit > 9) ? (digit - 10 + 'a') : (digit + '0');
        num /= base;
    } while (num > 0);

    if (sign < 0) {
        str[i++] = '-';
    }

    str[i] = '\0';
    reverse_str(str, str + i - 1);
    return str;
}