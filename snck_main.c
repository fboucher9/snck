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

/* */
#if defined(SNCK_FEATURE_LINENOISE)
#include <linenoise.h>
#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

static struct snck_ctxt o_ctxt;

static struct snck_ctxt * p_ctxt;

static struct snck_info o_info;

static struct snck_heap o_heap;

static struct snck_passwd o_passwd;

static char a_split[65536u];

static char a_line[65536u];

static char a_prompt[4096u];

static char a_folder[1024u];

static char suggest[1024u];

static char a_history_file[1024u];

static char * g_argv[1024u];

static unsigned int g_argc = 0;

static char b_history_loaded = 0;

static char * a_suggest[128u];

static int i_suggest = 0;

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
snck_builtin_cd(char const * p_args)
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
snck_execute_child(void)
{
    char b_result;

    char const * p_cmd;

    int i_cmd_len;

    if (snck_find_word(a_split, &(p_cmd), &(i_cmd_len)))
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
        else if (0 == strncmp(p_cmd, "cd", i_cmd_len))
        {
            b_result = snck_builtin_cd(p_cmd + i_cmd_len);
        }
        else if (0 == strncmp(p_cmd, "set", i_cmd_len))
        {
            b_result = snck_builtin_set(p_cmd + i_cmd_len);
        }
        else if (0 == strncmp(p_cmd, "unset", i_cmd_len))
        {
            b_result = snck_builtin_unset(p_cmd + i_cmd_len);
        }
        else if (0 == strncmp(p_cmd, "shell", i_cmd_len))
        {
            b_result = snck_builtin_shell(p_cmd + i_cmd_len);
        }
        else if ((0 == strncmp(p_cmd, "exit", i_cmd_len)) || (0 == strncmp(p_cmd, "logout", i_cmd_len)))
        {
            exit(0);
        }
        else
        {
            b_result = snck_fork_and_exec();
        }
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
    char * p_ps1;

    {
        char * p_env;

        p_env = getenv("PS1");

        if (p_env)
        {
            p_ps1 = p_env;
        }
        else
        {
            p_ps1 = "\\u@\\h:\\w\\$ ";
        }
    }

    {
        char * p_in;

        char * p_out;

        int i_len;

        p_in = p_ps1;

        p_out = a_prompt;

        while ('\000' != *p_in)
        {
            char c_in;

            c_in = *p_in;

            p_in ++;

            if ('\\' == c_in)
            {
                if ('\000' != *p_in)
                {
                    c_in = *p_in;

                    p_in ++;

                    if ('u' == c_in)
                    {
                        i_len = p_ctxt->p_info->o_user.i_buf_len;
                        memcpy(p_out, p_ctxt->p_info->o_user.p_buf, i_len);
                        p_out += i_len;
                    }
                    else if ('h' == c_in)
                    {
                        i_len = p_ctxt->p_info->o_host.i_buf_len;
                        memcpy(p_out, p_ctxt->p_info->o_host.p_buf, i_len);
                        p_out += i_len;
                    }
                    else if ('w' == c_in)
                    {
                        if (0 == strncmp(p_ctxt->p_info->o_pwd.p_buf, p_ctxt->p_info->o_home.p_buf, p_ctxt->p_info->o_home.i_buf_len))
                        {
                            *p_out = '~';
                            p_out ++;
                            i_len = p_ctxt->p_info->o_pwd.i_buf_len - p_ctxt->p_info->o_home.i_buf_len;
                            memcpy(p_out, p_ctxt->p_info->o_pwd.p_buf + p_ctxt->p_info->o_home.i_buf_len, i_len);
                            p_out += i_len;
                        }
                        else
                        {
                            i_len = p_ctxt->p_info->o_pwd.i_buf_len;
                            memcpy(p_out, p_ctxt->p_info->o_pwd.p_buf, i_len);
                            p_out += i_len;
                        }
                    }
                    else if ('$' == c_in)
                    {
                        *p_out = '$';

                        p_out ++;
                    }
                    else if ('_' == c_in)
                    {
                        *p_out = ' ';

                        p_out ++;
                    }
                    else
                    {
                        *p_out = c_in;

                        p_out ++;
                    }
                }
                else
                {
                    /* error */
                }
            }
            else
            {
                *p_out = c_in;

                p_out ++;
            }
        }

        *p_out = '\000';

        p_out ++;
    }

} /* snck_build_prompt() */

