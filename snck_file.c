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
    struct snck_string const * const p_line,
    struct snck_string * const p_cmd,
    struct snck_string * const p_args)
{
    char b_found;

    int i_args_it = snck_find_word_begin(p_line->p_buf);

    if (p_line->p_buf[i_args_it])
    {
        int i_word_len = snck_find_word_end(p_line->p_buf + i_args_it);

        snck_string_init_ref_buffer(p_cmd, p_line->p_buf + i_args_it, i_word_len);

        i_word_len += snck_find_word_begin(p_line->p_buf + i_args_it + i_word_len);

        snck_string_init_ref_buffer(p_args, p_line->p_buf + i_args_it + i_word_len, p_line->i_buf_len - i_args_it - i_word_len);

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
    static char const a_fmt[] = "PATH= echo -n %s";

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
    struct snck_string const * const p_line)
{
    char b_result;

    int i_result;

    char const * p_path = NULL;

    char const * p_path_expand = NULL;

    if (p_line->p_buf[0u])
    {
        if (0 == strcmp(p_line->p_buf, "-"))
        {
            p_path = p_ctxt->p_info->o_old_pwd.p_buf;
        }
        else
        {
            /* Use sh -c 'echo ...' to expand the argument */
            p_path_expand = snck_expand_get(p_ctxt, p_line->p_buf);

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
    struct snck_string const * const p_line)
{
    char b_result;

    struct snck_string o_name;

    struct snck_string o_rest;

    snck_string_init(p_ctxt, &(o_name));

    snck_string_init(p_ctxt, &(o_rest));

    if (snck_find_word(p_line, &(o_name), &(o_rest)))
    {
        if (o_rest.p_buf[0u])
        {
            char const * p_value;

            p_value = snck_expand_get(p_ctxt, o_rest.p_buf);

            if (p_value)
            {
                struct snck_string o_value;

                snck_string_init_ref(&(o_value), p_value);

                snck_env_set(p_ctxt, &(o_name), &(o_value));

                snck_expand_put(p_ctxt, p_value);
            }
        }
        else
        {
            /* */
            struct snck_string o_value;

            snck_string_init(p_ctxt, &(o_value));

            if (snck_env_get(p_ctxt, &(o_name), &(o_value)))
            {
                fprintf(stdout, "%.*s\n", o_value.i_buf_len, o_value.p_buf);
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

    char * l_argv[32u];

    unsigned int l_argc;

    unsigned int i;

    l_argc = 0;

    l_argv[l_argc] = "/bin/sh";

    l_argc ++;

    l_argv[l_argc] = "-c";

    l_argc ++;

    if (p_ctxt->p_opts->b_trace)
    {
        l_argv[l_argc] = "-x";

        l_argc ++;
    }

    l_argv[l_argc] = (char *)(p_line->p_buf);

    l_argc ++;

    l_argv[l_argc] = "sh";

    l_argc ++;

    for (i=0u; i<p_ctxt->p_opts->i_argc; i++)
    {
        if (l_argc < 32u)
        {
            l_argv[l_argc] = p_ctxt->p_opts->p_argv[i];

            l_argc ++;
        }
    }

    l_argv[l_argc] = NULL;

    l_argc ++;

    if (p_ctxt->p_opts->b_trace)
    {
        fprintf(stderr, "snck: exec sh -c [%s]\n", p_line->p_buf);
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
                fprintf(stderr, "%6d: %s\n",
                    -10+i,
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

    if (p_line->p_buf[0u])
    {
        char const * p_path;

        p_path = snck_expand_get(p_ctxt, p_line->p_buf);

        if (p_path)
        {
            if (snck_file_read(p_ctxt, p_path))
            {
            }
            else
            {
                fprintf(stderr, "snck: error opening file\n");
            }

            snck_expand_put(p_ctxt, p_path);
        }
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

    char const * p_editor;

    snck_string_init(p_ctxt, &(o_value_editor));

    if (snck_env_get(p_ctxt, &(o_name_editor), &(o_value_editor)))
    {
        p_editor = o_value_editor.p_buf;
    }
    else
    {
        p_editor = "vi";
    }

    l_argv[0u] = (char *)(p_editor);

    l_argv[1u] = (char *)(p_line->p_buf);

    l_argv[2u] = NULL;

    /* execute the command */
    b_result = snck_wrap_exec(p_ctxt, l_argv);

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

    if (snck_find_word(p_line, &(o_cmd), &(o_args)))
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

            struct snck_string o_cmd_sz;

            char const * p_cmd_expanded;

            snck_string_init(p_ctxt, &(o_cmd_sz));

            snck_string_copy_object(p_ctxt, &(o_cmd_sz), &(o_cmd));

            p_cmd_expanded = snck_expand_get(p_ctxt, o_cmd_sz.p_buf);

            b_result = 1;

            if (p_cmd_expanded)
            {
                char b_cmd_type;

                struct snck_string o_cmd_expanded;

                snck_string_init_ref(&(o_cmd_expanded), p_cmd_expanded);

                b_cmd_type = 0;

                i_stat_result = stat(p_cmd_expanded, &(o_stat_info));

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

                snck_string_cleanup(p_ctxt, &(o_cmd_expanded));

                snck_expand_put(p_ctxt, p_cmd_expanded);
            }
            else
            {
                fprintf(stderr, "snck: unable to expand cmd name\n");
            }

            snck_string_cleanup(p_ctxt, &(o_cmd_sz));
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
    char const * const
        p_name)
{
    char b_result = 1;

    char b_continue = 1;

    FILE * p_file;

    if (p_name)
    {
        p_file = fopen(p_name, "r");
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
        b_result = 0;
    }

    return b_result;
} /* snck_file_read() */

/* end-of-file: snck_file.c */
