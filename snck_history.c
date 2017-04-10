/* See LICENSE for license details. */

/*

Module: snck_history.c

Description:

    Command line history file.

*/

/* OS Headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Context */
#include "snck_ctxt.h"

/* List */
#include "snck_list.h"

/* String */
#include "snck_string.h"

/* Module */
#include "snck_history.h"

/* Heap */
#include "snck_heap.h"

/* Info */
#include "snck_info.h"

static
struct snck_history_line *
snck_history_line_create(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_history_line * p_history_line;

    p_history_line =
        (struct snck_history_line *)(
            snck_heap_realloc(
                p_ctxt,
                NULL,
                sizeof(struct snck_history_line)));

    if (p_history_line)
    {
        snck_list_init(
            &(
                p_history_line->o_list));

        snck_string_init(
            p_ctxt,
            &(
                p_history_line->o_buf));
    }

    return p_history_line;

} /* snck_history_line_create() */

static
void
snck_history_line_destroy(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_history_line * const
        p_history_line)
{
    snck_list_join(
        &(
            p_history_line->o_list),
        &(
            p_history_line->o_list));

    snck_string_cleanup(
        p_ctxt,
        &(
            p_history_line->o_buf));

    snck_heap_realloc(
        p_ctxt,
        (void *)(
            p_history_line),
        0);

} /* snck_history_line_destroy() */

static
void
snck_history_empty_list(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_list * const
        p_list)
{
    struct snck_list * p_it;

    p_it = p_list->p_next;

    while (p_it != p_list)
    {
        struct snck_list * p_next = p_it->p_next;

        snck_history_line_destroy(
            p_ctxt,
            (struct snck_history_line *)(p_it));

        p_it = p_next;
    }
}

char
snck_history_init(
    struct snck_ctxt const * const
        p_ctxt)
{
    char b_result;

    struct snck_history * const p_history =
        p_ctxt->p_history;

    snck_list_init(
        &(
            p_history->o_list));

    snck_string_init(
        p_ctxt,
        &(
            p_history->o_name));

    snck_list_init(
        &(
            p_history->o_cache));

    p_history->i_cache_len =
        0u;

    {
        static char a_histfile_suffix[] = { '/', '.', 's', 'n', 'c', 'k', 'h', 'i', 's', 't' };

        snck_string_copy(
            p_ctxt,
            &(
                p_history->o_name),
            p_ctxt->p_info->o_home.p_buf);

        snck_string_append_buffer(
            p_ctxt,
            &(
                p_history->o_name),
            a_histfile_suffix,
            sizeof(a_histfile_suffix));
    }

    b_result = 1;

    return b_result;

} /* snck_history_init() */

void
snck_history_cleanup(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_history * const p_history =
        p_ctxt->p_history;

    /* Empty the linked list */
    snck_history_unload(
        p_ctxt);

    snck_string_cleanup(
        p_ctxt,
        &(
            p_history->o_name));

    snck_history_empty_list(
        p_ctxt,
        &(
            p_history->o_cache));

    p_history->i_cache_len =
        0u;

} /* snck_history_cleanup() */

static
char *
snck_history_get_line(
    struct snck_ctxt const * const
        p_ctxt,
    FILE * const
        p_file)
{
    char * a_line;

    int i_line_max_len;

    int i_line_len;

    i_line_max_len = 128;

    i_line_len = 0;

    a_line = snck_heap_realloc(p_ctxt, NULL, i_line_max_len);

    if (a_line)
    {
        char b_more;

        b_more = 1;

        while (b_more)
        {
            int c;

            c = fgetc(p_file);

            if (EOF != c)
            {
                if ('\r' == c)
                {
                }
                else
                {
                    if (i_line_len >= i_line_max_len)
                    {
                        i_line_max_len <<= 1;

                        a_line = snck_heap_realloc(p_ctxt, a_line, i_line_max_len);
                    }

                    if (a_line)
                    {
                        if ('\n' == c)
                        {
                            a_line[i_line_len] = '\000';

                            i_line_len ++;

                            b_more = 0;
                        }
                        else
                        {
                            a_line[i_line_len] = (char)(c);

                            i_line_len ++;
                        }
                    }
                    else
                    {
                        b_more = 0;
                    }
                }
            }
            else
            {
                snck_heap_realloc(p_ctxt, a_line, 0);

                a_line = NULL;

                b_more = 0;
            }
        }
    }

    return a_line;

}

