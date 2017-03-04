/* See LICENSE for license details. */

/*

Module: snck_os.c

Description:

*/

/* OS headers */
#include "snck_os.h"

/* Configuration */
#include "snck_cfg.h"

/* Module */
#include "snck_main.h"

/*

Function: main()

Description:

*/
int
main(
    int argc,
    char ** argv)
{
    return snck_main(
        (unsigned int)(argc),
        (char * *)(argv));

} /* main() */

/* end-of-file: snck_os.c */
