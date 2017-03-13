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

struct snck_main_ctxt
{
    struct snck_ctxt o_ctxt;

    struct snck_heap o_heap;

    struct snck_passwd o_passwd;

    struct snck_info o_info;

    struct snck_line o_line;

}; /* struct snck_main_ctxt */

static struct snck_main_ctxt o_main_ctxt;

static char * g_argv[1024u];

static unsigned int g_argc = 0;

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
snck_expand(
    char const * p_ref)
{
    /* Use sh -c 'echo ...' to expand the argument */
    static char a_command[4096u];

    char const * p_path;

    sprintf(a_command, "sh -c \'echo -n %s\'", p_ref);

    {
        FILE * p_pipe = popen(a_command, "r");

        if (p_pipe)
        {
            if (NULL != fgets(a_command, sizeof(a_command)-1, p_pipe))
            {
                p_path = a_command;
            }
            else
            {
                p_path = p_ref;
            }

            pclose(p_pipe);
        }
        else
        {
            p_path = p_ref;
        }
    }

    return p_path;
}

static
char
snck_builtin_cd(
    struct snck_ctxt const * const p_ctxt,
    char const * p_args)
{
    char b_result;

    int i_result;

    char const * p_path = NULL;

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
            p_path = snck_expand(p_args + i_args_it);

#if 0 /* expand takes care of home folder */
            if ('~' == p_args[i_args_it])
            {
                static char a_line[65536u];

                sprintf(a_line, "%s%s", p_ctxt->p_info->o_home.p_buf, p_args + i_args_it + 1);

                p_path = a_line;
            }
            else
            {
                p_path = p_args + i_args_it;
            }
#endif /* expand takes care of home folder */
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
                snck_string_copy_object(
                    p_ctxt,
                    &(
                        p_ctxt->p_info->o_old_pwd),
                    &(
                        p_ctxt->p_info->o_pwd));

                {
                    static char a_pwd[1024u];

                    a_pwd[0u] = '\000';

                    if (NULL != getcwd(a_pwd, sizeof(a_pwd)))
                    {
                    }
                    else
                    {
                        strcpy(a_pwd, "/");
                    }

                    if (0 == setenv("PWD", a_pwd, 1))
                    {
                    }
                    else
                    {
                    }

                    snck_string_copy(
                        p_ctxt,
                        &(p_ctxt->p_info->o_pwd),
                        a_pwd);
                }

                b_result = 1;
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

    return b_result;

} /* snck_builtin_cd() */

extern char ** environ;

static
char
snck_builtin_set(
    char const * p_args)
{
    char b_result;

    int i_result;

    char const * p_name;

    int i_name_len;

    if (snck_find_word(p_args, &(p_name), &(i_name_len)))
    {
        static char a_name[256u];

        if ((size_t)(i_name_len) < sizeof(a_name))
        {
            memcpy(a_name, p_name, i_name_len);

            a_name[i_name_len] = '\000';

            i_name_len += snck_find_word_begin(p_name + i_name_len);

            if (p_name[i_name_len])
            {
                char const * p_value;

                p_value = snck_expand(p_name + i_name_len);

                i_result = setenv(a_name, p_value, 1);

                (void)(i_result);
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
    char const * p_args)
{
    char b_result;

    int i_result;

    char const * p_name;

    int i_name_len;

    if (snck_find_word(p_args, &(p_name), &(i_name_len)))
    {
        static char a_name[256u];

        if ((size_t)(i_name_len) < sizeof(a_name))
        {
            memcpy(a_name, p_name, i_name_len);

            a_name[i_name_len] = '\000';

            i_result = unsetenv(a_name);

            (void)(i_result);
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

#if 0
static
char
snck_tokenize_line(
    char * p_line)
{
    char b_result;

    g_argc = 0u;

    g_argv[g_argc] = strtok(p_line, " \t\n");

    if (g_argv[g_argc])
    {
        while (g_argv[g_argc])
        {
            g_argc ++;

            g_argv[g_argc] = strtok(NULL, " \t\n");
        }
    }

    b_result = 1;

    return b_result;

} /* snck_tokenize_line() */
#endif

static
char
snck_builtin_shell(
    char const * p_args)
{
    char b_result;

    int i_args_it = snck_find_word_begin(p_args);

    if (p_args[i_args_it])
    {
        g_argv[0u] = (char *)(p_args + i_args_it);

        g_argv[1u] = NULL;

        g_argc = 1;

        execvp(g_argv[0u], g_argv);

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

    (void)(p_ctxt);

    g_argv[0u] = "/bin/sh";

    g_argv[1u] = "-c";

    /* detect if exec is possible */
    if (strchr(p_line, ';') ||
        strchr(p_line, '&') ||
        strchr(p_line, '|'))
    {
        g_argv[2u] = (char *)(p_line);

        g_argv[3u] = NULL;
    }
    else
    {
        static char a_split[65536u];

        sprintf(a_split, "exec %s", p_line);

        g_argv[2u] = a_split;

        g_argv[3u] = NULL;
    }

    /* execute the command */
    iChildProcess = fork();
    if (0 == iChildProcess)
    {
        /* first child writes into output */
        signal(SIGINT, SIG_DFL);
        /* signal(SIGTSTP, SIG_DFL); */
        signal(SIGCHLD, SIG_DFL);
        execvp(g_argv[0u], g_argv);
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
            b_result = snck_builtin_set(p_cmd + i_cmd_len);
        }
        else if ((5 == i_cmd_len) && (0 == strncmp(p_cmd, "unset", i_cmd_len)))
        {
            b_result = snck_builtin_unset(p_cmd + i_cmd_len);
        }
        else if ((5 == i_cmd_len) && (0 == strncmp(p_cmd, "shell", i_cmd_len)))
        {
            b_result = snck_builtin_shell(p_cmd + i_cmd_len);
        }
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
