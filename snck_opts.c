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

    memset(p_opts, 0x00u, sizeof(p_opts));

    if ('-' == p_argv[0u][0u])
    {
        p_opts->b_login = 1;
    }

    i = 1;

    while (i < i_argc)
    {
        if ('-' == p_argv[i][0u])
        {
            if ('c' == p_argv[i][1u])
            {
                p_opts->b_command = 1;
            }
            else if ('l' == p_argv[i][1u])
            {
                p_opts->b_login = 1;
            }
            else if ('s' == p_argv[i][1u])
            {
                p_opts->b_input = 1;
            }

            i ++;
        }
        else
        {
            p_opts->p_argv = p_argv + i;

            p_opts->i_argc = (unsigned int)(i_argc - i);

            break;
        }
    }

    b_result = 1;

    return b_result;

} /* snck_opts_init() */

void
snck_opts_cleanup(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_opts * const p_opts = p_ctxt->p_opts;

    (void)(p_opts);

} /* snck_opts_cleanup() */

/* end-of-file: snck_opts.c */
