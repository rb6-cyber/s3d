/*
 * event.c
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

#include "s3dfm.h"
#include <s3d_keysym.h>
#include <stdio.h> /* printf() */



int typeinput = 0;

/* info packets handler, we're just interested in the cam */
int event_oinfo(struct s3d_evt *hrmz)
{
	struct s3d_obj_info *inf;
	inf = (struct s3d_obj_info *)hrmz->buf;
	if ((inf->object == 0) && (!ani_onstack(&cam))) {
		cam.dpx = inf->trans_x;
		cam.dpy = inf->trans_y;
		cam.dpz = inf->trans_z;
	}
	s3dw_object_info(hrmz);
	return(0);
}

/* keyevent handler */
int event_key(struct s3d_evt *evt)
{
	struct s3d_key_event *keys = (struct s3d_key_event *)evt->buf;
	char path[M_DIR];
	if (typeinput) { /* we have some inputfield now and want the s3dw to handle our input */
		printf("inputting text ...\n");
		s3dw_handle_key(evt);
		return(0);
	}
	node_path(node_getdir(focus), path);
	switch (keys->keysym) {
	case 'i':
	case 'I': {
		window_info(path);
	}
	break;
	case 'r':
	case 'R': {/* refresh this window ... */
		t_node *node;
		node = node_getdir(focus);
		printf("[R]efreshing %s\n", node->name);
		parse_dir(node);
		box_draw_icons(node);
		box_order_icons(node);
		box_order_subdirs(node);

	}
	break;
	case S3DK_F1:
		window_help();
		break;
	case S3DK_F5:
		window_copy(path);
		break;
	case S3DK_F6:
		window_move(path);
		break;
	case S3DK_F7:
		window_mkdir(path);
		break;
	case S3DK_F8:
		window_unlink();
		break;
	case S3DK_F10:
		/* some debugging stuff */
		node_getbypath(path);
		break;

	case S3DK_UP:
	case S3DK_LEFT:
	case S3DK_RIGHT:
	case S3DK_DOWN:
		focus_by_key(keys->keysym);
		break;
	case S3DK_RETURN:
	case S3DK_SPACE:
		node_select(focus);
		break;
	case S3DK_BACKSPACE:
		if (focus->disp == D_DIR)
			box_close(focus, 1);
		else if (focus->parent != NULL)
			box_close(focus->parent, 1);
		break;

	}
	s3dw_handle_key(evt);
	return(0);
}

/* object click handler */
int event_click(struct s3d_evt *evt)
{
	int oid;
	t_node *f;
	s3dw_handle_click(evt);
	oid = (int) * ((uint32_t *)evt->buf);
	if (NULL != (f = node_getbyoid(&root, oid))) {
		if (f->objs.close == oid) {
			box_close(f, 1);
			return(0);
		}
		if (f->objs.select == oid) {
			printf("[S]electing %s\n", f->name);
			node_select(f);
			return(0);
		}
		node_select(f);
	} else {
		/*  printf("[C]ould not find :/\n");*/
	}
	return(0);
}

