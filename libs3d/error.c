// SPDX-License-Identifier: LGPL-2.1-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


#include <stdarg.h>                     /* for va_start, va_end, va_list */
#include <stdio.h>                      /* for fprintf, stderr */
#include <string.h>                     /* for strerror */

/*  s3dprintf is only for internal use. */
#ifdef DEBUG
void s3dprintf(int relevance, const char *fmt, ...)
{
	char dbm[DBM_MAX];
	va_list args;
	if (relevance >= DEBUG) {
		va_start(args, fmt);
		vsnprintf((char *)&dbm, DBM_MAX, fmt, args);
		va_end(args);

		fprintf(stderr, "s3dlib: %s\n", (char *)&dbm);
	}
}
void errdn(int relevance, const char *func, int en)
{
	if (relevance >= DEBUG)
		fprintf(stderr, "s3dlib error: %s: (%d) %s\n", func, en, strerror(en));
}

void errds(int relevance, const char *func, const char *fmt, ...)
{
	char dbm[DBM_MAX];
	va_list args;
	if (relevance >= DEBUG) {
		va_start(args, fmt);
		vsnprintf((char *)&dbm, DBM_MAX, fmt, args);
		va_end(args);

		fprintf(stderr, "s3dlib error: %s:%s\n", func, (char *)&dbm);
	}
}
#endif
void errn(const char *func, int en)
{
	fprintf(stderr, "s3dlib error: %s: (%d) %s\n", func, en, strerror(en));
}
void errs(const char *func, const char *msg)
{
	fprintf(stderr, "s3dlib error: %s: %s\n", func, msg);
}


