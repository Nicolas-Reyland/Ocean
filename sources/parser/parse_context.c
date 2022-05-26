//
// Created on 23/05/2022.
//

#include <stdlib.h>
#include "parser/parse_context.h"

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

void parser_commit_single_token(void * void_ctx, Combinator * p, void * void_parent, void * void_child, int pos0)
{
    ParseContext * ctx = void_ctx;
    CSTNode * parent = void_parent;
    CSTNode * child = void_child;
    child->token = &ctx->tokens[pos0];
    child->children = NULL;
    child->num_children = 0;
    child->type = p->type;
    append_cst_to_children(parent, child);
}