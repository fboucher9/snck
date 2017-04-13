/* See LICENSE for license details. */

/*

Module: snck_string.c

Description:

    Null terminated character array.

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Context */
#include "snck_ctxt.h"

/* Module */
#include "snck_string.h"

/* Heap */
#include "snck_heap.h"

void
snck_string_init(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string)
{
    (void)(p_ctxt);

    p_string->p_buf = NULL;

    p_string->i_buf_len = 0u;

    p_string->i_alloc_len = 0u;

} /* snck_string_init() */

void
snck_string_init_ref(
    struct snck_string * const
        p_string,
    char const * const
        p_ref)
{
    snck_string_init_ref_buffer(p_string, p_ref, strlen(p_ref));
}

void
snck_string_init_ref_buffer(
    struct snck_string * const
        p_string,
    void const * const
        p_buf,
    size_t const
        i_buf_len)
{
    p_string->p_buf = (char *)(p_buf);

    p_string->i_buf_len = i_buf_len;

    p_string->i_alloc_len = 0u;

}

void
snck_string_init_ref_object(
    struct snck_string * const
        p_string,
    struct snck_string const * const
        p_object)
{
    snck_string_init_ref_buffer(p_string, p_object->p_buf, p_object->i_buf_len);
}

void
snck_string_cleanup(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string)
{
    if (p_string->i_alloc_len)
    {
        snck_heap_realloc(p_ctxt, p_string->p_buf, 0u);

        p_string->p_buf = NULL;

        p_string->i_buf_len = 0u;

        p_string->i_alloc_len = 0u;
    }

} /* snck_string_cleanup() */

char
snck_string_ref(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    char const * const
        p_ref)
{
    return snck_string_ref_buffer(p_ctxt, p_string, p_ref, strlen(p_ref));

} /* snck_string_ref() */

char
snck_string_ref_buffer(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    void const * const
        p_buf,
    size_t const
        i_buf_len)
{
    snck_string_cleanup(p_ctxt, p_string);

    snck_string_init_ref_buffer(p_string, p_buf, i_buf_len);

    return 1;

} /* snck_string_ref_buffer() */

char
snck_string_ref_object(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    struct snck_string const * const
        p_object)
{
    return snck_string_ref_buffer(p_ctxt, p_string, p_object->p_buf, p_object->i_buf_len);

} /* snck_string_ref_object() */

char
snck_string_copy(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    char const * const
        p_ref)
{
    size_t i_ref_len = strlen(p_ref);

    return snck_string_copy_buffer(p_ctxt, p_string, p_ref, i_ref_len);

} /* snck_string_copy() */

char
snck_string_resize(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    size_t const
        i_alloc_len)
{
    char b_result;

    if (i_alloc_len != p_string->i_alloc_len)
    {
        if (p_string->i_alloc_len)
        {
            p_string->p_buf = snck_heap_realloc(p_ctxt, p_string->p_buf, i_alloc_len);

            if (p_string->p_buf || !i_alloc_len)
            {
                p_string->i_alloc_len = i_alloc_len;

                b_result = 1;
            }
            else
            {
                p_string->i_alloc_len = 0u;

                p_string->i_buf_len = 0u;

                b_result = 0;
            }
        }
        else
        {
            char * p_new;

            p_new = snck_heap_realloc(p_ctxt, NULL, i_alloc_len);

            if (p_new && p_string->p_buf)
            {
                if (p_string->i_buf_len <= i_alloc_len)
                {
                    memcpy(p_new, p_string->p_buf, p_string->i_buf_len);
                }
                else
                {
                    memcpy(p_new, p_string->p_buf, i_alloc_len);
                }
            }

            if (p_new || !i_alloc_len)
            {
                p_string->p_buf = p_new;

                p_string->i_alloc_len = i_alloc_len;

                b_result = 1;
            }
            else
            {
                p_string->p_buf = NULL;

                p_string->i_alloc_len = 0u;

                p_string->i_buf_len = 0u;

                b_result = 0;
            }
        }
    }
    else
    {
        b_result = 1;
    }

    return b_result;

} /* snck_string_resize() */

char
snck_string_copy_buffer(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    void const * const
        p_buf,
    size_t const
        i_buf_len)
{
    char b_result;

    b_result = snck_string_resize(p_ctxt, p_string, i_buf_len);

    if (b_result)
    {
        memcpy(p_string->p_buf, p_buf, i_buf_len);

        p_string->i_buf_len = i_buf_len;
    }

    return b_result;

} /* snck_string_copy_buffer() */

char
snck_string_append(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    char const * const
        p_ref)
{
    size_t i_ref_len = strlen(p_ref);

    return snck_string_append_buffer(p_ctxt, p_string, p_ref, i_ref_len);

} /* snck_string_append() */

char
snck_string_append_buffer(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    void const * const
        p_buf,
    size_t const
        i_buf_len)
{
    char b_result;

    b_result = snck_string_resize(p_ctxt, p_string, p_string->i_buf_len + i_buf_len);

    if (b_result)
    {
        memcpy(p_string->p_buf + p_string->i_buf_len, p_buf, i_buf_len);

        p_string->i_buf_len += i_buf_len;
    }

    return b_result;

} /* snck_string_append_buffer() */

