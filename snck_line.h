/* See LICENSE for license details. */

/*

Module: snck_line.h

Description:

    Line editor.

*/

/* Reverse include guard */
#if defined(INC_SNCK_LINE_H)
#error include snck_line.h once
#endif /* #if defined(INC_SNCK_LINE_H) */

#define INC_SNCK_LINE_H

/* Predefine context handle */
struct snck_ctxt;

/*

Structure: snck_line

Description:

    Line editor.

*/
struct snck_line
{
    int i_dummy;

}; /* struct snck_line */

char
snck_line_init(
    struct snck_ctxt const * const
        p_ctxt);

void
snck_line_cleanup(
    struct snck_ctxt const * const
        p_ctxt);

char
snck_line_get(
    struct snck_ctxt const * const
        p_ctxt,
    FILE * const
        p_file,
    struct snck_string * const
        p_string,
    char const
        b_overflow);

/* end-of-file: snck_line.h */
