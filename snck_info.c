/* See LICENSE for license details. */

/*

Module: snck_info.c

Description:

    Collection of user and system information.

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* String */
#include "snck_string.h"

/* Information */
#include "snck_info.h"

/* Context */
#include "snck_ctxt.h"

/* Heap */
#include "snck_heap.h"

/* Password database */
#include "snck_passwd.h"

/* Env */
#include "snck_env.h"

static
char
snck_info_detect_user(
    struct snck_ctxt const * const
        p_ctxt)
{
    char b_result;

    struct snck_info * const p_info =
        p_ctxt->p_info;

    static char const a_name_user[] = { 'U', 'S', 'E', 'R' };

    static struct snck_string const o_name_user = { (char *)(a_name_user), sizeof(a_name_user), 0u };

    b_result = 1;

    if (b_result)
    {
        if (snck_env_get(p_ctxt, &(o_name_user), &(p_info->o_user)))
        {
            b_result = 1;
        }
        else
        {
            struct passwd * const pw = snck_passwd_get(p_ctxt);

            if (pw)
            {
                b_result = snck_string_copy(p_ctxt, &(p_info->o_user), pw->pw_name);
            }
            else
            {
                b_result = snck_string_copy(p_ctxt, &(p_info->o_user), "snck");
            }
        }
    }

    if (b_result)
    {
        b_result = snck_env_set(p_ctxt, &(o_name_user), &(p_info->o_user));
    }

    return b_result;

} /* snck_info_detect_user() */

static
char
snck_info_detect_home(
    struct snck_ctxt const * const
        p_ctxt)
{
    char b_result;

    struct snck_info * const p_info =
        p_ctxt->p_info;

    static char const a_name_home[] = { 'H', 'O', 'M', 'E' };

    static struct snck_string o_name_home = { (char*)(a_name_home), sizeof(a_name_home), 0u };

    b_result = 1;

    if (b_result)
    {
        if (snck_env_get(p_ctxt, &(o_name_home), &(p_info->o_home)))
        {
            b_result = 1;
        }
        else
        {
            struct passwd * const pw = snck_passwd_get(p_ctxt);

            if (pw)
            {
                b_result = snck_string_copy(p_ctxt, &(p_info->o_home), pw->pw_dir);
            }
            else
            {
                b_result = snck_string_copy(p_ctxt, &(p_info->o_home), "/home/");

                if (b_result)
                {
                    b_result = snck_string_append_object(p_ctxt, &(p_info->o_home), &(p_info->o_user));
                }
            }
        }
    }

    if (b_result)
    {
        if (snck_env_set(p_ctxt, &(o_name_home), &(p_info->o_home)))
        {
        }
        else
        {
            b_result = 0;
        }
    }

    return b_result;

} /* snck_info_detect_home() */

static
char
snck_info_detect_host(
    struct snck_ctxt const * const
        p_ctxt)
{
    char b_result;

    struct snck_info * const p_info =
        p_ctxt->p_info;

    if (snck_string_resize(p_ctxt, &(p_info->o_host), 256u))
    {
        p_info->o_host.p_buf[0u] = '\000';

        if (0 == gethostname(p_info->o_host.p_buf, 255u))
        {
            p_info->o_host.i_buf_len = strlen(p_info->o_host.p_buf);
        }
        else
        {
            snck_string_ref(p_ctxt, &(p_info->o_host), "snck");
        }

        b_result = 1;
    }
    else
    {
        b_result = 0;
    }

    return b_result;

} /* snck_info_detect_host() */

