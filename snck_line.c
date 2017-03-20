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

/* Module */
#include "snck_line.h"

/* String */
#include "snck_string.h"

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

/* Sorted list of strings */
/* Sort by alphatical order */
/* Sort by fuzzy order */
/* Unsorted list of strings */

static struct snck_ctxt const * g_ctxt = NULL;

static char a_folder[1024u];

static char suggest[1024u];

static char * a_suggest[128u];

static int a_score[128u];

static int i_suggest = 0;

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
snck_suggest_add(
    char const * const suggest,
    int const i_score)
{
    if (i_suggest < 128)
    {
        int i;

        int j;

        char b_inserted;

        i = 0;

        b_inserted = 0;

        while (!b_inserted && (i < i_suggest))
        {
            int i_compare;

            i_compare = strcmp(suggest, a_suggest[i]);

            if (0 == i_compare)
            {
                b_inserted = 1;
            }
            else if (
                (
                    i_score
                    && (
                        (i_score < a_score[i])
                        || ((i_score == a_score[i]) && (0 > i_compare))))
                || (
                    !i_score
                    && (
                        0 > i_compare)))
            {
                /* Do sort of suggestions */

                j = i_suggest;

                while (j > i)
                {
                    a_suggest[j] = a_suggest[j-1];

                    a_score[j] = a_score[j-1];

                    j--;
                }

                a_suggest[i] = strdup(suggest);

                a_score[i] = i_score;

                b_inserted = 1;

                i_suggest ++;
            }
            else
            {
                i++;
            }
        }

        if (!b_inserted)
        {
            a_suggest[i_suggest] = strdup(suggest);

            a_score[i_suggest] = i_score;

            i_suggest ++;
        }
    }
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

    char b_cmd_is_cd;

#if 0
    strcpy(a_split, buf);

    snck_tokenize_line();
#endif

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

#if 0
    if ((16 == key) && ((int)(pos) <= i_cmd_prefix) && !buf[i_cmd_prefix])
    {
        /* Full history search */
        struct snck_list const * p_it;

        int i;

        snck_history_load(p_ctxt);

        i = 0;

        p_it = p_ctxt->p_history->o_list.p_prev;

        while ((i < 128) && (p_it != &(p_ctxt->p_history->o_list)))
        {
            struct snck_history_line const * p_history_line = (struct snck_history_line const *)(p_it);

            linenoiseAddCompletion(lc, p_history_line->o_buf.p_buf);

            i ++;

            p_it = p_it->p_prev;
        }

        snck_history_unload(p_ctxt);
    }
#endif

    if (((9 == key) && ((int)(pos) > i_cmd_prefix)) || (256+1 == key) || (16 == key))
    {
        char * p_folder;

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

        if (pos1 > pos0)
        {
            a_folder[0u] = '\000';

            if (buf[pos0] == '~')
            {
                sprintf(a_folder, "%s%.*s", p_ctxt->p_info->o_home.p_buf, pos1 - pos0 - 1, buf + pos0 + 1);
            }
            else
            {
                memcpy(a_folder, buf + pos0, pos1 - pos0);

                a_folder[pos1 - pos0] = '\000';
            }

            p_folder = a_folder;
        }
        else
        {
            p_folder = ".";
        }

        i_suggest = 0;

        /* completing a history entry */
        if (key == 16)
        {
            struct snck_list const * p_it;

            snck_history_load(p_ctxt);

            p_it = p_ctxt->p_history->o_list.p_prev;

            while (p_it != &(p_ctxt->p_history->o_list))
            {
                struct snck_history_line const * p_history_line = (struct snck_history_line const *)(p_it);

                if (buf[i_cmd_prefix] != '\000')
                {
                    int i_score;

                    i_score = 0;

                    if (0 != (i_score = snck_fuzzy_compare(p_history_line->o_buf.p_buf, buf + i_cmd_prefix + 0, strlen(buf) - i_cmd_prefix - 0)))
                    {
                        snck_suggest_add(p_history_line->o_buf.p_buf, i_score);
                    }
                }
                else
                {
                    if (i_suggest < 128)
                    {
                        a_suggest[i_suggest] = strdup(p_history_line->o_buf.p_buf);

                        a_score[i_suggest] = 0;

                        i_suggest ++;
                    }
                }

                p_it = p_it->p_prev;
            }

            snck_history_unload(p_ctxt);
        }
        else if (key == 256+1)
        {
            /* Complete last argument of previous commands */
            /* Insert word at pos */
            struct snck_list const * p_it;

            snck_history_load(p_ctxt);

            p_it = p_ctxt->p_history->o_list.p_prev;

            while (p_it != &(p_ctxt->p_history->o_list))
            {
                struct snck_history_line const * p_history_line = (struct snck_history_line const *)(p_it);

                int i_len;

                i_len = strlen(p_history_line->o_buf.p_buf);

                while ((i_len > 0) && (' ' == p_history_line->o_buf.p_buf[i_len-1]))
                {
                    i_len--;
                }
                while ((i_len > 0) && (' ' != p_history_line->o_buf.p_buf[i_len-1]))
                {
                    i_len--;
                }

                {
                    if (i_suggest < 128)
                    {
                        sprintf(suggest, "%.*s%s%s", pos, buf, p_history_line->o_buf.p_buf + i_len, buf + pos);

                        a_suggest[i_suggest] = strdup(suggest);

                        a_score[i_suggest] = 0;

                        i_suggest ++;
                    }
                }

                p_it = p_it->p_prev;
            }

            snck_history_unload(p_ctxt);
        }
        /* completing a file name or full path to program */
        else if ((pos1 == i_cmd_prefix) && (buf[pos1] != '.') && (buf[pos1] != '/'))
        {
            char * p_env;

            p_env = getenv("PATH");

            if (p_env)
            {
                /* split of buffer */
                char * p_temp;

                p_temp = strdup(p_env);

                if (p_temp)
                {
                    char * p_comp;

                    p_comp = strtok(p_temp, ":");

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
                                    if (0 == strncmp(p_dir_entry->d_name, buf + pos1, pos - pos1))
                                    {
                                        if (pos1 > 0)
                                        {
                                            sprintf(suggest, "%.*s%s", pos1, buf, p_dir_entry->d_name);
                                        }
                                        else
                                        {
                                            sprintf(suggest, "%s", p_dir_entry->d_name);
                                        }

                                        if (0 != strcmp(suggest, buf))
                                        {
                                            snck_suggest_add(suggest, 0);
                                        }
                                    }
                                }

                                p_dir_entry = readdir(p_dir_object);
                            }

                            closedir(p_dir_object);
                        }

                        p_comp = strtok(NULL, ":");
                    }

                    free(p_temp);
                }
            }
        }
        else
        {
            DIR * d;

            d = opendir(p_folder);

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
                                if (pos1 > 0)
                                {
                                    sprintf(suggest, "%.*s%s", pos1, buf, e->d_name);
                                }
                                else
                                {
                                    sprintf(suggest, "%s", e->d_name);
                                }

                                if (0 != strcmp(suggest, buf))
                                {
                                    snck_suggest_add(suggest, i_score);
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

        /* If only one suggestion... */
        if (0 == i_suggest)
        {
        }
        else if (1 == i_suggest)
        {
            linenoiseAddCompletion(lc, a_suggest[0]);
        }
        else
        {
            int i;

#if 0
            int j;

            /* Find common prefix for suggestions */
            i = 1;

            j = strlen(a_suggest[0]);

            while (i < i_suggest)
            {
                int k;

                /* Find new common length */
                k = 0;

                while ((k < j) && a_suggest[0][k] && (a_suggest[0][k] == a_suggest[i][k]))
                {
                    k++;
                }

                j = k;

                i ++;
            }

            if ((j != (int)(strlen(a_suggest[0]))) && (j > (int)(strlen(buf))))
            {
                /* suggest only common prefix... */
                strcpy(suggest, a_suggest[0]);

                suggest[j] = '\000';

                linenoiseAddCompletion(lc, suggest);
            }

            fprintf(stderr, "\r\n");
#endif

            /* Merge list of suggestions into linenoise */
            i = 0;

            while (i < i_suggest)
            {
                if (a_suggest[i])
                {
#if 0
                    fprintf(stderr, "%s\r\n", a_suggest[i]);
#else
                    linenoiseAddCompletion(lc, a_suggest[i]);
#endif

                    free(a_suggest[i]);

                    a_suggest[i] = NULL;
                }

                i++;
            }

            i_suggest = 0;
        }
    }

} /* snck_completion */

