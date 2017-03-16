/* See LICENSE for license details. */

/*

Module: snck_list.h

Description:

    Generic linked list to be used for all lists in snck project.

Comments:

    The list is a circular doubly linked list.  A fake element is used
    to point to first and last elements.  The same structure is used for
    each element including the fake element.  However, the fake element
    does not have the same derived type as the other elements.

    The caller is reponsible of casting the element to the derived type.
    Before casting, it is important to make sure that the element is
    not the fake element.  If the list is not the first member of the
    derived type, then it may be useful to use the offsetof() macro to
    get a pointer to the start of the derived type.

    The circular feature ensures that each element is part of a list.
    Operations may be done on a single element or on groups of elements.
    The same method is used to insert elements or to remove elements.
    See the examples section for more details.

Examples:

    -   create an empty list

            snck_list_init(&o_list);

    -   insert a first list before a second

            snck_list_join(&o_first, &o_second);

    -   insert a first list after a second

            snck_list_join(&o_second, &o_first);

    -   remove a single element from a list

            snck_list_join(&o_element, &o_element);

    -   remove a group of elements from a list

            snck_list_join(&o_last, &o_first);

Notes:

    To insert all elements of a list into another, you first need to
    detach the elements from the fake or else the final list may end up
    with two fake elements.

        p_first = o_fake.p_next;
        snck_list_join(&o_fake, &o_fake);
        snck_list_join(p_first, &o_other);

*/

/* Header file dependencies */
#if !defined(INC_SNCK_OS_H)
#error include snck_os.h first
#endif /* #if !defined(INC_SNCK_OS_H) */

/* Reverse include guard */
#if defined(INC_SNCK_LIST_H)
#error include snck_list.h once
#endif /* #if defined(INC_SNCK_LIST_H) */

#define INC_SNCK_LIST_H

/*

Structure: snck_list

Description:

*/
struct snck_list
{
    struct snck_list *
        p_next;

    struct snck_list *
        p_prev;

}; /* struct snck_list */

/* Public functions ... */

#if defined(__cplusplus)
extern "C"
#endif /* #if defined(__cplusplus) */
void
snck_list_init(
    struct snck_list * const
        p_node);

#if defined(__cplusplus)
extern "C"
#endif /* #if defined(__cplusplus) */
void
snck_list_join(
    struct snck_list * const
        p_before,
    struct snck_list * const
        p_after);

#if defined(__cplusplus)
extern "C"
#endif /* #if defined(__cplusplus) */
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
        p_context);

/* end-of-file: snck_list.h */