#if defined(SNCK_FEATURE_LINENOISE)

static
int
snck_fuzzy_compare(
    char const * p_ref1,
    char const * p_ref2,
    int const i_len)
{
    int i_result;

    int i_ref1;

    int i_ref2;

    if (i_len > 1)
    {
        i_result = 1;

        i_ref1 = 0;

        i_ref2 = 0;

        /* try to find each letter of p_ref2[0:i_len-1] within p_ref1 */
        while ((i_ref1 < (int)(strlen(p_ref1))) && (i_ref2 < i_len))
        {
            /* Look for a letter */
            if (p_ref1[i_ref1] == p_ref2[i_ref2])
            {
                i_ref1 ++;
                i_ref2 ++;
            }
            else
            {
                i_ref1 ++;
            }
        }

        if (i_ref2 >= i_len)
        {
            i_result = 0;
        }
    }
    else
    {
        i_result = strncmp(p_ref1, p_ref2, i_len);
    }

    return i_result;

}

static
void
snck_suggest_add(
    char const * suggest)
{
    if (i_suggest < 128)
    {
        int i;

        int j;

        char b_inserted;

        i = 0;

        b_inserted = 0;

        while (!b_inserted && (i < i_suggest))
        {
            int i_compare;

            i_compare = strcmp(suggest, a_suggest[i]);

            if (0 == i_compare)
            {
                b_inserted = 1;
            }
            else if (0 > i_compare)
            {
                /* Do sort of suggestions */

                j = i_suggest;

                while (j > i)
                {
                    a_suggest[j] = a_suggest[j-1];

                    j--;
                }

                a_suggest[i] = strdup(suggest);

                b_inserted = 1;

                i_suggest ++;
            }
            else
            {
                i++;
            }
        }

        if (!b_inserted)
        {
            a_suggest[i_suggest] = strdup(suggest);

            i_suggest ++;
        }
    }
}

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

    int i_cmd_prefix;

    char b_cmd_is_cd;

#if 0
    strcpy(a_split, buf);

    snck_tokenize_line();
