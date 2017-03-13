/* See LICENSE for license details. */

/*

Module: snck_prompt.h

Description:

    Build a prompt used for interactive line editor.

*/

/* Reverse include guard */
#if defined(INC_SNCK_PROMPT_H)
#error include snck_prompt.h once
#endif /* #if defined(INC_SNCK_PROMPT_H) */

#define INC_SNCK_PROMPT_H

/* Predefine context handle */
struct snck_ctxt;

char const *
snck_prompt_get(
    struct snck_ctxt const * const
        p_ctxt);

void
snck_prompt_put(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_buf);

/* end-of-file: snck_prompt.h */
