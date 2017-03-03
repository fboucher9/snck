/* See LICENSE for license details. */

/*

Module: snck_main.c

Description:

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Module */
#include "snck_main.h"

/* */
#if defined(SNCK_FEATURE_LINENOISE)
#include <linenoise.h>
#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

static char a_line[65536u];

static char a_home[1024u];

static char a_user[1024u];

static char a_host[1024u];

static char * g_argv[1024u];

static unsigned int g_argc = 0;

static
char
snck_builtin_cd(void)
{
    char b_result;

    if (g_argc > 1u)
    {
        chdir(g_argv[1u]);
    }
    else
    {
        chdir(a_home);
    }

    b_result = 1;

    return b_result;

} /* snck_builtin_cd() */

static
char
snck_fork_and_exec(void)
{
    char b_result;

    pid_t iChildProcess;

    /* Create a command line for sh */
    g_argv[0u] = "/bin/sh";

    g_argv[1u] = "-c";

    g_argv[2u] = a_line;

    g_argv[3u] = NULL;

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

        fprintf(stderr, "snck: error code is %d\n", WEXITSTATUS(resultStatus));

        b_result = 1;
    }

    return b_result;

} /* snck_fork_and_exec() */

static
char
snck_execute_child(void)
{
    char b_result;

    if (0 == strcmp(g_argv[0u], "cd"))
    {
        b_result = snck_builtin_cd();
    }
    else
    {
        b_result = snck_fork_and_exec();
    }

    return b_result;

} /* snck_execute_child() */

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

/*

Function: snck_process_line

Description:

*/
static
char
snck_process_line(
    char * p_line)
{
    char b_result;

    /* tokenize the input buffer */
    /* token is word or function or mix of both: */
    /* word */
    /* [function] */
    /* wo[func][func]rd */
    /* wo[sp]rd */

    strcpy(a_line, p_line);

    b_result = snck_tokenize_line(p_line);

    if (b_result)
    {
        if (g_argc)
        {
            /* expand of functions */

            b_result = snck_execute_child();
        }
        else
        {
            b_result = 1;
        }
    }

    return b_result;

} /* snck_process_line() */

static
void
snck_build_prompt(
    char * a_prompt,
    size_t i_prompt_len)
{
    static char a_pwd[1024u];

    a_pwd[0] = '\000';

    getcwd(a_pwd, sizeof(a_pwd));

    if (0 == strncmp(a_pwd, a_home, strlen(a_home)))
    {
        sprintf(a_prompt, "%s@%s:~%s$ ", a_user, a_host, a_pwd + strlen(a_home));
    }
    else
    {
        sprintf(a_prompt, "%s@%s:%s$ ", a_user, a_host, a_pwd);
    }
}

#if defined(SNCK_FEATURE_LINENOISE)

static
void
snck_completion(
    char const * buf,
    size_t pos,
    linenoiseCompletions * lc)
{
    /* quick tokenize of current line */
    /* find the word under the cursor */
    /* locate words before and words after */
    /* complete entire line and replace word with other ... */

    int pos0;

    if (pos > 0)
    {
        pos0 = (int)(pos - 1);

        while (pos0 >= 0)
        {
            if (buf[pos0] == ' ')
            {
                break;
            }

            pos0 --;
        }

        pos0 ++;
    }
    else
    {
        pos0 = 0;
    }

    /* Find the directory name */

    /* From beginning of word to last slash */

    int pos1;

    if (pos > 0)
    {
        pos1 = (int)(pos - 1);

        while (pos1 >= pos0)
        {
            if (buf[pos1] == '/')
            {
                break;
            }

            pos1 --;
        }

        pos1++;
    }
    else
    {
        pos1 = 0;
    }

    static char a_folder[1024u];

    a_folder[0u] = '\000';

    if (pos1 > pos0)
    {
        memcpy(a_folder, buf + pos0, pos1 - pos0);

        a_folder[pos1 - pos0] = '\000';
    }
    else
    {
        strcpy(a_folder, ".");
    }

    DIR * d;

    d = opendir(a_folder);
    if (d)
    {
        while (1)
        {
            struct dirent * e;

            e = readdir(d);
            if (e)
            {
                if ((0 == strcmp(e->d_name, ".")) || (0 == strcmp(e->d_name, "..")))
                {
                }
                else if (0 == strncmp(e->d_name, buf + pos1, pos - pos1))
                {
                    static char suggest[1024u];

                    if (pos1 > 0)
                    {
                        sprintf(suggest, "%.*s%s", pos1, buf, e->d_name);
                    }
                    else
                    {
                        sprintf(suggest, "%s", e->d_name);
                    }

                    linenoiseAddCompletion(lc, suggest);
                }
            }
            else
            {
                break;
            }
        }
        closedir(d);
    }
} /* snck_completion */

