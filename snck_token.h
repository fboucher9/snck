/* See LICENSE for license details. */

/*

Module: snck_token.h

Description:

    Tokenizer for shell command lines

*/

/* Reverse include guard */
#if defined(INC_SNCK_TOKEN_H)
#error include snck_token.h once
#endif /* #if defined(INC_SNCK_TOKEN_H) */

#define INC_SNCK_TOKEN_H

/* Header file dependencies */
#if !defined(INC_SNCK_LIST_H)
#error include snck_list.h before snck_token.h
#endif /* #if !defined(INC_SNCK_LIST_H) */

#if !defined(INC_SNCK_STRING_H)
#error include snck_string.h before snck_token.h
#endif /* #if !defined(INC_SNCK_STRING_H) */

/* Predefine context handle */
struct snck_ctxt;

#if 0

struct snck_token_word
{
    struct snck_list o_list;

    struct snck_string o_buf;

    int i_pos;

}; /* struct snck_token_word */

struct snck_token_phrase
{
    struct snck_list o_list;

    int i_count;

}; /* struct snck_token_phrase */

struct snck_token_phrase *
snck_token_phrase_create(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_line);

void
snck_token_phrase_destroy(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_token_phrase * const
        p_token_phrase);

#endif

char
snck_token_find_next_word(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_line,
    struct snck_string * const
        p_word,
    struct snck_string * const
        p_args);

char
snck_token_is_complete(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_line);

/* end-of-file: snck_token.h */
