/* See LICENSE for license details. */

/*

Module: snck_token.c

Description:

    Tokenizer for shell command lines

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Context */
#include "snck_ctxt.h"

/* List */
#include "snck_list.h"

/* String */
#include "snck_string.h"

/* Module */
#include "snck_token.h"

/* Heap */
#include "snck_heap.h"

struct snck_token_iterator
{
    struct snck_string const *
        p_line;

    size_t
        i_offset;

}; /* struct snck_token_iterator */

#if 0

/*

Function: snck_token_phrase_init

Description:

*/
static
char
snck_token_phrase_init(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_token_phrase * const
        p_phrase,
    char const * const
        p_line)
{
    char b_result;

    size_t i_line_it;

    /* Split the phrase into words */
    i_line_it = 0;

    while (p_line[i_line_it])
    {
        /* Skip whitespace */
        while (
            (' ' == p_line[i_line_it])
            || ('\t' == p_line[i_line_it])
            || ('\r' == p_line[i_line_it])
            || ('\n' == p_line[i_line_it]))
        {
            i_line_it ++;
        }

        /* Find word length */
        {
            size_t i_word_len;
        }

        /* Insert word into linked list */
    }

    return b_result;

} /* snck_token_phrase_init() */

static
void
snck_token_phrase_cleanup(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_token_phrase * const
        p_phrase)
{
    /* Free list of words */
} /* snck_token_phrase_cleanup() */

/*

Function: snck_token_phrase_create

Description:

*/
struct snck_token_phrase *
snck_token_phrase_create(
    struct snck_ctxt const * const
        p_ctxt,
    char const * const
        p_line)
{
    struct snck_token_phrase *
        p_phrase;

    p_phrase = (struct snck_token_phrase *)(snck_heap_realloc(p_ctxt, NULL, sizeof(struct snck_token_phrase)));

    if (p_phrase)
    {
        if (snck_token_phrase_init(p_ctxt, p_phrase, p_line))
        {
        }
        else
        {
            snck_heap_realloc(p_ctxt, p_phrase, 0u);
        }
    }

    return p_phrase;

} /* snck_token_phrase_create() */

/*

Function: snck_token_phrase_destroy

Description:

    Undo all actions of snck_token_phrase_create().

*/
void
snck_token_phrase_destroy(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_token_phrase * const
        p_phrase)
{
    snck_token_phrase_cleanup(p_ctxt, p_phrase);

    snck_heap_realloc(p_ctxt, p_phrase, 0u);

} /* snck_token_phrase_destroy() */

#endif

static
char
snck_token_find_matching(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_token_iterator * const
        p_token_iterator,
    char const
        c_match)
{
    char b_result;

    char b_found;

    struct snck_string const * p_line = p_token_iterator->p_line;

    b_found = 0;

    b_result = 1;

    while (b_result && !b_found)
    {
        char c;

        if (p_token_iterator->i_offset < p_line->i_buf_len)
        {
            c = p_line->p_buf[p_token_iterator->i_offset];
        }
        else
        {
            c = '\000';
        }

        if (c_match == c)
        {
            b_found = 1;
        }
        else if ('\t' == c)
        {
            if (' ' == c_match)
            {
                b_found = 1;
            }
            else
            {
                p_token_iterator->i_offset ++;
            }
        }
        else if ('\000' == c)
        {
            if (' ' == c_match)
            {
                b_found = 1;
            }
            else
            {
                b_result = 0;
            }
        }
        else if ('\\' == c)
        {
            p_token_iterator->i_offset ++;

            if (p_token_iterator->i_offset < p_line->i_buf_len)
            {
                p_token_iterator->i_offset ++;
            }
            else
            {
                b_result = 0;
            }
        }
        else if ('\'' == c)
        {
            p_token_iterator->i_offset ++;

            b_result = 0;

            while (!b_result && (p_token_iterator->i_offset < p_line->i_buf_len))
            {
                if ('\'' == p_line->p_buf[p_token_iterator->i_offset])
                {
                    b_result = 1;

                    p_token_iterator->i_offset ++;
                }
                else
                {
                    p_token_iterator->i_offset ++;
                }
            }
        }
        else if ('$' == c)
        {
            p_token_iterator->i_offset ++;

            if (p_token_iterator->i_offset < p_line->i_buf_len)
            {
                if ('{' == p_line->p_buf[p_token_iterator->i_offset])
                {
                    /* Search for closing */
                    p_token_iterator->i_offset ++;

                    b_result = snck_token_find_matching(p_ctxt, p_token_iterator, '}');

                    if (b_result)
                    {
                        p_token_iterator->i_offset ++;
                    }
                }
                else if ('(' == p_line->p_buf[p_token_iterator->i_offset])
                {
                    /* Search for closing */
                    p_token_iterator->i_offset ++;

                    b_result = snck_token_find_matching(p_ctxt, p_token_iterator, ')');

                    if (b_result)
                    {
                        p_token_iterator->i_offset ++;
                    }
                }
                else
                {
                    p_token_iterator->i_offset ++;
                }
            }
        }
        else if ('(' == c)
        {
            /* Search for closing */
            p_token_iterator->i_offset ++;

            b_result = snck_token_find_matching(p_ctxt, p_token_iterator, ')');

            if (b_result)
            {
                p_token_iterator->i_offset ++;
            }
        }
        else if ('`' == c)
        {
            /* Search for closing */
            p_token_iterator->i_offset ++;

            b_result = snck_token_find_matching(p_ctxt, p_token_iterator, '`');

            if (b_result)
            {
                p_token_iterator->i_offset ++;
            }
        }
        else
        {
            p_token_iterator->i_offset ++;
        }
    }

    if (!b_found)
    {
        b_result = 0;
    }

    return b_result;

} /* snck_token_find_matching() */

