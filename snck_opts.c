/* See LICENSE for license details. */

/*

Module: snck_opts.h

Description:

    Command-line options of snck process.

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Context */
#include "snck_ctxt.h"

/* Module */
#include "snck_opts.h"

/*

Function: snck_opts_init

Description:

    Initialize module using array of command-line options.

Comments:

    .   Detect login shell when process name begins with dash

    .   When option begins with dash, option is set.  If option begins with
        a plus, option is reset.

*/
char
snck_opts_init(
    struct snck_ctxt const * const
        p_ctxt,
    unsigned int const
        i_argc,
    char * * const
        p_argv)
{
    char b_result;

    struct snck_opts * const p_opts = p_ctxt->p_opts;

    unsigned int i;

    p_opts->p_script = NULL;

    p_opts->p_argv = NULL;

    p_opts->i_argc = 0u;

    p_opts->b_login = 0;

    p_opts->b_command = 0;

    p_opts->b_input = 0;

    p_opts->b_trace = 0;

    p_opts->b_interact = -1;

    p_opts->b_dryrun = 0;

    if ('-' == p_argv[0u][0u])
    {
        p_opts->b_login = 1;
    }

    i = 1;

    while (i < i_argc)
    {
        if (('-' == p_argv[i][0u])
            || ('+' == p_argv[i][0u]))
        {
            char b = ('-' == p_argv[i][0u]);

            if ('c' == p_argv[i][1u])
            {
                p_opts->b_command = b;
            }
            else if ('l' == p_argv[i][1u])
            {
                p_opts->b_login = b;
            }
            else if ('s' == p_argv[i][1u])
            {
                p_opts->b_input = b;
            }
            else if ('x' == p_argv[i][1u])
            {
                p_opts->b_trace = b;
            }
            else if ('i' == p_argv[i][1u])
            {
                p_opts->b_interact = b;
            }
            else if ('n' == p_argv[i][1u])
            {
                p_opts->b_dryrun = b;
            }

            i ++;
        }
        else
        {
            if (!p_opts->b_input)
            {
                p_opts->p_script = p_argv[i];

                i ++;
            }

            if (i < i_argc)
            {
                p_opts->p_argv = p_argv + i;

                p_opts->i_argc = (unsigned int)(i_argc - i);

                i = i_argc;
            }
        }
    }

    /* Determine if we are in interactive mode */
    if (p_opts->b_interact < 0)
    {
        if (p_opts->b_command)
        {
            p_opts->b_interact = 0;
        }
        else if (p_opts->p_script)
        {
            p_opts->b_interact = 0;
        }
        else
        {
            if (isatty(0))
            {
                p_opts->b_interact = 1;
            }
            else
            {
                p_opts->b_interact = 0;
            }
        }
    }

    b_result = 1;

    return b_result;

} /* snck_opts_init() */

/*

Function: snck_opts_cleanup

Description:

    Free resources allocated by snck_opts_init().

*/
void
snck_opts_cleanup(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_opts * const p_opts = p_ctxt->p_opts;

    (void)(p_opts);

} /* snck_opts_cleanup() */

/* end-of-file: snck_opts.c */
