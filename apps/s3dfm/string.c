/*
 * string.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3dfm, a s3d file manager.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3dfm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3dfm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3dfm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* just a few helper functions which only operate on strings, so we put them
 * here ... */

#include "s3dfm.h"
#include <string.h> /* strlen(), strncpy() */

/* writes the path of the item d in string str with bufferlength n,
 * adds some dots to the beginning if its too long */
char *dots_at_start(char *str, unsigned int n, t_node *d)
{
	char *s = str;
	int i, j;
	i = n - 2;
	s[n-1] = 0;
	do {
		j = strlen(d->name) - 1;
		if (NULL != (d->parent)) {
			s[i] = '/';
			i--;
		}
		while ((i >= 0) && (j >= 0)) {
			s[i] = d->name[j];
			j--;
			i--;
		}
		if (i < 0)
			break;
	} while ((d = d->parent) != NULL);
	if (i < 0)   s[0] = s[1] = '.';
	else     s = (char *)s + i + 1; /* jump to start of the string */
	return(s);

}
/* add some dots to an integer value for better readability */
void dotted_int(char *s, unsigned int i)
{
	char st[M_DIR];
	unsigned int p;
	p = 0;
	st[0] = 0;
	while (i > 0) {
		if ((p + 1) % 4 == 0) {
			st[p] = '.';
			p++;
		}
		st[p] = (i % 10) + '0';
		i = i / 10;
		p++;
	}
	if (p > 0) p--;
	st[p+1] = 0;
	for (i = 0;i < p + 1;i++)
		s[i] = st[p-i];
	s[p+1] = 0;
}
/* save concatting 2 strings, this version takes argument n
 * as the size of the buffer of dest. */
char *mstrncat(char *dest, const char *src, int n)
{
	int i, j;
	dest[n-1] = 0;    /* for malformed destinations */
	j = 0;
	for (i = strlen(dest);i < (n - 1);i++) {
		dest[i] = src[j];
		if (dest[i] == 0) break;
		j++;
	}
	for (;i < n;i++)
		dest[i] = 0; /* pad the rest with zero */
	return(dest);
}
/* same as strncpy, but have a terminating zero even if
 * source is too big */
char *mstrncpy(char *dest, const char *src, int n)
{
	strncpy(dest, src, n);
	dest[n-1] = 0;
	return(dest);
}

