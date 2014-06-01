/*
 * window.c
 *
 * Copyright (C) 2007-2012  Simon Wunderlich <sw@simonwunderlich.de>
 *
 * This file is part of comptest, a proof-of-concept composite manager hack.
 * See http://s3d.sourceforge.net/ for more updates.
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
#include <stdlib.h> /* malloc(), free() */
#include <string.h> /* memcpy() */
#include <stdio.h> /* printf() */
#include <stdint.h>
#include <s3d.h>
#include <X11/Xlib.h>       /* Ximage, Display, X*() */
#include <X11/Xutil.h>       /* XDestroyImage() */
#include <X11/Xatom.h>
#include <config-s3d.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xrender.h>

struct window   *window_head = NULL;

void window_set_position(struct window *win)
{
	s3d_translate(win->oid, win->attr.x, -win->attr.y, -0.01 * win->no);
}

void window_restack(struct window *win, Window above)
{
	struct window **wp;
	Window old_above;
	int i;
	if (win->next == NULL)  old_above = None;
	else      old_above = win->next->id;

	if (old_above == above)  return;

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

	for (i = 0, wp = &window_head; *wp != NULL; wp = &(*wp)->next, i++)
		if (i != (*wp)->no) {
			(*wp)->no = i;
			if ((*wp)->oid != -1)
				window_set_position(*wp);
		}
}
struct window *window_find(Window id) {
	struct window *window;
	for (window = window_head; window != NULL; window = window->next) {
		if (window->id == id)
			return window;
	}
	return NULL;

}

struct window *window_add(Display *dpy, Window id) {
	struct window *win;
	win = (struct window *)malloc(sizeof(struct window));
	if (!win)
		return NULL;

	printf("window_add(%d)\n", (int) id);
	if (window_find(id) != NULL) {
		printf("!!!! Window already added\n");
		free(win);
		return NULL;
	}
	win->id = id;
	/* print_properties(id);*/
	win->name = x11_get_name(id);
	printf("###################### name = %s\n", win->name);
	win->next = window_head;
	window_head = win;
	win->damage = None;
	win->pix = None;
	win->no = 0;
	win->oid = -1;
	win->content_update_needed = 0;
	win->geometry_update_needed = 0;
	win->content_update.x = 0;
	win->content_update.y = 0;
	win->content_update.width = 0;
	win->content_update.height = 0;
	win->mapped = 0;



	/* TODO: at my place, windows are created and destroyed in the same moment. so this
	 * function fails sometimes.
	 * maybe there is a function asking something like "is there really a window with id ;win->id'...".
	 * that would help here. */
	if (!XGetWindowAttributes(dpy, win->id, &win->attr)) {
		/* window does not exit, next event is probably it's removal ... */
		return win;
	}

	/* XSelectInput(dpy, win->id, ExposureMask|ButtonPressMask|KeyPressMask*/
	/* XSelectInput(dpy, win->id, SubstructureNotifyMask | ExposureMask | StructureNotifyMask | PropertyChangeMask);*/
	XSelectInput(dpy, win->id, 0);

#if defined(__cplusplus) || defined(c_plusplus)
	if (win->attr.c_class != InputOnly)  /* don't create damage on these windows */
#else
	if (win->attr.class != InputOnly)  /* don't create damage on these windows */
#endif

		win->damage = XDamageCreate(dpy, win->id, XDamageReportNonEmpty);
	if (win->next == NULL)
		window_restack(win, None);
	else
		window_restack(win, win->next->id);

	XCompositeRedirectWindow(dpy, id, CompositeRedirectAutomatic);
	return win;
}
void window_map(struct window *win)
{
	printf("window_map(%d)\n", (int)win->id);
	if (win->mapped)
		return;
	win->mapped = 1;
	win->content_update_needed = 1;
	win->content_update.x = 0;
	win->content_update.y = 0;
	win->content_update.width = win->attr.width;
	win->content_update.height = win->attr.height;
	if (win->oid != -1)
		s3d_flags_on(win->oid, S3D_OF_VISIBLE);


}

void window_unmap(struct window *win)
{
	if (!win->mapped)
		return;
	win->mapped = 0;
	if (win->oid != -1)
		s3d_flags_off(win->oid, S3D_OF_VISIBLE);

	/* TODO: handle */
}

