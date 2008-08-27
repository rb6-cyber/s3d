/*
 * texture.c
 *
 * Copyright (C) 2007-2008 Simon Wunderlich <dotslash@packetmixer.de>
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
#ifdef SHM
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#include <stdlib.h>  /* malloc(), free() */
#include <errno.h>  /* errno */
#include <string.h>  /* memcpy() */

static int _s3d_compare_cb(const void *d1, const void *d2);
static int _s3d_choose_cb(const void *d1, int size);
static void _s3d_free_s3dtex(void *d1);
static struct hashtable_t *tex_hash = NULL;

static int _s3d_choose_cb(const void *d1, int size)
{
	struct s3d_texshm *t1 = (struct s3d_texshm *)d1;
	return((t1->oid*32 + t1->tex) % size);
}

static int _s3d_compare_cb(const void *d1, const void *d2)
{
	struct s3d_texshm *t1, *t2;
	t1 = (struct s3d_texshm *)d1;
	t2 = (struct s3d_texshm *)d2;
	if ((t1->oid == t2->oid) && (t1->tex == t2->tex))
		return(0);
	return(1);
}

static void _s3d_free_s3dtex(void *d1)
{
	struct s3d_tex *tex = (struct s3d_tex *)d1;
#ifdef SHM
	if (tex->buf != NULL) {
		shmdt(tex->buf);
		tex->buf = NULL;
	}
	shmctl(tex->tshm.shmid, IPC_RMID, NULL);
	free(tex);
#endif
	return;
}

void _s3d_handle_texshm(struct s3d_texshm *tshm)
{
	struct s3d_tex *tex = NULL;
	s3dprintf(HIGH, "handling new texture ...");
	if (tex_hash == NULL)
		return;
#ifdef SHM
	tex = (struct s3d_tex *)_s3d_hash_remove(tex_hash, tshm);
	if (tex != NULL)
		_s3d_free_s3dtex(tex);
	if (tshm->shmid == -1)
		return;

	tex = (struct s3d_tex *)malloc(sizeof(*tex));
	tex->tshm = *tshm;
	tex->buf = (char*)shmat(tex->tshm.shmid, (void *)0, 0);
	if ((key_t *)tex->buf == (key_t *)(-1)) {
		errn("shm_init():shmat()", errno);
		free(tex);
		return;
	}
	s3dprintf(HIGH, "adding new texture ...");
	_s3d_hash_add(tex_hash, tex);
#endif
	return;
}
int _s3d_load_texture_shm(int object, uint32_t tid, uint16_t xpos, uint16_t ypos, uint16_t w, uint16_t h, const uint8_t *data)
{
	struct s3d_texshm check;
	struct s3d_tex *tex;
	int32_t i, p1, p2, m;
	int16_t mw, mh;

	if (tex_hash == NULL)
		return(-1);
	check.oid = object;
	check.tex = tid;
	tex = (struct s3d_tex *)_s3d_hash_find(tex_hash, (void *) & check);
	if (tex == NULL)
		return(-1); /* coudn't find */
	s3dprintf(VLOW, "texture: oid %d, tex %d, shmid %d, tw %d, th %d, w %d, h %d ...",
	          tex->tshm.oid, tex->tshm.tex, tex->tshm.shmid, tex->tshm.tw, tex->tshm.th, tex->tshm.w, tex->tshm.th);
	/* found it, assume that it's properly attached. */
	m = tex->tshm.w * tex->tshm.th + tex->tshm.tw;     /*  maximum: position of the last pixel in the buffer */
	if ((xpos + w) > tex->tshm.tw) mw = (tex->tshm.tw - xpos);
	else       mw = w;
	if ((ypos + h) > tex->tshm.th) mh = (tex->tshm.th - ypos);
	else       mh = h;

	if (mw <= 0) { /*  nothing to do */
		return(0);
	}
	for (i = 0;i < mh;i++) {
		p1 = (ypos + i) * tex->tshm.w + xpos;  /*  scanline start position */
		p2 = mw;  /*  and length */
		if (p1 > m) {
			break;   /*  need to break here. */
		}
		memcpy(tex->buf + 4*p1,     /*  draw at p1 position ... */
		       data + 4*i*w,   /*  scanline number i ... */
		       4*p2);
	}
	return(0);
}
int _s3d_texture_init(void)
{
	tex_hash = _s3d_hash_new(256, _s3d_compare_cb, _s3d_choose_cb);
	if (tex_hash == NULL)
		return(-1);
	else
		return(0);
}
int _s3d_texture_quit(void)
{
	if (tex_hash == NULL)
		return(-1);
	_s3d_hash_delete(tex_hash, _s3d_free_s3dtex);
	tex_hash = NULL;
	return(0);
}
