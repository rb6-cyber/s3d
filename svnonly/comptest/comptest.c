/*
 * comptest.c
 *
 * Copyright (C) 2007 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of comptest, a proof-of-concept composite manager hack.
 * See http://s3d.berlios.de/ for more updates.
 *
 * comptest is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * comptest is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with comptest; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "comptest.h"
#include <s3d_keysym.h>
#include <stdio.h>
#include <stdlib.h>        /* malloc(), free() */
#include <time.h>         /* nanosleep() */
#include <errno.h>			/* errno */

#include <sys/time.h>		/* gettimeofday() */
#include <time.h>			/* gettimeofday() */


static struct timespec t = {
	0, 50*1000*1000
}; /* 50 mili seconds */

static void mainloop(void)
{
	struct timeval start, stop;
	float secs;
	gettimeofday(&start, NULL);
	event();
	gettimeofday(&stop, NULL);
	secs = (stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec)/1e6;
/*	printf("msecs to process events: %3.3f\n", secs*1e3);*/
	nanosleep(&t, NULL);
}

void deco_box(struct window *win)
{
	/* float vertices[8*3] = {
	  0, 0, 0,
	  1, 0, 0,
	  1, 1, 0,
	  0, 1, 0,
	  0, 0, 1,
	  1, 0, 1,
	  1, 1, 1,
	  0, 1, 1
	 };
	 float sver[8*3];
	 uint32_t polygon[12*4] = {
	  1, 5, 6, 1,
	  1, 6, 2, 1,
	  2, 6, 7, 1,
	  2, 7, 3, 1,
	  4, 0, 3, 1,
	  4, 3, 7, 1,
	  5, 4, 7, 1,
	  5, 7, 6, 1,
	  0, 4, 1, 1,
	  4, 5, 1, 1,
	  0, 1, 2, 0,
	  0, 2, 3, 0

	 };*/
	float tbuf[] = { 0.0, 0.0,  1.0, 0.0,  1.0, 1.0,
	                 0.0, 0.0,  1.0, 1.0,  0.0, 1.0
	               };
	/* int i;*/
	int x, y;
	int vindex, voffset, pindex;
	int xpos, ypos;

	win->oid = s3d_new_object();
	s3d_link(win->oid, screen_oid);
	/*
	 for (i = 0;i < 8;i++) {
	  sver[i*3 + 0] = vertices[i*3+0] * win->attr.width / 20;
	  sver[i*3 + 1] = vertices[i*3+1] * -win->attr.height / 20;
	  sver[i*3 + 2] = vertices[i*3+2] * -1;
	 }

	 s3d_push_material_a(win->oid,
	                     0.8, 0.0, 0.0 , 1.0,
	                     1.0, 1.0, 1.0 , 1.0,
	                     0.8, 0.0, 0.0 , 1.0);
	 s3d_push_texture(win->oid, win->attr.width, win->attr.height);
	 s3d_pep_material_texture(win->oid, 0); / *  assign texture 0 to material 0 * /
	 s3d_push_material_a(win->oid,
	                     0.0, 0.8, 0.0 , 1.0,
	                     1.0, 1.0, 1.0 , 1.0,
	                     0.0, 0.8, 0.0 , 1.0);

	 s3d_push_vertices(win->oid, sver, 8);

	 s3d_push_polygons(win->oid, polygon, 12);
	 s3d_pep_polygon_tex_coords(win->oid, tbuf, 2);*/
	voffset = 1;
	vindex = 0;
	pindex = 0;
	s3d_push_vertex(win->oid, 0, 0, -1); /* the first point */

	for (y = 0; y < win->attr.height;  y += TEXH) { /* the first column */
		ypos = (y + TEXH > win->attr.height) ? win->attr.height : y + TEXH ;
		s3d_push_vertex(win->oid, 0, -((float)ypos) , -1);
		voffset++;
	}
	for (x = 0; x < win->attr.width; x += TEXW) { /* the first row */
		xpos = (x + TEXW > win->attr.width) ? win->attr.width : x + TEXW ;
		s3d_push_vertex(win->oid, ((float)xpos) , 0, -1);

		for (y = 0; y < win->attr.height; y += TEXH) {
			ypos = (y + TEXH > win->attr.height) ? win->attr.height : y + TEXH  ;
			s3d_push_vertex(win->oid, ((float)xpos) , -((float)ypos) , -1);
			s3d_push_material_a(win->oid,
			                    0.0, 0.8, 0.0 , 1.0,
			                    1.0, 1.0, 1.0 , 1.0,
			                    0.0, 0.8, 0.0 , 1.0);
			s3d_push_texture(win->oid, xpos - x, ypos - y);
			s3d_pep_material_texture(win->oid, pindex);
			s3d_push_polygon(win->oid, vindex, vindex + voffset, vindex + voffset + 1, pindex);
			s3d_push_polygon(win->oid, vindex, vindex + voffset + 1, vindex + 1, pindex);
			s3d_pep_polygon_tex_coords(win->oid, tbuf, 2);
			pindex++;
			vindex++;
		}
		vindex++;
	}
	window_set_position(win);
	/*  push data on texture 0 position (0,0) */
	if (win->mapped)
		s3d_flags_on(win->oid, S3D_OF_VISIBLE);
}

static int key(struct s3d_evt *evt)
{
	struct s3d_key_event *key = (struct s3d_key_event *)evt->buf;
	if (key->keysym == S3DK_RETURN) {
		printf("camera into position ...\n");

		s3d_translate(0, 0,0, SCREEN_SCALE * ( 1 - 1/100.0));
		s3d_rotate(0,0,0,0);

		

	}
	return(0);

}
int screen_width = 0;
int screen_height = 0;
int screen_oid = -1;

static void set_screenpos(void) {
	XWindowAttributes    attr;
	XGetWindowAttributes(dpy, RootWindow(dpy, 0), &attr);
	screen_width = attr.width;
	screen_height = attr.height;

	screen_oid = s3d_new_object();
	s3d_translate(screen_oid, -SCREEN_SCALE * (float)screen_width/((float) screen_height), SCREEN_SCALE, 0);
	s3d_scale(screen_oid, 2* SCREEN_SCALE/((float)screen_height));

}

int main(int argc, char **argv)
{


	if (xinit())
		return(1);

	if (!s3d_init(&argc, &argv, "comptest")) {
		s3d_set_callback(S3D_EVENT_KEY, key);

		set_screenpos();

		x11_addwindows();
		s3d_mainloop(mainloop);
	}


	return(0);
}
