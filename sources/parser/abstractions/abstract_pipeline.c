//
// Created by Nicolas Reyland on 8/27/22.
//

#include <stdlib.h>
#include "parser/abstractions/abstract_pipeline.h"
#include "parser/abstractions/abstraction.h"
#include "misc/output.h"
#include "parser/abstractions/abstract_compound_command.h"
#include "parser/abstractions/imperfect_abstract_io_redirect.h"

static Executable abstract_command(CSTNode cst_node);
static Executable abstract_simple_command(CSTNode cst_node);
static Executable abstract_function_definition(CSTNode cst_node);

Executable abstract_pipeline(CSTNode cst_node) {
    if (cst_node.type == GENERATOR_PARSER)
        cst_node.type = PIPELINE_PARSER;
    NODE_COMPLIANCE(cst_node, PIPELINE_PARSER, 2, OPTIONAL_PARSER, PIPE_SEQUENCE_PARSER)
    // TODO: Bang symbol support
    cst_node = *cst_node.children[1]; // to PIPE_SEQUENCE
    NODE_COMPLIANCE(cst_node, PIPE_SEQUENCE_PARSER, 2, COMMAND_PARSER, SEPARATED_REPETITION_PARSER)
    Executable first_command = abstract_command(*cst_node.children[0]);
    if (has_children(*cst_node.children[1])) {
        // Multi commands which use pipes (at least two)
        size_t num_children = cst_node.children[1]->num_children + 1; // + first command
        struct ExecMultiExecutables pipe_sequence = {
                .execution_flags = EXE_PIPE_FLAG,
                .executables = calloc(num_children, sizeof(struct Executable)),
                .num_executables = num_children,
        };
        pipe_sequence.executables[0] = first_command;
        for (size_t i = 0; i < cst_node.children[1]->num_children; i++)
            pipe_sequence.executables[i + 1] = abstract_command(*cst_node.children[1]->children[i]);
        return (Executable) {
            .type = EXEC_MULTI,
            .executable = (union ExecutableUnion) {
                .multi = pipe_sequence,
            },
        };
    }
    // Single command
    return first_command;
}

Executable abstract_command(CSTNode cst_node) {
    PARENT_NODE_COMPLIANCE(cst_node, COMMAND_PARSER, 1)
    cst_node = *cst_node.children[0]; // to 
    // 3 possibilities :
    switch (cst_node.type) {
        case SIMPLE_COMMAND_PARSER: // 1. Simple command
            return abstract_simple_command(cst_node);
        case SEQUENCE_PARSER: // 2. Compound command
            // TODO: handle redirect here
            return abstract_compound_command(cst_node);
        case FUNCTION_DEFINITION_PARSER:
            return abstract_function_definition(cst_node);
        default:
            print_error(OCERR_EXIT, "Command child has type %d\n", cst_node.type);
            exit(1);
    }
}

static struct ExecRedirect extract_cmd_suffix(CSTNode cmd_suffix, struct ExecCommandWord ** words, size_t * num_words);

static Executable abstract_simple_command(CSTNode cst_node)
{
    PARENT_NODE_COMPLIANCE(cst_node, SEQUENCE_PARSER, 2);
    assert(cst_node.children[1]->type == OPTIONAL_PARSER);
    if (cst_node.children[0]->type == CMD_NAME_PARSER) {
        if (!has_children(*cst_node.children[1])) {
            // Just one word
            struct ExecCommandWord * command_word = malloc(sizeof(struct ExecCommandWord));
            *command_word = (struct ExecCommandWord) {
                .type = ECW_WORD,
                .str = cst_node.children[0]->token->str,
                .str_len = cst_node.children[0]->token->str_len,
            };
            return (Executable) {
                .type = EXEC_COMMAND,
                .executable = (union ExecutableUnion) {
                    .command = (struct ExecCommand) {
                            .num_words = 1,
                            .words = command_word,
                    },
                },
            };
        }
        // Multiple words in command
        CSTNode cst_cmd_name = *cst_node.children[0], // to CMD_NAME_PARSER
                cst_optional = *cst_node.children[1]; // to OPTIONAL_PARSER
        PARENT_NODE_COMPLIANCE(cst_cmd_name, CMD_NAME_PARSER, 0)
        NODE_COMPLIANCE(cst_optional, OPTIONAL_PARSER, 1, CMD_SUFFIX_PARSER)
        CSTNode cmd_suffix = *cst_optional.children[0]; // to CMD_SUFFIX_PARSER
        struct ExecCommandWord * words = NULL;
        size_t num_words;
        struct ExecRedirect redirects = extract_cmd_suffix(cmd_suffix, &words, &num_words);
        // TODO: the rest
    }
}

