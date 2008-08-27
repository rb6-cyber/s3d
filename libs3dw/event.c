/*
 * event.c
 *
 * Copyright (C) 2006-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of the s3d Widgets, a Widget Library for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3d Widgets is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * s3d Widgets is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d Widgets; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <s3d.h>
#include <s3dw.h>
#include <s3dw_int.h>

int s3dw_handle_click(const struct s3d_evt *evt)
{
	uint32_t oid = *((uint32_t *)evt->buf);
	return(s3dw_widget_event_click(s3dw_getroot(), oid));
}
int s3dw_handle_key(const struct s3d_evt *evt)
{
	struct s3d_key_event *keys = (struct s3d_key_event *)evt->buf;
	return(s3dw_widget_event_key(s3dw_getroot(), keys));
}

int s3dw_object_info(struct s3d_evt *evt)
{
	struct s3d_obj_info *info = (struct s3d_obj_info *)evt->buf;
	if (info->object == 0) { /* the _s3dw_cam */
		if (_s3dw_cam == NULL) s3dw_getroot(); /* init, get _s3dw_cam */
		_s3dw_cam->ax = _s3dw_cam->x = info->trans_x;
		_s3dw_cam->ay = _s3dw_cam->y = info->trans_y;
		_s3dw_cam->az = _s3dw_cam->z = info->trans_z;
		_s3dw_cam->arx = _s3dw_cam->rx = info->rot_x;
		_s3dw_cam->ary = _s3dw_cam->ry = info->rot_y;
		_s3dw_cam->arz = _s3dw_cam->rz = info->rot_z;

		_s3dw_cam->flags &= ~S3DW_ARRANGED;
		s3dw_ani_needarr();
	}
	return(0);
}
