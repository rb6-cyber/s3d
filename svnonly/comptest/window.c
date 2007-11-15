/*
 * window.c
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
#include <stdlib.h>	/* malloc(), free() */
#include <stdio.h>	/* printf() */
struct window   *window_head = NULL;

void window_set_position(struct window *win) {
	s3d_translate(win->oid, win->attr.x / 20, -win->attr.y / 20, -0.01 * win->no);
}

void window_restack(struct window *win, Window above) 
{
	struct window **wp;
	Window old_above;
	int i;
	if (win->next == NULL)		old_above = None;
	else						old_above = win->next->id;

	if (old_above == above)		return;

	/* unlink from list */
	for (wp = &window_head; *wp != NULL; wp = &(*wp)->next) 
		if (*wp == win)
			break;

	if (*wp == NULL) return;
	*wp = win->next;

	/* relink in front of the new "above" window */
	for (wp = &window_head; *wp != NULL; wp = &(*wp)->next) 
		if ((*wp)->id == above)
			break;

	win->next = *wp;
	*wp = win;

	for (i=0, wp = &window_head; *wp != NULL; wp = &(*wp)->next, i++)
		if (i != (*wp)->no) {
			(*wp)->no = i;
			if ((*wp)->oid != -1)
				window_set_position(*wp);
		}
}
struct window *window_find(Window id) 
{
	struct window *window;
	for (window = window_head; window != NULL; window = window->next) {
		if (window->id == id)
			return(window);
	}
	return(NULL);

}

void window_add(Display *dpy, Window id)
{
	struct window *win;
	win = malloc(sizeof(struct window));
	if (!win)
		return;

	if (window_find(id) != NULL) {
		printf("!!!! Window already added\n");
		return;
	}
	win->id = id;
	XGetWindowAttributes(dpy, win->id, &win->attr);

	/* XSelectInput(dpy, win->id, ExposureMask|ButtonPressMask|KeyPressMask*/
/*	XSelectInput(dpy, win->id, SubstructureNotifyMask | ExposureMask | StructureNotifyMask | PropertyChangeMask);*/
	XSelectInput(dpy, win->id, 0);

	win->damage = XDamageCreate(dpy, win->id, XDamageReportNonEmpty);
/*	win->format = XRenderFindVisualFormat(dpy, win->attr.visual);

	if (win->format != NULL) {
		/ * printf("add window: %d:%d size: %dx%d\n", win->attr.x, win->attr.y, win->attr.width, win->attr.height);* /
		win->pa.subwindow_mode = IncludeInferiors;
		win->picture = XRenderCreatePicture(dpy, win->id, win->format, CPSubwindowMode, &win->pa);
	}*/
	win->no = 0;
	win->oid = -1;
	win->next = window_head;
	window_head = win;
	win->already_updated = 0;
	window_update_content(win, 0, 0, win->attr.width, win->attr.height);
	printf("window (%d) added\n", (int)id);
}

void window_remove(Window id)
{
	struct window **wp, *window;
	for (wp = &window_head; *wp != NULL; wp = &(*wp)->next) 
		if ((*wp)->id == id)
			break;

	if (*wp == NULL) {
		printf("!!!! not found (window %d) for removal.\n", (int)id);
		return;
	}
	window = *wp;
	*wp = window->next;

	printf("window (%d) removed\n", (int)id);

	/* TODO: properly cleanup */
	if (window->oid != -1)
		s3d_del_object(window->oid);
	if (window->picture)
		XRenderFreePicture(dpy, window->picture);
	if (window->damage)
		XDamageDestroy(dpy, window->damage);

	free(window);
}
void window_update_geometry(struct window *win, int x, int y, int width, int height)
{

	printf("window_update_geometry()\n");
	if (win->oid == -1) {
		win->attr.x = x;
		win->attr.y = y;
		win->attr.width = width;
		win->attr.height = height;

		window_update_content(win, 0, 0, width, height);
		return;
	}
	if ((win->attr.width == width) && (win->attr.height == height)) {
		if ((win->attr.x == x) && (win->attr.y == y)) {
			printf("position did not change\n");
			return;
		} else {
			win->attr.x = x;
			win->attr.y = y;
			window_set_position(win);
		}
	} else {
		win->attr.x = x;
		win->attr.y = y;
		win->attr.width = width;
		win->attr.height = height;

		s3d_del_object(win->oid); /* delete the window and redraw */
		win->oid = -1;
		window_update_content(win, 0, 0, width, height);

	}
}

static int get_shift(unsigned long t)
{
	int i;
	for (i = 0; t ; i++)
		t >>= 1;
	return(i);
}


