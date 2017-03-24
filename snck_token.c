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
    struct snck_string const * const
        p_line,
    size_t * const
        p_pos,
    char const
        c_match)
{
    char b_result;

    char b_found;

    b_found = 0;

    b_result = 1;

    while (b_result && !b_found && ((*p_pos) < p_line->i_buf_len))
    {
        if (c_match == p_line->p_buf[(*p_pos)])
        {
            b_found = 1;

            (*p_pos) ++;
        }
        else if ('\\' == p_line->p_buf[(*p_pos)])
        {
            (*p_pos) ++;

            if ((*p_pos) < p_line->i_buf_len)
            {
                (*p_pos) ++;
            }
        }
        else if ('\'' == p_line->p_buf[(*p_pos)])
        {
            (*p_pos) ++;

            b_result = 0;

            while (!b_result && ((*p_pos) < p_line->i_buf_len))
            {
                if ('\'' == p_line->p_buf[(*p_pos)])
                {
                    b_result = 1;

                    (*p_pos) ++;
                }
                else
                {
                    (*p_pos) ++;
                }
            }
        }
        else if ('$' == p_line->p_buf[(*p_pos)])
        {
            (*p_pos) ++;

            if ((*p_pos) < p_line->i_buf_len)
            {
                if ('{' == p_line->p_buf[(*p_pos)])
                {
                    /* Search for closing */
                    (*p_pos) ++;

                    b_result = snck_token_find_matching(p_ctxt, p_line, p_pos, '}');
                }
                else if ('(' == p_line->p_buf[(*p_pos)])
                {
                    /* Search for closing */
                    (*p_pos) ++;

                    b_result = snck_token_find_matching(p_ctxt, p_line, p_pos, ')');
                }
                else
                {
                    (*p_pos) ++;
                }
            }
        }
        else if ('(' == p_line->p_buf[(*p_pos)])
        {
            /* Search for closing */
            (*p_pos) ++;

            b_result = snck_token_find_matching(p_ctxt, p_line, p_pos, ')');
        }
        else if ('`' == p_line->p_buf[(*p_pos)])
        {
            /* Search for closing */
            (*p_pos) ++;

            b_result = snck_token_find_matching(p_ctxt, p_line, p_pos, '`');
        }
        else
        {
            (*p_pos) ++;
        }
    }

    if (!b_found)
    {
        b_result = 0;
    }

    return b_result;

} /* snck_token_find_matching() */

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
    char b_result;

    /* Search for end-of-line */
    size_t i_pos;

    (void)(p_ctxt);

    i_pos = 0u;

    b_result = 1;

    while (b_result && (i_pos < p_line->i_buf_len))
    {
        if ('\\' == p_line->p_buf[i_pos])
        {
            i_pos ++;
            if (i_pos < p_line->i_buf_len)
            {
                i_pos ++;
            }
            else
            {
                b_result = 0;
            }
        }
        else if ('\'' == p_line->p_buf[i_pos])
        {
            i_pos ++;

            b_result = 0;

            while (!b_result && (i_pos < p_line->i_buf_len))
            {
                if ('\'' == p_line->p_buf[i_pos])
                {
                    b_result = 1;

                    i_pos ++;
                }
                else
                {
                    i_pos ++;
                }
            }
        }
        else if ('\"' == p_line->p_buf[i_pos])
        {
            i_pos ++;

            b_result = snck_token_find_matching(p_ctxt, p_line, &i_pos, '\"');
        }
        else if ('$' == p_line->p_buf[i_pos])
        {
            i_pos ++;
            if (i_pos < p_line->i_buf_len)
            {
                if ('{' == p_line->p_buf[i_pos])
                {
                    /* Search for closing */
                    i_pos ++;

                    b_result = snck_token_find_matching(p_ctxt, p_line, &i_pos, '}');
                }
                else if ('(' == p_line->p_buf[i_pos])
                {
                    /* Search for closing */
                    i_pos ++;

                    b_result = snck_token_find_matching(p_ctxt, p_line, &i_pos, ')');
                }
                else
                {
                    i_pos ++;
                }
            }
        }
        else if ('(' == p_line->p_buf[i_pos])
        {
            /* Search for closing */
            i_pos ++;

            b_result = snck_token_find_matching(p_ctxt, p_line, &i_pos, ')');
        }
        else if ('`' == p_line->p_buf[i_pos])
        {
            /* Search for closing */
            i_pos ++;

            b_result = snck_token_find_matching(p_ctxt, p_line, &i_pos, '`');
        }
        else
        {
            i_pos ++;
        }
    }

    return b_result;

} /* snck_token_is_complete() */

/* end-of-file: snck_token.c */
