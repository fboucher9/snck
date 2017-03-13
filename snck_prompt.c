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

static char a_prompt[4096u];

static
void
snck_build_prompt(
    struct snck_ctxt const * const p_ctxt)
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

char const *
snck_prompt_get(
    struct snck_ctxt const * const
        p_ctxt)
{
    snck_build_prompt(p_ctxt);

    return a_prompt;

} /* snck_prompt_get() */

void
snck_prompt_put(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_buf)
{
    (void)(
        p_ctxt);
    (void)(
        p_buf);

} /* snck_prompt_put() */

/* end-of-file: snck_prompt.c */
