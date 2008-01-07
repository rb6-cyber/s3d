/*
 * s3dclock.c
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
#include <stdio.h>  /*  NULL, sprintf() */
#include <time.h>  /*  nanosleep(), struct tm, time_t...  */
#include <string.h>  /*  strlen() */
#include "example.h" /* S3DUNUSED */
static struct timespec t = {
	0, 100*1000*1000
}; /* 100 mili seconds */
static int big_p, lil_p, bg, sec_p;
static int str_oid = -1, o_str_oid;
static struct tm *mytime;
static time_t now, onow;
static char time_str[256];

static void stop(struct s3d_evt *S3DUNUSED(evt))
{
	s3d_quit();
}

static void mainloop(void)
{
	onow = now;
	now = time(NULL);
	if (now != onow) {
		o_str_oid = str_oid;
		mytime = localtime(&now);
		s3d_rotate(lil_p, 0, 0, -((mytime->tm_hour % 12) / 12.0f)*360.0f);
		s3d_rotate(big_p, 0, 0, -(mytime->tm_min / 60.0f)*360.0f);
		s3d_rotate(sec_p, 0, 0, -(mytime->tm_sec / 60.0f)*360.0f);
		sprintf(time_str, "%02d:%02d:%02d", mytime->tm_hour, mytime->tm_min, mytime->tm_sec);
		str_oid = s3d_draw_string(time_str, NULL);
		s3d_translate(str_oid, -1, -1.3, 0);
		s3d_scale(str_oid, 0.5);
		s3d_flags_on(str_oid, S3D_OF_VISIBLE);
		if (str_oid != -1)
			s3d_del_object(o_str_oid);
	}
	/*  printf("now it's %s\n",time_str); */
	nanosleep(&t, NULL);

}
int main(int argc, char **argv)
{
	if (!s3d_init(&argc, &argv, "clock")) {
		lil_p = s3d_import_model_file("objs/lil_p.3ds");
		big_p = s3d_import_model_file("objs/big_p.3ds");
		sec_p = s3d_import_model_file("objs/sec_p.3ds");
		bg = s3d_import_model_file("objs/clock_bg.3ds");

		s3d_flags_on(big_p, S3D_OF_VISIBLE);
		s3d_flags_on(lil_p, S3D_OF_VISIBLE);
		s3d_flags_on(sec_p, S3D_OF_VISIBLE);
		s3d_flags_on(bg, S3D_OF_VISIBLE);
		s3d_select_font("vera");

		s3d_set_callback(S3D_EVENT_OBJ_CLICK, (s3d_cb)stop);
		s3d_set_callback(S3D_EVENT_QUIT, (s3d_cb)stop);
		s3d_mainloop(mainloop);
		/*  wait for some object to be clicked */
		s3d_quit();
	}
	return(0);
}

