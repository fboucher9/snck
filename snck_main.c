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

static char a_split[65536u];

static char a_line[65536u];

static char a_home[1024u];

static char a_user[1024u];

static char a_host[1024u];

static char a_pwd_old[1024u];

static char a_pwd[1024u];

static char a_prompt[4096u];

static char * g_argv[1024u];

static unsigned int g_argc = 0;

static
char
snck_builtin_cd(void)
{
    char b_result;

    int i_result;

    char * p_path;

    if (g_argc > 0u)
    {
        if (0 == strcmp(g_argv[0u], "-"))
        {
            p_path = a_pwd_old;
        }
        else
        {
            if ('~' == g_argv[0u][0u])
            {
                sprintf(a_line, "%s%s", a_home, g_argv[0u] + 1);

                p_path = a_line;
            }
            else
            {
                p_path = g_argv[0u];
            }
        }
    }
    else
    {
        p_path = a_home;
    }

    if (0 != strcmp(p_path, "."))
    {
        i_result = chdir(p_path);

        if (0 == i_result)
        {
            strcpy(a_pwd_old, a_pwd);

            if (NULL != getcwd(a_pwd, sizeof(a_pwd)))
            {
                if (0 == setenv("PWD", a_pwd, 1))
                {
                }
                else
                {
                }
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

    return b_result;

} /* snck_builtin_cd() */

extern char ** environ;

static
char
snck_builtin_set(void)
{
    char b_result;

    int i_result;

    if (g_argc > 0u)
    {
        char * p_name;

        p_name = g_argv[0u];

        if (g_argc > 1u)
        {
            char * p_value;

            p_value = g_argv[1u];

            i_result = setenv(p_name, p_value, 1);

            (void)(i_result);
        }
        else
        {
            /* */
            char * p_value;

            p_value = getenv(p_name);

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
snck_builtin_unset(void)
{
    char b_result;

    int i_result;

    if (g_argc > 0u)
    {
        char * p_name;

        p_name = g_argv[0u];

        i_result = unsetenv(p_name);

        (void)(i_result);
    }
    else
    {
    }

    b_result = 1;

    return b_result;

} /* snck_builtin_unset() */

static
char
snck_builtin_sexe(void)
{
    char b_result;

    if (g_argc > 0u)
    {
        execvp(g_argv[0u], g_argv);

        fprintf(stderr, "unable to sexecute\n");

        b_result = 1;
    }
    else
    {
        fprintf(stderr, "missing argument\n");

        b_result = 1;
    }

    return b_result;

} /* snck_builtin_sexe() */

static
char
snck_fork_and_exec(void)
{
    char b_result;

    pid_t iChildProcess;

    g_argv[0u] = "/bin/sh";

    g_argv[1u] = "-c";

    /* detect if exec is possible */
    if (strchr(a_line, ';') ||
        strchr(a_line, '&') ||
        strchr(a_line, '|'))
    {
        g_argv[2u] = a_line;

        g_argv[3u] = NULL;
    }
    else
    {
        sprintf(a_split, "exec %s", a_line);

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

        int iExitCode;

        iExitCode = WEXITSTATUS(resultStatus);

        if (iExitCode)
        {
            fprintf(stderr, "snck: error code is %d\n", iExitCode);
        }

        b_result = 1;
    }

    return b_result;

} /* snck_fork_and_exec() */

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

static
char
snck_execute_child(void)
{
    char b_result;

    char * p_cmd;

    unsigned int i_cmd_len;

    p_cmd = a_split;

    while ((' ' == *p_cmd) || ('\t' == *p_cmd))
    {
        p_cmd ++;
    }

    i_cmd_len = 0u;

    while (('\000' != p_cmd[i_cmd_len]) && (' ' != p_cmd[i_cmd_len]) && ('\t' != p_cmd[i_cmd_len]))
    {
        i_cmd_len ++;
    }

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
    else if (0 == strncmp(p_cmd, "cd", i_cmd_len))
    {
        snck_tokenize_line(p_cmd + i_cmd_len);

        b_result = snck_builtin_cd();
    }
    else if (0 == strncmp(p_cmd, "set", i_cmd_len))
    {
        snck_tokenize_line(p_cmd + i_cmd_len);

        b_result = snck_builtin_set();
    }
    else if (0 == strncmp(p_cmd, "unset", i_cmd_len))
    {
        snck_tokenize_line(p_cmd + i_cmd_len);

        b_result = snck_builtin_unset();
    }
    else if (0 == strncmp(p_cmd, "sexe", i_cmd_len))
    {
        snck_tokenize_line(p_cmd + i_cmd_len);

        b_result = snck_builtin_sexe();
    }
    else if ((0 == strncmp(p_cmd, "exit", i_cmd_len)) || (0 == strncmp(p_cmd, "logout", i_cmd_len)))
    {
        exit(0);
    }
    else
    {
        b_result = snck_fork_and_exec();
    }

    return b_result;

} /* snck_execute_child() */

/*

Function: snck_process_line

Description:

*/
static
char
snck_process_line(void)
{
    char b_result;

    /* tokenize the input buffer */
    /* token is word or function or mix of both: */
    /* word */
    /* [function] */
    /* wo[func][func]rd */
    /* wo[sp]rd */

    strcpy(a_line, a_split);

    /* expand of functions */

    b_result = snck_execute_child();

    return b_result;

} /* snck_process_line() */

static
void
snck_build_prompt(void)
{
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

#if 0
    strcpy(a_split, buf);

    snck_tokenize_line();
#endif

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
snck_read_line(void)
{
    char b_result;

    char * p_temp;

    snck_build_prompt();

#if defined(SNCK_FEATURE_LINENOISE)

    errno = 0;

    linenoiseSetCompletionCallback(snck_completion);

    p_temp = linenoise(a_prompt);

    if (p_temp)
    {
        linenoiseHistoryAdd(p_temp);

        strcpy(a_split, p_temp);

        b_result = 1;

        free(p_temp);
    }
    else
    {
        if (errno == EAGAIN)
        {
            /* ctrl+c was pressed */
            a_split[0] = '\n';

            a_split[1] = '\000';

            b_result = 1;
        }
        else
        {
#if 0
            fprintf(stderr, "... bye!\n");
#endif

            b_result = 0;
        }
    }

#else /* #if defined(SNCK_FEATURE_LINENOISE) */

    fprintf(stdout, "%s", a_prompt);

    fflush(stdout);

    if (NULL != fgets(a_split, sizeof(a_split) - 1u, stdin))
    {
        a_split[sizeof(a_split) - 1u] = '\0';

        b_result = 1;
    }
    else
    {
#if 0
        fprintf(stderr, "... bye!\n");
#endif

        b_result = 0;
    }

#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

    return b_result;
}

static
char
snck_read_file(void)
{
    char b_result;

    char b_continue;

    b_result = 1;

    b_continue = 1;

    while (b_result && b_continue)
    {
        a_split[0] = '\000';

        if (snck_read_line())
        {
            if (snck_process_line())
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

static
void
snck_detect_info(void)
{
    char b_found_user;

    char b_found_home;

    char b_found_host;

    a_user[0u] = '\000';

    a_home[0u] = '\000';

    a_host[0u] = '\000';

    b_found_user = 0;

    b_found_home = 0;

    b_found_host = 0;

    if (!b_found_user)
    {
        char * p_user;

        p_user = getenv("USER");

        if (p_user)
        {
            strcpy(a_user, p_user);

            b_found_user = 1;
        }
    }

    if (!b_found_home)
    {
        char * p_home;

        p_home = getenv("HOME");

        if (p_home)
        {
            strcpy(a_home, p_home);

            b_found_home = 1;
        }
    }

    if (!b_found_user || !b_found_home)
    {
        uid_t id;

        struct passwd * pw;

        id = getuid();

        pw = getpwuid(id);

        if (pw)
        {
            if (!b_found_user)
            {
                strcpy(a_user, pw->pw_name);

                b_found_user = 1;
            }

            if (!b_found_home)
            {
                strcpy(a_home, pw->pw_dir);

                b_found_home = 1;
            }
        }
    }

    if (!b_found_user)
    {
        strcpy(a_user, "snck");

        b_found_user = 1;
    }

    if (!b_found_home)
    {
        sprintf(a_home, "/home/%s", a_user);

        b_found_home = 1;
    }

    if (!b_found_host)
    {
        if (0 == gethostname(a_host, sizeof(a_host)))
        {
            b_found_host = 1;
        }
    }

    if (!b_found_host)
    {
        strcpy(a_host, "snck");

        b_found_host = 1;
    }

    a_pwd[0] = '\000';

    if (NULL != getcwd(a_pwd, sizeof(a_pwd)))
    {
        if (0 == setenv("PWD", a_pwd, 1))
        {
        }
        else
        {
        }
    }
    else
    {
        unsetenv("PWD");
    }

    strcpy(a_pwd_old, a_pwd);

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
        /* install a SIGCHLD handler */
        signal(SIGCHLD, snck_sigchld);
        signal(SIGINT, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        /* signal(SIGTSTP, SIG_IGN); */

        snck_detect_info();

        unsetenv("SHLVL");

        unsetenv("_");

        unsetenv("MAIL");

        if (snck_read_file())
        {
            i_exit_status = 0;
        }
        else
        {
            i_exit_status = 1;
        }
    }

    return i_exit_status;

} /* snck_main() */

/* end-of-file: snck_main.c */
