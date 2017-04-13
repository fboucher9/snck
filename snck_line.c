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

/* Options */
#include "snck_opts.h"

/* Suggestions */
#include "snck_suggest.h"

/* Sorted list of strings */
/* Sort by alphatical order */
/* Sort by fuzzy order */
/* Unsorted list of strings */

static struct snck_ctxt const * g_ctxt = NULL;

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

    size_t i_cmd_prefix;

    size_t buf_len;

    char b_cmd_is_cd;

    struct snck_suggest_list o_suggest_list;

    snck_suggest_list_init(p_ctxt, &(o_suggest_list));

    buf_len = strlen(buf);

    i_cmd_prefix = 0;

    while ((i_cmd_prefix <= pos) && ((buf[i_cmd_prefix] == ' ') || (buf[i_cmd_prefix] == '\t')))
    {
        i_cmd_prefix ++;
    }

    b_cmd_is_cd = 0;

    {
        size_t i_cmd_it;

        i_cmd_it = i_cmd_prefix;

        if ('c' == buf[i_cmd_it])
        {
            i_cmd_it ++;

            if ('d' == buf[i_cmd_it])
            {
                i_cmd_it ++;

                if ((i_cmd_it <= pos) && (('\000' == buf[i_cmd_it]) || (buf[i_cmd_it] == ' ') || (buf[i_cmd_it] == '\t')))
                {
                    b_cmd_is_cd = 1;
                }
            }
        }
    }

    if (((9 == key) && (pos > i_cmd_prefix)) || (256+1 == key) || (16 == key))
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
            struct snck_string o_wild;

            snck_string_init_ref_buffer(
                &(
                    o_wild),
                buf,
                buf_len);

            snck_suggest_from_history(
                p_ctxt,
                &(o_suggest_list),
                &(o_wild));

        }
        else if (key == 256+1)
        {
            /* Complete last argument of previous commands */
            snck_suggest_from_lastword(
                p_ctxt,
                &(o_suggest_list),
                buf,
                buf_len,
                pos);

        }
        /* completing a file name or full path to program */
        else if ((pos1 == (int)(i_cmd_prefix)) && (buf[pos1] != '.') && (buf[pos1] != '/'))
        {
            static char a_name_path[] = { 'P', 'A', 'T', 'H' };

            static struct snck_string const o_name_path = { a_name_path, sizeof(a_name_path), 0u };

            struct snck_string o_value_path;

            snck_string_init(p_ctxt, &(o_value_path));

            if (snck_env_get(p_ctxt, &(o_name_path), &(o_value_path)))
            {
                /* split of buffer */
                size_t i_path_it;

                i_path_it = 0;

                while (i_path_it < o_value_path.i_buf_len)
                {
                    size_t i_comp_start;

                    size_t i_comp_len;

                    i_comp_start = i_path_it;

                    i_comp_len = 0;

                    while ((i_path_it < o_value_path.i_buf_len)
                        && (':' != o_value_path.p_buf[i_path_it]))
                    {
                        i_comp_len ++;

                        i_path_it ++;
                    }

                    if (i_comp_len)
                    {
                        struct snck_string o_comp;

                        snck_string_init_ref_buffer(&(o_comp), o_value_path.p_buf + i_comp_start, i_comp_len);

                        /* enumerate executables in path */
                        {
                            DIR * p_dir_object;

                            {
                                char * p_comp0 = snck_string_get(p_ctxt, &(o_comp));

                                if (p_comp0)
                                {
                                    p_dir_object = opendir(p_comp0);

                                    snck_string_put(p_ctxt, p_comp0);
                                }
                                else
                                {
                                    p_dir_object = NULL;
                                }
                            }

                            if (p_dir_object)
                            {
                                struct dirent * p_dir_entry;

                                p_dir_entry = readdir(p_dir_object);

                                while (p_dir_entry)
                                {
                                    if (DT_DIR != p_dir_entry->d_type)
                                    {
                                        int i_score;

                                        struct snck_string o_dir_entry;

                                        struct snck_string o_wild;

                                        snck_string_init_ref(&(o_dir_entry), p_dir_entry->d_name);

                                        snck_string_init_ref_buffer(&(o_wild), buf + pos1, pos - pos1);

                                        i_score = snck_fuzzy_compare(&(o_dir_entry), &(o_wild));

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

                                                    p_suggest_node->o_buf.i_buf_len = strlen(p_suggest_node->o_buf.p_buf);

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
                        }

                        snck_string_cleanup(p_ctxt, &(o_comp));
                    }

                    /* Skip the colon separator */
                    i_path_it ++;
                }
            }

            snck_string_cleanup(p_ctxt, &(o_value_path));
        }
        else
        {
            DIR * d;

            char * p_folder0;

            p_folder0 = snck_string_get(p_ctxt, &(o_folder));

            if (p_folder0)
            {
                d = opendir(p_folder0);

                if (d)
                {
                    while (1)
                    {
                        struct dirent * e;

                        e = readdir(d);
                        if (e)
                        {
                            int i_score;

                            struct snck_string o_dir_entry;

                            struct snck_string o_wild;

                            snck_string_init_ref(&(o_dir_entry), e->d_name);

                            snck_string_init_ref_buffer(&(o_wild), buf + pos1, pos - pos1);

                            i_score = 0;

                            if ((0 == strcmp(e->d_name, ".")) || (0 == strcmp(e->d_name, "..")))
                            {
                            }
                            else if (0 != (i_score = snck_fuzzy_compare(&(o_dir_entry), &(o_wild))))
                            {
                                char b_filter;

                                if (b_cmd_is_cd)
                                {
                                    if (DT_DIR == e->d_type)
                                    {
                                        b_filter = 1;
                                    }
                                    else if (DT_LNK == e->d_type)
                                    {
                                        struct stat o_link_info;

                                        static char a_link_path[256];

                                        int i_stat_result;

                                        sprintf(a_link_path, "%s%s", p_folder0, e->d_name);

                                        i_stat_result =
                                            stat(
                                                a_link_path,
                                                &(o_link_info));

                                        if (
                                            0 == i_stat_result)
                                        {
                                            if (S_ISDIR(o_link_info.st_mode))
                                            {
                                                b_filter = 1;
                                            }
                                            else
                                            {
                                                b_filter = 0;
                                            }
                                        }
                                        else
                                        {
                                            b_filter = 0;
                                        }
                                    }
                                    else
                                    {
                                        b_filter = 0;
                                    }
                                }
                                else
                                {
                                    b_filter = 1;
                                }

                                if (b_filter)
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

                                            p_suggest_node->o_buf.i_buf_len = strlen(p_suggest_node->o_buf.p_buf);

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

                snck_string_put(p_ctxt, p_folder0);
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
                    char * p_buf0 = snck_string_get(p_ctxt, &(p_suggest_temp->o_buf));

                    if (p_buf0)
                    {
                        linenoiseAddCompletion(lc, p_buf0 + 16u);

                        snck_string_put(p_ctxt, p_buf0);
                    }
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

    if ((stdin == p_file) && (p_ctxt->p_opts->b_interact))
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

                {
                    char * p_prompt0 = snck_string_get(p_ctxt, &(o_prompt));

                    if (p_prompt0)
                    {
                        p_temp = linenoise(p_prompt0);

                        snck_string_put(p_ctxt, p_prompt0);
                    }
                    else
                    {
                        p_temp = NULL;
                    }
                }

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

                if ('\n' == p_string->p_buf[p_string->i_buf_len - 1])
                {
                    p_string->p_buf[p_string->i_buf_len - 1] = '\000';

                    p_string->i_buf_len --;
                }

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
