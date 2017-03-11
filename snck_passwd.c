/* See LICENSE for license details. */

/*

Module: snck_passwd.c

Description:

    User information obtained from /etc/passwd file.

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Context */
#include "snck_ctxt.h"

/* Module */
#include "snck_passwd.h"

void
snck_passwd_init(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_passwd * const p_passwd =
        p_ctxt->p_passwd;

    p_passwd->p_details =
        NULL;

} /* snck_passwd_init() */

void
snck_passwd_cleanup(
    struct snck_ctxt const * const
        p_ctxt)
{
    (void)(
        p_ctxt);

} /* snck_passwd_cleanup() */

struct passwd *
snck_passwd_get(
    struct snck_ctxt const * const
        p_ctxt)
{
    struct snck_passwd * const p_passwd =
        p_ctxt->p_passwd;

    if (
        !p_passwd->p_details)
    {
        p_passwd->p_details =
            getpwuid(
                getuid());
    }

    return
        p_passwd->p_details;

} /* snck_passwd_get() */

/* end-of-file: snck_passwd.c */