void window_remove(Window id)
{
	struct window **wp, *window;
	for (wp = &window_head; *wp != NULL; wp = &(*wp)->next)
		if ((*wp)->id == id)
			break;

	if (*wp == NULL) {
		printf("!!!! not found (window %d) for removal.\n", (int)id);
		exit(-1);
		return;
	}
	window = *wp;
	*wp = window->next;

	printf("window (%d) removed\n", (int)id);

	/* TODO: properly cleanup */
	if (window->pix != None)
		XFreePixmap(dpy, window->pix);
	if (window->oid != -1)
		s3d_del_object(window->oid);
	/*  Damage is automatically destroyed */
	/* if (window->damage != None)
	  XDamageDestroy(dpy, window->damage); */

	free(window);
}
void window_update_geometry(struct window *win)
{
	XWindowAttributes attr;
	win->geometry_update_needed = 0;

	if (!XGetWindowAttributes(dpy, win->id, &attr))
		return;

	if (win->oid == -1) {
		win->content_update_needed = 1;
	}

	if ((win->attr.width == attr.width) && (win->attr.height == attr.height)) {
		if ((win->attr.x == attr.x) && (win->attr.y == attr.y)) {
			printf("position did not change\n");
		} else {
			memcpy(&win->attr, &attr, sizeof(attr));
			window_set_position(win);
			return;
		}
	} else {
		if (win->pix != None) {
			XFreePixmap(dpy, win->pix);
			win->pix = None;
		}

		s3d_del_object(win->oid); /* delete the window and redraw */
		win->oid = -1;
		win->content_update_needed = 1;
	}
	memcpy(&win->attr, &attr, sizeof(attr));
}

/* convert X-format to s3ds RGBA 32bit format */
static int image_convert(XImage *image, char *bitmap)
{
	int x, y;
	char *img_ptr, *bmp_ptr;
	uint8_t *sc, *tc;

	if (image->format != ZPixmap)
		return -1;
	switch (image->bits_per_pixel) {
	case 32:
		for (y = 0; y < image->height ; y++) {
			img_ptr = image->data + (y * image->width) * 4;
			bmp_ptr = bitmap + (y * image->width) * 4;
			for (x = 0; x < image->width; x++) {
				sc = (uint8_t *) img_ptr;
				tc = (uint8_t *) bmp_ptr;

				tc[0] = sc[2];
				tc[1] = sc[1];
				tc[2] = sc[0];
				tc[3] = 255;


				bmp_ptr += 4;
				img_ptr += 4;
			}
		}
		break;
	default:
		/* not implemented. draw a red image. */
		for (y = 0; y < image->height ; y++) {
			bmp_ptr = bitmap + (y * image->width) * 4;
			for (x = 0; x < image->width; x++) {
				tc = (uint8_t *)  bmp_ptr;

				tc[0] = 255;
				tc[1] = 0;
				tc[2] = 0;
				tc[3] = 255;

				bmp_ptr += 4;
			}
		}
		break;
	}
	return -1;
}

/* takes a bounding box and updates its window contents */
void window_update_content(struct window *win)
{
	int x, y, width, height;
	int chunk_width, chunk_height;
	int xleft, xright;
	int ytop, ybottom;
	char *bitmap;
	XImage *image;

	x = win->content_update.x;
	y = win->content_update.y;
	width = win->content_update.width;
	height = win->content_update.height;

	/* done with updating, or at least we tried */
	win->content_update.width = 0;
	win->content_update.height = 0;
	win->content_update_needed = 0;

	if (win->attr.map_state == IsUnmapped) /* not mapped images can't be grabbed */
		return;

#if defined(__cplusplus) || defined(c_plusplus)
	if (win->attr.c_class == InputOnly)  /* can't grab image from this source */
		return;
#else
	if (win->attr.class == InputOnly)  /* can't grab image from this source */
		return;
#endif

	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (width > win->attr.width - x)   width = win->attr.width - x;
	if (height > win->attr.height - y)   height = win->attr.height - y;

	bitmap = (char*)malloc(TEXW * height * sizeof(uint32_t));
	for (xleft = x; xleft < x + width ; xleft = xright) {
		xright = (xleft + TEXW) & ~(TEXW - 1);
		if (xright > (x + width))
			xright = x + width;
		chunk_width = xright - xleft;

		if (win->pix == None)
			win->pix = XCompositeNameWindowPixmap(dpy, win->id);
		image = XGetImage(dpy, win->pix, xleft, y, chunk_width, height, AllPlanes, ZPixmap);
		if (!image) {
			printf("XGetImage Error: xleft = %d, xright = %d, width = %d, x:y = %d:%d, width:height = %d:%d\n",
			       xleft, xright, width, x, y, width, height);
			exit(-1);
			if (win->oid != -1) {
				s3d_del_object(win->oid);
				win->oid = -1;
			}

			return;
		}
		if (win->oid == -1)
			deco_box(win);

		image_convert(image, bitmap);

		for (ytop = y; ytop < y + height; ytop = ybottom) {
			ybottom = (ytop + TEXH) & ~(TEXH - 1);
			if (ybottom > y + height)
				ybottom = y + height;
			chunk_height = ybottom - ytop;
			s3d_load_texture(win->oid, TEXNUM(win, xleft, ytop), xleft % TEXW, ytop % TEXH,
			                 chunk_width, chunk_height, (unsigned char *)bitmap + chunk_width * (ytop - y) * 4);
		}
		XDestroyImage(image);
	}
	free(bitmap);
}