/* convert X-format to s3ds RGBA 32bit format */
static int image_convert(XImage *image, char *bitmap)
{
	int x, y;
	int rs, gs, bs;
	int bpp;
	char *img_ptr, *bmp_ptr;
	unsigned long *s;
	uint32_t *t;


	if (image->format == ZPixmap) {
		/*  printf("XImage: %dx%d, format %d (%d), bpp: %d, depth %d, pad %d\n",
		         image->width, image->height, image->format,
		         ZPixmap, image->bits_per_pixel, image->depth, image->bitmap_pad);*/
		rs = get_shift(image->red_mask) - 8;
		gs = get_shift(image->green_mask) - 8;
		bs = get_shift(image->blue_mask) - 8;

		bpp = (image->bits_per_pixel / 8);
		/* rgb is not bgr */
		rs = rs;
		gs = gs - 8;
		bs = bs - 16;
		/*  printf("Ximage: rgb: %d|%d|%d\n", rs, gs, bs);;*/
		/*  printf("red: size %d, offset %d\n",rs,roff);
		  printf("green: size %d, offset %d\n",gs,goff);
		  printf("blue: size %d, offset %d\n",bs,boff);
		  printf("bits per pixel:%d\n",bpp);*/
		for (y = 0; y < image->height ; y++) {
			img_ptr = image->data + (y * image->width) * bpp;
			bmp_ptr = bitmap + (y * image->width) * sizeof(uint32_t);
			for (x = 0; x < image->width; x++) {
				s = (unsigned long *) img_ptr;
				t = (uint32_t *)  bmp_ptr;
				/*    bmp_ptr[0] = (rs > 0 ? ((*d & image->red_mask) >> rs)  : ((*d  & image->red_mask) << -rs)) ;
				 bmp_ptr[1] = (gs > 0 ? ((*d & image->green_mask) >> gs) : ((*d  & image->green_mask) << -gs)) ;
				 bmp_ptr[2] = (bs > 0 ? ((*d & image->blue_mask) >> bs)  : ((*d  & image->blue_mask) << -bs));
				 bmp_ptr[3] = 255 ;*/
				*t = (rs > 0 ? ((*s & image->red_mask) >> rs)  : ((*s  & image->red_mask) << -rs)) |
				     (gs > 0 ? ((*s & image->green_mask) >> gs) : ((*s  & image->green_mask) << -gs)) |
				     (bs > 0 ? ((*s & image->blue_mask) >> bs)  : ((*s  & image->blue_mask) << -bs)) |
				     255 << 24;

				bmp_ptr += sizeof(uint32_t);
				img_ptr += bpp;
			}
		}
		return(0);
	}
	return(-1);
}

/* takes a bounding box and updates its window contents */
void window_update_content(struct window *win, int x, int y, int width, int height)
{
	int chunk_width, chunk_height;
	int xleft, xright;
	int ytop, ybottom;
	char *bitmap;
	XImage *image;

	if (win->attr.map_state == IsUnmapped)	/* not mapped images can't be grabbed */
		return;

	if (win->attr.class == InputOnly)		/* can't grab image from this source */
		return;

	/* update the whole window for now. */
	/* x = 50;
	 y = 50;
	 width = win->attr.width;
	 height = win->attr.height;*/
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (width > win->attr.width - x)   width = win->attr.width - x;
	if (height > win->attr.height - y)   height = win->attr.height - y;

	if (x == 0 && y == 0 && width == win->attr.width && height == win->attr.height) {
		if (win->already_updated)
			return;
		else
			win->already_updated = 1;
	}

	/* if (!win->oid)
	  deco_box(win);
	*/
	bitmap = malloc(TEXW * height * sizeof(uint32_t));
	for (xleft = x; xleft < x + width ; xleft = xright) {
		xright = (xleft + TEXW) & ~(TEXW - 1);
		if (xright > (x + width))
			xright = x + width;
		chunk_width = xright - xleft;
/*		printf("map-state = %d, backing_store = %d\n", win->attr.map_state);
		printf("request image: xleft = %d, xright = %d, width = %d, x:y = %d:%d, width:height = %d:%d, ~TEXW = %08x\n",
		       xleft, xright, width, x, y, width, height, ~TEXW);*/
		image = XGetImage(dpy, win->id, xleft, y, chunk_width, height, AllPlanes, ZPixmap);
		if (!image) {
/*			printf("XGetImage Error: xleft = %d, xright = %d, width = %d, x:y = %d:%d, width:height = %d:%d\n",
							xleft, xright, width, x, y, width, height);*/
			return;
		}
		if (win->oid == -1)
			deco_box(win);
		/*  printf("image_convert\n");*/
		image_convert(image, bitmap);
		/*  printf("load textures ...\n");*/
		for (ytop = y; ytop < y + height; ytop = ybottom) {
			ybottom = (ytop + TEXH) & ~(TEXH - 1);
			if (ybottom > y + height)
				ybottom = y + height;
			chunk_height = ybottom - ytop;
			s3d_load_texture(win->oid, TEXNUM(win, xleft, ytop), xleft % TEXW, ytop % TEXH,
			                 chunk_width, chunk_height, (unsigned char *)bitmap + chunk_width * (ytop - y) * 4);
			/*   printf("s3d_load_texture(%d, %d, %d, %d, %d, %d, %010p);\n",
			       win->oid, TEXNUM(win, xleft, ytop), xleft % TEXW, ytop % TEXH,
			        chunk_width, chunk_height, (unsigned char *)bitmap + chunk_width * (ytop - y) * 4);*/
		}
		/*  printf("done loading textures\n"); */
		XDestroyImage(image);
	}
	free(bitmap);
}

