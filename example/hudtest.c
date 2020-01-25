// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
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
	return 0;

}
