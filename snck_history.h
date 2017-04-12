/* See LICENSE for license details. */

/*

Module: snck_history.h

Description:

    Command line history file.

Comments:

    -   The file is loaded during command line completion.

    -   The file is updated after command line execution.

*/

/* Reverse include guard */
#if defined(INC_SNCK_HISTORY_H)
#error include snck_history.h once
#endif /* #if defined(INC_SNCK_HISTORY_H) */

#define INC_SNCK_HISTORY_H

/* Predefine context handle */
struct snck_ctxt;

/*

Structure: snck_history_line

Description:

    Single line from history file loaded into memory.

*/
struct snck_history_line
{
    struct snck_list o_list;

    struct snck_string o_buf;

}; /* struct snck_history_line */

/*

Structure: snck_history

Description:

    Contents of history file loaded into memory.

*/
struct snck_history
{
    struct snck_list o_list;

    struct snck_string o_name;

    struct snck_list o_cache;

}; /* struct snck_history */

/* --- Interface --- */

char
snck_history_init(
    struct snck_ctxt const * const
        p_ctxt);

void
snck_history_cleanup(
    struct snck_ctxt const * const
        p_ctxt);

void
snck_history_load(
    struct snck_ctxt const * const
        p_ctxt);

void
snck_history_unload(
    struct snck_ctxt const * const
        p_ctxt);

void
snck_history_save(
    struct snck_ctxt const * const
        p_ctxt);

void
snck_history_add(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_buf);

/* end-of-file: snck_history.h */