Executable abstract_function_definition(CSTNode cst_node) {
    // TODO: empty function
    Executable func_def;
    return func_def;
}

static struct ExecRedirect extract_cmd_suffix(CSTNode cmd_suffix, struct ExecCommandWord ** words, size_t * num_words) {
    NODE_COMPLIANCE(cmd_suffix, CMD_SUFFIX_PARSER, 2, CHOICE_PARSER, REPETITION_PARSER)
    CSTNode first_suffix = *cmd_suffix.children[0],
            suffix_repetition = *cmd_suffix.children[1];
    PARENT_NODE_COMPLIANCE(first_suffix, CHOICE_PARSER, 1)
    // First suffix extraction/flattening
    first_suffix = *first_suffix.children[0]; // to TK_WORD_PARSER || IO_REDIRECT_PARSER
    _Bool first_suffix_is_word = first_suffix.children[0]->type == TK_WORD_PARSER; // _Bool bc want to be sure to have 0 or 1
    // Start counting of words and redirects
    *num_words = first_suffix_is_word;
    // Setup arrays of words and redirects
    *words = malloc(*num_words * sizeof(struct ExecCommandWord));
    struct ExecRedirect redirects = {
            .num_redirects = 0,
            .flags = NULL,
            .files = NULL,
            .executable = NULL,
    };
    // Fill first elements of these arrays
    if (first_suffix_is_word)
        *words[0] = (struct ExecCommandWord) {
                .type = ECW_WORD,
                .str = first_suffix.token->str,
                .str_len = first_suffix.token->str_len,
        };
    else {
        redirects.num_redirects++;
        redirects.flags = malloc(sizeof(unsigned long));
        unsigned long flags;
        char * file;
        imperfect_abstract_io_redirect(first_suffix, &flags, &file);
        redirects.flags[0] = flags;
        redirects.files[0] = file;
    }
    // fill with the rest
    for (size_t i = 0; i < suffix_repetition.num_children; i++) {
        CSTNode suffix_repetition_child = *suffix_repetition.children[i];
        PARENT_NODE_COMPLIANCE(suffix_repetition_child, CHOICE_PARSER, 1)
        suffix_repetition_child = *suffix_repetition_child.children[0];
        // add a word
        if (suffix_repetition_child.type == TK_WORD_PARSER) {
            (*num_words)++;
            *words = realloc(*words, (*num_words) * (sizeof(struct ExecCommandWord)));
            *words[*num_words - 1] = (struct ExecCommandWord) {
                    .type = ECW_WORD,
                    .str = suffix_repetition_child.token->str,
                    .str_len = suffix_repetition_child.token->str_len,
            };
        }
        // add a redirect
        else if (suffix_repetition_child.type == IO_REDIRECT_PARSER) {
            redirects.num_redirects++;
            redirects.flags = realloc(redirects.flags, redirects.num_redirects * sizeof(unsigned long));
            redirects.files = realloc(redirects.files, redirects.num_redirects * sizeof(char *));
            unsigned long flags;
            char *file;
            imperfect_abstract_io_redirect(first_suffix, &flags, &file);
            redirects.flags[redirects.num_redirects - 1] = flags;
            redirects.files[redirects.num_redirects - 1] = file;
        }
            // anything else
        else
            print_error(OCERR_EXIT, "Unexpected type for suffix-repetition-child '%d'\n", suffix_repetition_child.type);
    }
    return redirects;
}