/* See LICENSE for license details. */

/*

Module: snck_list.c

Description:

    Generic linked list to be used for all lists in snck project.

Comments:

    See header file for more details.

*/

/* OS Headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Module */
#include "snck_list.h"

/*

Function: snck_list_init()

Description:

    Initialize an element so that it's a list with a single element, itself.

Parameters:

    p_node
        Pointer to element to initialize

Returns: None.

Comments: None.

*/
void
snck_list_init(
    struct snck_list * const
        p_node)
{
    p_node->p_next =
        p_node;

    p_node->p_prev =
        p_node;

} /* snck_list_init() */

/*

Function: snck_list_join()

Description:

    Join two elements together.

Parameters:

    p_before
        Pointer to first element to join.  This element will end up before
        the p_after element.

    p_after
        Pointer to second element to join.  This element will end up after
        the p_before element.

Comments:

    If the elements are in different lists, the two lists will become one.

    If the elements are in the same list, then there will be two lists.

*/
void
snck_list_join(
    struct snck_list * const
        p_before,
    struct snck_list * const
        p_after)
{
    /* AB(C) - (D)EF > ABCDEF */
    p_before->p_next->p_prev =
        p_after->p_prev;

    p_after->p_prev->p_next =
        p_before->p_next;

    p_before->p_next =
        p_after;

    p_after->p_prev =
        p_before;

} /* snck_list_join() */

/*

Function: snck_list_iterate()

Description:

    Iterate all elements of list.  Use the callback function on each element
    of the list.  The list must have a fake element, the callback function
    will not be called for the fake element.

Parameters:

    p_node
        Pointer to fake element of list

    p_callback
        Pointer to callback function

    p_context
        Pointer to context that is passed to callback function

Returns: None.

Comments:

    Do not use this function to modify the list.  Do not call snck_list_join()
    from the callback function and do not modify the members of the element.

*/
void
snck_list_iterate(
    struct snck_list * const
        p_node,
    void (* const p_callback)(
        void * const
            p_context,
        struct snck_list * const
            p_list),
    void * const
        p_context)
{
    struct snck_list *
        p_iterator;

    p_iterator =
        p_node->p_next;

    while (
        p_iterator
        != p_node)
    {
        struct snck_list *
            p_next;

        p_next =
            p_iterator->p_next;

        (*p_callback)(
            p_context,
            p_iterator);

        p_iterator =
            p_next;
    }

} /* snck_list_iterate() */

/* end-of-file: snck_list.c */
