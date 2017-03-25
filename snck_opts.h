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

/*

Structure: snck_opts

Description:

    Command-line options.

Comments:

    -i  interactive
    -I  ignore eof
    -l  login
    -n  dry run
    -x  trace
    -v  verbose
    -c  command
    -s  stdin

*/
struct snck_opts
{
    char * * p_argv;

    unsigned int i_argc;

    char b_login;

    char b_command;

    char b_input;

}; /* struct snck_opts */

char
snck_opts_init(
    struct snck_ctxt const * const
        p_ctxt,
    unsigned int const
        argc,
    char * * const
        argv);

void
snck_opts_cleanup(
    struct snck_ctxt const * const
        p_ctxt);

/* end-of-file: snck_opts.h */
