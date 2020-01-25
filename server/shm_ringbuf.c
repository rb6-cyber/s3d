// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */

#include "global.h"
#include <stdio.h>		/* printf(), getchar() */
#include <stdint.h>		/* uint32_t */
#include <string.h>		/* memcpy() */

int shm_write(struct buf_t *rb, char *buf, int n)
{
	int wrap = 0;
	int rs;
	int32_t e, s, size;
	char *data;

	e = rb->end;
	s = rb->start;
	size = rb->bufsize;
	data = ((char *)rb) + sizeof(struct buf_t);
	if (e < s) {
		wrap = 1;
	}
	if ((((s + size * (1 - wrap)) - e) < (n + 1))) {	/* checking free space */
		printf("buffer reached maxsize, no resizing possible");
	}
	if ((e + n) > size) {
		rs = size - e;
		memcpy(data + e, buf, rs);	/* copy the first part ... */
		memcpy(data, buf + rs, n - rs);	/* .. end the rest */
	} else {
		memcpy(data + e, buf, n);	/* plain copy */
	}
	rb->end = e + n;	/* update end of the buffer */
	if (rb->end >= rb->bufsize)
		rb->end -= rb->bufsize;
	return 0;
}

int shm_read(struct buf_t *rb, char *buf, int n)
{
	int wrap = 0;
	int mn;
	int rs;
	int32_t e, s, size;
	char *data;

	e = rb->end;
	s = rb->start;
	size = rb->bufsize;
	data = ((char *)rb) + sizeof(struct buf_t);
	if (e < s)
		wrap = 1;
	rs = (e + wrap * size - s);
	mn = (n > rs) ? rs : n;
	if ((wrap) && (mn > (size - s))) {
		rs = size - s;	/* size of the first part */
		memcpy(buf, data + s, rs);
		memcpy(buf + rs, data, mn - rs);
	} else {		/* no wrap (needed) */
		memcpy(buf, data + s, mn);
	}
	rb->start = s + mn;
	if (rb->start >= rb->bufsize)
		rb->start -= rb->bufsize;
	return mn;
}

void ringbuf_init(char *data, uint32_t init_size)
{
	struct buf_t *ringbuf = (struct buf_t *)data;
	ringbuf->start = 0;
	ringbuf->end = 0;
	ringbuf->bufsize = init_size - RB_OVERHEAD;
}
