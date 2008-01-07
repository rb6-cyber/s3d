/*
 * ptrtest.c
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
#include <time.h> /* nanosleep() */
#include <math.h> /* sin(), cos() */
#include "example.h" /* S3DUNUSED */
static int o;
static float asp = 1.0;
static float len = 1.0;
static int alpha = 0;
static struct timespec t = {
	0, 10*1000*1000
}; /* 100 mili seconds */
static int stop(struct s3d_evt *S3DUNUSED(evt))
{
	s3d_quit();
	return(0);
}

static void mainloop(void)
{
	float a;
	alpha = (alpha + 1) % 360;
	s3d_rotate(o, (float)alpha, 0.f, 0.f);
	a = (((float)alpha) * (float)M_PI / 180.f);
	s3d_translate(0.f, sinf(a)*30.f, 0.f, 30.f + cosf(a)*30.f);
	s3d_rotate(0, sinf(a)*30.f, (float)alpha, 0.f);
	nanosleep(&t, NULL);
}
static int object_info(struct s3d_evt *hrmz)
{
	struct s3d_obj_info *inf;
	inf = (struct s3d_obj_info *)hrmz->buf;
	if (inf->object == 0) {
		if (asp != inf->scale) {
			asp = inf->scale;
			printf("screen aspect: %f\n", asp);
		}
	}
	if (inf->object == 1) { /* of course, a link s3d_link(o,1 would be much easier ... */
		s3d_translate(o, (inf->trans_x)*2.0f, (inf->trans_y)*2.0f, -2);
	}
	return(0);
}
static int mbutton_press(struct s3d_evt *hrmz)
{
	struct s3d_but_info *inf;
	char s[256];
	inf = (struct s3d_but_info *)hrmz->buf;
	snprintf(s, 256, "please, watch your stomach! button %d, state %d", inf->button, inf->state);
	printf("button %d, state %d\n", inf->button, inf->state);
	s3d_del_object(o);
	o = s3d_draw_string(s, &len);
	s3d_translate(o, 0, 0, -2);
	s3d_scale(o, 0.2);
	s3d_link(o, 0);  /* link to cam */
	/* s3d_link(o,1);*/
	s3d_flags_on(o, S3D_OF_VISIBLE);
	return(0);
}
int main(int argc, char **argv)
{
	if (!s3d_init(&argc, &argv, "ptr and cam test")) {
		s3d_set_callback(S3D_EVENT_OBJ_INFO, object_info);
		s3d_set_callback(S3D_EVENT_MBUTTON, mbutton_press);
		s3d_set_callback(S3D_EVENT_QUIT, stop);
		s3d_set_callback(S3D_EVENT_OBJ_CLICK, stop);
		s3d_select_font("vera");
		o = s3d_draw_string("hello", &len);
		s3d_translate(o, 0, 0, -2);
		s3d_link(o, 0);  /* link to cam */
		/*  s3d_link(o,1);*/
		s3d_scale(o, 0.2);
		s3d_flags_on(o, S3D_OF_VISIBLE);
		s3d_mainloop(mainloop);
		/*  wait for some object to be clicked */
		s3d_quit();
	}
	return(0);
}

