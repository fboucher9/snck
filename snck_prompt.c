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

/* Module */
#include "snck_prompt.h"

/* String */
#include "snck_string.h"

/* Information */
#include "snck_info.h"

/* Heap */
#include "snck_heap.h"

char const *
snck_prompt_get(
    struct snck_ctxt const * const
        p_ctxt)
{
    char * p_result;

    char * p_ps1;

    int i_out_len;

    p_result = NULL;

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

    i_out_len = 0;

    {
        char * p_in;

        p_in = p_ps1;

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
    }

    p_result = (char *)(snck_heap_realloc(p_ctxt, NULL, i_out_len));

    if (p_result)
    {
        char * p_in;

        char * p_out;

        int i_len;

        p_in = p_ps1;

        p_out = p_result;

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
                        char * p_buf;
                        if (0 == strncmp(p_ctxt->p_info->o_pwd.p_buf, p_ctxt->p_info->o_home.p_buf, p_ctxt->p_info->o_home.i_buf_len))
                        {
                            *p_out = '~';
                            p_out ++;
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
                            memcpy(p_out, p_buf, 8);
                            p_out += 8;

                            *p_out = '.';
                            p_out ++;

                            *p_out = '.';
                            p_out ++;

                            memcpy(p_out, p_buf + i_len - 24, 24);
                            p_out += 24;
                        }
                        else
                        {
                            memcpy(p_out, p_buf, i_len);
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

    return p_result;

} /* snck_prompt_get() */

void
snck_prompt_put(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_buf)
{
    snck_heap_realloc(p_ctxt, (void *)(p_buf), 0);

} /* snck_prompt_put() */

/* end-of-file: snck_prompt.c */
