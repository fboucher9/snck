/* See LICENSE for license details. */

/*

Module: snck_suggest.c

Description:

    List of suggestions for completion.

*/

/* OS headers */
#include "snck_os.h"

/* List */
#include "snck_list.h"

/* String */
#include "snck_string.h"

/* Context */
#include "snck_ctxt.h"

/* Module */
#include "snck_suggest.h"

/* Heap */
#include "snck_heap.h"

/* History */
#include "snck_history.h"

struct snck_suggest_node *
snck_suggest_node_create(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_suggest_node *
        p_suggest_node;

    p_suggest_node =
        (struct snck_suggest_node *)(
            snck_heap_realloc(
                p_ctxt,
                NULL,
                sizeof(
                    struct snck_suggest_node)));

    if (
        p_suggest_node)
    {
        snck_list_init(
            &(
                p_suggest_node->o_list));

        snck_string_init(
            p_ctxt,
            &(
                p_suggest_node->o_buf));
    }

    return
        p_suggest_node;

} /* snck_suggest_node_create() */

void
snck_suggest_node_destroy(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_node * const
        p_suggest_node)
{
    snck_list_join(
        &(
            p_suggest_node->o_list),
        &(
            p_suggest_node->o_list));

    snck_string_cleanup(
        p_ctxt,
        &(
            p_suggest_node->o_buf));

    snck_heap_realloc(
        p_ctxt,
        (void *)(
            p_suggest_node),
        0u);

} /* snck_suggest_node_destroy() */

void
snck_suggest_list_init(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list)
{
    (void)(
        p_ctxt);

    snck_list_init(
        &(
            p_suggest_list->o_list));

    p_suggest_list->i_count =
        0;

} /* snck_suggest_list_init() */

void
snck_suggest_list_cleanup(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list)
{
    struct snck_list *
        p_it;

    p_it = p_suggest_list->o_list.p_next;

    while (p_it != &(p_suggest_list->o_list))
    {
        struct snck_list * p_next = p_it->p_next;

        {
            struct snck_suggest_node * const p_suggest_node =
                (struct snck_suggest_node *)(
                    p_it);

            snck_suggest_node_destroy(
                p_ctxt,
                p_suggest_node);
        }

        p_it = p_next;
    }

} /* snck_suggest_list_cleanup() */

