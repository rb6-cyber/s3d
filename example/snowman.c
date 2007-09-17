/*
 * snowman.c
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
#include <math.h> /* sin() */
#include <time.h> /* nanosleep() */
static struct timespec t = {
	0, 10*1000*1000
}; /* 10 mili seconds */

int a;
int oid_head;
int oid_middle;
int oid_foot;

void mainloop()
{
	float pos;
	a = (a + 3) % 360;
	pos = sin((a * M_PI) / 180) * 5;
	if (pos < 0) pos *= -1;
	s3d_rotate(oid_head, 0, a, 0);
	s3d_rotate(oid_middle, 0, a, 0);
	s3d_rotate(oid_foot, 0, a, 0);
	s3d_translate(oid_head,  0, 1.5 + 2.00*pos, 0);
	s3d_translate(oid_middle, 0, 0   + 1.25*pos, 0);
	s3d_translate(oid_foot,  0, -2  + 1.00*pos, 0);
	nanosleep(&t, NULL);
}
int main(int argc, char **argv)
{
	if (!s3d_init(&argc, &argv, "snowman")) {

		oid_head = s3d_import_model_file("objs/snow_head.3ds");
		oid_middle = s3d_import_model_file("objs/snow_body.3ds");
		oid_foot = s3d_import_model_file("objs/snow_foot.3ds");

		/*  s3d_link(oid_foot,oid_head);
		 *  s3d_link(oid_middle,oid_head);
		 *  s3d_translate(oid_head,0,4,0);

		 *  s3d_translate(oid_middle,0,-1.5,0);  * relative to head: *
		 *  s3d_translate(oid_foot,0,-3.5,0); */

		s3d_scale(oid_middle, 1.25);
		s3d_scale(oid_foot, 1.5);

		s3d_flags_on(oid_head, S3D_OF_VISIBLE);
		s3d_flags_on(oid_middle, S3D_OF_VISIBLE);
		s3d_flags_on(oid_foot, S3D_OF_VISIBLE);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
