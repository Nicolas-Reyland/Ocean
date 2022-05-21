//
// Created on 12/05/2022.
//

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parser/parser.h"
#include "parser/cst.h"

#ifdef OCEAN_DEBUG_MACRO
#include <stdio.h>
#endif

// Parse Context
static void parser_ctx_pos_push(ParseContext * ctx)
{
    ctx->pos_stack.push(&ctx->pos_stack, ctx->pos);
}

static void parser_ctx_pos_pop(ParseContext * ctx)
{
    ctx->pos = ctx->pos_stack.pop(&ctx->pos_stack);
}

ParseContext create_parse_ctx(Token * tokens, size_t num_tokens) {
    ParseContext ctx = {
            .tokens = tokens,
            .num_tokens = num_tokens,
            .pos = 0,
            .pos_stack = create_stack(),
            .volatile_parser_results = create_stack(),
            .cst = (CSTNode) {
                    .children = NULL,
                    .num_children = 0,
                    .type = CST_NONE,
                    .token = NULL,
            },
            .last_leaf = NULL,
            .pos_push = parser_ctx_pos_push,
            .pos_pop = parser_ctx_pos_pop,
    };
    ctx.pos_stack.push(&ctx.pos_stack, 0);
    return ctx;
}

// Utils for Parse Context
void append_cst_to_children(CSTNode * parent, CSTNode * child)
{
    if (parent->children == NULL || parent->num_children == 0) {
        parent->children = malloc(sizeof(CSTNode *));
        parent->num_children = 0;
    } else {
        parent->children = realloc(parent->children, (parent->num_children + 1) * sizeof(CSTNode *));
    }
    parent->children[parent->num_children++] = child;
}

void parser_commit_single_token(ParseContext * ctx, Parser * p, CSTNode * parent, CSTNode * child, int pos0)
{
    child->token = &ctx->tokens[pos0];
    child->children = NULL;
    child->num_children = 0;
    child->type = p->type;
    append_cst_to_children(parent, child);
}

// Parser
static bool execute_parser(ParseContext * ctx, Parser * p)
{
    if (p->decorator == NULL)
        return p->parse_f(ctx, p);
    else
        return p->decorator(ctx, p);
}

static bool parser_parse(ParseContext * ctx, Parser * p)
{
    // Check for token availability
    if (ctx->pos >= ctx->num_tokens)
        return false;

#ifdef OCEAN_DEBUG_MACRO
    static int depth = 0;
    depth++;

    for (int i = 0; i < depth; i++) putchar('\t');
    printf("p : %s '%s'\n",
           CONCRETE_NODE_TYPE_STRING[p->type],
           ctx->tokens[ctx->pos].str[0] == '\n' ? "\\n" : ctx->tokens[ctx->pos].str
    );
#endif

    // for the first call of parse function
    if (ctx->last_leaf == NULL)
        ctx->last_leaf = &ctx->cst;
    // setup local variables
    CSTNode * prev_leaf = ctx->last_leaf;
    CSTNode * cur_leaf = malloc(sizeof(CSTNode));
    cur_leaf->type = p->type;
    cur_leaf->children = NULL;
    cur_leaf->num_children = 0;
    cur_leaf->token = NULL;
    ctx->last_leaf = cur_leaf;
    int pos0 = ctx->pos;
    ctx->pos_push(ctx);
    // parse next token
    bool success = execute_parser(ctx, p);
    if (success) {
        // Add CST Node(s)
        assert(p->commit != NULL);
        p->commit(ctx, p, prev_leaf, cur_leaf, pos0);
    } else {
        ctx->pos_pop(ctx);
        free_cst_node(cur_leaf);
    }
    ctx->last_leaf = prev_leaf;
#ifdef OCEAN_DEBUG_MACRO
    depth--;
#endif
    return success;
}

