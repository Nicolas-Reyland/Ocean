//
// Created by nicolas on 5/10/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"
#include "states.h"
#include "char_categories.h"

static Token next_token(const char *, size_t *, STATE *, STATE *, STATE *);

static char escape_char(char c);

Token * tokenize(const char * content, size_t* num_tokens) {
    // Validate args
    if (content == NULL) {
        fprintf(stderr, "Content is NULL\n");
    }
    if (num_tokens == NULL) {
        fprintf(stderr, "Num_tokens is NULL\n");
    }
    // Tokenization
    Token tokens[256];
    Token token;
    STATE cur_state = STATE_GENERAL,
        next_state,
        prev_state;
    *num_tokens = 0;
    size_t content_index = 0;
    do {
        token = next_token(content, &content_index, &cur_state, &next_state, &prev_state);
        tokens[(*num_tokens)++] = token;
    } while (cur_state != STATE_EOF);

    size_t mem_size = (*num_tokens) * sizeof(Token);
    Token * heap_tokens = malloc(mem_size);
    memcpy(heap_tokens, tokens, mem_size);
    return heap_tokens;
}

#define ADD_CHAR_TO_TOKEN \
    token_str[cur_token_len++] = c;

#define TOKEN_DONE \
    *next_state = STATE_GENERAL; \
    finished_token = true;

#define BACK_ONE_CHAR_AND_DONE \
    *next_state = STATE_GENERAL; \
    (*content_index)--; \
    TOKEN_DONE

#define SYNTAX_ERROR \
    fprintf(stderr, "SyntaxError: %s (prev: %s) | %s (rep: %zu)\n", STATE_STRING(*cur_state), STATE_STRING(*prev_state), CHAR_CATEGORY_STRING(char_cat), cur_token_len); \
    exit(1);