char
snck_info_update_wd(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_ref)
{
    char b_result;

    struct snck_info * const p_info =
        p_ctxt->p_info;

    char * p_temp;

    b_result = 1;

    p_temp = NULL;

    if (p_ref)
    {
        p_temp = (char *)(p_ref);
    }
    else
    {
        size_t i_max_len;

        char b_retry;

        b_retry = 1;

        i_max_len = 128u;

        while (b_result && b_retry)
        {
            p_temp = snck_heap_realloc(p_ctxt, p_temp, i_max_len);

            if (p_temp)
            {
                if (NULL != getcwd(p_temp, i_max_len))
                {
                    b_retry = 0;
                }
                else
                {
                    if (ERANGE == errno)
                    {
                        i_max_len <<= 1;

                        b_retry = 1;
                    }
                    else
                    {
                        b_result = 0;
                    }
                }
            }
            else
            {
                b_result = 0;
            }
        }
    }

    if (b_result)
    {
        /* Detect if value has changed */
        if (!p_info->o_pwd.p_buf || (0 != strcmp(p_temp, p_info->o_pwd.p_buf)))
        {
            static char const a_name_pwd[] = { 'P', 'W', 'D' };

            static struct snck_string o_name_pwd = { (char *)(a_name_pwd), sizeof(a_name_pwd), 0u };

            struct snck_string o_value;

            snck_string_init_ref(&(o_value), p_temp);

            if (snck_env_set(p_ctxt, &(o_name_pwd), &(o_value)))
            {
                if (p_info->o_pwd.p_buf)
                {
                    b_result =
                        snck_string_copy(
                            p_ctxt,
                            &(p_info->o_old_pwd),
                            p_info->o_pwd.p_buf);
                }

                if (b_result)
                {
                    b_result =
                        snck_string_copy(
                            p_ctxt,
                            &(p_info->o_pwd),
                            p_temp);
                }
            }
            else
            {
                b_result = 0;
            }
        }
        else
        {
            /* Value has not changed */
            b_result = 1;
        }
    }

    if (p_ref)
    {
    }
    else
    {
        if (p_temp)
        {
            snck_heap_realloc(p_ctxt, p_temp, 0u);

            p_temp = NULL;
        }
    }

    return b_result;

} /* snck_info_update_wd() */

static
char
snck_info_detect(
    struct snck_ctxt const * const
        p_ctxt)
{
    char b_result;

    b_result = 1;

    if (
        b_result)
    {
        b_result =
            snck_info_detect_user(
                p_ctxt);
    }

    if (
        b_result)
    {
        b_result =
            snck_info_detect_home(
                p_ctxt);
    }

    if (
        b_result)
    {
        b_result =
            snck_info_detect_host(
                p_ctxt);
    }

    if (
        b_result)
    {
        b_result =
            snck_info_update_wd(
                p_ctxt,
                NULL);
    }

    return b_result;

} /* snck_info_detect() */

/*

Function: snck_info_init

Description:

    Get user and system information.

*/
char
snck_info_init(
    struct snck_ctxt const * const
        p_ctxt)
{
    char b_result;

    struct snck_info * const p_info =
        p_ctxt->p_info;

    snck_string_init(
        p_ctxt,
        &(
            p_info->o_user));

    snck_string_init(
        p_ctxt,
        &(
            p_info->o_home));

    snck_string_init(
        p_ctxt,
        &(
            p_info->o_host));

    snck_string_init(
        p_ctxt,
        &(
            p_info->o_pwd));

    snck_string_init(
        p_ctxt,
        &(
            p_info->o_old_pwd));

    b_result =
        snck_info_detect(
            p_ctxt);

    if (!b_result)
    {
        snck_string_cleanup(
            p_ctxt,
            &(
                p_info->o_user));

        snck_string_cleanup(
            p_ctxt,
            &(
                p_info->o_home));

        snck_string_cleanup(
            p_ctxt,
            &(
                p_info->o_host));

        snck_string_cleanup(
            p_ctxt,
            &(
                p_info->o_pwd));

        snck_string_cleanup(
            p_ctxt,
            &(
                p_info->o_old_pwd));
    }

    return b_result;

} /* snck_info_init() */

/*

Function: snck_info_cleanup

Description:

    Release user and system information.

*/
void
snck_info_cleanup(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_info * const p_info =
        p_ctxt->p_info;

    snck_string_cleanup(
        p_ctxt,
        &(
            p_info->o_user));

    snck_string_cleanup(
        p_ctxt,
        &(
            p_info->o_home));

    snck_string_cleanup(
        p_ctxt,
        &(
            p_info->o_host));

    snck_string_cleanup(
        p_ctxt,
        &(
            p_info->o_pwd));

    snck_string_cleanup(
        p_ctxt,
        &(
            p_info->o_old_pwd));

} /* snck_info_cleanup() */

/* end-of-file: snck_info.c */
