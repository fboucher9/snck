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

/* */
#if defined(SNCK_FEATURE_LINENOISE)
#include <linenoise.h>
#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

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

static
int
snck_find_word_begin(
    char const * p_args)
{
    int i_args_it = 0;

    while ((p_args[i_args_it] == ' ') || (p_args[i_args_it] == '\t'))
    {
        i_args_it ++;
    }

    return i_args_it;
}

int
snck_find_word_end(
    char const * p_args)
{
    int i_args_it = 0;

    while (p_args[i_args_it] && (' ' != p_args[i_args_it]) && ('\t' != p_args[i_args_it]))
    {
        i_args_it ++;
    }

    return i_args_it;
}

static
char
snck_find_word(
    char const * p_args,
    char const * * p_word_buf,
    int * p_word_len)
{
    char b_found;

    int i_args_it = snck_find_word_begin(p_args);

    if (p_args[i_args_it])
    {
        *p_word_buf = p_args + i_args_it;

        *p_word_len = snck_find_word_end(p_args + i_args_it);

        b_found = 1;
    }
    else
    {
        b_found = 0;
    }

    return b_found;

} /* snck_find_word() */

static
char const *
snck_expand_get(
    struct snck_ctxt const * const p_ctxt,
    char const * p_ref)
{
    /* Use sh -c 'echo ...' to expand the argument */
    static char const a_fmt[] = "sh -c \'echo -n %s\'";

    char * p_buf;

    size_t i_buf_max_len;

    size_t i_buf_len;

    i_buf_max_len = 32u;

    i_buf_len = 0;

    p_buf = snck_heap_realloc(p_ctxt, NULL, i_buf_max_len + 1);

    if (p_buf)
    {
        char * p_path;

        p_path = snck_heap_realloc(p_ctxt, NULL, sizeof(a_fmt) + strlen(p_ref) + 1);

        if (p_path)
        {
            sprintf(p_path, a_fmt, p_ref);

            {
                FILE * p_pipe = popen(p_path, "r");

                if (p_pipe)
                {
                    char b_more;

                    b_more = 1;

                    while (b_more)
                    {
                        int c;

                        c = fgetc(p_pipe);

                        if (EOF != c)
                        {
                            if (i_buf_len >= i_buf_max_len)
                            {
                                i_buf_max_len += 32u;

                                p_buf = snck_heap_realloc(p_ctxt, p_buf, i_buf_max_len + 1);
                            }

                            if (p_buf)
                            {
                                p_buf[i_buf_len] = (char)(c);

                                i_buf_len ++;
                            }
                            else
                            {
                                b_more = 0;
                            }
                        }
                        else
                        {
                            b_more = 0;
                        }
                    }

                    if (p_buf)
                    {
                        p_buf[i_buf_len] = '\000';

                        i_buf_len ++;
                    }

                    pclose(p_pipe);
                }
                else
                {
                    i_buf_max_len = strlen(p_ref) + 1;

                    p_buf = snck_heap_realloc(p_ctxt, p_buf, i_buf_max_len + 1);

                    if (p_buf)
                    {
                        strcpy(p_buf, p_ref);
                    }
                }
            }

            snck_heap_realloc(p_ctxt, p_path, 0u);
        }
        else
        {
            i_buf_max_len = strlen(p_ref) + 1;

            p_buf = snck_heap_realloc(p_ctxt, p_buf, i_buf_max_len + 1);

            if (p_buf)
            {
                strcpy(p_buf, p_ref);
            }
        }
    }

    return p_buf;

} /* snck_expand_get() */

static
void
snck_expand_put(
    struct snck_ctxt const * const p_ctxt,
    char const * p_buf)
{
    snck_heap_realloc(p_ctxt, (void *)(p_buf), 0u);

} /* snck_expand_put() */

static
char
snck_builtin_cd(
    struct snck_ctxt const * const p_ctxt,
    char const * p_args)
{
    char b_result;

    int i_result;

    char const * p_path = NULL;

    char const * p_path_expand = NULL;

    int i_args_it = snck_find_word_begin(p_args);

    if (p_args[i_args_it])
    {
        if (0 == strcmp(p_args + i_args_it, "-"))
        {
            p_path = p_ctxt->p_info->o_old_pwd.p_buf;
        }
        else
        {
            /* Use sh -c 'echo ...' to expand the argument */
            p_path_expand = snck_expand_get(p_ctxt, p_args + i_args_it);

            p_path = p_path_expand;
        }
    }
    else
    {
        p_path = p_ctxt->p_info->o_home.p_buf;
    }

    if (p_path)
    {
        if (0 != strcmp(p_path, "."))
        {
            i_result = chdir(p_path);

            if (0 == i_result)
            {
                b_result = snck_info_update_wd(p_ctxt);
            }
            else
            {
                fprintf(stderr, "failure to change directory\n");

                b_result = 1;
            }
        }
        else
        {
            /* Nothing changed */
            b_result = 1;
        }
    }
    else
    {
        /* Nothing changed */
        b_result = 1;
    }

    if (p_path_expand)
    {
        snck_expand_put(p_ctxt, p_path_expand);

        p_path_expand = NULL;
    }

    return b_result;

} /* snck_builtin_cd() */

