/*
 * katze.c
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

static int a;
static int rot_point, body, legfr, legbr, legfl, legbl, tail;

static void mainloop(void)
{
	float pos;
	a = (a + 2) % 360;
	pos = sinf((5.f * a * (float)M_PI) / 180.f) * 20.f;
	s3d_rotate(legfr, 0, 90, 180 + pos);
	s3d_rotate(legfl, 0, 90, 180 - pos);
	s3d_rotate(legbr, 0, 0, 180 + pos);
	s3d_rotate(legbl, 0, 0, 180 - pos);

	s3d_rotate(tail, 0, 30, 110 + pos);
	s3d_rotate(rot_point, 0, (float)-a, 0);
	nanosleep(&t, NULL);
}
int main(int argc, char **argv)
{
	if (!s3d_init(&argc, &argv, "running cat")) {

		rot_point = s3d_new_object();
		body = s3d_import_model_file("objs/katze_body.3ds");
		legfr = s3d_import_model_file("objs/katze_leg.3ds");
		tail = s3d_import_model_file("objs/katze_tail.3ds");
		legfl = s3d_clone(legfr);
		legbl = s3d_clone(legfr);
		legbr = s3d_clone(legfr);
		s3d_translate(legfl, 2.3, 1.0, 0.5);
		s3d_translate(legfr, 0, 1.0, 0.2);
		s3d_translate(legbl, -1.2, 1.0, -1.8);
		s3d_translate(legbr, -1.2, 1.0, 0.0);
		s3d_translate(tail, -1.6, 1.6, -0.8);
		s3d_translate(body, 1.3, 0.0, -1.3);
		s3d_link(legfr, body);
		s3d_link(legfl, body);
		s3d_link(legbr, body);
		s3d_link(legbl, body);
		s3d_link(tail, body);
		s3d_link(body, rot_point);
		/*  s3d_link(oid_foot,oid_head);
		 *  s3d_link(oid_middle,oid_head);
		 *  s3d_translate(oid_head,0,4,0);

		 *  s3d_translate(oid_middle,0,-1.5,0);  * relative to head: *
		 *  s3d_translate(oid_foot,0,-3.5,0); */


		s3d_flags_on(body, S3D_OF_VISIBLE);
		s3d_flags_on(legfr, S3D_OF_VISIBLE);
		s3d_flags_on(legfl, S3D_OF_VISIBLE);
		s3d_flags_on(legbr, S3D_OF_VISIBLE);
		s3d_flags_on(legbl, S3D_OF_VISIBLE);
		s3d_flags_on(tail, S3D_OF_VISIBLE);
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}
