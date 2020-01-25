/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 * SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


/* use the global config.h */
#include <config-s3d.h>

/*  this is definitly the better way to pick fonts. */

#define WITH_FONTCONFIG
/*  our level of debug messages */
/* #define DEBUG   LOW */  /*  standard debug level, should be set with compiler, e.g. -DDEBUG=LOW */
#define SHM_MAX_IDLE 200 /* maximum wait for server timeout */
#define SEI_SS   200 /* seidel algorithm maximum point number */
