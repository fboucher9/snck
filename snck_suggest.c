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

/* Environment */
#include "snck_env.h"

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

        b_inserted = 1;

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

                p_suggest_list->i_count --;

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

void
snck_suggest_command(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    char const * const
        buf,
    size_t const
        buf_len,
    size_t const
        pos,
    int const
        pos1)
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

} /* snck_suggest_command() */

char
snck_suggest_is_special(
    char const
        c)
{
    char
        b_special;

    if (
        (
            ('a' <= c)
            && ('z' >= c))
        || (
            ('A' <= c)
            && ('Z' >= c))
        || (
            ('0' <= c)
            && ('9' >= c))
        || ('.' == c)
        || ('-' == c)
        || ('_' == c))
    {
        b_special = 0;
    }
    else
    {
        b_special = 1;
    }

    return
        b_special;

} /* snck_suggest_is_special() */

void
snck_suggest_file(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    char const * const
        buf,
    size_t const
        buf_len,
    size_t const
        pos,
    int const
        pos1,
    struct snck_string const * const
        p_folder,
    char const
        b_cmd_is_cd)
{
    DIR * d;

    char * p_folder0;

    p_folder0 = snck_string_get(p_ctxt, p_folder);

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

                                if ((p_folder->i_buf_len > 0) && ('/' == p_folder->p_buf[p_folder->i_buf_len - 1u]))
                                {
                                    sprintf(a_link_path, "%.*s%s",
                                        (int)(p_folder->i_buf_len),
                                        p_folder->p_buf,
                                        e->d_name);
                                }
                                else
                                {
                                    sprintf(a_link_path, "%.*s/%s",
                                        (int)(p_folder->i_buf_len),
                                        p_folder->p_buf,
                                        e->d_name);
                                }

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

                                if (snck_string_resize(p_ctxt, &(p_suggest_node->o_buf), buf_len + (strlen(e->d_name) * 2) + 16u + 1u))
                                {
                                    sprintf(
                                        p_suggest_node->o_buf.p_buf,
                                        "%08x%08x",
                                        (unsigned int)(i_score),
                                        (unsigned int)(0u));

                                    p_suggest_node->o_buf.i_buf_len = strlen(p_suggest_node->o_buf.p_buf);

                                    if (pos1 > 0)
                                    {
                                        memcpy(
                                            p_suggest_node->o_buf.p_buf + p_suggest_node->o_buf.i_buf_len,
                                            buf,
                                            pos1);

                                        p_suggest_node->o_buf.i_buf_len += pos1;
                                    }

                                    {
                                        char const * p_name_it;

                                        p_name_it = e->d_name;

                                        while (*p_name_it)
                                        {
                                            if (snck_suggest_is_special(*p_name_it))
                                            {
                                                p_suggest_node->o_buf.p_buf[p_suggest_node->o_buf.i_buf_len] = '\\';
                                                p_suggest_node->o_buf.i_buf_len ++;
                                            }

                                            p_suggest_node->o_buf.p_buf[p_suggest_node->o_buf.i_buf_len] = *p_name_it;
                                            p_suggest_node->o_buf.i_buf_len ++;

                                            p_name_it ++;
                                        }
                                    }

                                    if (buf_len > pos)
                                    {
                                        memcpy(
                                            p_suggest_node->o_buf.p_buf + p_suggest_node->o_buf.i_buf_len,
                                            buf + pos,
                                            buf_len - pos);

                                        p_suggest_node->o_buf.i_buf_len += (buf_len - pos);
                                    }

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

/* end-of-file: snck_suggest.c */
