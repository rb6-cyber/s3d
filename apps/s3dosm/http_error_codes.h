/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2001  Lyle Hanson <lhanson@cs.nmu.edu>
 */

#ifndef HTTP_ERROR_CODES_H
#define HTTP_ERROR_CODES_H

/* Error sources */
#define FETCHER_ERROR 0
#define ERRNO   1
#define H_ERRNO   2

/* HTTP Fetcher error codes */
#define HF_SUCCESS  0
#define HF_METAERROR 1
#define HF_NULLURL  2
#define HF_HEADTIMEOUT 3
#define HF_DATATIMEOUT 4
#define HF_FRETURNCODE 5
#define HF_CRETURNCODE 6
#define HF_STATUSCODE 7
#define HF_CONTENTLEN 8

extern const char *http_errlist[];
extern char convertedError[128];

#endif
