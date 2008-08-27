/*
 * callback.c
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
static int _s3d_ignore(struct s3d_evt *evt);
/*  the s3d callback list */
/* i know it's ugly, but it's better to have ugly code somewhere than provoke
 * race conditions in the applications code */
#define S3D_CBNIL (s3d_cb)NULL
s3d_cb s3d_cb_list[MAX_CB] = {
	S3D_CBNIL, _s3d_ignore, _s3d_ignore, _s3d_ignore, _s3d_ignore, _s3d_ignore, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	_s3d_ignore, _s3d_ignore, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	_s3d_ignore, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,

	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,

	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,

	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL,
	S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL, S3D_CBNIL
};
/* the ignore-handler ;) */
static int _s3d_ignore(struct s3d_evt *S3DUNUSED(evt))
{
	/* do plain nothing */
	return(0);
}
/*  sets a callback */
void s3d_set_callback(uint8_t event, s3d_cb func)
{
	s3d_cb_list[(int)event] = func;
	s3d_process_stack();
}
/*  clears a callback, same as s3d_set_callback(event, (s3d_cb) S3D_CBNIL); */
void s3d_clear_callback(uint8_t event)
{
	s3d_cb_list[(int)event] = S3D_CBNIL;
}
/* ignores an event ... */
void s3d_ignore_callback(uint8_t event)
{
	s3d_set_callback(event, _s3d_ignore);
}
s3d_cb s3d_get_callback(uint8_t event)
{
	return(s3d_cb_list[(int)event]);
}
