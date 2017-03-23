/* See LICENSE for license details. */

/*

Module: snck_main.c

Description:

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Context */
#include "snck_ctxt.h"

/* Module */
#include "snck_main.h"

/* String */
#include "snck_string.h"

/* Heap */
#include "snck_heap.h"

/* Information */
#include "snck_info.h"

/* Password database */
#include "snck_passwd.h"

/* Line editor */
#include "snck_line.h"

/* List */
#include "snck_list.h"

/* History */
#include "snck_history.h"

/* Env */
#include "snck_env.h"

/* File */
#include "snck_file.h"

struct snck_main_ctxt
{
    struct snck_ctxt o_ctxt;

    struct snck_heap o_heap;

    struct snck_passwd o_passwd;

    struct snck_info o_info;

    struct snck_line o_line;

    struct snck_history o_history;

}; /* struct snck_main_ctxt */

static struct snck_main_ctxt o_main_ctxt;

static void snck_sigchld(int unused)
{
    (void)(unused);
    while (0 < waitpid(-1, NULL, WNOHANG));
}

static
void
snck_signal_setup(
    struct snck_ctxt const * const p_ctxt)
{
    (void)(p_ctxt);

    /* install a SIGCHLD handler */
    signal(SIGCHLD, snck_sigchld);

    signal(SIGINT, SIG_IGN);

    signal(SIGHUP, SIG_IGN);

    /* signal(SIGTSTP, SIG_IGN); */

}

static
void
snck_env_setup(
    struct snck_ctxt const * const p_ctxt)
{
    static char a_name_shlvl[] = { 'S', 'H', 'L', 'V', 'L' };

    static char a_name_underscore[] = { '_' };

    static char a_name_mail[] = { 'M', 'A', 'I', 'L' };

    static char a_name_oldpwd[] = { 'O', 'L', 'D', 'P', 'W', 'D' };

    static struct snck_string const o_name_shlvl = { a_name_shlvl, sizeof(a_name_shlvl), 0u };

    static struct snck_string const o_name_underscore = { a_name_underscore, sizeof(a_name_underscore), 0u };

    static struct snck_string const o_name_mail = { a_name_mail, sizeof(a_name_mail), 0u };

    static struct snck_string const o_name_oldpwd = { a_name_oldpwd, sizeof(a_name_oldpwd), 0u };

    snck_env_set(p_ctxt, &(o_name_shlvl), NULL);

    snck_env_set(p_ctxt, &(o_name_underscore), NULL);

    snck_env_set(p_ctxt, &(o_name_mail), NULL);

    snck_env_set(p_ctxt, &(o_name_oldpwd), NULL);

} /* snck_env_setup() */

static
char
snck_main_init(
    struct snck_main_ctxt * const
        p_main)
{
    char b_result;

    struct snck_ctxt * p_ctxt;

    p_ctxt = &(p_main->o_ctxt);

    p_ctxt->p_heap = &(p_main->o_heap);

    p_ctxt->p_passwd = &(p_main->o_passwd);

    p_ctxt->p_info = &(p_main->o_info);

    p_ctxt->p_line = &(p_main->o_line);

    p_ctxt->p_history = &(p_main->o_history);

    if (snck_heap_init(p_ctxt))
    {
        snck_passwd_init(p_ctxt);

        if (snck_info_init(p_ctxt))
        {
            snck_signal_setup(p_ctxt);

            snck_env_setup(p_ctxt);

            snck_line_init(p_ctxt);

            snck_history_init(p_ctxt);

            b_result = 1;
        }
        else
        {
            b_result = 0;
        }

        if (!b_result)
        {
            snck_heap_cleanup(p_ctxt);
        }
    }
    else
    {
        b_result = 0;
    }

    return b_result;

} /* snck_main_init() */

static
void
snck_main_cleanup(
    struct snck_main_ctxt * const
        p_main)
{
    struct snck_ctxt const * const p_ctxt =
        &(p_main->o_ctxt);

    snck_history_cleanup(p_ctxt);

    snck_line_cleanup(p_ctxt);

    snck_info_cleanup(p_ctxt);

    snck_passwd_cleanup(p_ctxt);

    snck_heap_cleanup(p_ctxt);
}

/*

Function: snck_main

Description:

*/
int
snck_main(
    unsigned int i_argc,
    char * * p_argv)
{
    /* read commands from stdin */
    int i_exit_status;

    if (i_argc > 1)
    {
        p_argv[0u] = "/bin/sh";

        execvp(p_argv[0u], p_argv);

        i_exit_status = 1;
    }
    else
    {
        struct snck_main_ctxt * const p_main =
            &(o_main_ctxt);

        if (snck_main_init(p_main))
        {
            struct snck_ctxt const * const p_ctxt =
                &(p_main->o_ctxt);

            if (snck_file_read(p_ctxt, NULL))
            {
                i_exit_status = 0;
            }
            else
            {
                i_exit_status = 1;
            }

            snck_main_cleanup(p_main);
        }
        else
        {
            i_exit_status = 1;
        }
    }

    return i_exit_status;

} /* snck_main() */

/* end-of-file: snck_main.c */
