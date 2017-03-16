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

    {
        static char a_histfile_suffix[] = "/.snckhist";

        snck_string_copy(
            p_ctxt,
            &(
                p_history->o_name),
            p_ctxt->p_info->o_home.p_buf);

        snck_string_append(
            p_ctxt,
            &(
                p_history->o_name),
            a_histfile_suffix);
    }

    b_result = 1;

    return b_result;

} /* snck_history_init() */

static
void
snck_history_empty(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_history * const p_history =
        p_ctxt->p_history;

    struct snck_list * p_it;

    p_it = p_history->o_list.p_next;

    while (p_it != &(p_history->o_list))
    {
        struct snck_list * p_next = p_it->p_next;

        snck_history_line_destroy(
            p_ctxt,
            (struct snck_history_line *)(p_it));

        p_it = p_next;
    }
}

void
snck_history_cleanup(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_history * const p_history =
        p_ctxt->p_history;

    /* Empty the linked list */
    snck_history_empty(
        p_ctxt);

    snck_string_cleanup(
        p_ctxt,
        &(
            p_history->o_name));

} /* snck_history_cleanup() */

void
snck_history_load(
    struct snck_ctxt const * const
        p_ctxt)
{
    FILE * p_file;

    struct snck_history * const p_history =
        p_ctxt->p_history;

    p_file = fopen(p_history->o_name.p_buf, "r");

    if (p_file)
    {
        static char a_line[4096u];

        snck_history_empty(p_ctxt);

        while (NULL != fgets(a_line, 4095u, p_file))
        {
            {
                int i_line_len;

                i_line_len = strlen(a_line);

                while (
                    (0 < i_line_len)
                    && (
                        ('\n' == a_line[i_line_len-1])
                        || ('\r' == a_line[i_line_len-1])))
                {
                    a_line[i_line_len-1] = '\000';

                    i_line_len --;
                }
            }

            {
                struct snck_history_line * p_history_line;

                p_history_line = snck_history_line_create(p_ctxt);

                if (p_history_line)
                {
                    snck_string_copy(p_ctxt, &(p_history_line->o_buf), a_line);

                    snck_list_join(&(p_history_line->o_list), &(p_history->o_list));
                }
            }
        }

        fclose(p_file);
    }

} /* snck_history_load() */

void
snck_history_save(
    struct snck_ctxt const * const
        p_ctxt)
{
    FILE * p_file;

    struct snck_history * const p_history =
        p_ctxt->p_history;

    p_file = fopen(p_history->o_name.p_buf, "w");

    if (p_file)
    {
        struct snck_list const * p_it;

        p_it = p_history->o_list.p_next;

        while (p_it != &(p_history->o_list))
        {
            struct snck_history_line const * p_history_line = (struct snck_history_line const *)(p_it);

            fprintf(p_file, "%s\n", p_history_line->o_buf.p_buf);

            p_it = p_it->p_next;
        }

        fclose(p_file);
    }

} /* snck_history_save() */

void
snck_history_add(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_buf)
{
    char b_found;

    struct snck_history * const p_history =
        p_ctxt->p_history;

    b_found = 0;

    /* Find existing entry */
    if (!b_found)
    {
        struct snck_list * p_it;

        p_it = p_history->o_list.p_next;

        while (!b_found && (p_it != &(p_history->o_list)))
        {
            struct snck_history_line * p_history_line = (struct snck_history_line *)(p_it);

            if (0 == strcmp(p_buf, p_history_line->o_buf.p_buf))
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
                    &(
                        p_history->o_list));
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
                &(
                    p_history->o_list));

            snck_string_copy(
                p_ctxt,
                &(
                    p_history_line->o_buf),
                p_buf);
        }
    }

} /* snck_history_add() */

/* end-of-file: snck_history.c */
