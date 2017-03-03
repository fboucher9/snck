/* See LICENSE for license details. */

/*

Module: snck_os.h

Description:

*/

/* Reverse include guard */
#if defined(INC_SNCK_OS_H)
#error include snck_os.h once
#endif /* #if defined(INC_SNCK_OS_H) */

#define INC_SNCK_OS_H

/* */
#include <stdio.h>

/* */
#include <stdlib.h>

/* */
#include <string.h>

/* */
#include <unistd.h>

/* errno */
#include <errno.h>

/* waitpid() */
#include <sys/wait.h>

/* struct passwd */
#include <sys/types.h>

/* getpwuid() */
#include <pwd.h>

/* opendir() */
#include <dirent.h>

/* end-of-file: snck_os.h */