#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

static
char
snck_read_line(
    char * a_line,
    size_t i_line_len)
{
    static char a_prompt[4096u];

    char b_result;

    char * p_line;

    snck_build_prompt(a_prompt, sizeof(a_prompt));

#if defined(SNCK_FEATURE_LINENOISE)

    errno = 0;

    linenoiseSetCompletionCallback(snck_completion);

    p_line = linenoise(a_prompt);

    if (p_line)
    {
        linenoiseHistoryAdd(p_line);

        strcpy(a_line, p_line);

        b_result = 1;

        free(p_line);
    }
    else
    {
        if (errno == EAGAIN)
        {
            /* ctrl+c was pressed */
            a_line[0] = '\n';

            a_line[1] = '\000';

            b_result = 1;
        }
        else
        {
            fprintf(stderr, "... bye!\n");

            b_result = 0;
        }
    }

#else /* #if defined(SNCK_FEATURE_LINENOISE) */

    fprintf(stdout, "%s", a_prompt);

    fflush(stdout);

    if (NULL != fgets(a_line, i_line_len, stdin))
    {
        a_line[65535u] = '\0';

        b_result = 1;
    }
    else
    {
        fprintf(stderr, "... bye!\n");

        b_result = 0;
    }

#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

    return b_result;
}

static
char
snck_read_file(void)
{
    static char a_line[65536u];

    char b_result;

    char b_continue;

    b_result = 1;

    b_continue = 1;

    while (b_result && b_continue)
    {
        a_line[0] = '\000';

        if (snck_read_line(a_line, sizeof(a_line)))
        {
            if (snck_process_line(a_line))
            {
            }
            else
            {
                b_result = 0;

                b_continue = 0;
            }
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

/*

Function: snck_main

Description:

*/
int
snck_main(
    unsigned int i_argc,
    char const * const * const p_argv)
{
    /* read commands from stdin */
    int i_exit_status;

    (void)(i_argc);
    (void)(p_argv);

    /* install a SIGCHLD handler */
    signal(SIGCHLD, snck_sigchld);
    signal(SIGINT, SIG_IGN);
    /* signal(SIGTSTP, SIG_IGN); */

    {
        uid_t id;

        struct passwd * pw;

        char * p_home;

        a_user[0u] = '\000';

        a_home[0u] = '\000';

        id = getuid();

        pw = getpwuid(id);

        if (pw)
        {
            strcpy(a_user, pw->pw_name);

            strcpy(a_home, pw->pw_dir);
        }
        else
        {
            strcpy(a_user, "snck");

            p_home = getenv("HOME");

            if (p_home)
            {
                strcpy(a_home, p_home);
            }
            else
            {
                sprintf(a_home, "/home/%s", a_user);
            }
        }
    }

    a_host[0] = '\000';

    if (0 == gethostname(a_host, sizeof(a_host)))
    {
    }
    else
    {
        strcpy(a_host, "snck");
    }

    if (snck_read_file())
    {
        i_exit_status = 0;
    }
    else
    {
        i_exit_status = 1;
    }

    return i_exit_status;
}
