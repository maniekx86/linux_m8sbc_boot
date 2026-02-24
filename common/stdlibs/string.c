#include "string.h"

void *memset(void *dest, int value, size_t count) {
    unsigned char *ptr = (unsigned char *)dest;
    for (size_t i = 0; i < count; i++) {
        ptr[i] = (unsigned char)value;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t count) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

int memcmp(const void *ptr1, const void *ptr2, size_t count) {
    const unsigned char *a = (const unsigned char *)ptr1;
    const unsigned char *b = (const unsigned char *)ptr2;
    for (size_t i = 0; i < count; i++) {
        if (a[i] != b[i]) {
            return (a[i] - b[i]);
        }
    }
    return 0;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++)) { }
    return dest;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i] || !s1[i] || !s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
    }
    return 0;
}

char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (*d) {
        d++;
    }
    while (n-- && *src) {
        *d++ = *src++;
    }
    *d = '\0';
    return dest;
}

char *strcat(char *dest, const char *src) {
    char *ptr = dest + strlen(dest);
    while (*src != '\0') {
        *ptr++ = *src++;
    }
    *ptr = '\0';
    return dest;
}

char *strrchr(const char *str, int character) {
    const char *last = NULL;
    while (*str) {
        if (*str == (char)character) {
            last = str;
        }
        str++;
    }
    if ((char)character == '\0') {
        return (char *)str;
    }
    return (char *)last;
}