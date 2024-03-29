//
// Created on 16/05/2022.
//

#ifndef OCEAN_CST_H
#define OCEAN_CST_H

#include <stddef.h>
#include "lexer/token.h"
#include "misc/stack.h"
#include "combinators/combinator.h"

/*
typedef enum {
    CST_NONE = COMBINATOR_NONE_TYPE,
    // Shell Command Language grammar rules names
    // none here

    // Generated parsers
    CST_STRING_PARSER,
    CST_STRING_STATE_PARSER,
    CST_STATE_PARSER,

    // Auto-type
    CST_INVERTED = COMBINATOR_INVERTED_TYPE,
    CST_SEQUENCE = COMBINATOR_SEQUENCE_TYPE,
    CST_SEQUENCE_UNIT = COMBINATOR_SEQUENCE_UNIT_TYPE,
    CST_REPETITION = COMBINATOR_REPETITION_TYPE,
    CST_CHOICE = COMBINATOR_CHOICE_TYPE,
    CST_OPTIONAL = COMBINATOR_OPTIONAL_TYPE,

    CST_SEPARATED = COMBINATOR_SEPARATED_TYPE,
    CST_SEPARATED_REPETITION = COMBINATOR_SEPARATED_REPETITION_TYPE,

    // Volatile Parsers
    CST_GENERATOR = COMBINATOR_GENERATOR_TYPE,
    CST_LOOKAHEAD = COMBINATOR_LOOKAHEAD_TYPE,
} ConcreteNodeType;
 */

extern const char * const CONCRETE_NODE_TYPE_STRING[];

typedef struct CSTNode {
    ParserType type;
    Token * token;
    struct CSTNode ** children;
    size_t num_children;
} CSTNode;

void free_cst_node(CSTNode * node);
void free_cst_node_children(CSTNode node);
void prune_cst(CSTNode * node);

#endif // OCEAN_CST_H
