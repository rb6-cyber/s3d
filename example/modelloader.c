// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */




#include <s3d.h>
#include <stdio.h>  /* NULL */
#include <time.h> /* nanosleep() */
#include "example.h" /* S3DUNUSED */
static struct timespec t = {
	0, 100*1000*1000
}; /* 100 mili seconds */
static int i, oid;
static void mainloop(void)
{
	s3d_rotate(oid, 0, (float)i, 0);
	i = (i + 1) % 360;
	nanosleep(&t, NULL);
}
static int object_click(struct s3d_evt *S3DUNUSED(evt))
{
	s3d_quit();
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("usage: %s [somefile.3ds]\n", argv[0]);
		return -1;
	}
	if (!s3d_init(&argc, &argv, "modelloader")) {
		s3d_set_callback(S3D_EVENT_OBJ_CLICK, object_click);
		i = 0;
		if (-1 != (oid = s3d_import_model_file(argv[1]))) {
			s3d_flags_on(oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			s3d_mainloop(mainloop);
		} else {
			printf("file not found ... \n");
		}
		s3d_quit();
	}
	return 0;
}
