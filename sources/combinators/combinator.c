//
// Created on 12/05/2022.
//

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "combinators/combinator.h"
#include "parser/cst.h"
#include "parser/parse_context.h"

// Combinator
Combinator cmb_create(
        bool (*exec)(void *, struct Combinator *),
        bool (*exec_f)(void *, struct Combinator *),
        void (*commit)(void *, struct Combinator *, struct CSTNode *, struct CSTNode *, int)
)
{
    return (Combinator) {
            .type = CST_NONE,
            .sub_combinators = NULL,
            .num_sub_combinators = 0,
            .decorator = NULL,
            .exec_f = exec_f,
            .exec = exec,
            .commit = commit,
            .cmb_generator = NULL,
    };
}

bool execute_cmb(void * ctx, Combinator * p)
{
    if (p->decorator == NULL)
        return p->exec_f(ctx, p);
    else
        return p->decorator(ctx, p);
}

static bool forward_ref_decorator(void * void_ctx, Combinator * generator)
{
    // Generate origin parser
    Combinator origin = generator->cmb_generator();
    // Keep type on local stack
    ConcreteNodeType generator_type = generator->type;
    // Overwrite self with origin
    memcpy(generator, &origin, sizeof(Combinator));
    // Check for special type
    if (generator_type != CST_GENERATOR)
        generator->type = generator_type;
    // Execute final parser of self
    return execute_cmb(void_ctx, generator);
}

Combinator cmb_forward_ref(cmb_exec_function cmb_exec, struct Combinator (*cmb_generator)(void))
{
    Combinator p = cmb_create(cmb_exec, NULL, NULL);
    p.type = CST_GENERATOR;
    p.decorator = forward_ref_decorator;
    p.cmb_generator = cmb_generator;
    return p;
}

// Inverted
static bool parser_inverted_parse_f(void * void_ctx, Combinator * p)
{
    // Cast ctx
    ParseContext * ctx = void_ctx;
    return !p->sub_combinators[0].exec(ctx, &p->sub_combinators[0]);
}

Combinator cmb_inverted(Combinator p)
{
    Combinator inverted = typed_cmb(
            cmb_create(p.exec, parser_inverted_parse_f, parser_commit_single_token),
            CST_INVERTED);
    inverted.num_sub_combinators = 1;
    inverted.sub_combinators = malloc(sizeof(Combinator));
    inverted.sub_combinators[0] = p;
    return inverted;
}

// Sequence
static bool parser_sequence_parse_f(void * void_ctx, Combinator * p)
{
    ParseContext * ctx = void_ctx;
    CSTNode * parent = NULL, * seq_child = NULL;
    parent = ctx->last_leaf;
    seq_child = malloc(sizeof(CSTNode));
    seq_child->children = NULL;
    seq_child->num_children = 0;
    seq_child->token = NULL;
    seq_child->type = CST_SEQUENCE_UNIT;
    ctx->last_leaf = seq_child;

    int pos0 = ctx->pos;
    for (size_t i = 0; i < p->num_sub_combinators; i++) {
        Combinator sub_parser = p->sub_combinators[i];
        if (!sub_parser.exec(ctx, &sub_parser)) {
            ctx->pos = pos0;
            // free sequence node
            free_cst_node(seq_child);
            // re-set the last leaf
            ctx->last_leaf = parent;
            return false;
        }
    }

    // manually commit the sequence child to parent node
    append_cst_to_children(parent, seq_child);
    // re-set the last leaf
    ctx->last_leaf = parent;
    return true;
}

static void parser_sequence_commit(void * void_ctx, Combinator * p, CSTNode * parent, CSTNode * child, int pos0)
{
    (void)void_ctx;
    (void)pos0;
    child->token = NULL;
    child->type = p->type;
    append_cst_to_children(parent, child);
}

Combinator cmb_sequence(cmb_exec_function cmb_exec, unsigned int count, ...)
{
    Combinator * parsers = calloc(count, sizeof(Combinator));

    va_list args;
    va_start(args, count);

    for (unsigned int i = 0; i < count; i++)
    {
        parsers[i] = va_arg(args, Combinator);
    }

    va_end(args);

    // Create Combinator and return it
    Combinator parser = cmb_create(cmb_exec, parser_sequence_parse_f, parser_sequence_commit);
    parser.sub_combinators = parsers;
    parser.num_sub_combinators = count;
    parser.type = CST_SEQUENCE;

    return parser;
}

// Repetition
static bool parser_repetition_decorator(void * ctx, Combinator * p)
{
    int count = 0;
    bool success = p->exec_f(ctx, p);
    for (; success; ) {
        count++;
        success = p->exec_f(ctx, p);
    }
    return true;
}

static void parser_repetition_commit(void * void_ctx, Combinator * p, CSTNode * parent, CSTNode * child, int pos0)
{
    (void)void_ctx;
    (void)p;
    (void)pos0;
    child->token = NULL;
    append_cst_to_children(parent, child);
}

