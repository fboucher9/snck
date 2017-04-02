/* See LICENSE for license details. */

/*

Module: snck_opts.h

Description:

    Command-line options of snck process.

*/

/* Reverse include guard */
#if defined(INC_SNCK_OPTS_H)
#error include snck_opts.h once
#endif /* #if defined(INC_SNCK_OPTS_H) */

#define INC_SNCK_OPTS_H

/* Predefine context handle */
struct snck_ctxt;

/* Predefine main argument list */
struct snck_string;

/*

Structure: snck_opts

Description:

    Command-line options.

Comments:

    -l  login
    -c  command
    -s  stdin

*/
struct snck_opts
{
    struct snck_string const * p_script;

    struct snck_string const * p_argv;

    size_t i_argc;

    char b_login;

    char b_command;

    char b_input;

    char b_trace;

    char b_interact;

    char b_dryrun;

}; /* struct snck_opts */

char
snck_opts_init(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_arg_list,
    size_t const
        i_arg_count);

void
snck_opts_cleanup(
    struct snck_ctxt const * const
        p_ctxt);

/* end-of-file: snck_opts.h */
