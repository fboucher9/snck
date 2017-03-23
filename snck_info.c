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

    struct snck_string o_name;

    snck_string_init_ref(&(o_name), "USER");

    b_result = 1;

    if (b_result)
    {
        if (snck_env_get(p_ctxt, &(o_name), &(p_info->o_user)))
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
        b_result = snck_env_set(p_ctxt, &(o_name), &(p_info->o_user));
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

    struct snck_string o_name;

    snck_string_init_ref(&(o_name), "HOME");

    b_result = 1;

    if (b_result)
    {
        if (snck_env_get(p_ctxt, &(o_name), &(p_info->o_home)))
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
        if (snck_env_set(p_ctxt, &(o_name), &(p_info->o_home)))
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

    char * p_temp;

    p_temp = (char *)(snck_heap_realloc(p_ctxt, NULL, 256u));

    if (p_temp)
    {
        p_temp[0u] = '\000';

        if (0 == gethostname(p_temp, 255u))
        {
        }
        else
        {
#if 0
            fprintf(stderr, "gethostname error\n");
#endif

            strcpy(p_temp, "snck");
        }

        b_result = snck_string_copy(p_ctxt, &(p_info->o_host), p_temp);

        snck_heap_realloc(p_ctxt, (void *)(p_temp), 0u);
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
        p_ctxt)
{
    char b_result;

    struct snck_info * const p_info =
        p_ctxt->p_info;

    size_t i_max_len;

    char b_retry;

    b_result = 1;

    b_retry = 1;

    i_max_len = 128u;

    while (b_result && b_retry)
    {
        char * p_temp;

        p_temp = snck_heap_realloc(p_ctxt, NULL, i_max_len);

        if (p_temp)
        {
            if (NULL != getcwd(p_temp, i_max_len))
            {
                b_retry = 0;

                /* Detect if value has changed */
                if (!p_info->o_pwd.p_buf || (0 != strcmp(p_temp, p_info->o_pwd.p_buf)))
                {
                    struct snck_string o_name;

                    struct snck_string o_value;

                    snck_string_init_ref(&(o_name), "PWD");

                    snck_string_init_ref(&(o_value), p_temp);

                    if (snck_env_set(p_ctxt, &(o_name), &(o_value)))
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

            snck_heap_realloc(p_ctxt, p_temp, 0u);
        }
        else
        {
            b_result = 0;
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
                p_ctxt);
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