char
snck_token_skip_whitespace(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_token_iterator * const
        p_token_iterator)
{
    char b_result;

    char b_found;

    (void)(p_ctxt);

    b_result = 1;

    b_found = 0;

    while (
        b_result
        && (
            !b_found)
        && (
            p_token_iterator->i_offset < p_token_iterator->p_line->i_buf_len))
    {
        char c;

        c = p_token_iterator->p_line->p_buf[p_token_iterator->i_offset];

        if ((' ' == c)
            || ('\t' == c)
            || ('\r' == c)
            || ('\n' == c))
        {
            p_token_iterator->i_offset ++;
        }
        else
        {
            b_found = 1;
        }
    }

    if (!b_found)
    {
        b_result = 0;
    }

    return b_result;
}

char
snck_token_find_next_word(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_line,
    struct snck_string * const
        p_word,
    struct snck_string * const
        p_args)
{
    char b_result;

    struct snck_token_iterator o_token_iterator;

    o_token_iterator.p_line = p_line;

    o_token_iterator.i_offset = 0u;

    /* Advance iterator until beginning of next word */
    if (snck_token_skip_whitespace(p_ctxt, &(o_token_iterator)))
    {
        size_t const i_word_begin = o_token_iterator.i_offset;

        /* Advance iterator to end of word */
        if (snck_token_find_matching(p_ctxt, &(o_token_iterator), ' '))
        {
            size_t const i_word_end = o_token_iterator.i_offset;

            size_t const i_word_len = i_word_end - i_word_begin;

            /* Return found word */
            snck_string_ref_buffer(
                p_ctxt,
                p_word,
                p_line->p_buf + i_word_begin,
                i_word_len);

            if (snck_token_skip_whitespace(p_ctxt, &(o_token_iterator)))
            {
                snck_string_ref(
                    p_ctxt,
                    p_args,
                    p_line->p_buf + o_token_iterator.i_offset);
            }
            else
            {
                snck_string_ref(
                    p_ctxt,
                    p_args,
                    "");
            }

            b_result = 1;
        }
        else
        {
            b_result = 0;
        }
    }
    else
    {
        b_result = 0;
    }

    return b_result;

} /* snck_token_find_next_word() */

/*

Function: snck_token_is_complete

Description:

    Detect if current contains a complete command, else requires another
    line from input file.

*/
char
snck_token_is_complete(
    struct snck_ctxt const * const
        p_ctxt,
    struct snck_string const * const
        p_line)
{
    struct snck_token_iterator o_token_iterator;

    o_token_iterator.p_line = p_line;

    o_token_iterator.i_offset = 0u;

    return snck_token_find_matching(p_ctxt, &(o_token_iterator), '\000');

} /* snck_token_is_complete() */

/* end-of-file: snck_token.c */
