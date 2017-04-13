/* See LICENSE for license details. */

/*

Module: snck_suggest.h

Description:

*/

/* Reverse include guard */
#if defined(INC_SNCK_SUGGEST_H)
#error include snck_suggest.h once
#endif /* #if defined(INC_SNCK_SUGGEST_H) */

#define INC_SNCK_SUGGEST_H

/* Header file dependencies */
#if !defined(INC_SNCK_LIST_H)
#error include snck_list.h before
#endif /* #if !defined(INC_SNCK_LIST_H) */

#if !defined(INC_SNCK_STRING_H)
#error include snck_string.h before
#endif /* #if !defined(INC_SNCK_STRING_H) */

/* Predefine context handle */
struct snck_ctxt;

struct snck_suggest_node
{
    struct snck_list o_list;

    struct snck_string o_buf;

}; /* struct snck_suggest_node */

struct snck_suggest_node *
snck_suggest_node_create(
    struct snck_ctxt const * const
        p_ctxt);

void
snck_suggest_node_destroy(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_node * const
        p_suggest_node);

struct snck_suggest_list
{
    struct snck_list o_list;

    int i_count;

}; /* struct snck_suggest_list */

void
snck_suggest_list_init(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list);

void
snck_suggest_list_cleanup(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list);

char
snck_suggest_list_add(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    struct snck_suggest_node * const
        p_suggest_node);

void
snck_suggest_from_history(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    struct snck_string const * const
        p_wild);

void
snck_suggest_from_lastword(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    char const * const
        buf,
    size_t const
        buf_len,
    size_t const
        pos);

void
snck_suggest_command(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    char const * const
        buf,
    size_t const
        buf_len,
    size_t const
        pos,
    int const
        pos1);

void
snck_suggest_file(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_suggest_list * const
        p_suggest_list,
    char const * const
        buf,
    size_t const
        buf_len,
    size_t const
        pos,
    int const
        pos1,
    struct snck_string const * const
        p_folder,
    char const
        b_cmd_is_cd);

/* end-of-file: snck_suggest.h */
