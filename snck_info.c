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

static
char
snck_info_detect_user_and_home(
    struct snck_ctxt const * const
        p_ctxt)
{
    char b_result;

    struct snck_info * const p_info =
        p_ctxt->p_info;

    char * p_env;

    char b_found_user = 0;

    char b_found_home = 0;

    b_result = 1;

    if (b_result && !b_found_user)
    {
        p_env = getenv("USER");

        if (p_env)
        {
            b_result = snck_string_copy(p_ctxt, &(p_info->o_user), p_env);

            if (b_result)
            {
                b_found_user = 1;
            }
        }
    }

    if (b_result && !b_found_home)
    {
        p_env = getenv("HOME");

        if (p_env)
        {
            b_result = snck_string_copy(p_ctxt, &(p_info->o_home), p_env);

            if (b_result)
            {
                b_found_home = 1;
            }
        }
    }

    if (b_result && (!b_found_user || !b_found_home))
    {
        uid_t id;

        struct passwd * pw;

        id = getuid();

        pw = getpwuid(id);

        if (pw)
        {
            if (b_result && !b_found_user)
            {
                b_result = snck_string_copy(p_ctxt, &(p_info->o_user), pw->pw_name);

                if (b_result)
                {
                    b_found_user = 1;
                }
            }

            if (b_result && !b_found_home)
            {
                b_result = snck_string_copy(p_ctxt, &(p_info->o_home), pw->pw_dir);

                if (b_result)
                {
                    b_found_home = 1;
                }
            }
        }
    }

    if (b_result && !b_found_user)
    {
        b_result = snck_string_copy(p_ctxt, &(p_info->o_user), "snck");

        if (b_result)
        {
            b_found_user = 1;
        }
    }

    if (b_result && !b_found_home)
    {
        static char a_home[256u];

        sprintf(a_home, "/home/%s", p_info->o_user.p_buf);

        b_result = snck_string_copy(p_ctxt, &(p_info->o_home), a_home);

        if (b_result)
        {
            b_found_home = 1;
        }
    }

    if (b_result && b_found_user)
    {
        if (0 == setenv("USER", p_info->o_user.p_buf, 1))
        {
        }
        else
        {
            b_result = 0;
        }
    }

    if (b_result && b_found_home)
    {
        if (0 == setenv("HOME", p_info->o_home.p_buf, 1))
        {
        }
        else
        {
            b_result = 0;
        }
    }

    return b_result;

} /* snck_info_detect_user_and_home() */

static
char
snck_info_detect_host(
    struct snck_ctxt const * const
        p_ctxt)
{
    char b_result;

    struct snck_info * const p_info =
        p_ctxt->p_info;

    static char a_host[256u];

    a_host[0u] = '\000';

    if (0 == gethostname(a_host, sizeof(a_host)))
    {
    }
    else
    {
        strcpy(a_host, "snck");
    }

    b_result = snck_string_copy(p_ctxt, &(p_info->o_host), a_host);

    return b_result;

} /* snck_info_detect_host() */

static
char
snck_info_detect_pwd(
    struct snck_ctxt const * const
        p_ctxt)
{
    char b_result;

    struct snck_info * const p_info =
        p_ctxt->p_info;

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

    b_result =
        snck_string_copy(
            p_ctxt,
            &(p_info->o_pwd),
            a_pwd);

    if (b_result)
    {
        b_result =
            snck_string_copy_object(
                p_ctxt,
                &(p_info->o_old_pwd),
                &(p_info->o_pwd));
    }

    return b_result;

} /* snck_info_detect_pwd() */

static
char
snck_info_detect(
    struct snck_ctxt const * const
        p_ctxt)
{
    char b_result;

    b_result =
        snck_info_detect_user_and_home(
            p_ctxt);

    if (
        b_result)
    {
        b_result =
            snck_info_detect_host(
                p_ctxt);

        if (
            b_result)
        {
            b_result =
                snck_info_detect_pwd(
                    p_ctxt);
        }
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
