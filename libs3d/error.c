/*
 * error.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of the s3d API, the API of s3d (the 3d network display server).
 * See http://s3d.berlios.de/ for more updates.
 *
 * The s3d API is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * The s3d API is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d API; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "s3d.h"
#include "s3dlib.h"
#include <stdarg.h>   /*  va_list */
#include <stdio.h>    /*  perror(),fprintf() */
#include <string.h>   /*  sterror */
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


