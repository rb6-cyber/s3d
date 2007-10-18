/*
 * linetest.c
 *
 * Copyright (C) 2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3d, a 3d network display server.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */



#include <s3d.h>
#include <stdio.h>  /*  NULL*/
#include <unistd.h> /* sleep() */
#include "example.h" /* S3DUNUSED */
static int i;
static int o;
static void stop(struct s3d_evt *S3DUNUSED(evt))
{
	s3d_quit();
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
	return(0);
}



