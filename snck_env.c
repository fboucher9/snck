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

/* Heap */
#include "snck_heap.h"

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

    char * p_temp_name;

    char * p_temp_value;

    p_temp_name = (char *)(snck_heap_realloc(p_ctxt, NULL, p_name->i_buf_len + 1u));

    if (p_temp_name)
    {
        memcpy(p_temp_name, p_name->p_buf, p_name->i_buf_len);

        p_temp_name[p_name->i_buf_len] = '\000';

        p_temp_value = getenv(p_temp_name);

        if (p_temp_value)
        {
            b_result = snck_string_copy(p_ctxt, p_value, p_temp_value);
        }
        else
        {
            b_result = 0;
        }

        snck_heap_realloc(p_ctxt, p_temp_name, 0u);
    }
    else
    {
        b_result = 0;
    }

    return b_result;

} /* snck_env_get() */

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

    char * p_temp_name;

    char * p_temp_value;

    p_temp_name = (char *)(snck_heap_realloc(p_ctxt, NULL, p_name->i_buf_len + 1u));

    if (p_temp_name)
    {
        memcpy(p_temp_name, p_name->p_buf, p_name->i_buf_len);

        p_temp_name[p_name->i_buf_len] = '\000';

        if (p_value)
        {
            p_temp_value = (char *)(snck_heap_realloc(p_ctxt, NULL, p_value->i_buf_len + 1u));

            if (p_temp_value)
            {
                memcpy(p_temp_value, p_value->p_buf, p_value->i_buf_len);

                p_temp_value[p_value->i_buf_len] = '\000';

                if (0 == setenv(p_temp_name, p_temp_value, 1))
                {
                    b_result = 1;
                }
                else
                {
                    b_result = 0;
                }

                snck_heap_realloc(p_ctxt, p_temp_value, 0u);
            }
            else
            {
                b_result = 0;
            }
        }
        else
        {
            if (0 == unsetenv(p_temp_name))
            {
                b_result = 1;
            }
            else
            {
                b_result = 0;
            }
        }

        snck_heap_realloc(p_ctxt, p_temp_name, 0u);
    }
    else
    {
        b_result = 0;
    }

    return b_result;

} /* snck_env_set() */

/* end-of-file: snck_env.c */