Combinator cmb_repetition(Combinator p) {
    p.decorator = parser_repetition_decorator;
    p.commit = parser_repetition_commit;
    p.type = CST_REPETITION;
    return p;
}

// Optional
static bool parser_optional_decorator(void * void_ctx, Combinator * p)
{
    ParseContext * ctx = void_ctx;
    int pos0 = ctx->pos;
    if (!p->exec_f(ctx, p))
        ctx->pos = pos0;
    return true;
}

Combinator cmb_optional(Combinator p) {
    p.decorator = parser_optional_decorator;
    p.type = CST_OPTIONAL;
    return p;
}

// Choice
static bool parser_choice_parse_f(void * void_ctx, Combinator * p)
{
    ParseContext * ctx = void_ctx;
    for (size_t i = 0; i < p->num_sub_combinators; i++) {
        Combinator sub_parser = p->sub_combinators[i];
        if (sub_parser.exec(ctx, &sub_parser)) {
            ctx->volatile_parser_results.push(&ctx->volatile_parser_results, 1);
            return true;
        }
    }
    ctx->volatile_parser_results.push(&ctx->volatile_parser_results, 0);
    return false;
}

static void parser_choice_commit(void * void_ctx, Combinator * p, CSTNode * parent, CSTNode * child, int pos0)
{
    (void)p;
    (void)pos0;
    // Cast ctx
    ParseContext * ctx = void_ctx;
    int success = ctx->volatile_parser_results.pop(&ctx->volatile_parser_results);
    // success should be 0 or 1
    if (success) {
        append_cst_to_children(parent, child);
    }
}

Combinator cmb_choice(cmb_exec_function cmb_exec, unsigned int count, ...)
{
    Combinator * parsers = calloc(count, sizeof(Combinator));

    va_list args;
    va_start(args, count);

    for (unsigned int i = 0; i < count; i++)
    {
        parsers[i] = va_arg(args, Combinator);
    }

    va_end(args);

    // Create Combinator and return it
    Combinator parser = cmb_create(cmb_exec, parser_choice_parse_f, parser_choice_commit);
    parser.sub_combinators = parsers;
    parser.num_sub_combinators = count;
    parser.type = CST_CHOICE;

    return parser;
}

// Separated Parsers
static bool separated_parser_decorator(void * void_ctx, Combinator * p)
{
    if (!p->exec_f(void_ctx, p))
        return false;
    // Cast ctx
    ParseContext * ctx = void_ctx;
    // Remove the sequence_unit cst node
    assert(ctx->last_leaf->num_children == 1);
    ConcreteNodeType last_leaf_type = ctx->last_leaf->type;
    CSTNode * seq_unit = ctx->last_leaf->children[0];
    memcpy(ctx->last_leaf, seq_unit, sizeof(CSTNode));
    ctx->last_leaf->type = last_leaf_type;
    free(seq_unit);
    return true;
}

Combinator cmb_separated(cmb_exec_function cmb_exec, Combinator p, Combinator separator)
{
    Combinator separated_p = typed_cmb(
            cmb_sequence(cmb_exec, 2,
                         p,
                         typed_cmb(
                                 cmb_repetition(
                                         cmb_sequence(cmb_exec,2,
                                                      separator,
                                                      p
                                         )),
                                 CST_SEPARATED_REPETITION)),
            CST_SEPARATED);
    assert(separated_p.decorator == NULL);
    separated_p.decorator = separated_parser_decorator;
    return separated_p;
}

// Lookahead
static bool lookahead_parser_parse(void * void_ctx, Combinator * p)
{
    // Cast ctx
    ParseContext * ctx = void_ctx;
    ParseContext dummy_ctx = *ctx;
    size_t last_leaf_num_children = ctx->last_leaf->num_children;
    bool result = execute_cmb(&dummy_ctx, &p->sub_combinators[0]);
    size_t last_leaf_num_children_after = dummy_ctx.last_leaf->num_children;
    // free up all that was added
    if (last_leaf_num_children != dummy_ctx.last_leaf->num_children) {
        // remove the children that got added during the lookahead step
        for (size_t i = last_leaf_num_children; i < last_leaf_num_children_after; i++)
            free_cst_node(ctx->last_leaf->children[i]);
        ctx->last_leaf->num_children = last_leaf_num_children;
        ctx->last_leaf->children = realloc(ctx->last_leaf->children,  last_leaf_num_children * sizeof(CSTNode *));
    }
    return result;
}

Combinator cmb_lookahead(cmb_exec_function cmb_exec, Combinator p) {
    Combinator lookahead_p = cmb_create(cmb_exec, NULL, NULL);
    lookahead_p.num_sub_combinators = 1;
    lookahead_p.sub_combinators = malloc(sizeof(Combinator));
    lookahead_p.sub_combinators[0] = p;
    // Yes, overwriting 'exec' itself
    lookahead_p.exec = lookahead_parser_parse;
    return typed_cmb(
            lookahead_p,
            CST_LOOKAHEAD);
}

// Typed (misc)
Combinator typed_cmb(Combinator p, int type) {
    p.type = type;
    return p;
}
