/* http_error_codes.c - Error code declarations
 *
 * HTTP Fetcher
 *  Copyright (C) 2001 Lyle Hanson (lhanson@cs.nmu.edu)
 *
 * This file is part of s3dosm, a gps card application for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3dosm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3dosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3dosm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
