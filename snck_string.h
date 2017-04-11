/* See LICENSE for license details. */

/*

Module: snck_string.h

Description:

    Null terminated character array.

*/

/* Reverse include guard */
#if defined(INC_SNCK_STRING_H)
#error include snck_string.h once
#endif /* #if defined(INC_SNCK_STRING_H) */

#define INC_SNCK_STRING_H

/* Predefine context handle */
struct snck_ctxt;

/*

Structure: snck_string

Description:

    Null terminated character array.

*/
struct snck_string
{
    char * p_buf;

    size_t i_buf_len;

    size_t i_alloc_len;

}; /* struct snck_string */

void
snck_string_init(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string);

void
snck_string_init_ref(
    struct snck_string * const
        p_string,
    char const * const
        p_ref);

void
snck_string_init_ref_buffer(
    struct snck_string * const
        p_string,
    void const * const
        p_buf,
    size_t const
        i_buf_len);

void
snck_string_init_ref_object(
    struct snck_string * const
        p_string,
    struct snck_string const * const
        p_object);

void
snck_string_cleanup(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string);

char
snck_string_ref(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    char const * const
        p_ref);

char
snck_string_ref_buffer(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    void const * const
        p_buf,
    size_t const
        i_buf_len);

char
snck_string_ref_object(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    struct snck_string const * const
        p_object);

char
snck_string_resize(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    size_t const
        i_alloc_len);

char
snck_string_copy(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    char const * const
        p_ref);

char
snck_string_copy_buffer(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    void const * const
        p_buf,
    size_t const
        i_buf_len);

char
snck_string_copy_object(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    struct snck_string const * const
        p_object);

char
snck_string_append(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    char const * const
        p_ref);

char
snck_string_append_buffer(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    void const * const
        p_buf,
    size_t const
        i_buf_len);

char
snck_string_append_object(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string * const
        p_string,
    struct snck_string const * const
        p_object);

int
snck_string_compare(
    struct snck_string const * const
        p_ref1,
    struct snck_string const * const
        p_ref2);

char *
snck_string_get(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_string);

void
snck_string_put(
    struct snck_ctxt const * const
        p_ctxt,
    char * const
        p_buf);

void
snck_string_lower(
    struct snck_string * const
        p_string);

void
snck_string_upper(
    struct snck_string * const
        p_string);

/* end-of-file: snck_string.h */