char
snck_suggest_list_add(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    struct snck_suggest_node * const
        p_suggest_node)
{
    char b_consumed;

    struct snck_list * p_it;

    char b_inserted;

    p_it = p_suggest_list->o_list.p_next;

    b_inserted = 0;

    b_consumed = 1;

    /* Remove bad duplicates */
    while (p_it != &(p_suggest_list->o_list))
    {
        struct snck_list * const p_next = p_it->p_next;

        struct snck_suggest_node * const p_suggest_temp =
            (struct snck_suggest_node *)(
                p_it);

        struct snck_string o_ref1;

        struct snck_string o_ref2;

        int i_duplicate;

        snck_string_init_ref_buffer(&(o_ref1), p_suggest_node->o_buf.p_buf + 16u, p_suggest_node->o_buf.i_buf_len - 16u);

        snck_string_init_ref_buffer(&(o_ref2), p_suggest_temp->o_buf.p_buf + 16u, p_suggest_temp->o_buf.i_buf_len - 16u);

        i_duplicate =
            snck_string_compare(
                &(o_ref1),
                &(o_ref2));

        if (0 == i_duplicate)
        {
            int const i_compare =
                snck_string_compare(
                    &(p_suggest_node->o_buf),
                    &(p_suggest_temp->o_buf));

            if (0 > i_compare)
            {
                snck_suggest_node_destroy(p_ctxt, p_suggest_temp);

                p_suggest_list->i_count --;
            }
            else
            {
                b_consumed = 0;

                b_inserted = 1;
            }
        }

        p_it = p_next;
    }

    p_it = p_suggest_list->o_list.p_next;

    while (!b_inserted && (p_it != &(p_suggest_list->o_list)))
    {
        struct snck_suggest_node * const p_suggest_temp =
            (struct snck_suggest_node *)(
                p_it);

        {
            int const i_compare =
                snck_string_compare(
                    &(p_suggest_node->o_buf),
                    &(p_suggest_temp->o_buf));

            if (0 == i_compare)
            {
                b_consumed = 0;

                b_inserted = 1;
            }
            else if (0 > i_compare)
            {
                snck_list_join(
                    &(
                        p_suggest_node->o_list),
                    &(
                        p_suggest_temp->o_list));

                b_inserted = 1;

                p_suggest_list->i_count ++;

                /* Scan after to remove duplicates */
            }
            else
            {
                p_it = p_it->p_next;
            }
        }
    }

    if (!b_inserted)
    {
        snck_list_join(
            &(
                p_suggest_node->o_list),
            &(
                p_suggest_list->o_list));

        p_suggest_list->i_count ++;
    }

    if (p_suggest_list->i_count > 128)
    {
        /* Remove the last */
        struct snck_suggest_node * p_suggest_last =
            (struct snck_suggest_node *)(
                p_suggest_list->o_list.p_prev);

        /* Do not remove empty list */
        if (&(p_suggest_last->o_list) != &(p_suggest_list->o_list))
        {
            /* Special case for remove of item just inserted */
            if (b_consumed && (p_suggest_last == p_suggest_node))
            {
                snck_list_join(
                    &(
                        p_suggest_node->o_list),
                    &(
                        p_suggest_node->o_list));

                b_consumed = 0;
            }
            else
            {
                snck_suggest_node_destroy(p_ctxt, p_suggest_last);

                p_suggest_list->i_count --;
            }
        }
    }

    return b_consumed;

} /* snck_suggest_list_add() */

static
void
snck_suggest_from_history_line(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    size_t const
        i_history_index,
    struct snck_history_line const * const
        p_history_line,
    struct snck_string const * const
        p_wild)
{
    int i_score;

    i_score = snck_fuzzy_compare(&(p_history_line->o_buf), p_wild);

    if (0 != i_score)
    {
        struct snck_suggest_node * p_suggest_node;

        p_suggest_node = snck_suggest_node_create(p_ctxt);

        if (p_suggest_node)
        {
            char b_consumed;

            if (snck_string_resize(p_ctxt, &(p_suggest_node->o_buf), p_history_line->o_buf.i_buf_len + 16u + 1u))
            {
                sprintf(p_suggest_node->o_buf.p_buf, "%08x%08x%.*s",
                    (unsigned int)(i_score),
                    (unsigned int)(i_history_index),
                    (int)(p_history_line->o_buf.i_buf_len),
                    p_history_line->o_buf.p_buf);

                p_suggest_node->o_buf.i_buf_len = strlen(p_suggest_node->o_buf.p_buf);

                b_consumed = snck_suggest_list_add(p_ctxt, p_suggest_list, p_suggest_node);
            }
            else
            {
                b_consumed = 0;
            }

            if (!b_consumed)
            {
                snck_suggest_node_destroy(p_ctxt, p_suggest_node);
            }
        }
    }

}

static
size_t
snck_suggest_from_history_list(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    size_t
        i_history_index,
    struct snck_list * const
        p_list,
    struct snck_string const * const
        p_wild)
{
    struct snck_list const * p_it;

    p_it = p_list->p_prev;

    while (p_it != p_list)
    {
        struct snck_history_line const * p_history_line = (struct snck_history_line const *)(p_it);

        snck_suggest_from_history_line(
            p_ctxt,
            p_suggest_list,
            i_history_index,
            p_history_line,
            p_wild);

        i_history_index ++;

        p_it = p_it->p_prev;
    }

    return i_history_index;

}

