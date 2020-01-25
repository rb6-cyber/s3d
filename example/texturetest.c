// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


#include <s3d.h>
#include <stdio.h>  /*  NULL */
#include <stdlib.h>  /* malloc(),free() */
#include <time.h> /* nanosleep() */
static struct timespec t = {
	0, 10*1000*1000
}; /* 10 mili seconds */

static int i, oid;
static void mainloop(void)
{
	i = (i + 1) % 360;
	s3d_rotate(oid, 0, (float)i, 0);
	nanosleep(&t, NULL);
}
#define MAXX 24
#define MAXY 24
int main(int argc, char **argv)
{
	unsigned int x, y;
	unsigned char *data;

	if (!s3d_init(&argc, &argv, "texturetest")) {
		data = (unsigned char *)malloc(MAXX * MAXY * 4);
		oid = s3d_new_object();
		s3d_push_vertex(oid, -1, -1, 0);
		s3d_push_vertex(oid, 1, -1, 0);
		s3d_push_vertex(oid, 1, 1, 0);
		s3d_push_vertex(oid, -1, 1, 0);
		s3d_push_material_a(oid,
		                    0.8, 0.0, 0.0 , 1.0,
		                    1.0, 1.0, 1.0 , 1.0,
		                    0.8, 0.0, 0.0 , 1.0);
		s3d_push_polygon(oid, 0, 1, 2, 0);
		s3d_pep_polygon_tex_coord(oid, 0.0, 0.0,
		                          1.0, 0.0,
		                          1.0, 1.0);
		s3d_push_polygon(oid, 0, 2, 3, 0);
		s3d_pep_polygon_tex_coord(oid, 0.0, 0.0,
		                          1.0, 1.0,
		                          0.0, 1.0);
		s3d_translate(oid, 0, 0, -5);
		for (y = 0; y < MAXY; y++)
			for (x = 0; x < MAXX; x++) {
				data[(y*MAXX+x)*4+0] = (char)((x * 255) / MAXX);
				data[(y*MAXX+x)*4+1] = (char)((x * y) / (MAXX * MAXY));
				data[(y*MAXX+x)*4+2] = (char)((y * 255) / MAXX);
				data[(y*MAXX+x)*4+3] = 255;
			}
		s3d_push_texture(oid, MAXX, MAXY);
		s3d_load_texture(oid, 0, 0, 0, MAXX, MAXY, data);
		/*  push data on texture 0 position (0,0) */
		free(data);
		s3d_pep_material_texture(oid, 0); /*  assign texture 0 to material 0 */
		s3d_flags_on(oid, S3D_OF_VISIBLE);
		i = 0;
		s3d_mainloop(mainloop);
		/*  wait for some object to be clicked */
		s3d_quit();
	}
	return 0;
}