char
snck_string_copy_object(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    struct snck_string const * const
        p_object)
{
    return snck_string_copy_buffer(p_ctxt, p_string, p_object->p_buf, p_object->i_buf_len);

} /* snck_string_init() */

char
snck_string_append_object(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    struct snck_string const * const
        p_object)
{
    return snck_string_append_buffer(p_ctxt, p_string, p_object->p_buf, p_object->i_buf_len);

} /* snck_string_append_object() */

int
snck_string_compare(
    struct snck_string const * const
        p_ref1,
    struct snck_string const * const
        p_ref2)
{
    int i_result;

    size_t i_pos;

    i_result = 0;

    i_pos = 0u;

    while ((0 == i_result) && ((i_pos < p_ref1->i_buf_len) || (i_pos < p_ref2->i_buf_len)))
    {
        char c_ref1;

        char c_ref2;

        c_ref1 = (i_pos < p_ref1->i_buf_len) ? p_ref1->p_buf[i_pos] : '\000';

        c_ref2 = (i_pos < p_ref2->i_buf_len) ? p_ref2->p_buf[i_pos] : '\000';

        if (c_ref1 == c_ref2)
        {
            i_pos ++;
        }
        else
        {
            i_result = (c_ref1 - c_ref2);
        }
    }

    return i_result;

} /* snck_string_compare() */

int
snck_string_compare_n(
    struct snck_string const * const
        p_ref1,
    struct snck_string const * const
        p_ref2,
    size_t const
        i_max_len)
{
    int i_result;

    size_t i_pos;

    i_result = 0;

    i_pos = 0u;

    while ((0 == i_result) && ((i_pos < p_ref1->i_buf_len) || (i_pos < p_ref2->i_buf_len)) && (i_pos < i_max_len))
    {
        char c_ref1;

        char c_ref2;

        c_ref1 = (i_pos < p_ref1->i_buf_len) ? p_ref1->p_buf[i_pos] : '\000';

        c_ref2 = (i_pos < p_ref2->i_buf_len) ? p_ref2->p_buf[i_pos] : '\000';

        if (c_ref1 == c_ref2)
        {
            i_pos ++;
        }
        else
        {
            i_result = (c_ref1 - c_ref2);
        }
    }

    return i_result;

} /* snck_string_compare_n() */

char *
snck_string_get(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_string)
{
    char * p_buf;

    p_buf = snck_heap_realloc(p_ctxt, NULL, p_string->i_buf_len + 1u);

    if (p_buf)
    {
        memcpy(p_buf, p_string->p_buf, p_string->i_buf_len);

        p_buf[p_string->i_buf_len] = '\000';
    }

    return p_buf;

} /* snck_string_get() */

void
snck_string_put(
    struct snck_ctxt const * const
        p_ctxt,
    char * const
        p_buf)
{
    snck_heap_realloc(p_ctxt, p_buf, 0u);

} /* snck_string_put() */

void
snck_string_lower(
    struct snck_string * const
        p_string)
{
    size_t i;

    i = 0u;

    while (i < p_string->i_buf_len)
    {
        if ((p_string->p_buf[i] >= 'A')
            && (p_string->p_buf[i] <= 'Z'))
        {
            p_string->p_buf[i] =
                (char)(
                    p_string->p_buf[i] + ('a' - 'A'));
        }

        i++;
    }

} /* snck_string_lower() */

void
snck_string_upper(
    struct snck_string * const
        p_string)
{
    size_t i;

    i = 0u;

    while (i < p_string->i_buf_len)
    {
        if ((p_string->p_buf[i] >= 'a')
            && (p_string->p_buf[i] <= 'z'))
        {
            p_string->p_buf[i] =
                (char)(
                    p_string->p_buf[i] + ('A' - 'a'));
        }

        i++;
    }

} /* snck_string_upper() */

int
snck_fuzzy_compare(
    struct snck_string const * const
        p_ref1,
    struct snck_string const * const
        p_ref2)
{
    int i_result;

    size_t i_ref1;

    size_t i_ref2;

    if (p_ref2->i_buf_len >= 1)
    {
        i_result = 0;

        i_ref1 = 0;

        i_ref2 = 0;

        /* try to find each letter of p_ref2[0:i_ref2_len-1] within p_ref1 */
        while ((i_ref1 < p_ref1->i_buf_len) && (i_ref2 < p_ref2->i_buf_len))
        {
            /* Look for a letter */
            if (p_ref1->p_buf[i_ref1] == p_ref2->p_buf[i_ref2])
            {
                i_ref1 ++;
                i_ref2 ++;
            }
            else
            {
                i_ref1 ++;
            }
        }

        if (i_ref2 >= p_ref2->i_buf_len)
        {
            i_result = i_ref1;
        }
    }
    else
    {
        i_result = 1;
    }

    return i_result;

} /* snck_fuzzy_compare() */

/* end-of-file: snck_string.c */
