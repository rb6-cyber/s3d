/*
 * tag.c
 *
 * Copyright (C) 2006-2008 Simon Wunderlich <dotslash@packetmixer.de>
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
#include "s3dosm.h"
#include <stdio.h> /* printf() */
#include <string.h> /* strcmp() */
#include <stdlib.h> /* realloc() */
void tag_add(object_t *obj, const char *k, char *v)
{
	tag_t *t;
	obj->tag_n++;
	obj->tag_p = (tag_t*)realloc(obj->tag_p, obj->tag_n * sizeof(tag_t));
	if (k != NULL && v != NULL) {
		t = &(obj->tag_p[obj->tag_n-1]);
		t->ttype = TAG_UNKNOWN;
		t->k = strdup(k);
		t->v = strdup(v);
		t->d.s = v;
		if (0 == strcmp(k, "name")) t->ttype = TAG_NAME;
	}
}
tag_t *tag_get(object_t *obj, const char *k)
{
	int i;
	for (i = 0;i < obj->tag_n;i++) {
		if (0 == strcmp(obj->tag_p[i].k, k)) return(&(obj->tag_p[i]));
	}
	return(NULL);
}
void tag_free(tag_t *tag)
{
	if (tag->d.s != tag->v)
		free(tag->d.s);
	free(tag->v);
	free(tag->k);
	tag->k = NULL;
	tag->d.s = NULL;
	tag->v = NULL;
}
