/*
 * shm_ringbuf.c
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
#include <stdint.h> /* uint32_t */
#include <string.h> /* memcpy() */
#ifdef SHM
unsigned int shm_write(struct buf_t *rb, char *buf, unsigned int n)
{
	unsigned int wrap = 0;
	int rs;
	uint32_t e, s, size;
	char *data;

	e = rb->end;
	s = rb->start;
	size = rb->bufsize;
	data = ((char *)rb) + sizeof(struct buf_t);
	if (e < s) {
		wrap = 1;
	}
	while ((((s + size*(1 - wrap)) - e) < (n + 1))) { /* checking free space */
		if /*((size*2)>RB_MAX_SIZE)*/ (1) {
			/*  s3dprintf(MED,"buffer reached maxsize, no resizing possible");*/
			return(0);
		}
		/*  printf("buffer full!! resizing ... (to size %d)",(int)size*2);
		  if (NULL==(realloc(rb, size*2+RB_OVERHEAD)))
		  {
		   printf("realloc failed - fatal!!");
		   return(-1);
		  }
		  if (wrap)
		  {
		   memcpy(data+size,data,e);
		   e+=size;
		   wrap=0;
		  }
		  size=rb->bufsize=size*2;
		  rb->end=e;*/
	}
	if ((e + n) > size) {
		rs = size - e;
		memcpy(data + e, buf, rs);   /* copy the first part ... */
		memcpy(data, buf + rs, n - rs);   /* .. end the rest */
	} else {
		memcpy(data + e, buf, n);   /* plain copy */
	}
	rb->end = e + n; /* update end of the buffer */
	if (rb->end >= rb->bufsize) rb->end -= rb->bufsize;
	return(n);
}
unsigned int shm_read(struct buf_t *rb, char *buf, unsigned int n)
{
	int wrap = 0;
	unsigned int mn;
	unsigned int rs;
	uint32_t e, s, size;
	char *data;

	e = rb->end;
	s = rb->start;
	size = rb->bufsize;
	data = ((char *)rb) + sizeof(struct buf_t);
	if (e < s) wrap = 1;
	rs = (e + wrap * size - s);
	mn = (n > rs) ? rs : n;
	if ((wrap) && (mn > (size - s))) {
		rs = size - s; /* size of the first part */
		memcpy(buf, data + s, rs);
		memcpy(buf + rs, data, mn - rs);
	} else { /* no wrap (needed)*/
		memcpy(buf, data + s, mn);
	}
	rb->start = s + mn;
	if (rb->start >= rb->bufsize) rb->start -= rb->bufsize;
	return(mn);
}
#endif
