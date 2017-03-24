/* See LICENSE for license details. */

/*

Module: snck_prompt.c

Description:

    Build a prompt used for interactive line editor.

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Context */
#include "snck_ctxt.h"

/* String */
#include "snck_string.h"

/* Module */
#include "snck_prompt.h"

/* Information */
#include "snck_info.h"

/* Environment */
#include "snck_env.h"

static
size_t
snck_prompt_pass1(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_value)
{
    size_t i_out_len;

    size_t i_in;

    i_out_len = 0u;

    i_in = 0u;

    while (i_in < p_value->i_buf_len)
    {
        char c_in;

        c_in = p_value->p_buf[i_in];

        i_in ++;

        if ('\\' == c_in)
        {
            if (i_in < p_value->i_buf_len)
            {
                c_in = p_value->p_buf[i_in];

                i_in ++;

                if ('u' == c_in)
                {
                    i_out_len += p_ctxt->p_info->o_user.i_buf_len;
                }
                else if ('h' == c_in)
                {
                    i_out_len += p_ctxt->p_info->o_host.i_buf_len;
                }
                else if ('w' == c_in)
                {
                    if (0 == strncmp(p_ctxt->p_info->o_pwd.p_buf, p_ctxt->p_info->o_home.p_buf, p_ctxt->p_info->o_home.i_buf_len))
                    {
                        i_out_len += p_ctxt->p_info->o_pwd.i_buf_len - p_ctxt->p_info->o_home.i_buf_len + 1;
                    }
                    else
                    {
                        i_out_len += p_ctxt->p_info->o_pwd.i_buf_len;
                    }
                }
                else if ('$' == c_in)
                {
                    i_out_len ++;
                }
                else if ('_' == c_in)
                {
                    i_out_len ++;
                }
                else
                {
                    i_out_len ++;
                }
            }
            else
            {
                /* error */
            }
        }
        else
        {
            i_out_len ++;
        }
    }

    i_out_len ++;

    return i_out_len;
}

static
char
snck_prompt_pass2(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_value,
    struct snck_string * const
        p_out)
{
    char b_result;

    size_t i_in;

    int i_len;

    i_in = 0u;

    p_out->i_buf_len = 0u;

    while (i_in < p_value->i_buf_len)
    {
        char c_in;

        c_in = p_value->p_buf[i_in];

        i_in ++;

        if ('\\' == c_in)
        {
            if (i_in < p_value->i_buf_len)
            {
                c_in = p_value->p_buf[i_in];

                i_in ++;

                if ('u' == c_in)
                {
                    i_len = p_ctxt->p_info->o_user.i_buf_len;
                    memcpy(p_out->p_buf + p_out->i_buf_len, p_ctxt->p_info->o_user.p_buf, i_len);
                    p_out->i_buf_len += i_len;
                }
                else if ('h' == c_in)
                {
                    i_len = p_ctxt->p_info->o_host.i_buf_len;
                    memcpy(p_out->p_buf + p_out->i_buf_len, p_ctxt->p_info->o_host.p_buf, i_len);
                    p_out->i_buf_len += i_len;
                }
                else if ('w' == c_in)
                {
                    char * p_buf;
                    if (0 == strncmp(p_ctxt->p_info->o_pwd.p_buf, p_ctxt->p_info->o_home.p_buf, p_ctxt->p_info->o_home.i_buf_len))
                    {
                        p_out->p_buf[p_out->i_buf_len] = '~';
                        p_out->i_buf_len ++;
                        i_len = p_ctxt->p_info->o_pwd.i_buf_len - p_ctxt->p_info->o_home.i_buf_len;
                        p_buf = p_ctxt->p_info->o_pwd.p_buf + p_ctxt->p_info->o_home.i_buf_len;
                    }
                    else
                    {
                        i_len = p_ctxt->p_info->o_pwd.i_buf_len;
                        p_buf = p_ctxt->p_info->o_pwd.p_buf;
                    }

                    if (i_len > 32)
                    {
                        memcpy(p_out->p_buf + p_out->i_buf_len, p_buf, 8);
                        p_out->i_buf_len += 8;

                        p_out->p_buf[p_out->i_buf_len] = '.';
                        p_out->i_buf_len ++;

                        p_out->p_buf[p_out->i_buf_len] = '.';
                        p_out->i_buf_len ++;

                        memcpy(p_out->p_buf + p_out->i_buf_len, p_buf + i_len - 24, 24);
                        p_out->i_buf_len += 24;
                    }
                    else
                    {
                        memcpy(p_out->p_buf + p_out->i_buf_len, p_buf, i_len);
                        p_out->i_buf_len += i_len;
                    }
                }
                else if ('$' == c_in)
                {
                    p_out->p_buf[p_out->i_buf_len] = '$';

                    p_out->i_buf_len ++;
                }
                else if ('_' == c_in)
                {
                    p_out->p_buf[p_out->i_buf_len] = ' ';

                    p_out->i_buf_len ++;
                }
                else
                {
                    p_out->p_buf[p_out->i_buf_len] = c_in;

                    p_out->i_buf_len ++;
                }
            }
            else
            {
                /* error */
            }
        }
        else
        {
            p_out->p_buf[p_out->i_buf_len] = c_in;

            p_out->i_buf_len ++;
        }
    }

    p_out->p_buf[p_out->i_buf_len] = '\000';

    p_out->i_buf_len ++;

    b_result = 1;

    return b_result;

}

static
char
snck_prompt_detect_env(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_value)
{
    char b_result;

    struct snck_string o_name;

    static char a_ps1_ref[] = { 'P', 'S', '1' };

    snck_string_init_ref_buffer(&(o_name), a_ps1_ref, sizeof(a_ps1_ref));

    if (snck_env_get(p_ctxt, &(o_name), p_value))
    {
        b_result = 1;
    }
    else
    {
        static char a_ps1_default[] = { '\\', 'u', '@', '\\', 'h', ':', '\\', 'w', '\\', '$', ' ' };

        b_result = snck_string_ref_buffer(p_ctxt, p_value, a_ps1_default, sizeof(a_ps1_default));
    }

    return b_result;
}

char
snck_prompt_get(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_out,
    char const
        b_overflow)
{
    char b_result;

    if (!b_overflow)
    {
        size_t i_out_len;

        struct snck_string o_value;

        snck_string_init(p_ctxt, &(o_value));

        if (snck_prompt_detect_env(p_ctxt, &(o_value)))
        {
            i_out_len = snck_prompt_pass1(p_ctxt, &(o_value));

            if (snck_string_resize(p_ctxt, p_out, i_out_len))
            {
                b_result = snck_prompt_pass2(p_ctxt, &(o_value), p_out);
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

        snck_string_cleanup(p_ctxt, &(o_value));
    }
    else
    {
        b_result = snck_string_ref(p_ctxt, p_out, "> ");
    }

    return b_result;

} /* snck_prompt_get() */

/* end-of-file: snck_prompt.c */
