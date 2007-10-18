/*
 * hudtest.c
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
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
#include <unistd.h>  /*  sleep() */
#include <stdio.h>  /*  printf() */
static void mainloop(void)
{
	sleep(1);
}
int main(int argc, char **argv)
{
	int o, m;
	if (!s3d_init(&argc, &argv, "hud-test")) {
		if (s3d_select_font("vera"))
			printf("font not found\n");
		o = s3d_draw_string("hud-test", NULL);
		m = s3d_import_model_file("objs/star.3ds");
		s3d_translate(o, 0, 0, -5);
		s3d_link(o, 0);
		s3d_flags_on(o, S3D_OF_VISIBLE);
		s3d_flags_on(m, S3D_OF_VISIBLE);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);

}
