/* See LICENSE for license details. */

/*

Module: snck_heap.h

Description:

    Memory management.

*/

/* Reverse include guard */
#if defined(INC_SNCK_HEAP_H)
#error include snck_heap.h once
#endif /* #if defined(INC_SNCK_HEAP_H) */

#define INC_SNCK_HEAP_H

/* Predefine context handle */
struct snck_ctxt;

/*

Structure: snck_heap

Description:

    Memory management.

*/
struct snck_heap
{
    int i_count;

}; /* struct snck_heap */

char
snck_heap_init(
    struct snck_ctxt const * const
        p_ctxt);

void
snck_heap_cleanup(
    struct snck_ctxt const * const
        p_ctxt);

void *
snck_heap_realloc(
    struct snck_ctxt const * const
        p_ctxt,
    void * const
        p_buf,
    size_t const
        i_buf_len);

/* end-of-file: snck_heap.h */
