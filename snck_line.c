/* See LICENSE for license details. */

/*

Module: snck_line.c

Description:

    Line editor.

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Context */
#include "snck_ctxt.h"

/* String */
#include "snck_string.h"

/* Module */
#include "snck_line.h"

/* Information */
#include "snck_info.h"

/* Prompt */
#include "snck_prompt.h"

/* */
#if defined(SNCK_FEATURE_LINENOISE)
#include <linenoise.h>
#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

/* List */
#include "snck_list.h"

/* History */
#include "snck_history.h"

/* Heap */
#include "snck_heap.h"

/* Environment */
#include "snck_env.h"

/* Sorted list of strings */
/* Sort by alphatical order */
/* Sort by fuzzy order */
/* Unsorted list of strings */

static struct snck_ctxt const * g_ctxt = NULL;

struct snck_suggest_node
{
    struct snck_list o_list;

    struct snck_string o_buf;

}; /* struct snck_suggest_node */

static
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

static
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

struct snck_suggest_list
{
    struct snck_list o_list;

    int i_count;

}; /* struct snck_suggest_list */

static
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

static
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

static
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

    (void)(
        p_ctxt);

    p_it = p_suggest_list->o_list.p_next;

    b_inserted = 0;

    b_consumed = 1;

    while (!b_inserted && (p_it != &(p_suggest_list->o_list)))
    {
        struct snck_suggest_node * const p_suggest_temp =
            (struct snck_suggest_node *)(
                p_it);

        int const i_compare =
            strcmp(
                p_suggest_node->o_buf.p_buf,
                p_suggest_temp->o_buf.p_buf);

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
        }
        else
        {
            p_it = p_it->p_next;
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

char
snck_line_init(
    struct snck_ctxt const * const
        p_ctxt)
{
    char b_result;

    (void)(p_ctxt);

    b_result = 1;

    return b_result;

} /* snck_line_init() */

void
snck_line_cleanup(
    struct snck_ctxt const * const
        p_ctxt)
{
    (void)(p_ctxt);

} /* snck_line_cleanup() */

#if defined(SNCK_FEATURE_LINENOISE)

static
int
snck_fuzzy_compare(
    char const * p_ref1,
    char const * p_ref2,
    int const i_ref2_len)
{
    int i_result;

    int i_ref1;

    int i_ref2;

    int i_ref1_len;

    if (i_ref2_len >= 1)
    {
        i_result = 0;

        i_ref1 = 0;

        i_ref2 = 0;

        i_ref1_len = (int)(strlen(p_ref1));

        /* try to find each letter of p_ref2[0:i_ref2_len-1] within p_ref1 */
        while ((i_ref1 < i_ref1_len) && (i_ref2 < i_ref2_len))
        {
            /* Look for a letter */
            if (p_ref1[i_ref1] == p_ref2[i_ref2])
            {
                i_ref1 ++;
                i_ref2 ++;
            }
            else
            {
                i_ref1 ++;
            }
        }

        if (i_ref2 >= i_ref2_len)
        {
            i_result = i_ref1;
        }
    }
    else
    {
        if (0 == strncmp(p_ref1, p_ref2, i_ref2_len))
        {
            i_result = 1;
        }
        else
        {
            i_result = 0;
        }
    }

    return i_result;

}

static
void
snck_completion(
    char const * buf,
    linenoiseCompletions * lc,
    size_t pos,
    int key)
{
    /* quick tokenize of current line */
    /* find the word under the cursor */
    /* locate words before and words after */
    /* complete entire line and replace word with other ... */

    struct snck_ctxt const * const p_ctxt = g_ctxt;

    int i_cmd_prefix;

    int buf_len;

    char b_cmd_is_cd;

    struct snck_suggest_list o_suggest_list;

    snck_suggest_list_init(p_ctxt, &(o_suggest_list));

    buf_len = strlen(buf);

    i_cmd_prefix = 0;

    while ((i_cmd_prefix <= (int)(pos)) && ((buf[i_cmd_prefix] == ' ') || (buf[i_cmd_prefix] == '\t')))
    {
        i_cmd_prefix ++;
    }

    b_cmd_is_cd = 0;

    {
        int i_cmd_it;

        i_cmd_it = i_cmd_prefix;

        if ('c' == buf[i_cmd_it])
        {
            i_cmd_it ++;

            if ('d' == buf[i_cmd_it])
            {
                i_cmd_it ++;

                if ((i_cmd_it <= (int)(pos)) && (('\000' == buf[i_cmd_it]) || (buf[i_cmd_it] == ' ') || (buf[i_cmd_it] == '\t')))
                {
                    b_cmd_is_cd = 1;
                }
            }
        }
    }

    if (((9 == key) && ((int)(pos) > i_cmd_prefix)) || (256+1 == key) || (16 == key))
    {
        struct snck_string o_folder;

        int pos0;

        int pos1;

        if (pos > 0)
        {
            pos0 = (int)(pos - 1);

            while (pos0 >= 0)
            {
                if (buf[pos0] == ' ')
                {
                    break;
                }

                pos0 --;
            }

            pos0 ++;
        }
        else
        {
            pos0 = 0;
        }

        /* Find the directory name */

        /* From beginning of word to last slash */

        if (pos > 0)
        {
            pos1 = (int)(pos - 1);

            while (pos1 >= pos0)
            {
                if (buf[pos1] == '/')
                {
                    break;
                }

                pos1 --;
            }

            pos1++;
        }
        else
        {
            pos1 = 0;
        }

        snck_string_init(p_ctxt, &(o_folder));

        if (pos1 > pos0)
        {
            if (buf[pos0] == '~')
            {
                snck_string_copy_object(p_ctxt, &(o_folder), &(p_ctxt->p_info->o_home));

                snck_string_append_buffer(p_ctxt, &(o_folder), buf + pos0 + 1, pos1 - pos0 - 1);
            }
            else
            {
                snck_string_copy_buffer(p_ctxt, &(o_folder), buf + pos0, pos1 - pos0);
            }
        }
        else
        {
            snck_string_ref(p_ctxt, &(o_folder), ".");
        }

        /* completing a history entry */
        if (key == 16)
        {
            struct snck_list const * p_it;

            int i_history_index;

            snck_history_load(p_ctxt);

            i_history_index = 1;

            p_it = p_ctxt->p_history->o_list.p_prev;

            while (p_it != &(p_ctxt->p_history->o_list))
            {
                struct snck_history_line const * p_history_line = (struct snck_history_line const *)(p_it);

                int i_score;

                i_score = snck_fuzzy_compare(p_history_line->o_buf.p_buf, buf + i_cmd_prefix + 0, buf_len - i_cmd_prefix - 0);

                if (0 != i_score)
                {
                    struct snck_suggest_node * p_suggest_node;

                    p_suggest_node = snck_suggest_node_create(p_ctxt);

                    if (p_suggest_node)
                    {
                        char b_consumed;

                        if (snck_string_resize(p_ctxt, &(p_suggest_node->o_buf), p_history_line->o_buf.i_buf_len + 16u + 1u))
                        {
                            sprintf(p_suggest_node->o_buf.p_buf, "%08x%08x%s",
                                (unsigned int)(i_score),
                                (unsigned int)(i_history_index),
                                p_history_line->o_buf.p_buf);

                            b_consumed = snck_suggest_list_add(p_ctxt, &(o_suggest_list), p_suggest_node);
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

                i_history_index ++;

                p_it = p_it->p_prev;
            }

            snck_history_unload(p_ctxt);
        }
        else if (key == 256+1)
        {
            /* Complete last argument of previous commands */
            /* Insert word at pos */
            struct snck_list const * p_it;

            int i_history_index;

            snck_history_load(p_ctxt);

            i_history_index = 1;

            p_it = p_ctxt->p_history->o_list.p_prev;

            while (p_it != &(p_ctxt->p_history->o_list))
            {
                struct snck_history_line const * p_history_line = (struct snck_history_line const *)(p_it);

                int i_len;

                i_len = p_history_line->o_buf.i_buf_len;

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
                            sprintf(p_suggest_node->o_buf.p_buf, "%08x%08x%.*s%s%s",
                                (unsigned int)(1u),
                                (unsigned int)(i_history_index),
                                pos,
                                buf,
                                p_history_line->o_buf.p_buf + i_len,
                                buf + pos);

                            b_consumed = snck_suggest_list_add(p_ctxt, &(o_suggest_list), p_suggest_node);
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

                p_it = p_it->p_prev;

                i_history_index ++;
            }

            snck_history_unload(p_ctxt);
        }
        /* completing a file name or full path to program */
        else if ((pos1 == i_cmd_prefix) && (buf[pos1] != '.') && (buf[pos1] != '/'))
        {
            static char a_name_path[] = { 'P', 'A', 'T', 'H' };

            static struct snck_string const o_name_path = { a_name_path, sizeof(a_name_path), 0u };

            struct snck_string o_value_path;

            snck_string_init(p_ctxt, &(o_value_path));

            if (snck_env_get(p_ctxt, &(o_name_path), &(o_value_path)))
            {
                /* split of buffer */
                char * p_comp;

                p_comp = strtok(o_value_path.p_buf, ":");

                while (p_comp)
                {
                    /* enumerate executables in path */
                    DIR * p_dir_object;

                    p_dir_object = opendir(p_comp);

                    if (p_dir_object)
                    {
                        struct dirent * p_dir_entry;

                        p_dir_entry = readdir(p_dir_object);

                        while (p_dir_entry)
                        {
                            if (DT_DIR != p_dir_entry->d_type)
                            {
                                int i_score;

                                i_score = snck_fuzzy_compare(p_dir_entry->d_name, buf + pos1, pos - pos1);

                                if (0 != i_score)
                                {
                                    struct snck_suggest_node * p_suggest_node;

                                    p_suggest_node = snck_suggest_node_create(p_ctxt);

                                    if (p_suggest_node)
                                    {
                                        char b_consumed;

                                        if (snck_string_resize(p_ctxt, &(p_suggest_node->o_buf), buf_len + strlen(p_dir_entry->d_name) + 16u + 1u))
                                        {
                                            if (pos1 > 0)
                                            {
                                                sprintf(p_suggest_node->o_buf.p_buf, "%08x%08x%.*s%s", (unsigned int)(i_score), (unsigned int)(0u), (int)(pos1), buf, p_dir_entry->d_name);
                                            }
                                            else
                                            {
                                                sprintf(p_suggest_node->o_buf.p_buf, "%08x%08x%s", (unsigned int)(i_score), (unsigned int)(0u), p_dir_entry->d_name);
                                            }

                                            b_consumed = snck_suggest_list_add(p_ctxt, &(o_suggest_list), p_suggest_node);
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

                            p_dir_entry = readdir(p_dir_object);
                        }

                        closedir(p_dir_object);
                    }

                    p_comp = strtok(NULL, ":");
                }
            }

            snck_string_cleanup(p_ctxt, &(o_value_path));
        }
        else
        {
            DIR * d;

            d = opendir(o_folder.p_buf);

            if (d)
            {
                while (1)
                {
                    struct dirent * e;

                    e = readdir(d);
                    if (e)
                    {
                        int i_score;

                        i_score = 0;

                        if ((0 == strcmp(e->d_name, ".")) || (0 == strcmp(e->d_name, "..")))
                        {
                        }
                        else if (0 != (i_score = snck_fuzzy_compare(e->d_name, buf + pos1, pos - pos1)))
                        {
                            if (!b_cmd_is_cd || (DT_DIR == e->d_type))
                            {
                                struct snck_suggest_node * p_suggest_node;

                                p_suggest_node = snck_suggest_node_create(p_ctxt);

                                if (p_suggest_node)
                                {
                                    char b_consumed;

                                    if (snck_string_resize(p_ctxt, &(p_suggest_node->o_buf), buf_len + strlen(e->d_name) + 16u + 1u))
                                    {
                                        if (pos1 > 0)
                                        {
                                            sprintf(p_suggest_node->o_buf.p_buf, "%08x%08x%.*s%s",
                                                (unsigned int)(i_score),
                                                (unsigned int)(0u),
                                                (int)(pos1),
                                                buf,
                                                e->d_name);
                                        }
                                        else
                                        {
                                            sprintf(p_suggest_node->o_buf.p_buf,
                                                "%08x%08x%s",
                                                (unsigned int)(i_score),
                                                (unsigned int)(0u),
                                                e->d_name);
                                        }

                                        b_consumed = snck_suggest_list_add(p_ctxt, &(o_suggest_list), p_suggest_node);
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
                    }
                    else
                    {
                        break;
                    }
                }

                closedir(d);
            }
        }

        snck_string_cleanup(p_ctxt, &(o_folder));

        {
            struct snck_list * p_it = o_suggest_list.o_list.p_next;

            while (p_it != &(o_suggest_list.o_list))
            {
                struct snck_suggest_node * const p_suggest_temp =
                    (struct snck_suggest_node *)(
                        p_it);

                if (p_suggest_temp->o_buf.p_buf)
                {
                    linenoiseAddCompletion(lc, p_suggest_temp->o_buf.p_buf + 16u);
                }

                p_it = p_it->p_next;
            }
        }
    }

    snck_suggest_list_cleanup(p_ctxt, &(o_suggest_list));

} /* snck_completion */

#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

char
snck_line_get(
    struct snck_ctxt const * const
        p_ctxt,
    FILE * const
        p_file,
    struct snck_string * const
        p_string,
    char const
        b_overflow)
{
    char b_result;

    if (stdin == p_file)
    {
        struct snck_string o_prompt;

        snck_string_init(p_ctxt, &(o_prompt));

        if (snck_prompt_get(p_ctxt, &(o_prompt), b_overflow))
        {

#if defined(SNCK_FEATURE_LINENOISE)
            {
                char * p_temp;

                linenoiseSetCompletionCallback(snck_completion);

                g_ctxt = p_ctxt;

                errno = 0;

                p_temp = linenoise(o_prompt.p_buf);

                g_ctxt = NULL;

                if (p_temp)
                {
                    if ((' ' != p_temp[0u]) && ('\000' != p_temp[0u]))
                    {
                        /* Detect duplicate entries... */

                        snck_history_load(p_ctxt);

                        snck_history_add(p_ctxt, p_temp);

                        snck_history_save(p_ctxt);

                        snck_history_unload(p_ctxt);
                    }

                    b_result = snck_string_copy(p_ctxt, p_string, p_temp);

                    free(p_temp);
                }
                else
                {
                    if (errno == EAGAIN)
                    {
                        /* ctrl+c was pressed */
                        b_result = snck_string_copy(p_ctxt, p_string, "");

                        if (b_result)
                        {
                            b_result = 2;
                        }
                    }
                    else
                    {
                        b_result = 0;
                    }
                }
            }
#else /* #if defined(SNCK_FEATURE_LINENOISE) */
            {
                if (snck_string_resize(p_ctxt, p_string, 65536u))
                {
                    fprintf(stdout, "%s", o_prompt.p_buf);

                    fflush(stdout);

                    if (NULL != fgets(p_string->p_buf, p_string->i_alloc_len, stdin))
                    {
                        p_string->i_buf_len = strlen(p_string->p_buf);

                        b_result = 1;
                    }
                    else
                    {
                        snck_string_resize(p_ctxt, p_string, 0u);

                        b_result = 0;
                    }
                }
                else
                {
                    b_result = 0;
                }
            }
#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

        }
        else
        {
            b_result = 0;
        }

        snck_string_cleanup(p_ctxt, &(o_prompt));

    }
    else
    {
        if (snck_string_resize(p_ctxt, p_string, 65536u))
        {
            if (NULL != fgets(p_string->p_buf, p_string->i_alloc_len, p_file))
            {
                p_string->i_buf_len = strlen(p_string->p_buf);

                b_result = 1;
            }
            else
            {
                snck_string_resize(p_ctxt, p_string, 0u);

                b_result = 0;
            }
        }
        else
        {
            b_result = 0;
        }
    }

#if 0
    if (!b_result)
    {
        fprintf(stderr, "... bye!\n");
    }
#endif


    return b_result;

} /* snck_line_get() */

/* end-of-file: snck_line.c */
