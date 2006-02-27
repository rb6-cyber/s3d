/*
 * callback.c
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
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
/*  the s3d callback list */
s3d_cb s3d_cb_list[MAX_CB];
/* the ignore-handler ;) */
static void _s3d_ignore(struct s3d_evt *evt)
{
	/* do plain nothing */
}
/*  sets a callback */
void s3d_set_callback(unsigned char event, s3d_cb func)
{
	s3d_cb_list[(int)event]=func;
	s3d_process_stack();
}
/*  clears a callback, same as s3d_set_callback(event, (s3d_cb) NULL); */
void s3d_clear_callback(unsigned char event)
{
	s3d_cb_list[(int)event]=NULL;
}
/* ignores an event ... */
void s3d_ignore_callback(unsigned char event)
{
	s3d_set_callback(event,_s3d_ignore);
}
s3d_cb s3d_get_callback(unsigned char event)
{
	return(s3d_cb_list[(int)event]);
}
