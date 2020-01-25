// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */

#include <s3d.h>
#include <stdio.h>  /*  NULL */
#include <time.h> /* nanosleep() */
static struct timespec t = {
	0, 10*1000*1000
}; /* 10 mili seconds */

#include <math.h> /* sin(), cos() */
static int item1, item2, item3;
static int i = 0;
static void mainloop(void)
{
	float f, g, h;
	f = sinf(((float)M_PI * (i % 360)) / 180.0f);
	g = cosf(((float)M_PI * (i % 360)) / 180.0f);
	h = sinf(((float)M_PI * ((3 * i) % 360)) / 180.0f);

	s3d_translate(item1, f*10, h*2, g*5);
	s3d_rotate(item1, 0, (float)(i % 360), 0);
	/* s3d_scale(item1,h+3,h+3,h+3);*/

	/* s3d_translate(item2,f*10,0, g*5);*/
	s3d_translate(item2, 0, 0, 10);
	/* s3d_rotate(item2,i%360,0,-i%360);
	 s3d_scale(item2,2*f+5,2*f+5,2*f+5);*/

	s3d_translate(item3, 0, 0, 10);
	/* s3d_rotate(item3,((8*i)%360),0,-((8*i)%360));
	 s3d_scale(item3,2*f+5,2*f+5,2*f+5);*/



	i++;
	nanosleep(&t, NULL);
}
int main(int argc, char **argv)
{
	if (!s3d_init(&argc, &argv, "radius test")) {
		item1 = s3d_import_model_file("objs/cubeyholes.3ds");
		item2 = s3d_import_model_file("objs/folder.3ds");
		s3d_select_font("vera");
		item3 = s3d_draw_string("radius test", NULL);
		s3d_link(item2, item1);
		s3d_link(item3, item2);
		s3d_flags_on(item1, S3D_OF_VISIBLE);
		s3d_flags_on(item2, S3D_OF_VISIBLE);
		s3d_flags_on(item3, S3D_OF_VISIBLE);
		s3d_mainloop(mainloop);
		/*  wait for some object to be clicked */
		s3d_quit();
	}
	return 0;
}
