#ifndef _CTYPE_H
#define _CTYPE_H

#include <stdbool.h>

bool is_alpha(char c);
bool is_digit(char c);
bool is_alnum(char c);
bool is_space(char c);
bool is_upper(char c);
bool is_lower(char c);
char to_upper(char c);
char to_lower(char c);


#endif // _CTYPE_H