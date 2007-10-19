/* http_error_codes.h - Error code definitions
 *
 * Copyright (C) 2001 Lyle Hanson (lhanson@cs.nmu.edu)
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
