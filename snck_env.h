/* See LICENSE for license details. */

/*

Module: snck_env.h

Description:

    Access to environment variables.

*/

/* Predefine context handle */
struct snck_ctxt;

char
snck_env_get(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_name,
    struct snck_string * const
        p_value);

char
snck_env_set(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_name,
    struct snck_string const * const
        p_value);

#if 0

char
snck_env_query(
    struct snck_ctxt const * const
        p_ctxt,
    void (* p_callback )(
        struct snck_ctxt const * const
            p_ctxt,
        void * const
            p_private,
        struct snck_string const * const
            p_item),
    void * const
        p_private);

#endif

/* end-of-file: snck_env.h */
