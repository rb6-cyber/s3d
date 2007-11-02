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

#define MAXEVENTS 50  /* maximum events per loop. */

/* must be 2^x */
#define TEXW 256
#define TEXH 256
#define TEXNUM(win, x, y) \
  ((((win->attr.height + TEXH - 1)& ~(TEXH-1))/TEXH) * ((int)(x/TEXH)) + ((int)(y/TEXW)))

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
	int     already_updated;
	int        oid;
	int        no;

	struct window     *next;
};

static struct extension    xrender, xcomposite, xdamage, xfixes;
static struct window   *window_head = NULL;
static Display       *dpy;
static char    *display = NULL;

static int     win_no = 0;    /* XXX: REMOVE */
static struct timespec t = {
	0, 50*1000*1000
}; /* 50 mili seconds */

int xinit(void);
void window_update_content(struct window *win, int x, int y, int width, int height);
static int print_event(Display *dpy, XEvent *event);
void event();

static void mainloop(void)
{
	event();
	nanosleep(&t, NULL);
}

static int get_shift(unsigned long t)
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

	/* printf("Event: %s\n", name);*/
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


int xinit(void)
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

static void deco_box(struct window *win)
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
	s3d_translate(win->oid, win->attr.x / 20, -win->attr.y / 20, 1 * win->no);
	/*  push data on texture 0 position (0,0) */
	s3d_flags_on(win->oid, S3D_OF_VISIBLE);
}
static struct window *window_find(Window id) {
	struct window *window;
	for (window = window_head; window != NULL; window = window->next) {
		if (window->id == id)
			return(window);
	}
	printf("not found (window %d). ;(\n", (int)id);
	return(NULL);

}


static void window_add(Display *dpy, Window id)
{
	struct window *win;
	win = malloc(sizeof(*win));
	if (!win)
		return;
	win->id = id;
	XGetWindowAttributes(dpy, win->id, &win->attr);

	win->no = win_no++;
	/* XSelectInput(dpy, win->id, ExposureMask|ButtonPressMask|KeyPressMask*/
	XSelectInput(dpy, win->id, ExposureMask
	             | SubstructureNotifyMask | ExposureMask | StructureNotifyMask | PropertyChangeMask);
	/* XSelectInput(dpy, win->id, ExposureMask);*/
	win->format = XRenderFindVisualFormat(dpy, win->attr.visual);

	if (win->format != NULL) {
		/* printf("add window: %d:%d size: %dx%d\n", win->attr.x, win->attr.y, win->attr.width, win->attr.height);*/
		win->damage = XDamageCreate(dpy, win->id, XDamageReportNonEmpty);

		win->pa.subwindow_mode = IncludeInferiors;
		win->picture = XRenderCreatePicture(dpy, win->id, win->format, CPSubwindowMode, &win->pa);

		win->oid = -1;
		win->already_updated = 0;

		window_update_content(win, 0, 0, win->attr.width, win->attr.height);

		win->next = window_head;
		window_head = win;
	}
}

static void window_remove(struct window *win)
{
	/* TODO */
	free(win);
}

static void window_update_geometry(struct window *win, int x, int y, int width, int height)
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
			s3d_translate(win->oid, win->attr.x / 20, -win->attr.y / 20, 1 * win->no);
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
		printf("request image: xleft = %d, xright = %d, width = %d, x:y = %d:%d, width:height = %d:%d, ~TEXW = %08x\n",
		       xleft, xright, width, x, y, width, height, ~TEXW);
		image = XGetImage(dpy, win->id, xleft, y, chunk_width, height, AllPlanes, ZPixmap);
		if (!image)
			return;
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


void event(void)
{
	XEvent event;
	struct window *window;
	int i;
	for (window = window_head; window != NULL; window = window->next)
		window->already_updated = 0;

	for (i = 0; i < MAXEVENTS && XPending(dpy); i++) {
		XNextEvent(dpy, &event);
		print_event(dpy, &event);
		if (event.type == xdamage.event + XDamageNotify) {
			XDamageNotifyEvent *e = (XDamageNotifyEvent*) & event;
			/*   printf("window = %d, geometry = %d:%d (at %d:%d), area = %d:%d (at %d:%d)\n",
			          (int)e->drawable, e->geometry.width, e->geometry.height, e->geometry.x, e->geometry.y,
			          e->area.width, e->area.height, e->area.x, e->area.y);*/
			XDamageSubtract(dpy, e->damage, None, None);
			window = window_find(e->drawable);
			if (window != NULL)
				window_update_content(window, e->area.x, e->area.y, e->area.width, e->area.height);
		} else if (event.type == ConfigureNotify) {
			XConfigureEvent *e = &event.xconfigure;
			window = window_find(e->window);
			if (window != NULL) {
				/*    printf("Configure: window = %d, geometry = %d:%d (at %d:%d)\n",
				           (int)e->window, e->width, e->height, e->x, e->y);*/
				window_update_geometry(window, e->x, e->y, e->width, e->height);
			} else {
				printf("Configure: Could not find window to configure.\n");
			}
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
