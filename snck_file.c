/* See LICENSE for license details. */

/*

Module: snck_file.c

Description:

    Process an entire file as input to shell.

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Context */
#include "snck_ctxt.h"

/* Module */
#include "snck_file.h"

/* String */
#include "snck_string.h"

/* List */
#include "snck_list.h"

/* Line editor */
#include "snck_line.h"

/* Heap */
#include "snck_heap.h"

/* Information */
#include "snck_info.h"

/* Environment */
#include "snck_env.h"

/* History */
#include "snck_history.h"

/* Tokenizer */
#include "snck_token.h"

/* Options */
#include "snck_opts.h"

static
char
snck_expand_do_pipe(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string * p_output,
    char const * p_path)
{
    char b_result;

    size_t i_buf_max_len;

    i_buf_max_len = 32u;

    p_output->i_buf_len = 0;

    b_result = snck_string_resize(p_ctxt, p_output, i_buf_max_len + 1);

    if (b_result)
    {
        FILE * p_pipe = popen(p_path, "r");

        if (p_pipe)
        {
            char b_more;

            b_more = 1;

            while (b_result && b_more)
            {
                int c;

                c = fgetc(p_pipe);

                if (EOF != c)
                {
                    if (p_output->i_buf_len >= i_buf_max_len)
                    {
                        i_buf_max_len += 32u;

                        b_result = snck_string_resize(p_ctxt, p_output, i_buf_max_len + 1);
                    }

                    if (b_result)
                    {
                        p_output->p_buf[p_output->i_buf_len] = (char)(c);

                        p_output->i_buf_len ++;
                    }
                }
                else
                {
                    b_more = 0;
                }
            }

            if (b_result)
            {
                if ((p_output->i_buf_len) && ('\n' == p_output->p_buf[p_output->i_buf_len - 1u]))
                {
                    p_output->i_buf_len --;
                }
            }

            pclose(p_pipe);
        }
        else
        {
            b_result = 0;
        }
    }
    else
    {
        b_result = 0;
    }

    return b_result;
}

static
char
snck_expand_get(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_ref,
    struct snck_string * const p_expand)
{
    /* Use sh -c 'echo ...' to expand the argument */
    static char const a_fmt[] = "PATH= echo -n %.*s";

    char * p_path;

    char b_result;

    p_path = snck_heap_realloc(p_ctxt, NULL, sizeof(a_fmt) + p_ref->i_buf_len + 1);

    if (p_path)
    {
        sprintf(p_path, a_fmt, (int)(p_ref->i_buf_len), p_ref->p_buf);

        b_result = snck_expand_do_pipe(p_ctxt, p_expand, p_path);

        if (b_result)
        {
        }
        else
        {
            b_result = snck_string_copy_object(p_ctxt, p_expand, p_ref);
        }

        snck_heap_realloc(p_ctxt, p_path, 0u);
    }
    else
    {
        b_result = 0;
    }

    return b_result;

} /* snck_expand_get() */

static
char
snck_expand_folder_get(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_ref,
    struct snck_string * const p_expand)
{
    /* Use sh -c 'echo ...' to expand the argument */
    static char const a_fmt[] = "cd %.*s; pwd";

    char * p_path;

    char b_result;

    p_path = snck_heap_realloc(p_ctxt, NULL, sizeof(a_fmt) + (p_ref->i_buf_len) + 1);

    if (p_path)
    {
        sprintf(p_path, a_fmt, (int)(p_ref->i_buf_len), p_ref->p_buf);

        if (snck_expand_do_pipe(p_ctxt, p_expand, p_path))
        {
            b_result = 1;
        }
        else
        {
            b_result = snck_string_copy_object(p_ctxt, p_expand, p_ref);
        }

        snck_heap_realloc(p_ctxt, p_path, 0u);
    }
    else
    {
        b_result = 0;
    }

    return b_result;

} /* snck_expand_folder_get() */

