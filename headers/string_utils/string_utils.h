//
// Created by Nicolas Reyland on 5/26/22.
//

#ifndef OCEAN_STRING_UTILS_H
#define OCEAN_STRING_UTILS_H

#include <stddef.h>
#include <stdbool.h>
#include "lexer/char_categories.h"
#include "lexer/xbd_operators.h"

size_t find_corresponding_char(const char * str, size_t str_len, char open, char close, bool backslash_escape);

inline bool is_blank_char(char c) {
    return c == ' ' || c == '\t';
}

inline bool is_special_char(char c, CHAR_CATEGORY category) {
    return is_operator_start(c, category) ||
           c == '"'  ||
           c == '\'' ||
           c == '{'  ||
           c == '}'  ||
           c == '!'  ;
}

#endif // OCEAN_STRING_UTILS_H
