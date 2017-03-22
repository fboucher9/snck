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

} /* snck_string_init() */

void
snck_string_cleanup(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string)
{
    if (p_string->p_buf)
    {
        snck_heap_realloc(p_ctxt, p_string->p_buf, 0u);

        p_string->p_buf = NULL;

        p_string->i_buf_len = 0;
    }

} /* snck_string_cleanup() */

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

    p_string->p_buf = snck_heap_realloc(p_ctxt, p_string->p_buf, i_buf_len + 1);

    if (p_string->p_buf)
    {
        memcpy(p_string->p_buf, p_buf, i_buf_len);

        p_string->p_buf[i_buf_len] = '\000';

        p_string->i_buf_len = i_buf_len;

        b_result = 1;
    }
    else
    {
        p_string->i_buf_len = 0u;

        b_result = 0;
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

    p_string->p_buf = snck_heap_realloc(p_ctxt, p_string->p_buf, p_string->i_buf_len + i_buf_len + 1);

    if (p_string->p_buf)
    {
        memcpy(p_string->p_buf + p_string->i_buf_len, p_buf, i_buf_len);

        p_string->p_buf[p_string->i_buf_len + i_buf_len] = '\000';

        p_string->i_buf_len += i_buf_len;

        b_result = 1;
    }
    else
    {
        p_string->i_buf_len = 0u;

        b_result = 0;
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

/* end-of-file: snck_string.c */