#endif

    i_cmd_prefix = 0;

    while ((i_cmd_prefix <= (int)(pos)) && ((buf[i_cmd_prefix] == ' ') || (buf[i_cmd_prefix] == '\t')))
    {
        i_cmd_prefix ++;
    }

    b_cmd_is_cd = 0;

    {
        int i_cmd_it;

        i_cmd_it = i_cmd_prefix;

        if ('c' == buf[i_cmd_it])
        {
            i_cmd_it ++;

            if ('d' == buf[i_cmd_it])
            {
                i_cmd_it ++;

                if ((i_cmd_it <= (int)(pos)) && (('\000' == buf[i_cmd_it]) || (buf[i_cmd_it] == ' ') || (buf[i_cmd_it] == '\t')))
                {
                    b_cmd_is_cd = 1;
                }
            }
        }
    }

    if ((int)(pos) > i_cmd_prefix)
    {
        char * p_folder;

        int pos0;

        int pos1;

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

        if (pos1 > pos0)
        {
            a_folder[0u] = '\000';

            if (buf[pos0] == '~')
            {
                sprintf(a_folder, "%s%.*s", p_ctxt->p_info->o_home.p_buf, pos1 - pos0 - 1, buf + pos0 + 1);
            }
            else
            {
                memcpy(a_folder, buf + pos0, pos1 - pos0);

                a_folder[pos1 - pos0] = '\000';
            }

            p_folder = a_folder;
        }
        else
        {
            p_folder = ".";
        }

        i_suggest = 0;

        /* completing a file name or full path to program */
        if ((pos1 == i_cmd_prefix) && (buf[pos1] != '.') && (buf[pos1] != '/'))
        {
            char * p_env;

            p_env = getenv("PATH");

            if (p_env)
            {
                /* split of buffer */
                char * p_temp;

                p_temp = strdup(p_env);

                if (p_temp)
                {
                    char * p_comp;

                    p_comp = strtok(p_temp, ":");

                    while (p_comp)
                    {
                        /* enumerate executables in path */
                        DIR * p_dir_object;

                        p_dir_object = opendir(p_comp);

                        if (p_dir_object)
                        {
                            struct dirent * p_dir_entry;

                            p_dir_entry = readdir(p_dir_object);

                            while (p_dir_entry)
                            {
                                if (DT_DIR != p_dir_entry->d_type)
                                {
                                    if (0 == strncmp(p_dir_entry->d_name, buf + pos1, pos - pos1))
                                    {
                                        if (pos1 > 0)
                                        {
                                            sprintf(suggest, "%.*s%s", pos1, buf, p_dir_entry->d_name);
                                        }
                                        else
                                        {
                                            sprintf(suggest, "%s", p_dir_entry->d_name);
                                        }

                                        if (0 != strcmp(suggest, buf))
                                        {
                                            snck_suggest_add(suggest);
                                        }
                                    }
                                }

                                p_dir_entry = readdir(p_dir_object);
                            }

                            closedir(p_dir_object);
                        }

                        p_comp = strtok(NULL, ":");
                    }

                    free(p_temp);
                }
            }
        }
        else
        {
            DIR * d;

            d = opendir(p_folder);

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
                        else if (0 == snck_fuzzy_compare(e->d_name, buf + pos1, pos - pos1))
                        {
                            if (!b_cmd_is_cd || (DT_DIR == e->d_type))
                            {
                                if (pos1 > 0)
                                {
                                    sprintf(suggest, "%.*s%s", pos1, buf, e->d_name);
                                }
                                else
                                {
                                    sprintf(suggest, "%s", e->d_name);
                                }

                                if (0 != strcmp(suggest, buf))
                                {
                                    snck_suggest_add(suggest);
                                }
                            }
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                closedir(d);
            }
        }

        /* If only one suggestion... */
        if (0 == i_suggest)
        {
        }
        else if (1 == i_suggest)
        {
            linenoiseAddCompletion(lc, a_suggest[0]);
        }
        else
        {
            int i;

#if 0
            int j;

            /* Find common prefix for suggestions */
            i = 1;

            j = strlen(a_suggest[0]);

            while (i < i_suggest)
            {
                int k;

                /* Find new common length */
                k = 0;

                while ((k < j) && a_suggest[0][k] && (a_suggest[0][k] == a_suggest[i][k]))
                {
                    k++;
                }

                j = k;

                i ++;
            }

            if ((j != (int)(strlen(a_suggest[0]))) && (j > (int)(strlen(buf))))
            {
                /* suggest only common prefix... */
                strcpy(suggest, a_suggest[0]);

                suggest[j] = '\000';

                linenoiseAddCompletion(lc, suggest);
            }
#endif

            /* Merge list of suggestions into linenoise */
            i = 0;

            while (i < i_suggest)
            {
                if (a_suggest[i])
                {
                    linenoiseAddCompletion(lc, a_suggest[i]);

                    free(a_suggest[i]);

                    a_suggest[i] = NULL;
                }

                i++;
            }

            i_suggest = 0;
        }
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

    {
        if (!b_history_loaded)
        {
            sprintf(a_history_file, "%s/.snckhist", p_ctxt->p_info->o_home.p_buf);

            linenoiseHistoryLoad(a_history_file);

            b_history_loaded = 1;
        }
    }

    linenoiseSetCompletionCallback(snck_completion);

    p_temp = linenoise(a_prompt);

    if (p_temp)
    {
        if (' ' != p_temp[0u])
        {
            /* Detect duplicate entries... */

            if (linenoiseHistoryAdd(p_temp))
            {
                linenoiseHistorySave(a_history_file);
            }
        }

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
        b_result = 0;
    }

#endif /* #if defined(SNCK_FEATURE_LINENOISE) */

#if 0
    if (!b_result)
    {
        fprintf(stderr, "... bye!\n");
    }
#endif

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
        p_ctxt = &(o_ctxt);

        p_ctxt->p_heap = &(o_heap);

        p_ctxt->p_info = &(o_info);

        p_ctxt->p_passwd = &(o_passwd);

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

                if (snck_read_file())
                {
                    i_exit_status = 0;
                }
                else
                {
                    i_exit_status = 1;
                }

                snck_info_cleanup(p_ctxt);
            }
            else
            {
                i_exit_status = 1;
            }

            snck_passwd_cleanup(p_ctxt);

            snck_heap_cleanup(p_ctxt);
        }
        else
        {
            i_exit_status = 1;
        }
    }

    return i_exit_status;

} /* snck_main() */

/* end-of-file: snck_main.c */
