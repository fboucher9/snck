/* See LICENSE for license details. */

/*

Module: snck_main.h

Description:

    OS-independant application entry point.

*/

/* Reverse include guard */
#if defined(INC_SNCK_MAIN_H)
#error include snck_main.h once
#endif /* #if defined(INC_SNCK_MAIN_H) */

#define INC_SNCK_MAIN_H

/* Predefine os argument list */
struct snck_string;

/* --- Interface --- */

int
snck_main(
    struct snck_string const * const
        p_arg_list,
    size_t const
        i_arg_count);

/* end-of-file: snck_main.h */