#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

char const *
snck_line_get(
    struct snck_ctxt const * const
        p_ctxt)
{
    char * a_split = NULL;

    char const * p_prompt = snck_prompt_get(p_ctxt);

    if (p_prompt)
    {

#if defined(SNCK_FEATURE_LINENOISE)
        {
            char * p_temp;

            linenoiseSetCompletionCallback(snck_completion);

            g_ctxt = p_ctxt;

            errno = 0;

            p_temp = linenoise(p_prompt);

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

                a_split = snck_heap_realloc(p_ctxt, NULL, strlen(p_temp) + 1);

                if (a_split)
                {
                    strcpy(a_split, p_temp);
                }

                free(p_temp);
            }
            else
            {
                if (errno == EAGAIN)
                {
                    /* ctrl+c was pressed */
                    a_split = snck_heap_realloc(p_ctxt, NULL, 2u);

                    if (a_split)
                    {
                        a_split[0] = '\n';

                        a_split[1] = '\000';
                    }
                }
                else
                {
                    a_split = NULL;
                }
            }
        }
#else /* #if defined(SNCK_FEATURE_LINENOISE) */
        {
            a_split = snck_heap_realloc(p_ctxt, NULL, 65536u);

            if (a_split)
            {
                fprintf(stdout, "%s", p_prompt);

                fflush(stdout);

                if (NULL != fgets(a_split, 65536u, stdin))
                {
                }
                else
                {
                    snck_heap_realloc(p_ctxt, a_split, 0u);

                    a_split = NULL;
                }
            }
        }
#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

        snck_prompt_put(p_ctxt, p_prompt);
    }

#if 0
    if (!b_result)
    {
        fprintf(stderr, "... bye!\n");
    }
#endif

    return a_split;

} /* snck_line_get() */

void
snck_line_put(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_buf)
{
    snck_heap_realloc(p_ctxt, (void *)(p_buf), 0u);
} /* snck_line_put() */

/* end-of-file: snck_line.c */