static Token next_token(const char * content, size_t * content_index, STATE * cur_state, STATE * next_state, STATE * prev_state) {
    Token token = {
            .char_index = (int)*content_index,
            .eof_or_empty = false,
    };
    char token_str[32];
    CHAR_CATEGORY char_cat;
    size_t cur_token_len = 0;
    bool finished_token = false;
    do {
        const char c = content[(*content_index)++];
        char_cat = char_category(c);

        if (*cur_state == STATE_ESCAPE) {
            switch (*prev_state) {
                case STATE_GENERAL:
                case STATE_FILENAME: {
                    // token_str[cur_token_len++] = '\\';
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_DOUBLE_QUOTES: {
                    char escaped = escape_char(c);
                    if (escaped != 0)
                        token_str[cur_token_len++] = escaped;
                    else {
                        token_str[cur_token_len++] = '\\';
                        ADD_CHAR_TO_TOKEN
                    }
                } break;
                default: {
                    SYNTAX_ERROR
                }
            }
            *next_state = *prev_state;
        } else {
            unsigned int selector = *cur_state | char_cat;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-branch-clone"
            switch (selector) {
                // GENERAL CASE
                case STATE_GENERAL | CHAR_GENERAL: {
                    *next_state = STATE_FILENAME;
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_GENERAL | CHAR_WHITESPACE: {
                    token.eof_or_empty = true;
                    TOKEN_DONE
                } break;
                case STATE_GENERAL | CHAR_AMPERSAND: {
                    *next_state = STATE_AMPERSAND;
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_GENERAL | CHAR_PIPE: {
                    *next_state = STATE_PIPE;
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_GENERAL | CHAR_SINGLE_QUOTE: {
                    *next_state = STATE_SINGLE_QUOTES;
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_GENERAL | CHAR_DOUBLE_QUOTE: {
                    *next_state = STATE_DOUBLE_QUOTES;
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_GENERAL | CHAR_RIGHT_ANGLE_BRACKET: {
                    *next_state = STATE_RIGHT_ANGLE_BRACKET;
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_GENERAL | CHAR_LEFT_ANGLE_BRACKET: {
                    *next_state = STATE_LEFT_ANGLE_BRACKET;
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_GENERAL | CHAR_SINGLE_CHAR: {
                    ADD_CHAR_TO_TOKEN
                    TOKEN_DONE
                } break;
                case STATE_GENERAL | CHAR_ESCAPE: {
                    *prev_state = *cur_state;
                    *next_state = STATE_ESCAPE;
                } break;
                case STATE_GENERAL | CHAR_EOF: {
                    token.eof_or_empty = true;
                    TOKEN_DONE
                } break;
                // FILENAME CASE
                case STATE_FILENAME | CHAR_GENERAL: {
                    *next_state = STATE_FILENAME;
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_FILENAME | CHAR_ESCAPE: {
                    *prev_state = *cur_state;
                    *next_state = STATE_ESCAPE;
                } break;
                case STATE_FILENAME | CHAR_EOF: {
                    TOKEN_DONE
                } break;
                case STATE_FILENAME | CHAR_WHITESPACE:
                case STATE_FILENAME | CHAR_AMPERSAND:
                case STATE_FILENAME | CHAR_PIPE:
                case STATE_FILENAME | CHAR_SINGLE_QUOTE:
                case STATE_FILENAME | CHAR_DOUBLE_QUOTE:
                case STATE_FILENAME | CHAR_RIGHT_ANGLE_BRACKET:
                case STATE_FILENAME | CHAR_LEFT_ANGLE_BRACKET:
                case STATE_FILENAME | CHAR_SINGLE_CHAR: {
                    BACK_ONE_CHAR_AND_DONE
                } break;
                // AMPERSAND CASE
                case STATE_AMPERSAND | CHAR_AMPERSAND: {
                    if (cur_token_len == 2) {
                        SYNTAX_ERROR
                    }
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_AMPERSAND | CHAR_PIPE: {
                    SYNTAX_ERROR
                }
                case STATE_AMPERSAND | CHAR_WHITESPACE: {
                    TOKEN_DONE
                } break;
                case STATE_AMPERSAND | CHAR_ESCAPE: {
                    *prev_state = STATE_GENERAL;
                    *next_state = STATE_ESCAPE;
                    finished_token = true;
                } break;
                case STATE_AMPERSAND | CHAR_GENERAL:
                case STATE_AMPERSAND | CHAR_SINGLE_QUOTE:
                case STATE_AMPERSAND | CHAR_DOUBLE_QUOTE:
                case STATE_AMPERSAND | CHAR_RIGHT_ANGLE_BRACKET:
                case STATE_AMPERSAND | CHAR_LEFT_ANGLE_BRACKET:
                case STATE_AMPERSAND | CHAR_SINGLE_CHAR:
                case STATE_AMPERSAND | CHAR_EOF: {
                    BACK_ONE_CHAR_AND_DONE
                } break;
                // PIPE CASE
                case STATE_PIPE | CHAR_PIPE: {
                    if (cur_token_len == 2) {
                        SYNTAX_ERROR
                    }
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_PIPE | CHAR_AMPERSAND: {
                    SYNTAX_ERROR
                }
                case STATE_PIPE | CHAR_WHITESPACE: {
                    TOKEN_DONE
                } break;
                case STATE_PIPE | CHAR_ESCAPE: {
                    *prev_state = STATE_GENERAL;
                    *next_state = STATE_ESCAPE;
                    finished_token = true;
                } break;
                case STATE_PIPE | CHAR_GENERAL:
                case STATE_PIPE | CHAR_SINGLE_QUOTE:
                case STATE_PIPE | CHAR_DOUBLE_QUOTE:
                case STATE_PIPE | CHAR_RIGHT_ANGLE_BRACKET:
                case STATE_PIPE | CHAR_LEFT_ANGLE_BRACKET:
                case STATE_PIPE | CHAR_SINGLE_CHAR:
                case STATE_PIPE | CHAR_EOF: {
                    BACK_ONE_CHAR_AND_DONE
                } break;
                // SINGLE QUOTES CASE
                case STATE_SINGLE_QUOTES | CHAR_SINGLE_QUOTE: {
                    ADD_CHAR_TO_TOKEN
                    TOKEN_DONE
                } break;
                case STATE_SINGLE_QUOTES | CHAR_EOF: {
                    SYNTAX_ERROR
                }
                case STATE_SINGLE_QUOTES | CHAR_GENERAL:
                case STATE_SINGLE_QUOTES | CHAR_WHITESPACE:
                case STATE_SINGLE_QUOTES | CHAR_AMPERSAND:
                case STATE_SINGLE_QUOTES | CHAR_PIPE:
                case STATE_SINGLE_QUOTES | CHAR_DOUBLE_QUOTE:
                case STATE_SINGLE_QUOTES | CHAR_RIGHT_ANGLE_BRACKET:
                case STATE_SINGLE_QUOTES | CHAR_LEFT_ANGLE_BRACKET:
                case STATE_SINGLE_QUOTES | CHAR_SINGLE_CHAR:
                case STATE_SINGLE_QUOTES | CHAR_ESCAPE: {
                    ADD_CHAR_TO_TOKEN
                } break;
                // DOUBLE QUOTES CASE
                case STATE_DOUBLE_QUOTES | CHAR_DOUBLE_QUOTE: {
                    ADD_CHAR_TO_TOKEN
                    TOKEN_DONE
                } break;
                case STATE_DOUBLE_QUOTES | CHAR_ESCAPE: {
                    *prev_state = *cur_state;
                    *next_state = STATE_ESCAPE;
                } break;
                case STATE_DOUBLE_QUOTES | CHAR_EOF: {
                    SYNTAX_ERROR
                }
                case STATE_DOUBLE_QUOTES | CHAR_GENERAL:
                case STATE_DOUBLE_QUOTES | CHAR_WHITESPACE:
                case STATE_DOUBLE_QUOTES | CHAR_AMPERSAND:
                case STATE_DOUBLE_QUOTES | CHAR_PIPE:
                case STATE_DOUBLE_QUOTES | CHAR_SINGLE_QUOTE:
                case STATE_DOUBLE_QUOTES | CHAR_RIGHT_ANGLE_BRACKET:
                case STATE_DOUBLE_QUOTES | CHAR_LEFT_ANGLE_BRACKET:
                case STATE_DOUBLE_QUOTES | CHAR_SINGLE_CHAR: {
                    ADD_CHAR_TO_TOKEN
                } break;
                // RIGHT ANGLE CASE
                case STATE_RIGHT_ANGLE_BRACKET | CHAR_RIGHT_ANGLE_BRACKET: {
                    if (cur_token_len == 2) {
                        SYNTAX_ERROR
                    }
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_RIGHT_ANGLE_BRACKET | CHAR_ESCAPE: {
                    *prev_state = STATE_GENERAL;
                    *next_state = STATE_ESCAPE;
                    finished_token = true;
                } break;
                case STATE_RIGHT_ANGLE_BRACKET | CHAR_EOF: {
                    SYNTAX_ERROR
                }
                case STATE_RIGHT_ANGLE_BRACKET | CHAR_GENERAL:
                case STATE_RIGHT_ANGLE_BRACKET | CHAR_WHITESPACE:
                case STATE_RIGHT_ANGLE_BRACKET | CHAR_AMPERSAND:
                case STATE_RIGHT_ANGLE_BRACKET | CHAR_PIPE:
                case STATE_RIGHT_ANGLE_BRACKET | CHAR_SINGLE_QUOTE:
                case STATE_RIGHT_ANGLE_BRACKET | CHAR_DOUBLE_QUOTE:
                case STATE_RIGHT_ANGLE_BRACKET | CHAR_LEFT_ANGLE_BRACKET:
                case STATE_RIGHT_ANGLE_BRACKET | CHAR_SINGLE_CHAR: {
                    BACK_ONE_CHAR_AND_DONE
                } break;
                // LEFT ANGLE CASE
                case STATE_LEFT_ANGLE_BRACKET | CHAR_LEFT_ANGLE_BRACKET: {
                    if (cur_token_len == 2) {
                        SYNTAX_ERROR
                    }
                    ADD_CHAR_TO_TOKEN
                } break;
                case STATE_LEFT_ANGLE_BRACKET | CHAR_ESCAPE: {
                    *prev_state = STATE_GENERAL;
                    *next_state = STATE_ESCAPE;
                    finished_token = true;
                } break;
                case STATE_LEFT_ANGLE_BRACKET | CHAR_EOF: {
                    SYNTAX_ERROR
                }
                case STATE_LEFT_ANGLE_BRACKET | CHAR_GENERAL:
                case STATE_LEFT_ANGLE_BRACKET | CHAR_WHITESPACE:
                case STATE_LEFT_ANGLE_BRACKET | CHAR_AMPERSAND:
                case STATE_LEFT_ANGLE_BRACKET | CHAR_PIPE:
                case STATE_LEFT_ANGLE_BRACKET | CHAR_SINGLE_QUOTE:
                case STATE_LEFT_ANGLE_BRACKET | CHAR_DOUBLE_QUOTE:
                case STATE_LEFT_ANGLE_BRACKET | CHAR_RIGHT_ANGLE_BRACKET:
                case STATE_LEFT_ANGLE_BRACKET | CHAR_SINGLE_CHAR: {
                    BACK_ONE_CHAR_AND_DONE
                } break;
                // SINGLE CHAR CASE
                case STATE_SINGLE_CHAR | CHAR_GENERAL:
                case STATE_SINGLE_CHAR | CHAR_WHITESPACE:
                case STATE_SINGLE_CHAR | CHAR_AMPERSAND:
                case STATE_SINGLE_CHAR | CHAR_PIPE:
                case STATE_SINGLE_CHAR | CHAR_SINGLE_QUOTE:
                case STATE_SINGLE_CHAR | CHAR_DOUBLE_QUOTE:
                case STATE_SINGLE_CHAR | CHAR_RIGHT_ANGLE_BRACKET:
                case STATE_SINGLE_CHAR | CHAR_LEFT_ANGLE_BRACKET:
                case STATE_SINGLE_CHAR | CHAR_SINGLE_CHAR:
                case STATE_SINGLE_CHAR | CHAR_ESCAPE:
                case STATE_SINGLE_CHAR | CHAR_EOF: {
                    fprintf(stderr, "Should never be in single char state\n");
                    exit(1);
                }
                // Not implemented
                default:
                    fprintf(stderr, "Missing implementation for situation: %s | %s\n", STATE_STRING(*cur_state),
                            CHAR_CATEGORY_STRING(char_cat));
                    exit(1);
            }
#pragma clang diagnostic pop
        }

        *cur_state = *next_state;
    } while(char_cat != CHAR_EOF && !finished_token);
    if (char_cat == CHAR_EOF)
        *cur_state = STATE_EOF;

    // copy string into token
    token.str = malloc((cur_token_len + 1) * sizeof(char));
    memcpy(token.str, token_str, cur_token_len);
    token.str[cur_token_len] = 0x0;
    token.str_len = cur_token_len;

    return token;
}

static char escape_char(const char c) {
    switch (c) {
        case 'a':
            return 0x7;
        case 'b':
            return 0x8;
        case 'e':
            return 0x1b;
        case 'f':
            return 0xc;
        case 'n':
            return 0xa;
        case 'r':
            return 0xd;
        case 't':
            return 0x9;
        case 'v':
            return 0xb;
        default:
            return 0;
    }
}