// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */



#include <s3d.h>
#include <stdio.h>  /*  NULL*/
#include <unistd.h> /* sleep() */
#include "example.h" /* S3DUNUSED */
static int o;
static int stop(struct s3d_evt *S3DUNUSED(evt))
{
	s3d_quit();
	return 0;
}

static void mainloop(void)
{
	/*  printf("now it's %s\n",time_str); */
	sleep(1);

}
int main(int argc, char **argv)
{
	char c[256];
	int i;
	if (!s3d_init(&argc, &argv, "strtest")) {
		s3d_select_font("vera");
		/*  o=s3d_draw_string("The lazy fox is bored enough to jump over everything it sees. weird, isn't it?!",NULL);  */
		for (i = 0; i < 256; i++)
			c[255-i] = (char)i;
		o = s3d_draw_string(c, NULL);
		/*  o=s3d_draw_string("A",NULL);*/
		s3d_flags_on(o, S3D_OF_VISIBLE);

		s3d_set_callback(S3D_EVENT_OBJ_CLICK, (s3d_cb)stop);
		s3d_set_callback(S3D_EVENT_QUIT, (s3d_cb)stop);
		s3d_mainloop(mainloop);
		/*  wait for some object to be clicked */
		s3d_quit();
	}
	return 0;
}


