/* See LICENSE for license details. */

/*

Module: snck_info.h

Description:

    Collection of user and system information.

*/

/* Reverse include guard */
#if defined(INC_SNCK_INFO_H)
#error include snck_info.h once
#endif /* #if defined(INC_SNCK_INFO_H) */

#define INC_SNCK_INFO_H

/* Predefine context handle */
struct snck_ctxt;

/*

Structure: snck_info

Description:

    Collection of user and system information.

*/
struct snck_info
{
    struct snck_string o_user;

    struct snck_string o_home;

    struct snck_string o_host;

    struct snck_string o_pwd;

    struct snck_string o_old_pwd;

}; /* struct snck_info */

char
snck_info_init(
    struct snck_ctxt const * const
        p_ctxt);

void
snck_info_cleanup(
    struct snck_ctxt const * const
        p_ctxt);

char
snck_info_update_wd(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_ref);

/* end-of-file: snck_info.h */
