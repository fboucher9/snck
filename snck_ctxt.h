/* See LICENSE for license details. */

/*

Module: snck_ctxt.h

Description:

    Context handle for all modules.

*/

/* Reverse include guard */
#if defined(INC_SNCK_CTXT_H)
#error include snck_ctxt.h once
#endif /* #if defined(INC_SNCK_CTXT_H) */

#define INC_SNCK_CTXT_H

/* Predefine module handles */
struct snck_heap;

struct snck_info;

struct snck_passwd;

struct snck_line;

struct snck_history;

/*

Structure: snck_ctxt

Description:

    Context handle for all modules.

*/
struct snck_ctxt
{
    struct snck_heap * p_heap;

    struct snck_info * p_info;

    struct snck_passwd * p_passwd;

    struct snck_line * p_line;

    struct snck_history * p_history;

}; /* struct snck_ctxt */

/* end-of-file: snck_ctxt.h */