extern char ** environ;

static
char
snck_builtin_set(
    struct snck_ctxt const * const p_ctxt,
    char const * p_args)
{
    char b_result;

    int i_result;

    char const * p_name;

    int i_name_len;

    if (snck_find_word(p_args, &(p_name), &(i_name_len)))
    {
        char * a_name;

        a_name = snck_heap_realloc(p_ctxt, NULL, i_name_len + 1);

        if (a_name)
        {
            memcpy(a_name, p_name, i_name_len);

            a_name[i_name_len] = '\000';

            i_name_len += snck_find_word_begin(p_name + i_name_len);

            if (p_name[i_name_len])
            {
                char const * p_value;

                p_value = snck_expand_get(p_ctxt, p_name + i_name_len);

                if (p_value)
                {
                    i_result = setenv(a_name, p_value, 1);

                    (void)(i_result);

                    snck_expand_put(p_ctxt, p_value);
                }
            }
            else
            {
                /* */
                char * p_value;

                p_value = getenv(a_name);

                if (p_value)
                {
                    fprintf(stdout, "%s\n", p_value);
                }
                else
                {
                    fprintf(stderr, "not set\n");
                }
            }

            snck_heap_realloc(p_ctxt, a_name, 0u);
        }
        else
        {
            fprintf(stderr, "snck: name too long\n");
        }
    }
    else
    {
        /* dump all vars */
        int i;

        i = 0;

        while (environ[i])
        {
            fprintf(stdout, "%s\n", environ[i]);

            i++;
        }
    }

    b_result = 1;

    return b_result;

} /* snck_builtin_set() */

static
char
snck_builtin_unset(
    struct snck_ctxt const * const p_ctxt,
    char const * p_args)
{
    char b_result;

    int i_result;

    char const * p_name;

    int i_name_len;

    if (snck_find_word(p_args, &(p_name), &(i_name_len)))
    {
        char * a_name;

        a_name = snck_heap_realloc(p_ctxt, NULL, i_name_len + 1);

        if (a_name)
        {
            memcpy(a_name, p_name, i_name_len);

            a_name[i_name_len] = '\000';

            i_result = unsetenv(a_name);

            (void)(i_result);

            snck_heap_realloc(p_ctxt, a_name, 0u);
        }
        else
        {
            fprintf(stderr, "snck: name too long\n");
        }
    }
    else
    {
    }

    b_result = 1;

    return b_result;

} /* snck_builtin_unset() */

static
char
snck_builtin_shell(
    char const * p_args)
{
    char b_result;

    int i_args_it = snck_find_word_begin(p_args);

    if (p_args[i_args_it])
    {
        char * l_argv[2u];

        l_argv[0u] = (char *)(p_args + i_args_it);

        l_argv[1u] = NULL;

        execvp(l_argv[0u], l_argv);

        fprintf(stderr, "unable to replace shell\n");

        b_result = 1;
    }
    else
    {
        fprintf(stderr, "missing argument\n");

        b_result = 1;
    }

    return b_result;

} /* snck_builtin_shell() */

