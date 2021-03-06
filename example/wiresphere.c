// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


#include <s3d.h>
#include <stdio.h>  /* NULL, sprintf() */
#include <time.h> /* nanosleep()  */
#include <math.h> /* M_PI, cos(), sin() */
#include <stdlib.h> /* malloc(), free() */
#include "example.h" /* S3DUNUSED */
static struct timespec t = {
	0, 100*1000*1000
}; /* 100 mili seconds */
static int oid;
static int r;
static int wire_sphere(int slices, int stacks)
{
	int x, y, i, o;
	unsigned int num_v, num_l;
	float *v, *n;  /* vertices, normals */
	float alpha, beta;
	unsigned int *l; /* lines */
	num_v = (stacks + 1) * slices;
	num_l = stacks * slices + (stacks - 1) * slices; /* vertical + horizontal */
	v = (float*)malloc(sizeof(float) * 3 * num_v);
	n = (float*)malloc(sizeof(float) * 6 * num_l);
	l = (unsigned int*)malloc(sizeof(unsigned int) * 3 * num_l);
	i = 0;
	for (x = 0; x < slices; x++) {
		alpha = (x * 360.0f / slices) * (float)M_PI / 180.0f;
		for (y = 0; y < (stacks + 1); y++) {
			beta = ((y * 180 / slices) - 90.0f) * (float)M_PI / 180.0f;
			v[i*3+0] = cosf(alpha) * cosf(beta);
			v[i*3+1] = sinf(beta);
			v[i*3+2] = sinf(alpha) * cosf(beta);
			i++;
		}
	}
	i = 0;
	for (x = 0; x < slices; x++) {
		for (y = 0; y < stacks; y++) {
			if ((y != 0) && (y != stacks)) { /* no horizontal lines at the poles */
				l[i*3+0] = (x * (stacks + 1)) + y;
				l[i*3+1] = (((x + 1) % slices) * (stacks + 1)) + y;
				l[i*3+2] = 0;
				n[i*6+0] = v[ l[i*3+0] * 3 + 0];
				n[i*6+1] = v[ l[i*3+0] * 3 + 1];
				n[i*6+2] = v[ l[i*3+0] * 3 + 2];
				n[i*6+3] = v[ l[i*3+1] * 3 + 0];
				n[i*6+4] = v[ l[i*3+1] * 3 + 1];
				n[i*6+5] = v[ l[i*3+1] * 3 + 2];

				i++;

			}
			/* vertical lines */
			l[i*3+0] = (x * (stacks + 1)) + y;
			l[i*3+1] = (x * (stacks + 1)) + y + 1;
			l[i*3+2] = 0;
			n[i*6+0] = v[ l[i*3+0] * 3 + 0];
			n[i*6+1] = v[ l[i*3+0] * 3 + 1];
			n[i*6+2] = v[ l[i*3+0] * 3 + 2];
			n[i*6+3] = v[ l[i*3+1] * 3 + 0];
			n[i*6+4] = v[ l[i*3+1] * 3 + 1];
			n[i*6+5] = v[ l[i*3+1] * 3 + 2];
			i++;

		}
	}
	o = s3d_new_object();
	s3d_push_material(o, 0, 0, 1,
	                  1, 0, 0,
	                  0, 1, 0);
	s3d_push_vertices(o, v, (uint16_t)num_v);
	s3d_push_lines(o, l, (uint16_t)num_l);
	s3d_load_line_normals(o, n, 0, (uint16_t)num_l);
	free(v);
	free(n);
	free(l);
	return o;
}
static int stop(struct s3d_evt *S3DUNUSED(evt))
{
	s3d_quit();
	return 0;
}

static void mainloop(void)
{
	r = (r + 1) % 360;
	s3d_rotate(oid, 0, (float)r, 0);
	nanosleep(&t, NULL);

}
int main(int argc, char **argv)
{
	if (!s3d_init(&argc, &argv, "wiresphere")) {
		oid = wire_sphere(30, 30);
		s3d_scale(oid, 10);
		s3d_flags_on(oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
		s3d_set_callback(S3D_EVENT_OBJ_CLICK, (s3d_cb)stop);
		s3d_set_callback(S3D_EVENT_QUIT, (s3d_cb)stop);
		s3d_mainloop(mainloop);
		/*  wait for some object to be clicked */
		s3d_quit();
	}
	return 0;
}