Parser parser_create(bool (*parse_f)(ParseContext *, struct Parser *), void (*commit)(ParseContext *, struct Parser *, CSTNode *, CSTNode *, int))
{
    return (Parser) {
        .type = CST_NONE,
        .sub_parsers = NULL,
        .num_sub_parsers = 0,
        .decorator = NULL,
        .parse_f = parse_f,
        .parse = parser_parse,
        .commit = commit,
        .parser_generator = NULL,
    };
}

static bool forward_ref_decorator(ParseContext * ctx, Parser * generator)
{
    // Generate origin parser
    Parser origin = generator->parser_generator();
    // Keep type on local stack
    ConcreteNodeType generator_type = generator->type;
    // Overwrite self with origin
    memcpy(generator, &origin, sizeof(Parser));
    // Check for special type
    if (generator_type != CST_GENERATOR)
        generator->type = generator_type;
    // Execute final parser of self
    return execute_parser(ctx, generator);
}

Parser parser_forward_ref(struct Parser (*parser_generator)(void))
{
    Parser p = parser_create(NULL, NULL);
    p.type = CST_GENERATOR;
    p.decorator = forward_ref_decorator;
    p.parser_generator = parser_generator;
    return p;
}

// Inverted
static bool parser_inverted_parse_f(ParseContext * ctx, Parser * p)
{
    return !p->sub_parsers[0].parse(ctx, &p->sub_parsers[0]);
}

Parser parser_inverted(Parser p)
{
    Parser inverted = typed_parser(
            parser_create(parser_inverted_parse_f, parser_commit_single_token),
            CST_INVERTED);
    inverted.num_sub_parsers = 1;
    inverted.sub_parsers = malloc(sizeof(Parser));
    inverted.sub_parsers[0] = p;
    return inverted;
}

