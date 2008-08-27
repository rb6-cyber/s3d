/*
 * main.c
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
#include <string.h>  /*  strlen(), strncmp(), strrchr() */
#include <time.h> /* nanosleep() */
static struct timespec t = {
	0, 33*1000*1000
};
t_node root, cam, *focus;

static void mainloop(void)
{
	ani_mate();
	s3dw_ani_mate();
	window_fsani();
	nanosleep(&t, NULL);
}
int main(int argc, char **argv)
{

	s3d_set_callback(S3D_EVENT_OBJ_CLICK, event_click);
	s3d_set_callback(S3D_EVENT_OBJ_INFO, event_oinfo);
	s3d_set_callback(S3D_EVENT_KEY, event_key);
	if (!s3d_init(&argc, &argv, "s3dfm")) {
		s3d_select_font("vera");

		node_init(&cam); /* a virtual object, just to push the cam throu our animation stack */
		cam.oid = 0;
		/* set up file system representation */
		node_init(&root);
		strncpy(root.name, "/", M_NAME);
		focus = &root;
		root.dscale = 0.1;
		root.type = T_FOLDER;
		parse_dir(&root);
		box_draw(&root);
		ani_doit(&root);
		focus_set(&root);



		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
