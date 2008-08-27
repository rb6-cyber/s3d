/*
 * error.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3d, a 3d network display server.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "global.h"
#include <stdio.h>  /*  for printf() */
#include <stdarg.h>  /*  va_start, va_end */
#include <string.h>  /*  for strerror() */
#include <stdlib.h>  /*  for exit() */
#define  DBM_MAX  1024  /*  debug message buffer size */
/*  this function writes an error somewhere */
/*  basicly, this is for upcoming logfiles, or maybe draw error-messages into */
/*  the 3d-space */
/*  this is the generic failure routine ... */
void errn(const char *func, int en)
{
	fprintf(stderr, "error: %s: (%d) %s\n", func, en, strerror(en));
}
/*  ... and it's fatal pendant */
void errnf(const char *func, int en)
{
	fprintf(stderr, "FATAL: %s: (%d) %s\n", func, en, strerror(en));
	exit(-1);
}

/*  prints an error with the function and it's error-message */
void errs(const char *func, const char *msg)
{
	fprintf(stderr, "error: %s: %s\n", func, msg);
}

void errsf(const char *func, const char *msg)
{
	fprintf(stderr, "FATAL: %s: %s\n", func, msg);
	exit(-1);
}
#ifdef DEBUG
/*  printing error message */
void errds(int relevance, const char *func, const char *fmt, ...)
{
	char dbm[DBM_MAX];
	va_list args;
	if (relevance >= DEBUG) {
		va_start(args, fmt);
		vsnprintf((char *)&dbm, DBM_MAX, fmt, args);
		va_end(args);

		fprintf(stderr, "error: %s:%s\n", func, (char *)&dbm);
	}
}
/*  printing debug message */

void s3dprintf(int relevance, const char *fmt, ...)
{
	char dbm[DBM_MAX];
	va_list args;
	if (relevance >= DEBUG) {
		va_start(args, fmt);
		vsnprintf((char *)&dbm, DBM_MAX, fmt, args);
		va_end(args);

		/*  fprintf(stderr,"debug: %s\n",(char *)&dbm);*/
		fprintf(stdout, "debug: %s\n", (char *)&dbm);
	}
}

#endif
