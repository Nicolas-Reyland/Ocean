//
// Created by Nicolas Reyland on 5/10/22.
//

#include <stdio.h>
#include <stdlib.h>
#include "char_categories.h"

CHAR_CATEGORY char_category(const char c) {
    switch (c) {
        case ' ': case '\t': case '\n':
            return CHAR_WHITESPACE;
        case '&':
            return CHAR_AMPERSAND;
        case '|':
            return CHAR_PIPE;
        case EOF: case 0x0:
            return CHAR_EOF;
        case '\'':
            return CHAR_SINGLE_QUOTE;
        case '"':
            return CHAR_DOUBLE_QUOTE;
        case '>':
            return CHAR_RIGHT_ANGLE_BRACKET;
        case '<':
            return CHAR_LEFT_ANGLE_BRACKET;
        default:
            return CHAR_GENERAL;
    }
}

char * CHAR_CATEGORY_STRING(CHAR_CATEGORY cat) {
    switch (cat) {
        case CHAR_GENERAL:
            return "CHAR_GENERAL";
        case CHAR_WHITESPACE:
            return "CHAR_WHITESPACE";
        case CHAR_AMPERSAND:
            return "CHAR_AMPERSAND";
        case CHAR_PIPE:
            return "CHAR_PIPE";
        case CHAR_SINGLE_QUOTE:
            return "CHAR_SINGLE_QUOTE";
        case CHAR_DOUBLE_QUOTE:
            return "CHAR_DOUBLE_QUOTE";
        case CHAR_RIGHT_ANGLE_BRACKET:
            return "CHAR_RIGHT_ANGLE_BRACKET";
        case CHAR_LEFT_ANGLE_BRACKET:
            return "CHAR_LEFT_ANGLE_BRACKET";
        case CHAR_SINGLE_CHAR:
            return "CHAR_SINGLE_CHAR";
        case CHAR_EOF:
            return "CHAR_EOF";
        default:
            fprintf(stderr, "Unknown char category: %d\n", cat);
            exit(1);
    }
}