static
void
snck_history_put_line(
    struct snck_ctxt const * const
        p_ctxt,
    char * const
        a_line)
{
    snck_heap_realloc(p_ctxt, a_line, 0u);
}

void
snck_history_load(
    struct snck_ctxt const * const
        p_ctxt)
{
    FILE * p_file;

    struct snck_history * const p_history =
        p_ctxt->p_history;

    char * p_name0 = snck_string_get(p_ctxt, &(p_history->o_name));

    if (p_name0)
    {
        p_file = fopen(p_name0, "r");

        snck_string_put(p_ctxt, p_name0);
    }
    else
    {
        p_file = NULL;
    }

    if (p_file)
    {
        char b_more;

        snck_history_unload(p_ctxt);

        b_more = 1;

        while (b_more)
        {
            char * a_line;

            a_line = snck_history_get_line(p_ctxt, p_file);

            if (a_line)
            {
                {
                    struct snck_history_line * p_history_line;

                    p_history_line = snck_history_line_create(p_ctxt);

                    if (p_history_line)
                    {
                        snck_string_copy(p_ctxt, &(p_history_line->o_buf), a_line);

                        snck_list_join(&(p_history_line->o_list), &(p_history->o_list));
                    }
                }

                snck_history_put_line(p_ctxt, a_line);
            }
            else
            {
                b_more = 0;
            }
        }

        fclose(p_file);
    }

} /* snck_history_load() */

void
snck_history_unload(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_history * const p_history =
        p_ctxt->p_history;

    snck_history_empty_list(
        p_ctxt,
        &(
            p_history->o_list));

} /* snck_history_unload() */

void
snck_history_save(
    struct snck_ctxt const * const
        p_ctxt)
{
    FILE * p_file;

    struct snck_history * const p_history =
        p_ctxt->p_history;

    char * p_name0 = snck_string_get(p_ctxt, &(p_history->o_name));

    if (p_name0)
    {
        p_file = fopen(p_name0, "w");

        snck_string_put(p_ctxt, p_name0);
    }
    else
    {
        p_file = NULL;
    }

    if (p_file)
    {
        struct snck_list const * p_it;

        p_it = p_history->o_list.p_next;

        while (p_it != &(p_history->o_list))
        {
            struct snck_history_line const * p_history_line = (struct snck_history_line const *)(p_it);

            fprintf(p_file, "%.*s\n",
                (int)(p_history_line->o_buf.i_buf_len),
                p_history_line->o_buf.p_buf);

            p_it = p_it->p_next;
        }

        fclose(p_file);
    }

} /* snck_history_save() */

static
void
snck_history_add_to_list(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_list * const
        p_list,
    char const * const
        p_buf)
{
    char b_found;

    b_found = 0;

    /* Find existing entry */
    if (!b_found)
    {
        struct snck_list * p_it;

        p_it = p_list->p_next;

        while (!b_found && (p_it != p_list))
        {
            struct snck_history_line * p_history_line = (struct snck_history_line *)(p_it);

            struct snck_string o_buf;

            snck_string_init_ref(&(o_buf), p_buf);

            if (0 == snck_string_compare(&(o_buf), &(p_history_line->o_buf)))
            {
                b_found = 1;

                snck_list_join(
                    &(
                        p_history_line->o_list),
                    &(
                        p_history_line->o_list));

                snck_list_join(
                    &(
                        p_history_line->o_list),
                    p_list);
            }
            else
            {
                p_it = p_it->p_next;
            }
        }
    }

    /* Else add new entry */
    if (!b_found)
    {
        struct snck_history_line * p_history_line;

        p_history_line = snck_history_line_create(p_ctxt);

        if (p_history_line)
        {
            b_found = 1;

            snck_list_join(
                &(
                    p_history_line->o_list),
                p_list);

            snck_string_copy(
                p_ctxt,
                &(
                    p_history_line->o_buf),
                p_buf);
        }
    }
}

void
snck_history_add(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_buf)
{
    struct snck_history * const p_history =
        p_ctxt->p_history;

    snck_history_add_to_list(
        p_ctxt,
        &(
            p_history->o_list),
        p_buf);

    snck_history_add_to_list(
        p_ctxt,
        &(
            p_history->o_cache),
        p_buf);

    p_history->i_cache_len ++;

    if (p_history->i_cache_len > 8)
    {
        snck_history_line_destroy(
            p_ctxt,
            (struct snck_history_line *)(p_history->o_cache.p_next));

        p_history->i_cache_len --;
    }

} /* snck_history_add() */

/* end-of-file: snck_history.c */
