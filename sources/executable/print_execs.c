//
// Created by Nicolas Reyland on 8/29/22.
//

#include <stdio.h>
#include "executable/print_execs.h"
#include "misc/output.h"

static void print_depth_prefix(int depth);

void print_exec_command(struct ExecCommand command, int depth)
{
    print_depth_prefix(depth);
    printf("Command: ");
    for (size_t i = 0; i < command.num_words; i++)
        printf("'%s' ", command.words[i].str);
    putchar('\n');
}

void print_exec_for_loop(struct ExecForLoop for_loop, int depth)
{
    // elements
    print_depth_prefix(depth++);
    printf("ForLoop: %s in [", for_loop.var_name);
    for (size_t i = 0; i < for_loop.wordlist_len; i++)
        printf("%s, ", for_loop.wordlist[i]);
    // body
    printf("] -> {\n");
    for (size_t i = 0; i < for_loop.num_body_commands; i++)
        traverse_executable(for_loop.body_commands[i], depth);
    print_depth_prefix(depth - 1);
    printf("}\n");
}

void print_exec_while_loop(struct ExecWhileLoop while_loop, int depth)
{
    // conditions
    print_depth_prefix(depth++);
    printf("WhileLoop: (\n");
    for (size_t i = 0; i < while_loop.num_condition_commands; i++)
        traverse_executable(while_loop.condition_commands[i], depth);
    // body
    print_depth_prefix(depth - 1);
    printf(") -> {\n");
    for (size_t i = 0; i < while_loop.num_body_commands; i++)
        traverse_executable(while_loop.body_commands[i], depth);
    print_depth_prefix(depth - 1);
    printf("}\n");
}

void print_exec_until_loop(struct ExecUntilLoop until_loop, int depth)
{
    // conditions
    print_depth_prefix(depth++);
    printf("UntilLoop: !(\n");
    for (size_t i = 0; i < until_loop.num_condition_commands; i++)
        traverse_executable(until_loop.condition_commands[i], depth);
    // body
    print_depth_prefix(depth - 1);
    printf(") -> {\n");
    for (size_t i = 0; i < until_loop.num_body_commands; i++)
        traverse_executable(until_loop.body_commands[i], depth);
    print_depth_prefix(depth - 1);
    printf("}\n");
}

void print_exec_case(struct ExecCase case_clause, int depth)
{
    // value
    print_depth_prefix(depth++);
    printf("Case: match '%s' with:\n", case_clause.word);
    // Patterns-Clauses
    for (size_t i = 0; i < case_clause.num_patterns; i++) {
        struct ExecMultiExecutables pattern_clause = case_clause.cases[i].executable.multi;
        // Patterns (only, not clause)
        print_depth_prefix(depth);
        printf("Pattern: [");
        size_t num_pattern_executables = pattern_clause.num_executables;
        struct ExecCommand pattern_fake_command = pattern_clause.executables[num_pattern_executables - 1].executable.command;
        size_t num_local_patterns = pattern_fake_command.num_words;
        for (size_t j = 0; j < num_local_patterns; j++) {
            // Print a pattern
            struct ExecCommandWord pattern_word = pattern_fake_command.words[j];
            printf("%s, ", pattern_word.str);
        }
        printf("] -> {\n");
        // Print the body
        for (size_t k = 0; k < num_pattern_executables - 1; k++)
            print_exec_command(pattern_clause.executables[k].executable.command, depth + 1);
        print_depth_prefix(depth);
        printf("}\n");
    }
}

void print_exec_if(struct ExecIf if_clause, int depth)
{
    // conditions
    print_depth_prefix(depth++);
    printf("If: (\n");
    for (size_t i = 0; i < if_clause.num_condition_commands; i++)
        traverse_executable(if_clause.condition_commands[i], depth);
    // body
    print_depth_prefix(depth - 1);
    printf(") -> {\n");
    for (size_t i = 0; i < if_clause.num_body_commands; i++)
        traverse_executable(if_clause.body_commands[i], depth + 1);
    print_depth_prefix(depth - 1);
    if (if_clause.else_commands == NULL) {
        printf("}\n");
        return;
    }
    // else
    printf("} else {\n");
    for (size_t i = 0; i < if_clause.num_else_commands; i++)
        traverse_executable(if_clause.else_commands[i], depth + 1);
    print_depth_prefix(depth - 1);
    printf("}\n");
}

void print_exec_multi(struct ExecMultiExecutables multi, int depth)
{
    // executables
    print_depth_prefix(depth++);
    printf("Multi: %lu | %d\n", multi.num_executables, multi.execution_flags);
    for (size_t i = 0; i < multi.num_executables; i++)
        traverse_executable(multi.executables[i], depth);
}

void print_exec_redirect(struct ExecRedirect redirect, int depth)
{
    // redirect
    print_depth_prefix(depth++);
    printf("Redirect:\n");
    // each redirect
    for (size_t i = 0; i < redirect.num_redirects; i++) {
        // flag
        print_depth_prefix(depth);
        printf("mode: '%lu'", redirect.flags[i]);
        // file
        printf("\t\tfile: '%s'\n", redirect.files[i]);
    }
    // executable
    print_depth_prefix(depth);
    printf("for executable:\n");
    traverse_executable(*redirect.executable, depth + 1);
}

static void print_depth_prefix(int depth)
{
    for (int i = 0; i < depth; i++)
        putchar('\t');
}