void
snck_suggest_from_history(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    struct snck_string const * const
        p_wild)
{
    size_t i_history_index;

    i_history_index = 1;

    /* Suggest from cache first ... */
    i_history_index =
        snck_suggest_from_history_list(
            p_ctxt,
            p_suggest_list,
            i_history_index,
            &(p_ctxt->p_history->o_cache),
            p_wild);

    snck_history_load(p_ctxt);

    i_history_index =
        snck_suggest_from_history_list(
            p_ctxt,
            p_suggest_list,
            i_history_index,
            &(p_ctxt->p_history->o_list),
            p_wild);

    snck_history_unload(p_ctxt);

} /* snck_suggest_from_history() */

static
void
snck_suggest_from_lastword_node(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    struct snck_history_line const * const
        p_history_line,
    int const
        i_history_index,
    char const * const
        buf,
    size_t const
        buf_len,
    size_t const
        pos)
{
    int i_len;

    i_len = (int)(p_history_line->o_buf.i_buf_len);

    while ((i_len > 0) && (' ' == p_history_line->o_buf.p_buf[i_len-1]))
    {
        i_len--;
    }
    while ((i_len > 0) && (' ' != p_history_line->o_buf.p_buf[i_len-1]))
    {
        i_len--;
    }

    {
        struct snck_suggest_node * p_suggest_node;

        p_suggest_node = snck_suggest_node_create(p_ctxt);

        if (p_suggest_node)
        {
            char b_consumed;

            if (snck_string_resize(p_ctxt, &(p_suggest_node->o_buf), buf_len + p_history_line->o_buf.i_buf_len - i_len + 16u + 1u))
            {
                sprintf(p_suggest_node->o_buf.p_buf, "%08x%08x%.*s%.*s%.*s",
                    (unsigned int)(1u),
                    (unsigned int)(i_history_index),
                    (int)(pos),
                    buf,
                    (int)(p_history_line->o_buf.i_buf_len - i_len),
                    p_history_line->o_buf.p_buf + i_len,
                    (int)(buf_len - pos),
                    buf + pos);

                p_suggest_node->o_buf.i_buf_len = strlen(p_suggest_node->o_buf.p_buf);

                b_consumed = snck_suggest_list_add(p_ctxt, p_suggest_list, p_suggest_node);
            }
            else
            {
                b_consumed = 0;
            }

            if (!b_consumed)
            {
                snck_suggest_node_destroy(p_ctxt, p_suggest_node);
            }
        }
    }
}

static
int
snck_suggest_from_lastword_list(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    struct snck_list * const
        p_list,
    int
        i_history_index,
    char const * const
        buf,
    size_t const
        buf_len,
    size_t const
        pos)
{
    /* Insert word at pos */
    struct snck_list const * p_it;

    p_it = p_list->p_prev;

    while (p_it != p_list)
    {
        struct snck_history_line const * p_history_line = (struct snck_history_line const *)(p_it);

        snck_suggest_from_lastword_node(
            p_ctxt,
            p_suggest_list,
            p_history_line,
            i_history_index,
            buf,
            buf_len,
            pos);

        p_it = p_it->p_prev;

        i_history_index ++;
    }

    return i_history_index;
}

void
snck_suggest_from_lastword(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    char const * const
        buf,
    size_t const
        buf_len,
    size_t const
        pos)
{
    int i_history_index;

    i_history_index = 1;

    i_history_index =
        snck_suggest_from_lastword_list(
            p_ctxt,
            p_suggest_list,
            &(p_ctxt->p_history->o_cache),
            i_history_index,
            buf,
            buf_len,
            pos);

    snck_history_load(p_ctxt);

    i_history_index =
        snck_suggest_from_lastword_list(
            p_ctxt,
            p_suggest_list,
            &(p_ctxt->p_history->o_list),
            i_history_index,
            buf,
            buf_len,
            pos);

    snck_history_unload(p_ctxt);

} /* snck_suggest_from_lastword() */

/* end-of-file: snck_suggest.c */
