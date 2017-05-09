/* See LICENSE for license details. */

/*

Module: snck_env.c

Description:

    Access to environment variables.

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
#include "snck_env.h"

/*

Function: snck_env_get()

Description:

    Get value of an environment variable.

Parameters:

    p_ctxt
        Pointer to snck_ctxt structure

    p_name
        Pointer to snck_string for name of environment variable

    p_value
        Pointer to snck_string for value of environment variable

Returns:

    true on success, false otherwise.

*/
char
snck_env_get(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_name,
    struct snck_string * const
        p_value)
{
    /* Create a zero-terminated name string */

    char b_result;

    char * p_name0;

    p_name0 = snck_string_get(p_ctxt, p_name);

    if (p_name0)
    {
        char * p_value0;

        p_value0 = getenv(p_name0);

        if (p_value0)
        {
            b_result = snck_string_copy(p_ctxt, p_value, p_value0);
        }
        else
        {
            b_result = 0;
        }

        snck_string_put(p_ctxt, p_name0);
    }
    else
    {
        b_result = 0;
    }

    return b_result;

} /* snck_env_get() */

/*

Function: snck_env_set()

Description:

    p_ctxt
        Pointer to snck_ctxt structure

    p_name
        Pointer to snck_string for name of environment variable

    p_value
        Pointer to snck_string for value of environment variable

Returns:

    true on success, false otherwise.

*/
char
snck_env_set(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_name,
    struct snck_string const * const
        p_value)
{
    char b_result;

    char * p_name0;

    p_name0 = snck_string_get(p_ctxt, p_name);

    if (p_name0)
    {
        if (p_value)
        {
            char * p_value0;

            p_value0 = snck_string_get(p_ctxt, p_value);

            if (p_value0)
            {
                if (0 == setenv(p_name0, p_value0, 1))
                {
                    b_result = 1;
                }
                else
                {
                    b_result = 0;
                }

                snck_string_put(p_ctxt, p_value0);
            }
            else
            {
                b_result = 0;
            }
        }
        else
        {
            if (0 == unsetenv(p_name0))
            {
                b_result = 1;
            }
            else
            {
                b_result = 0;
            }
        }

        snck_string_put(p_ctxt, p_name0);
    }
    else
    {
        b_result = 0;
    }

    return b_result;

} /* snck_env_set() */

/* end-of-file: snck_env.c */
