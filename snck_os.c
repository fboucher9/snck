/* See LICENSE for license details. */

/*

Module: snck_os.c

Description:

    OS-specific main function.

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Module */
#include "snck_main.h"

/* String */
#include "snck_string.h"

static
struct snck_string *
snck_os_arg_list_create(
    int const
        argc,
    char * * const
        argv)
{
    struct snck_string * p_arg_list;

    p_arg_list = (struct snck_string *)(malloc(sizeof(struct snck_string) * argc));

    if (p_arg_list)
    {
        int argi;

        char b_result;

        b_result = 1;

        argi = 0;

        while (b_result && (argi < argc))
        {
            int const i_buf_len = strlen(argv[argi]);

            char * const p_buf = (char *)(malloc(i_buf_len + 4));

            if (p_buf)
            {
                memcpy(p_buf, argv[argi], i_buf_len);

                memset(p_buf + i_buf_len, '\000', 4);

                p_arg_list[argi].p_buf = p_buf;

                p_arg_list[argi].i_buf_len = i_buf_len;

                p_arg_list[argi].i_alloc_len = 0u;
            }
            else
            {
                int i;

                i = 0;

                while (i < argi)
                {
                    if (p_arg_list[i].p_buf)
                    {
                        free(p_arg_list[i].p_buf);

                        p_arg_list[i].p_buf = NULL;
                    }

                    p_arg_list[i].i_buf_len = 0u;

                    p_arg_list[i].i_alloc_len = 0u;

                    i ++;
                }

                b_result = 0;
            }

            argi ++;
        }
    }

    return p_arg_list;

} /* snck_os_arg_list_create() */

static
void
snck_os_arg_list_destroy(
    struct snck_string * const
        p_arg_list,
    size_t const
        i_arg_count)
{
    size_t i_arg_iterator;

    for (i_arg_iterator = 0; i_arg_iterator < i_arg_count; i_arg_iterator++)
    {
        if (p_arg_list[i_arg_iterator].p_buf)
        {
            free(p_arg_list[i_arg_iterator].p_buf);

            p_arg_list[i_arg_iterator].p_buf = NULL;
        }

        p_arg_list[i_arg_iterator].i_buf_len = 0u;
    }

    free(p_arg_list);
} /* snck_os_arg_list_destroy() */

/*

Function: main()

Description:

    OS-specific main function.  Convert parameters to OS-indendant values and
    dispatch to snck_main module.

*/
int
main(
    int argc,
    char ** argv)
{
    int i_exit_code;

    struct snck_string * p_arg_list;

    p_arg_list = snck_os_arg_list_create(argc, argv);

    if (p_arg_list)
    {
        i_exit_code = snck_main(p_arg_list, (size_t)(argc));

        snck_os_arg_list_destroy(p_arg_list, (size_t)(argc));
    }
    else
    {
        i_exit_code = 1;
    }

    return i_exit_code;

} /* main() */

/* end-of-file: snck_os.c */
