// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2001  Lyle Hanson <lhanson@cs.nmu.edu>
 */

#include "http_error_codes.h"

/* Note that '%d' cannot be escaped at this time */
const char *http_errlist[] = {
	"Success",          /* HF_SUCCESS  */
	"Internal Error. What the hell?!",    /* HF_METAERROR  */
	"Got NULL url",         /* HF_NULLURL  */
	"Timed out, no metadata for %d seconds",  /* HF_HEADTIMEOUT  */
	"Timed out, no data for %d seconds",   /* HF_DATATIMEOUT */
	"Couldn't find return code in HTTP response", /* HF_FRETURNCODE */
	"Couldn't convert return code in HTTP response",/* HF_CRETURNCODE */
	"Request returned a status code of %d",   /* HF_STATUSCODE */
	"Couldn't convert Content-Length to integer" /* HF_CONTENTLEN */
};

/* Used to copy in messages from http_errlist[] and replace %d's with
 * the value of errorInt.  Then we can pass the pointer to THIS */
char convertedError[128];
