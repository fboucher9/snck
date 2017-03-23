/* See LICENSE for license details. */

/*

Module: snck_file.h

Description:

    Process an entire file as input to shell.

*/

/* Reverse include guard */
#if defined(INC_SNCK_FILE_H)
#error include snck_file.h once
#endif /* #if defined(INC_SNCK_FILE_H) */

#define INC_SNCK_FILE_H

/* Predefine context handle */
struct snck_ctxt;

/* --- Interface --- */

char
snck_file_read(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_name);

/* end-of-file: snck_file.h */
