// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


#include "s3dfm.h"
#include <s3d.h>
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
		mstrncpy(root.name, "/", M_NAME);
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
	return 0;
}
