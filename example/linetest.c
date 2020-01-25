// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */



#include <s3d.h>
#include <stdio.h>  /*  NULL*/
#include <unistd.h> /* sleep() */
#include "example.h" /* S3DUNUSED */
static int i;
static int o;
static int stop(struct s3d_evt *S3DUNUSED(evt))
{
	s3d_quit();
	return 0;
}

static void mainloop(void)
{
	i = (i + 1) % 2;
	if (i) {
		s3d_pep_vertex(o, 1, -2, 0);
		s3d_pep_line(o, 0, 2, 3);
	} else {
		s3d_pep_vertex(o, 1, -1, 0);
		s3d_pep_line(o, 0, 1, 0);

	}
	/*  printf("now it's %s\n",time_str); */
	sleep(1);

}
int main(int argc, char **argv)
{
	i = 0;
	if (!s3d_init(&argc, &argv, "linetest")) {
		o = s3d_new_object();

		s3d_push_material(o,
		                  1, 0, 0,
		                  1, 0, 0,
		                  1, 0, 0);
		s3d_push_material(o,
		                  0, 1, 0,
		                  0, 1, 0,
		                  0, 1, 0);
		s3d_push_material(o,
		                  0, 1, 0,
		                  0, 1, 0,
		                  0, 1, 0);
		s3d_push_material(o,
		                  1, 1, 0,
		                  1, 1, 0,
		                  1, 1, 0);
		s3d_push_vertex(o, -1, -1, 0);
		s3d_push_vertex(o, -1, 1, 0);
		s3d_push_vertex(o, 1, 1, 0);
		s3d_push_vertex(o, 1, -1, 0);
		s3d_push_line(o, 2, 3, 1);
		s3d_push_line(o, 0, 1, 0);

		s3d_flags_on(o, S3D_OF_VISIBLE);
		s3d_set_callback(S3D_EVENT_OBJ_CLICK, (s3d_cb)stop);
		s3d_set_callback(S3D_EVENT_QUIT, (s3d_cb)stop);
		s3d_mainloop(mainloop);
		/*  wait for some object to be clicked */
		s3d_quit();
	}
	return 0;
}



