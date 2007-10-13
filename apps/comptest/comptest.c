/*
 * comptest.c
 *
 * Copyright (C) 2007 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of comptest, a proof-of-concept composite manager hack.
 * See http://s3d.berlios.de/ for more updates.
 *
 * olsrs3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * olsrs3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsrs3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>        /* malloc(), free() */
#include <time.h>         /* nanosleep() */
#include <s3d.h>
#include <X11/Xlib.h>       /* Ximage, Display, X*() */
#include <X11/Xutil.h>       /* XDestroyImage() */
#include <X11/Xatom.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xrender.h>
#include <config-s3d.h>

#ifndef COMPUNUSED
#if defined(UNUSEDPARAM_ATTRIBUTE)
#define COMPUNUSED(x) (x)__attribute__((unused))
#elif defined(UNUSEDPARAM_OMIT)
#define COMPUNUSED(x) /* x */
#else
#define COMPUNUSED(x) x
#endif
#endif

#define TEXW 256
#define TEXH 256

struct extension {
	int event, error;
};

struct window {
	Window        id;
	XWindowAttributes    attr;   /* position, size etc. */
	XImage      *image;
	Damage       damage;  /* damage notification */
	XRenderPictureAttributes  pa;
	XRenderPictFormat    *format;
	Picture       picture;
	int        oid;
	int        no;

	struct window     *next;
};

struct extension    xrender, xcomposite, xdamage, xfixes;
struct window   *window_head = NULL;
Display       *dpy;
char    *display = NULL;
int      scr;
Window     root;

static int     win_no = 0;    /* XXX: REMOVE */
static struct timespec t = {
	0, 10*1000*1000
}; /* 10 mili seconds */

int xinit();
void window_update(struct window *win, int x, int y, int width, int height);
static int print_event(Display *dpy, XEvent *event);
void event();

void mainloop()
{
	event();
	nanosleep(&t, NULL);
}

int get_shift(unsigned long t)
{
	int i;
	for (i = 0; t ; i++)
		t >>= 1;
	return(i);
}

static int print_event(Display *COMPUNUSED(dpy), XEvent *event)
{
	char *name = "unknown";
	switch (event->type & 0x7f) {
	case Expose:
		name = "Expose";
		break;
	case MapNotify:
		name = "Map";
		break;
	case UnmapNotify:
		name = "Unmap";
		break;
	case ReparentNotify:
		name = "Reparent";
		break;
	case CirculateNotify:
		name = "Circulate";
		break;
	}
	if (event->type == xdamage.event + XDamageNotify) {
		/* XDamageNotifyEvent *e = (XDamageNotifyEvent*) event; */
		/* e->drawable is the window ID of the damaged window
		   e->geometry is the geometry of the damaged window
		   e->area     is the bounding rect for the damaged area
		   e->damage   is the damage handle returned by XDamageCreate() */

		/* Subtract all the damage, repairing the window. */
		name = "Damage!!";
	} else if (event->type == ConfigureNotify) {
		/* XConfigureEvent *e = &event->xconfigure; */
		/* The windows size, position or Z index in the stacking
		   order has changed */
		name = "Configure!!";
	}

	printf("Event: %s\n", name);
	return(0);
}

/*static int error(Display *COMPUNUSED(dpy), XErrorEvent *COMPUNUSED(event))*/
static int error(Display *COMPUNUSED(dpy), XErrorEvent *event)
{
	char *name = "";
	int     o;

	o = event->error_code - xfixes.error;
	switch (o) {
	case BadRegion:
		name = "BadRegion";
		break;
	default:
		break;
	}
	o = event->error_code - xdamage.error;
	switch (o) {
	case BadDamage:
		name = "BadDamage";
		break;
	default:
		break;
	}
	o = event->error_code - xrender.error;
	switch (o) {
	case BadPictFormat:
		name = "BadPictFormat";
		break;
	case BadPicture:
		name = "BadPicture";
		break;
	case BadPictOp:
		name = "BadPictOp";
		break;
	case BadGlyphSet:
		name = "BadGlyphSet";
		break;
	case BadGlyph:
		name = "BadGlyph";
		break;
	default:
		break;
	}

	printf("error %d (name: %s) request %d minor %d serial %d\n",
	       event->error_code, name, event->request_code, event->minor_code, (int)event->serial);
	return(0);
}