static
char
snck_fork_and_exec(
    struct snck_ctxt const * const p_ctxt,
    char const * const p_line)
{
    char b_result;

    pid_t iChildProcess;

    char * a_split = NULL;

    char * l_argv[4u];

    (void)(p_ctxt);

    l_argv[0u] = "/bin/sh";

    l_argv[1u] = "-c";

    /* detect if exec is possible */
    if (strchr(p_line, ';') ||
        strchr(p_line, '&') ||
        strchr(p_line, '|'))
    {
        l_argv[2u] = (char *)(p_line);
    }
    else
    {
        static char a_fmt[] = "exec %s";

        a_split = snck_heap_realloc(p_ctxt, NULL, sizeof(a_fmt) + strlen(p_line) + 1);

        if (a_split)
        {
            sprintf(a_split, a_fmt, p_line);
        }

        l_argv[2u] = a_split;
    }

    l_argv[3u] = NULL;

    /* execute the command */
    iChildProcess = fork();
    if (0 == iChildProcess)
    {
        /* first child writes into output */
        signal(SIGINT, SIG_DFL);
        /* signal(SIGTSTP, SIG_DFL); */
        signal(SIGCHLD, SIG_DFL);
        execvp(l_argv[0u], l_argv);
        fprintf(stderr, "snck: execvp failure!\n");

        b_result = 0;
    }
    else
    {
        /* wait for task to finish executing... */
        int resultStatus;
        do
        {
            waitpid(iChildProcess, &resultStatus, WUNTRACED);
        }
        while (!WIFEXITED(resultStatus) && !WIFSIGNALED(resultStatus));

        {
            int iExitCode;

            iExitCode = WEXITSTATUS(resultStatus);

            if (iExitCode)
            {
                fprintf(stderr, "snck: error code is %d\n", iExitCode);
            }
        }

        b_result = 1;
    }

    if (a_split)
    {
        snck_heap_realloc(p_ctxt, a_split, 0u);

        a_split = NULL;
    }

    return b_result;

} /* snck_fork_and_exec() */

static
char
snck_execute_child(
    struct snck_ctxt const * const p_ctxt,
    char const * const p_line)
{
    char b_result;

    char const * p_cmd;

    int i_cmd_len;

    if (snck_find_word(p_line, &(p_cmd), &(i_cmd_len)))
    {
        if ('#' == p_cmd[0u])
        {
            /* This is a comment line */
            b_result = 1;
        }
        else if (0 == i_cmd_len)
        {
            /* Empty line */
            b_result = 1;
        }
        else if ((2 == i_cmd_len) && (0 == strncmp(p_cmd, "cd", i_cmd_len)))
        {
            b_result = snck_builtin_cd(p_ctxt, p_cmd + i_cmd_len);
        }
        else if ((3 == i_cmd_len) && (0 == strncmp(p_cmd, "set", i_cmd_len)))
        {
            b_result = snck_builtin_set(p_ctxt, p_cmd + i_cmd_len);
        }
        else if ((5 == i_cmd_len) && (0 == strncmp(p_cmd, "unset", i_cmd_len)))
        {
            b_result = snck_builtin_unset(p_ctxt, p_cmd + i_cmd_len);
        }
        else if ((5 == i_cmd_len) && (0 == strncmp(p_cmd, "shell", i_cmd_len)))
        {
            b_result = snck_builtin_shell(p_cmd + i_cmd_len);
        }
#if 0
        else if ((4 == i_cmd_len) && (0 == strncmp(p_cmd, "hist", i_cmd_len)))
        {
            int i;

            for (i = 9; i >= 0; i--)
            {
                fprintf(stderr, "%6d: %s\n",
                    history_len - 1 - i,
                    history[history_len - 1 - i]);
            }

            b_result = 1;
        }
#endif
        else if (
            (
                (
                    4 == i_cmd_len)
                && (
                    0 == strncmp(p_cmd, "exit", i_cmd_len)))
            || (
                (
                    6 == i_cmd_len)
                && (
                    0 == strncmp(p_cmd, "logout", i_cmd_len))))
        {
            exit(0);
        }
        else
        {
            b_result = snck_fork_and_exec(p_ctxt, p_line);
        }
    }
    else
    {
        b_result = 1;
    }

    return b_result;

} /* snck_execute_child() */

static
char
snck_read_file(
    struct snck_ctxt const * const p_ctxt)
{
    char b_result = 1;

    char b_continue = 1;

    while (b_result && b_continue)
    {
        char const * p_line;

        p_line = snck_line_get(p_ctxt);

        if (p_line)
        {
            if (snck_execute_child(p_ctxt, p_line))
            {
            }
            else
            {
                b_result = 0;

                b_continue = 0;
            }

            snck_line_put(p_ctxt, p_line);
        }
        else
        {
            b_continue = 0;
        }
    }

    return b_result;
}

static void snck_sigchld(int unused)
{
    (void)(unused);
    while (0 < waitpid(-1, NULL, WNOHANG));
}

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
            /* install a SIGCHLD handler */
            signal(SIGCHLD, snck_sigchld);

            signal(SIGINT, SIG_IGN);

            signal(SIGHUP, SIG_IGN);

            /* signal(SIGTSTP, SIG_IGN); */

            unsetenv("SHLVL");

            unsetenv("_");

            unsetenv("MAIL");

            unsetenv("OLDPWD");

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

            if (snck_read_file(p_ctxt))
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
