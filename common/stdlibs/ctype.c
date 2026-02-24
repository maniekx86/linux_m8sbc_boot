#include <stdbool.h>

#include "ctype.h"

bool is_alpha(char c) {
    return ( (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') );
}

bool is_digit(char c) {
    return (c >= '0' && c <= '9');
}

bool is_alnum(char c) {
    return is_alpha(c) || is_digit(c);
}

bool is_space(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
}

bool is_upper(char c) {
    return (c >= 'A' && c <= 'Z');
}

bool is_lower(char c) {
    return (c >= 'a' && c <= 'z');
}

char to_upper(char c) {
    if (is_lower(c)) {
        return c - ('a' - 'A');
    }
    return c;
}

char to_lower(char c) {
    if (is_upper(c)) {
        return c + ('a' - 'A');
    }
    return c;
}