static
char
snck_builtin_cd(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_line)
{
    char b_result;

    int i_result;

    struct snck_string o_path;

    snck_string_init(p_ctxt, &(o_path));

    if (p_line->p_buf[0u])
    {
        static char const a_dash[] = { '-' };

        static struct snck_string const o_dash = { (char *)(a_dash), sizeof(a_dash), 0u };

        if (0 == snck_string_compare(p_line, &(o_dash)))
        {
            snck_string_copy_object(p_ctxt, &(o_path), &(p_ctxt->p_info->o_old_pwd));
        }
        else
        {
            /* Use sh -c 'echo ...' to expand the argument */
            if (snck_expand_folder_get(p_ctxt, p_line, &(o_path)))
            {
            }
            else
            {
                snck_string_copy(p_ctxt, &(o_path), "");
            }
        }
    }
    else
    {
        snck_string_copy_object(p_ctxt, &(o_path), &(p_ctxt->p_info->o_home));
    }

    if (o_path.i_buf_len)
    {
        static char const a_period[] = { '.' };

        static struct snck_string const o_period = { (char *)(a_period), sizeof(a_period), 0u };

        if (0 != snck_string_compare(&(o_path), &(o_period)))
        {
            char * p_path0 = snck_string_get(p_ctxt, &(o_path));

            if (p_path0)
            {
                i_result = chdir(p_path0);

                if (0 == i_result)
                {
                    b_result = snck_info_update_wd(p_ctxt, &(o_path));
                }
                else
                {
                    fprintf(stderr, "failure to change directory\n");

                    b_result = 1;
                }

                snck_string_put(p_ctxt, p_path0);
            }
            else
            {
                b_result = 0;
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

    snck_string_cleanup(p_ctxt, &(o_path));

    return b_result;

} /* snck_builtin_cd() */

extern char ** environ;

static
char
snck_builtin_set(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_line)
{
    char b_result;

    struct snck_string o_name;

    struct snck_string o_rest;

    snck_string_init(p_ctxt, &(o_name));

    snck_string_init(p_ctxt, &(o_rest));

    if (snck_token_find_next_word(p_ctxt, p_line, &(o_name), &(o_rest)))
    {
        if (o_rest.i_buf_len)
        {
            struct snck_string o_value;

            snck_string_init(p_ctxt, &(o_value));

            if (snck_expand_get(p_ctxt, &(o_rest), &(o_value)))
            {
                snck_env_set(p_ctxt, &(o_name), &(o_value));
            }

            snck_string_cleanup(p_ctxt, &(o_value));
        }
        else
        {
            /* */
            struct snck_string o_value;

            snck_string_init(p_ctxt, &(o_value));

            if (snck_env_get(p_ctxt, &(o_name), &(o_value)))
            {
                fprintf(stdout, "%.*s\n", (int)(o_value.i_buf_len), o_value.p_buf);
            }
            else
            {
                fprintf(stderr, "not set\n");
            }

            snck_string_cleanup(p_ctxt, &(o_value));
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

    snck_string_cleanup(p_ctxt, &(o_rest));

    snck_string_cleanup(p_ctxt, &(o_name));

    b_result = 1;

    return b_result;

} /* snck_builtin_set() */

static
char
snck_builtin_unset(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_line)
{
    char b_result;

    snck_env_set(p_ctxt, p_line, NULL);

    b_result = 1;

    return b_result;

} /* snck_builtin_unset() */

static
char
snck_builtin_shell(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_line)
{
    char b_result;

    (void)(p_ctxt);

    if (p_line->p_buf[0u])
    {
        char * l_argv[2u];

        l_argv[0u] = (char *)(p_line->p_buf);

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
snck_wrap_exec(
    struct snck_ctxt const * const p_ctxt,
    char * * l_argv)
{
    char b_result;

    pid_t iChildProcess;

    (void)(p_ctxt);

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

    return b_result;

}

static
char
snck_file_exec_line(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_line)
{
    char b_result;

    char b_argv_put[32u];

    char * l_argv[32u];

    unsigned int l_argc;

    unsigned int i;

    l_argc = 0;

    l_argv[l_argc] = "/bin/sh";

    b_argv_put[l_argc] = 0;

    l_argc ++;

    l_argv[l_argc] = "-c";

    b_argv_put[l_argc] = 0;

    l_argc ++;

    if (p_ctxt->p_opts->b_trace)
    {
        l_argv[l_argc] = "-x";

        b_argv_put[l_argc] = 0;

        l_argc ++;
    }

    l_argv[l_argc] = snck_string_get(p_ctxt, p_line);

    b_argv_put[l_argc] = 1;

    l_argc ++;

    l_argv[l_argc] = "sh";

    b_argv_put[l_argc] = 0;

    l_argc ++;

    for (i=0u; i<p_ctxt->p_opts->i_argc; i++)
    {
        if (l_argc < 32u)
        {
            l_argv[l_argc] = snck_string_get(p_ctxt, p_ctxt->p_opts->p_argv + i);

            b_argv_put[l_argc] = 1;

            l_argc ++;
        }
    }

    l_argv[l_argc] = NULL;

    b_argv_put[l_argc] = 0;

    l_argc ++;

    if (p_ctxt->p_opts->b_trace)
    {
        fprintf(stderr, "snck: exec sh -c [%.*s]\n", (int)(p_line->i_buf_len), p_line->p_buf);
    }

    if (p_ctxt->p_opts->b_dryrun)
    {
        fprintf(stderr, "snck: skip exec for dry run\n");

        b_result = 1;
    }
    else
    {
        b_result = snck_wrap_exec(p_ctxt, l_argv);
    }

    for (i=0u; i<l_argc; i++)
    {
        if (b_argv_put[i])
        {
            if (l_argv[i])
            {
                snck_string_put(p_ctxt, l_argv[i]);
            }
        }
    }

    return b_result;

} /* snck_file_exec_line() */

static
char
snck_builtin_hist(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_line)
{
    char b_result;

    int i;

    struct snck_history * const p_history = p_ctxt->p_history;

    struct snck_list * p_it;

    (void)(p_line);

    snck_history_load(p_ctxt);

    p_it = &(p_history->o_list);

    for (i = 0; i < 10; i++)
    {
        if (p_it->p_prev != &(p_history->o_list))
        {
            p_it = p_it->p_prev;
        }
    }

    for (i = 0; i < 10; i++)
    {
        if (p_it != &(p_history->o_list))
        {
            struct snck_history_line const * const p_history_line =
                (struct snck_history_line const * const)(p_it);

            if (p_history_line->o_buf.p_buf)
            {
                fprintf(stderr, "%6d: len=%d, buf=[%.*s]\n",
                    -10+i,
                    (int)(p_history_line->o_buf.i_buf_len),
                    (int)(p_history_line->o_buf.i_buf_len),
                    p_history_line->o_buf.p_buf);
            }

            p_it = p_it->p_next;
        }
    }

    snck_history_unload(p_ctxt);

    b_result = 1;

    return b_result;

} /* snck_builtin_hist() */

static
char
snck_builtin_source(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_line)
{
    char b_result;

    if (p_line->i_buf_len)
    {
        struct snck_string o_path;

        snck_string_init(p_ctxt, &(o_path));

        if (snck_expand_get(p_ctxt, p_line, &(o_path)))
        {
            if (snck_file_read(p_ctxt, &(o_path)))
            {
            }
            else
            {
                fprintf(stderr, "snck: error opening file\n");
            }
        }

        snck_string_cleanup(p_ctxt, &(o_path));
    }
    else
    {
        fprintf(stderr, "snck: missing argument\n");
    }

    b_result = 1;

    return b_result;

} /* snck_builtin_source() */

static
char
snck_builtin_exit(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_line)
{
    (void)(p_ctxt);

    (void)(p_line);

    exit(0);

    return 1;
}

static
char
snck_builtin_edit(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_line)
{
    char b_result;

    char * l_argv[3u];

    struct snck_string o_value_editor;

    static char const a_name_editor[] = { 'E', 'D', 'I', 'T', 'O', 'R' };

    static struct snck_string o_name_editor = { (char *)(a_name_editor), sizeof(a_name_editor), 0u };

    char * p_editor0;

    char * p_line0;

    snck_string_init(p_ctxt, &(o_value_editor));

    if (snck_env_get(p_ctxt, &(o_name_editor), &(o_value_editor)))
    {
    }
    else
    {
        static char const a_default_editor[] = { 'v', 'i' };

        snck_string_ref_buffer(p_ctxt, &(o_value_editor), a_default_editor, sizeof(a_default_editor));
    }

    p_editor0 = snck_string_get(p_ctxt, &(o_value_editor));

    if (p_editor0)
    {
        p_line0 = snck_string_get(p_ctxt, p_line);

        if (p_line0)
        {
            l_argv[0u] = (char *)(p_editor0);

            l_argv[1u] = (char *)(p_line0);

            l_argv[2u] = NULL;

            /* execute the command */
            b_result = snck_wrap_exec(p_ctxt, l_argv);

            snck_string_put(p_ctxt, p_line0);
        }

        snck_string_put(p_ctxt, p_editor0);
    }

    snck_string_cleanup(p_ctxt, &(o_value_editor));

    return b_result;
}

static
char
snck_file_decode_line(
    struct snck_ctxt const * const p_ctxt,
    struct snck_string const * const p_line)
{
    char b_result;

    struct snck_string o_cmd;

    struct snck_string o_args;

    snck_string_init(p_ctxt, &(o_cmd));

    snck_string_init(p_ctxt, &(o_args));

#if defined(SNCK_DBG)
    {
        fprintf(stderr, "dbg: decode line [%.*s]\n",
            (int)(p_line->i_buf_len),
            p_line->p_buf);
    }
#endif /* #if defined(SNCK_DBG) */

    if (snck_token_find_next_word(p_ctxt, p_line, &(o_cmd), &(o_args)))
    {
        static char const a_name_cd[] = { 'c', 'd' };

        static char const a_name_set[] = { 's', 'e', 't' };

        static char const a_name_unset[] = { 'u', 'n', 's', 'e', 't' };

        static char const a_name_shell[] = { 's', 'h', 'e', 'l', 'l' };

        static char const a_name_hist[] = { 'h', 'i', 's', 't' };

        static char const a_name_source[] = { 's', 'o', 'u', 'r', 'c', 'e' };

        static char const a_name_exit[] = { 'e', 'x', 'i', 't' };

        static char const a_name_logout[] = { 'l', 'o', 'g', 'o', 'u', 't' };

        static struct snck_string const o_name_cd = { (char *)(a_name_cd), sizeof(a_name_cd), 0u };

        static struct snck_string const o_name_set = { (char *)(a_name_set), sizeof(a_name_set), 0u };

        static struct snck_string const o_name_unset = { (char *)(a_name_unset), sizeof(a_name_unset), 0u };

        static struct snck_string const o_name_shell = { (char *)(a_name_shell), sizeof(a_name_shell), 0u };

        static struct snck_string const o_name_hist = { (char *)(a_name_hist), sizeof(a_name_hist), 0u };

        static struct snck_string const o_name_source = { (char *)(a_name_source), sizeof(a_name_source), 0u };

        static struct snck_string const o_name_exit = { (char *)(a_name_exit), sizeof(a_name_exit), 0u };

        static struct snck_string const o_name_logout = { (char *)(a_name_logout), sizeof(a_name_logout), 0u };

#if defined(SNCK_DBG)
        {
            fprintf(stderr, "dbg: command word is [%.*s]\n",
                (int)(o_cmd.i_buf_len),
                o_cmd.p_buf);

            fprintf(stderr, "dbg: command args is [%.*s]\n",
                (int)(o_args.i_buf_len),
                o_args.p_buf);
        }
#endif /* #if defined(SNCK_DBG) */

        if (0 == o_cmd.i_buf_len)
        {
            /* Empty line */
            b_result = 1;
        }
        else if ('#' == o_cmd.p_buf[0u])
        {
            /* This is a comment line */
            b_result = 1;
        }
        else if (0 == snck_string_compare(&(o_cmd), &(o_name_cd)))
        {
            b_result = snck_builtin_cd(p_ctxt, &(o_args));
        }
        else if (0 == snck_string_compare(&(o_cmd), &(o_name_set)))
        {
            b_result = snck_builtin_set(p_ctxt, &(o_args));
        }
        else if (0 == snck_string_compare(&(o_cmd), &(o_name_unset)))
        {
            b_result = snck_builtin_unset(p_ctxt, &(o_args));
        }
        else if (0 == snck_string_compare(&(o_cmd), &(o_name_shell)))
        {
            b_result = snck_builtin_shell(p_ctxt, &(o_args));
        }
        else if (0 == snck_string_compare(&(o_cmd), &(o_name_hist)))
        {
            b_result = snck_builtin_hist(p_ctxt, &(o_args));
        }
        else if (0 == snck_string_compare(&(o_cmd), &(o_name_source)))
        {
            b_result = snck_builtin_source(p_ctxt, &(o_args));
        }
        else if ((0 == snck_string_compare(&(o_cmd), &(o_name_exit)))
            || (0 == snck_string_compare(&(o_cmd), &(o_name_logout))))
        {
            b_result = snck_builtin_exit(p_ctxt, &(o_args));
        }
        else
        {
            /* Use which to detect executable */
            /* Detect if cmd is a directory */
            /* Detect if cmd is a text file */
            struct stat o_stat_info;

            int i_stat_result;

            char * p_cmd_expanded0;

            struct snck_string o_cmd_expanded;

            snck_string_init(p_ctxt, &(o_cmd_expanded));

            if (snck_expand_get(p_ctxt, &(o_cmd), &(o_cmd_expanded)))
            {
                p_cmd_expanded0 = snck_string_get(p_ctxt, &(o_cmd_expanded));

                if (p_cmd_expanded0)
                {
                    char b_cmd_type;

                    b_cmd_type = 0;

                    i_stat_result = stat(p_cmd_expanded0, &(o_stat_info));

                    if (0 == i_stat_result)
                    {
                        if (S_ISDIR(o_stat_info.st_mode))
                        {
                            b_cmd_type = 1;
                        }
                        else if (S_ISREG(o_stat_info.st_mode))
                        {
                            if ((S_IXUSR|S_IXGRP|S_IXOTH) & o_stat_info.st_mode)
                            {
                                /* Executable */
                            }
                            else if ((S_IRUSR|S_IRGRP|S_IROTH) & o_stat_info.st_mode)
                            {
                                /* Detect file name extension */

                                /* Special case for jpg */

                                /* Launch editor */
                                if (o_stat_info.st_size < (off_t)(1024ul * 1024ul))
                                {
                                    b_cmd_type = 2;
                                }
                                else
                                {
                                    fprintf(stderr, "snck: file too big to edit\n");

                                    b_cmd_type = 3;
                                }
                            }
                            else
                            {
                                fprintf(stderr, "snck: unsupported mode\n");

                                b_cmd_type = 3;
                            }
                        }
                        else
                        {
                            fprintf(stderr, "snck: special file\n");

                            b_cmd_type = 3;
                        }
                    }

                    if (0 == b_cmd_type)
                    {
                        b_result = snck_file_exec_line(p_ctxt, p_line);
                    }
                    else if (1 == b_cmd_type)
                    {
                        b_result = snck_builtin_cd(p_ctxt, &(o_cmd_expanded));
                    }
                    else if (2 == b_cmd_type)
                    {
                        b_result = snck_builtin_edit(p_ctxt, &(o_cmd_expanded));
                    }
                    else
                    {
                    }

                    snck_string_put(p_ctxt, p_cmd_expanded0);
                }
            }
            else
            {
                fprintf(stderr, "snck: unable to expand cmd name\n");
            }

            snck_string_cleanup(p_ctxt, &(o_cmd_expanded));

            b_result = 1;
        }
    }
    else
    {
        b_result = 1;
    }

    snck_string_cleanup(p_ctxt, &(o_args));

    snck_string_cleanup(p_ctxt, &(o_cmd));

    return b_result;

} /* snck_file_decode_line() */

/*

Function: snck_file_read

Description:

    Read the given file line by line and execute each line.

*/
char
snck_file_read(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_name)
{
    char b_result = 1;

    char b_continue = 1;

    FILE * p_file;

#if defined(SNCK_DBG)
    {
        if (p_name)
        {
            fprintf(stderr, "dbg: snck_file read [%.*s]\n",
                (int)(p_name->i_buf_len),
                p_name->p_buf);
        }
        else
        {
            fprintf(stderr, "dbg: snck_file read stdin\n");
        }
    }
#endif /* #if defined(SNCK_DBG) */

    if (p_name)
    {
        char * p_name0 = snck_string_get(p_ctxt, p_name);

        if (p_name0)
        {
            p_file = fopen(p_name0, "r");

            snck_string_put(p_ctxt, p_name0);
        }
        else
        {
            p_file = NULL;
        }
    }
    else
    {
        p_file = stdin;
    }

    if (p_file)
    {
        struct snck_string o_line_accum;

        char b_overflow;

        b_overflow = 0;

        snck_string_init(p_ctxt, &(o_line_accum));

        while (b_result && b_continue)
        {
            struct snck_string o_line;

            char b_line_status;

            snck_string_init(p_ctxt, &(o_line));

            b_line_status = snck_line_get(p_ctxt, p_file, &(o_line), b_overflow);

            if (1 == b_line_status)
            {
                /* Append to line accumulator */
                snck_string_append_object(p_ctxt, &(o_line_accum), &(o_line));

                if (snck_token_is_complete(p_ctxt, &(o_line_accum)))
                {
                    if (snck_file_decode_line(p_ctxt, &(o_line_accum)))
                    {
                    }
                    else
                    {
                        b_result = 0;

                        b_continue = 0;
                    }

                    snck_string_ref(p_ctxt, &(o_line_accum), "");

                    b_overflow = 0;
                }
                else
                {
                    snck_string_append(p_ctxt, &(o_line_accum), "\n");

                    b_overflow = 1;
                }
            }
            else if (2 == b_line_status)
            {
                snck_string_ref(p_ctxt, &(o_line_accum), "");

                b_overflow = 0;

                b_continue = 1;
            }
            else
            {
                b_continue = 0;
            }

            snck_string_cleanup(p_ctxt, &(o_line));
        }

        if (stdin != p_file)
        {
            fclose(p_file);
        }
    }
    else
    {
#if defined(SNCK_DBG)
        {
            fprintf(stderr, "dbg: snck_file error opening file\n");
        }
#endif /* #if defined(SNCK_DBG) */

        b_result = 0;
    }

    return b_result;
} /* snck_file_read() */

/* end-of-file: snck_file.c */
