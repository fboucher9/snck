/* See LICENSE for license details. */

/*

Module: snck_heap.c

Description:

    Memory management.

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Context */
#include "snck_ctxt.h"

/* Module */
#include "snck_heap.h"

char
snck_heap_init(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_heap * const p_heap =
        p_ctxt->p_heap;

    p_heap->i_count =
        0;

    return 1;

} /* snck_heap_init() */

void
snck_heap_cleanup(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_heap * const p_heap =
        p_ctxt->p_heap;

    if (0 != p_heap->i_count)
    {
        fprintf(stderr, "%d memory leak(s) detected\n", p_heap->i_count);
    }

} /* snck_heap_cleanup() */

void *
snck_heap_realloc(
    struct snck_ctxt const * const
        p_ctxt,
    void * const
        p_buf,
    size_t const
        i_buf_len)
{
    struct snck_heap * const p_heap =
        p_ctxt->p_heap;

    void * p_result;

    if (p_buf)
    {
        p_heap->i_count --;
    }

    if (i_buf_len)
    {
        if (p_buf)
        {
            p_result =
                realloc(
                    p_buf,
                    i_buf_len);
        }
        else
        {
            p_result =
                malloc(
                    i_buf_len);
        }
    }
    else
    {
        free(p_buf);

        p_result = NULL;
    }

    if (
        p_result)
    {
        p_heap->i_count ++;
    }

    return
        p_result;

} /* snck_heap_realloc() */

/* end-of-file: snck_heap.c */
