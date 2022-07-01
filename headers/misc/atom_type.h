//
// Created by Nicolas Reyland on 5/25/22.
//

#ifndef OCEAN_ATOM_TYPE_H
#define OCEAN_ATOM_TYPE_H

#include "combinators/combinator_types.h"
// #include "parser/cst.h"

typedef enum TokenType TokenType;
enum TokenType {
    NONE_TOKEN,
    // Tokens
    OPERATOR_TOKEN,
    IO_NUMBER_TOKEN,
    TOKEN_TOKEN,
    // Specific 'Token' Tokens
    WORD_TOKEN,
    ASSIGNMENT_WORD_TOKEN,
    NAME_TOKEN,
    // Specific 'Word' Tokens
    BANG_TOKEN,
    OPENING_BRACKET_TOKEN,
    CLOSING_BRACKET_TOKEN,
    CASE_TOKEN,
    DO_TOKEN,
    DONE_TOKEN,
    ELIF_TOKEN,
    ELSE_TOKEN,
    ESAC_TOKEN,
    FI_TOKEN,
    FOR_TOKEN,
    IF_TOKEN,
    IN_TOKEN,
    THEN_TOKEN,
    UNTIL_TOKEN,
    WHILE_TOKEN,
    // Specific operator Tokens
    NEWLINE_TOKEN,
    AND_IF_TOKEN,
    OR_IF_TOKEN,
    DSEMI_TOKEN,
    DLESS_TOKEN,
    DGREAT_TOKEN,
    LESSAND_TOKEN,
    GREATAND_TOKEN,
    LESSGREAT_TOKEN,
    DLESSDASH_TOKEN,
    CLOBBER_TOKEN,
};

typedef enum ParserType ParserType;
enum ParserType {
    NONE_PARSER = COMBINATOR_NONE_TYPE,
    // Grammar Parsers
    PROGRAM_PARSER,
    COMPLETE_COMMANDS_PARSER,
    COMPLETE_COMMAND_PARSER,
    LIST_PARSER,
    AND_OR_PARSER,
    PIPELINE_PARSER,
    PIPE_SEQUENCE_PARSER,
    COMMAND_PARSER,
    COMPOUND_COMMAND_PARSER,
    SUBSHELL_PARSER,
    COMPOUND_LIST_PARSER,
    TERM_PARSER,
    FOR_CLAUSE_PARSER,
    NAME_PARSER,
    IN_PARSER,
    WORDLIST_PARSER,
    CASE_CLAUSE_PARSER,
    CASE_LIST_NS_PARSER,
    CASE_LIST_PARSER,
    CASE_ITEM_NS_PARSER,
    CASE_ITEM_PARSER,
    PATTERN_PARSER,
    IF_CLAUSE_PARSER,
    ELSE_PART_PARSER,
    WHILE_CLAUSE_PARSER,
    UNTIL_CLAUSE_PARSER,
    FUNCTION_DEFINITION_PARSER,
    FUNCTION_BODY_PARSER,
    FNAME_PARSER,
    BRACE_GROUP_PARSER,
    DO_GROUP_PARSER,
    SIMPLE_COMMAND_PARSER,
    CMD_NAME_PARSER,
    CMD_WORD_PARSER,
    CMD_PREFIX_PARSER,
    CMD_SUFFIX_PARSER,
    REDIRECT_LIST_PARSER,
    IO_REDIRECT_PARSER,
    IO_FILE_PARSER,
    FILENAME_PARSER,
    IO_HERE_PARSER,
    HERE_END_PARSER,
    NEWLINE_LIST_PARSER,
    LINEBREAK_PARSER,
    SEPARATOR_OP_PARSER,
    SEPARATOR_PARSER,
    SEQUENTIAL_SEP_PARSER,
    // Token Parsers
    TK_WORD_PARSER,
    TK_ASSIGNMENT_WORD_PARSER,
    TK_NAME_PARSER,
    TK_NEWLINE_PARSER,
    TK_IO_NUMBER_PARSER,
    // Generated Parsers
    GEN_STRING_PARSER,
    GEN_STRING_R1_PARSER,
    // Abstract Parsers
    INVERTED_PARSER = COMBINATOR_INVERTED_TYPE,
    SEQUENCE_PARSER = COMBINATOR_SEQUENCE_TYPE,
    SEQUENCE_PARSER_UNIT = COMBINATOR_SEQUENCE_UNIT_TYPE,
    REPETITION_PARSER = COMBINATOR_REPETITION_TYPE,
    CHOICE_PARSER = COMBINATOR_CHOICE_TYPE,
    OPTIONAL_PARSER = COMBINATOR_OPTIONAL_TYPE,
    SEPARATED_PARSER = COMBINATOR_SEPARATED_TYPE,
    SEPARATED_REPETITION_PARSER = COMBINATOR_SEPARATED_REPETITION_TYPE,
    LOOKAHEAD_PARSER = COMBINATOR_LOOKAHEAD_TYPE,
    // Generator Parser
    GENERATOR_PARSER = COMBINATOR_GENERATOR_TYPE,
};

char * TOKEN_TYPE_STRING(TokenType type);
char * PARSER_TYPE_STRING(ParserType type);

#endif // OCEAN_ATOM_TYPE_H