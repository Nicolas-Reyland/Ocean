//
// Created by Nicolas Reyland on 5/26/22.
//

#include <stddef.h>
#include "atom_type.h"

#define STRINGIFY_ATOM_TYPE_CASE(type) case type: { return #type; }

char * ATOM_TYPE_STRING(AtomType type) {
    switch (type) {
        // Tokens
        STRINGIFY_ATOM_TYPE_CASE(NEWLINE_TOKEN)
        STRINGIFY_ATOM_TYPE_CASE(OPERATOR_TOKEN)
        STRINGIFY_ATOM_TYPE_CASE(IO_NUMBER_TOKEN)
        STRINGIFY_ATOM_TYPE_CASE(TOKEN_TOKEN)
        STRINGIFY_ATOM_TYPE_CASE(WORD_TOKEN)
        STRINGIFY_ATOM_TYPE_CASE(ASSIGNMENT_WORD_TOKEN)
        STRINGIFY_ATOM_TYPE_CASE(NAME_TOKEN)
        // Grammar Parser
        STRINGIFY_ATOM_TYPE_CASE(PROGRAM_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(COMPLETE_COMMANDS_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(COMPLETE_COMMAND_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(LIST_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(AND_OR_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(PIPELINE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(PIPE_SEQUENCE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(COMMAND_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(COMPOUND_COMMAND_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(SUBSHELL_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(COMPOUND_LIST_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(TERM_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(FOR_CLAUSE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(NAME_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(IN_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(WORDLIST_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(CASE_CLAUSE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(CASE_LIST_NS_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(CASE_LIST_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(CASE_ITEM_NS_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(CASE_ITEM_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(PATTERN_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(IF_CLAUSE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(ELSE_PART_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(WHILE_CLAUSE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(UNTIL_CLAUSE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(FUNCTION_DEFINITION_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(FUNCTION_BODY_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(FNAME_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(BRACE_GROUP_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(DO_GROUP_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(SIMPLE_COMMAND_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(CMD_NAME_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(CMD_WORD_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(CMD_PREFIX_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(CMD_SUFFIX_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(REDIRECT_LIST_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(IO_REDIRECT_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(IO_FILE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(FILENAME_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(IO_HERE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(HERE_END_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(NEWLINE_LIST_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(LINEBREAK_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(SEPARATOR_OP_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(SEPARATOR_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(SEQUENTIAL_SEP_PARSER)
        // Token Parser
        STRINGIFY_ATOM_TYPE_CASE(TK_WORD_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(TK_ASSIGNMENT_WORD_PARSE)
        STRINGIFY_ATOM_TYPE_CASE(TK_NAME_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(TK_NEWLINE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(TK_IO_NUMBER)
        // Abstract Combinator
        STRINGIFY_ATOM_TYPE_CASE(SEQUENCE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(SEQUENCE_PARSER_UNIT)
        STRINGIFY_ATOM_TYPE_CASE(CHOICE_PARSER)
        STRINGIFY_ATOM_TYPE_CASE(OPTIONAL_PARSER)
        default:
            return NULL;
    }
}