int xinit()
{
	dpy = XOpenDisplay(display);
	if (!dpy) {
		fprintf(stderr, "Can't open display\n");
		return(1);
	}
	if (!XRenderQueryExtension(dpy, &xrender.event, &xrender.error)) {
		fprintf(stderr, "No render extension\n");
		return(1);
	}
	/*    XCompositeQueryVersion (dpy, &composite_major, &composite_minor); */

	if (!XCompositeQueryExtension(dpy, &xcomposite.event, &xcomposite.error)) {
		fprintf(stderr, "No composite extension\n");
		return(1);
	}

	if (!XDamageQueryExtension(dpy, &xdamage.event, &xdamage.error)) {
		fprintf(stderr, "No damage extension\n");
		return(1);
	}
	if (!XFixesQueryExtension(dpy, &xfixes.event, &xfixes.error)) {
		fprintf(stderr, "No XFixes extension\n");
		return(1);
	}
	XSetErrorHandler(error);
	return(0);
}

void deco_box(struct window *win)
{
/*	float vertices[8*3] = {
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

	win->no = win_no++;  /* TODO: REMOVE */
	win->oid = s3d_new_object();
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
		s3d_push_vertex(win->oid, 0, -((float)ypos) / 20, -1);
		voffset++;
	}
	for (x = 0; x < win->attr.width; x += TEXW) { /* the first row */
		xpos = (x + TEXW > win->attr.width) ? win->attr.width : x + TEXW ;
		s3d_push_vertex(win->oid, ((float)xpos) / 20, 0, -1);

		for (y = 0; y < win->attr.height; y += TEXH) {
			ypos = (y + TEXH > win->attr.height) ? win->attr.height : y + TEXH  ;
			s3d_push_vertex(win->oid, ((float)xpos) / 20, -((float)ypos) / 20, -1);
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
	s3d_translate(win->oid, win->attr.x / 20, -win->attr.y / 20, 5 * win->no);
	/*  push data on texture 0 position (0,0) */
	s3d_flags_on(win->oid, S3D_OF_VISIBLE);
}

void window_add(Display *dpy, Window id)
{
	struct window *win;
	win = malloc(sizeof(*win));
	if (!win)
		return;
	win->id = id;
	XGetWindowAttributes(dpy, win->id, &win->attr);
	/* XSelectInput(dpy, win->id, ExposureMask|ButtonPressMask|KeyPressMask*/
	XSelectInput(dpy, win->id, ExposureMask
	             | SubstructureNotifyMask | ExposureMask | StructureNotifyMask | PropertyChangeMask);
	/* XSelectInput(dpy, win->id, ExposureMask);*/
	win->format = XRenderFindVisualFormat(dpy, win->attr.visual);

	if (win->format != 0) {
		/* printf("add window: %d:%d size: %dx%d\n", win->attr.x, win->attr.y, win->attr.width, win->attr.height);*/
		win->damage = XDamageCreate(dpy, win->id, XDamageReportNonEmpty);

		win->pa.subwindow_mode = IncludeInferiors;
		win->picture = XRenderCreatePicture(dpy, win->id, win->format, CPSubwindowMode, &win->pa);

		win->oid = 0;

		window_update(win, 0, 0, win->attr.width, win->attr.height);

		win->next = window_head;
		window_head = win;
	}
}

void window_remove(struct window *win)
{
	/* TODO */
	free(win);
}

void window_update(struct window *win, int x, int y, int width, int height)
{
	int chunk_width, chunk_height;
	int xleft, xright;
	int ytop, ybottom;
	int texnum;
	int xi, yi;
	int rs, gs, bs;
	int bpp;
	char *img_ptr, *bmp_ptr;
	char *bitmap;
	unsigned long *s;
	uint32_t *t;
	XImage *image;

	/* update the whole window for now. */
	x = 0;
	y = 0;
	width = win->attr.width;
	height = win->attr.height;

	texnum = 0;


	/* if (!win->oid)
	  deco_box(win);
	*/
	for (xleft = 0; xleft < width ; xleft += TEXW) {
		xright = (xleft + TEXW > width) ? width : xleft + TEXW;
		chunk_width = xright - xleft;
		image = XGetImage(dpy, win->id, xleft, y, chunk_width, win->attr.height, AllPlanes, ZPixmap);
		if (!image)
			return;
		bitmap = malloc(chunk_width * height * sizeof(uint32_t));
		if (!win->oid)
			deco_box(win);
		if (image->format == ZPixmap) {
			printf("XImage: %dx%d, format %d (%d), bpp: %d, depth %d, pad %d\n",
			       image->width, image->height, image->format,
			       ZPixmap, image->bits_per_pixel, image->depth, image->bitmap_pad);
			rs = get_shift(image->red_mask) - 8;
			gs = get_shift(image->green_mask) - 8;
			bs = get_shift(image->blue_mask) - 8;

			bpp = (image->bits_per_pixel / 8);
			/* rgb is not bgr */
			rs = rs;
			gs = gs - 8;
			bs = bs - 16;
			printf("Ximage: rgb: %d|%d|%d\n", rs, gs, bs);;
			/*  printf("red: size %d, offset %d\n",rs,roff);
			  printf("green: size %d, offset %d\n",gs,goff);
			  printf("blue: size %d, offset %d\n",bs,boff);
			  printf("bits per pixel:%d\n",bpp);*/
			for (yi = 0; yi < height ; yi++) {
				img_ptr = image->data + (yi * image->width) * bpp;
				bmp_ptr = bitmap + ((y + yi) * chunk_width + x) * sizeof(uint32_t);
				for (xi = 0; xi < xright - xleft; xi++) {
					s = (unsigned long *)img_ptr;
					t = (uint32_t *)bmp_ptr;
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
			/*  s3d_load_texture(win->oid,0,x,y,width,height, ???); */
			for (ytop = 0; ytop < height; ytop += TEXH) {
				ybottom = (ytop + TEXH > height) ? height : ytop + TEXH;
				chunk_height = ybottom - ytop;
				s3d_load_texture(win->oid, texnum, 0, 0, chunk_width, chunk_height, (unsigned char *)bitmap + chunk_width * ytop * 4);
				texnum++;
			}
			/*  swap images */
		}
		XDestroyImage(image);
		free(bitmap);
	}
}


void event()
{
	XEvent event;
	while (QLength(dpy)) {
		XNextEvent(dpy, &event);
		print_event(dpy, &event);
		if (event.type == xdamage.event + XDamageNotify) {
			XDamageNotifyEvent *e = (XDamageNotifyEvent*) & event ;
			printf("window = %d, geometry = %d:%d (at %d:%d), area = %d:%d (at %d:%d)\n",
			       (int)e->drawable, e->geometry.width, e->geometry.height, e->geometry.x, e->geometry.y,
			       e->area.width, e->area.height, e->area.x, e->area.y);
			XDamageSubtract(dpy, e->damage, None, None);
		}
	}
}


int main(int argc, char **argv)
{
	Window        root_return, parent_return;
	unsigned int     nchildren;
	Window       *children;
	int     i, scr_no;


	if (xinit())
		return(1);
	if (!s3d_init(&argc, &argv, "compotest")) {
		for (scr_no = 0; scr_no < ScreenCount(dpy); scr_no++) {
			XCompositeRedirectSubwindows(dpy, RootWindow(dpy, scr_no), CompositeRedirectAutomatic);
			/*   XCompositeRedirectSubwindows(dpy, RootWindow(dpy, scr_no), CompositeRedirectManual);*/
			XSelectInput(dpy, RootWindow(dpy, scr_no),
			             SubstructureNotifyMask | ExposureMask | StructureNotifyMask | PropertyChangeMask);

			XQueryTree(dpy, RootWindow(dpy, scr_no), &root_return, &parent_return, &children, &nchildren);
			for (i = 0; i < (int)nchildren; i++)
				window_add(dpy, children[i]);
			XFree(children);
		}
		s3d_mainloop(mainloop);
	}


	return(0);
}
