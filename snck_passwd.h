/* See LICENSE for license details. */

/*

Module: snck_passwd.h

Description:

    User information obtained from /etc/passwd file.

*/

/* Reverse include guard */
#if defined(INC_SNCK_PASSWD_H)
#error include snck_passwd.h once
#endif /* #if defined(INC_SNCK_PASSWD_H) */

#define INC_SNCK_PASSWD_H

/* Predefine context handle */
struct snck_ctxt;

/*

Structure: snck_passwd

Description:

    User information obtained from /etc/passwd file.

*/
struct snck_passwd
{
    struct passwd * p_details;

}; /* struct snck_passwd */

void
snck_passwd_init(
    struct snck_ctxt const * const
        p_ctxt);

void
snck_passwd_cleanup(
    struct snck_ctxt const * const
        p_ctxt);

struct passwd *
snck_passwd_get(
    struct snck_ctxt const * const
        p_ctxt);

/* end-of-file: snck_passwd.h */
