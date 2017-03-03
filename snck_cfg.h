/* See LICENSE for license details. */

/*

Module: snck_cfg.h

Description:

    Configuration options.

*/

/* Reverse include guard */
#if defined(INC_SNCK_CFG_H)
#error include snck_cfg.h
#endif /* #if defined(INC_SNCK_CFG_H) */

#define INC_SNCK_CFG_H

/* Use of linenoise library for input */
#if defined(SNCK_HAVE_LINENOISE)
#define SNCK_FEATURE_LINENOISE
#endif /* #if defined(SNCK_HAVE_LINENOISE) */

/* end-of-file: snck_cfg.h */