// Sequence
static bool parser_sequence_parse_f(ParseContext * ctx, Parser * p)
{
    CSTNode * parent = NULL, * seq_child = NULL;
    parent = ctx->last_leaf;
    seq_child = malloc(sizeof(CSTNode));
    seq_child->children = NULL;
    seq_child->num_children = 0;
    seq_child->token = NULL;
    seq_child->type = CST_SEQUENCE_UNIT;
    ctx->last_leaf = seq_child;

    int pos0 = ctx->pos;
    for (size_t i = 0; i < p->num_sub_parsers; i++) {
        Parser sub_parser = p->sub_parsers[i];
        if (!sub_parser.parse(ctx, &sub_parser)) {
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

static void parser_sequence_commit(ParseContext * ctx, Parser * p, CSTNode * parent, CSTNode * child, int pos0)
{
    (void)ctx;
    (void)pos0;
    child->token = NULL;
    child->type = p->type;
    append_cst_to_children(parent, child);
}

Parser parser_sequence(unsigned int count, ...)
{
    Parser * parsers = calloc(count, sizeof(Parser));

    va_list args;
    va_start(args, count);

    for (unsigned int i = 0; i < count; i++)
    {
        parsers[i] = va_arg(args, Parser);
    }

    va_end(args);

    // Create Parser and return it
    Parser parser = parser_create(parser_sequence_parse_f, parser_sequence_commit);
    parser.sub_parsers = parsers;
    parser.num_sub_parsers = count;
    parser.type = CST_SEQUENCE;

    return parser;
}

// Repetition
static bool parser_repetition_decorator(ParseContext * ctx, Parser * p)
{
    int count = 0;
    bool success = p->parse_f(ctx, p);
    for (; success; ) {
        count++;
        success = p->parse_f(ctx, p);
    }
    return true;
}

static void parser_repetition_commit(ParseContext * ctx, Parser * p, CSTNode * parent, CSTNode * child, int pos0)
{
    (void)ctx;
    (void)p;
    (void)pos0;
    child->token = NULL;
    append_cst_to_children(parent, child);
}

Parser parser_repetition(Parser p) {
    p.decorator = parser_repetition_decorator;
    p.commit = parser_repetition_commit;
    p.type = CST_REPETITION;
    return p;
}

// Optional
static bool parser_optional_decorator(ParseContext * ctx, Parser * p)
{
    int pos0 = ctx->pos;
    if (!p->parse_f(ctx, p))
        ctx->pos = pos0;
    return true;
}

Parser parser_optional(Parser p) {
    p.decorator = parser_optional_decorator;
    p.type = CST_OPTIONAL;
    return p;
}

// Choice
static bool parser_choice_parse_f(ParseContext * ctx, Parser * p)
{
    for (size_t i = 0; i < p->num_sub_parsers; i++) {
        Parser sub_parser = p->sub_parsers[i];
        if (sub_parser.parse(ctx, &sub_parser)) {
            ctx->volatile_parser_results.push(&ctx->volatile_parser_results, 1);
            return true;
        }
    }
    ctx->volatile_parser_results.push(&ctx->volatile_parser_results, 0);
    return false;
}

static void parser_choice_commit(ParseContext * ctx, Parser * p, CSTNode * parent, CSTNode * child, int pos0)
{
    (void)p;
    (void)pos0;
    int success = ctx->volatile_parser_results.pop(&ctx->volatile_parser_results);
    // success should be 0 or 1
    if (success) {
        append_cst_to_children(parent, child);
    }
}

Parser parser_choice(unsigned int count, ...)
{
    Parser * parsers = calloc(count, sizeof(Parser));

    va_list args;
    va_start(args, count);

    for (unsigned int i = 0; i < count; i++)
    {
        parsers[i] = va_arg(args, Parser);
    }

    va_end(args);

    // Create Parser and return it
    Parser parser = parser_create(parser_choice_parse_f, parser_choice_commit);
    parser.sub_parsers = parsers;
    parser.num_sub_parsers = count;
    parser.type = CST_CHOICE;

    return parser;
}

// Separated Parsers
static bool separated_parser_decorator(ParseContext * ctx, Parser * p)
{
    if (!p->parse_f(ctx, p))
        return false;
    // Remove the sequence_unit cst node
    assert(ctx->last_leaf->num_children == 1);
    ConcreteNodeType last_leaf_type = ctx->last_leaf->type;
    CSTNode * seq_unit = ctx->last_leaf->children[0];
    memcpy(ctx->last_leaf, seq_unit, sizeof(CSTNode));
    ctx->last_leaf->type = last_leaf_type;
    free(seq_unit);
    return true;
}

Parser parser_separated(Parser p, Parser separator)
{
    Parser separated_p = typed_parser(
            parser_sequence(2,
                            p,
                            typed_parser(
                                    parser_repetition(
                                            parser_sequence(2,
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
static bool lookahead_parser_parse(ParseContext * ctx, Parser * p)
{
    ParseContext dummy_ctx = *ctx;
    size_t last_leaf_num_children = ctx->last_leaf->num_children;
    bool result = execute_parser(&dummy_ctx, &p->sub_parsers[0]);
    size_t last_leaf_num_children_after = dummy_ctx.last_leaf->num_children;
    if (last_leaf_num_children != dummy_ctx.last_leaf->num_children) {
        // remove the children that got added during the lookahead step
        for (size_t i = last_leaf_num_children; i < last_leaf_num_children_after; i++)
            free_cst_node(ctx->last_leaf->children[i]);
        ctx->last_leaf->num_children = last_leaf_num_children;
        ctx->last_leaf->children = realloc(ctx->last_leaf->children,  last_leaf_num_children * sizeof(CSTNode *));
    }
    return result;
}

Parser parser_lookahead(Parser p) {
    Parser lookahead_p = parser_create(NULL, NULL);
    lookahead_p.num_sub_parsers = 1;
    lookahead_p.sub_parsers = malloc(sizeof(Parser));
    lookahead_p.sub_parsers[0] = p;
    // Yes, overwriting 'parse' itself
    lookahead_p.parse = lookahead_parser_parse;
    return typed_parser(
            lookahead_p,
            CST_LOOKAHEAD);
}

// Typed (misc)
Parser typed_parser(Parser p, ConcreteNodeType type) {
    p.type = type;
    return p;
}
