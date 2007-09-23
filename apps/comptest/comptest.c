/*
 * comptest.c
 *
 * Copyright (C) 2007 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of comptest, a one-day-proof-of-concept composite manager hack.
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
#include <stdlib.h>								/* malloc(), free() */
#include <time.h> 								/* nanosleep() */
#include <s3d.h>
#include <X11/Xlib.h>							/* Ximage, Display, X*() */
#include <X11/Xutil.h>							/* XDestroyImage() */
#include <X11/Xatom.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xrender.h>

struct extension {
	int event, error;
};

struct window {
	Window 						 id;
	XWindowAttributes			 attr;			/* position, size etc. */
	XImage						*image;
	Damage						 damage;		/* damage notification */
	XRenderPictureAttributes	 pa;
	XRenderPictFormat 			*format; 
	Picture						 picture;
	int							 oid;
	int 						 no;

	char						*bitmap;		/* bitmap to upload */
	struct window 				*next;
};

struct extension 	  xrender, xcomposite, xdamage, xfixes;
struct window		 *window_head = NULL;
Display     		*dpy;
char				*display = NULL;
int					 scr;
Window				 root;

static int 			 win_no = 0;				/* XXX: REMOVE */
static struct timespec t = {
	0, 10*1000*1000
}; /* 10 mili seconds */

int xinit();
void window_update(struct window *win, int x, int y, int width, int height);

void mainloop()
{
	nanosleep(&t, NULL);
}

int get_shift(unsigned long t)
{
	int i;
	for (i = 0; t ; i++)
		t >>= 1;
	return(i);
}

static int error(Display *dpy, XErrorEvent *event)		
{
/*    printf ("error %d request %d minor %d serial %d\n",
        event->error_code, event->request_code, event->minor_code, event->serial);*/
    return 0;
}


int xinit() 
{
	dpy = XOpenDisplay(display);
	if (!dpy) {
		fprintf(stderr, "Can't open display\n");
		return(1);
	}
    if (!XRenderQueryExtension (dpy, &xrender.event, &xrender.error)) {
	    fprintf(stderr, "No render extension\n");
		return(1);
    }
/*    XCompositeQueryVersion (dpy, &composite_major, &composite_minor); */

	if (!XCompositeQueryExtension (dpy, &xcomposite.event, &xcomposite.error)) {
	    fprintf (stderr, "No composite extension\n");
		return(1);
	}

    if (!XDamageQueryExtension (dpy, &xdamage.event, &xdamage.error))
    {
	    fprintf (stderr, "No damage extension\n");
		return(1);
    }
    if (!XFixesQueryExtension (dpy, &xfixes.event, &xfixes.error))
    {
	    fprintf (stderr, "No XFixes extension\n");
	    return(1);
    }
	XSetErrorHandler(error);
	return(0);
}

void deco_box(struct window *win) {
	float vertices[8*3] = {
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

	};
	float tbuf[] = { 0.0, 0.0,  1.0, 0.0,  1.0, 1.0,
					 0.0, 0.0,  1.0, 1.0,  0.0, 1.0};
	int i;

	win->no = win_no++;		/* TODO: REMOVE */
	win->oid = s3d_new_object();

	for (i = 0;i < 8;i++) {
		sver[i*3 + 0] = vertices[i*3+0] * win->attr.width/20;
		sver[i*3 + 1] = vertices[i*3+1] * -win->attr.height/20;
		sver[i*3 + 2] = vertices[i*3+2] * -1;
	}

	s3d_push_material_a(win->oid,
						0.8, 0.0, 0.0 , 1.0,
						1.0, 1.0, 1.0 , 1.0,
						0.8, 0.0, 0.0 , 1.0);
	s3d_push_texture(win->oid, win->attr.width, win->attr.height);
	s3d_pep_material_texture(win->oid, 0); /*  assign texture 0 to material 0 */
	s3d_push_material_a(win->oid,
						0.0, 0.8, 0.0 , 1.0,
						1.0, 1.0, 1.0 , 1.0,
						0.0, 0.8, 0.0 , 1.0);

	s3d_push_vertices(win->oid, sver, 8);

	s3d_push_polygons(win->oid, polygon, 12);
	s3d_pep_polygon_tex_coords(win->oid, tbuf, 2);
	s3d_translate(win->oid, win->attr.x/20, -win->attr.y/20, 5 * win->no);
	/*  push data on texture 0 position (0,0) */
	s3d_flags_on(win->oid, S3D_OF_VISIBLE);
	
}

void window_add(Display *dpy, Window id) {
	struct window *win;
	win = malloc(sizeof(*win));
	if (!win) 
		return;
	win->id = id;
	XGetWindowAttributes(dpy, win->id, &win->attr);
	win->format = XRenderFindVisualFormat( dpy, win->attr.visual );
/*	printf("add window: %d:%d size: %dx%d\n", win->attr.x, win->attr.y, win->attr.width, win->attr.height);*/
	win->damage = XDamageCreate(dpy, win->id, XDamageReportNonEmpty );

	win->pa.subwindow_mode = IncludeInferiors;
	win->picture = XRenderCreatePicture(dpy, win->id, win->format, CPSubwindowMode, &win->pa);

	win->oid = 0;

	win->bitmap = NULL;
	window_update(win, 0, 0, win->attr.width, win->attr.height);

	win->next = window_head;
	window_head = win;
	
}

void window_remove(struct window *win) {
	/* TODO */
	free(win);
}

void window_update(struct window *win, int x, int y, int width, int height) {
	int xi, yi;
	int rs, gs, bs;
	int bpp;
	unsigned char *img_ptr, *bmp_ptr;
	unsigned long *s, *t;
	XImage *image;

/*	printf("[D]oing window update for window %d at position %d:%d with size %dx%d\n",win->id, x,y,width,height);*/
	image = XGetImage(dpy, win->id, x, y, win->attr.width, win->attr.height, AllPlanes, ZPixmap);
	if (!image) {
/*		printf("[P]roblem with the image\n");*/
		return;
	}
	if (!win->bitmap)
		win->bitmap = malloc(win->attr.width * win->attr.height * 4);
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
			img_ptr = image->data + (yi * width ) * bpp;
			bmp_ptr = win->bitmap + ((y + yi) * win->attr.width + x) * 4;
			for (xi = 0; xi < width; xi++) {
				s = (unsigned long *)img_ptr;
				t = (unsigned long *)bmp_ptr;
/*				bmp_ptr[0] = (rs > 0 ? ((*d & image->red_mask) >> rs) 	: ((*d  & image->red_mask) << -rs)) ;
				bmp_ptr[1] = (gs > 0 ? ((*d & image->green_mask) >> gs) : ((*d  & image->green_mask) << -gs)) ;
				bmp_ptr[2] = (bs > 0 ? ((*d & image->blue_mask) >> bs)  : ((*d  & image->blue_mask) << -bs));
				bmp_ptr[3] = 255 ;*/
				*t = (rs > 0 ? ((*s & image->red_mask) >> rs) 	: ((*s  & image->red_mask) << -rs)) |
					 (gs > 0 ? ((*s & image->green_mask) >> gs) : ((*s  & image->green_mask) << -gs)) |
				     (bs > 0 ? ((*s & image->blue_mask) >> bs)  : ((*s  & image->blue_mask) << -bs)) |
				     255 << 24;

				bmp_ptr += 4;
				img_ptr += bpp;
			}
			bmp_ptr = win->bitmap + ((y + yi) * win->attr.width + x) * 4;
		}
/*		s3d_load_texture(win->oid,0,x,y,width,height, ???); */
		s3d_load_texture(win->oid,0,0,0,win->attr.width, win->attr.height, win->bitmap);
		/*  swap images */
	}
	XDestroyImage(image);
}

void event() {
/*	XNextEvent (dpy, &ev);*/
}


int main(int argc, char **argv) {
    Window      	 root_return, parent_return;
    unsigned int     nchildren;
    Window      	*children;
	int 			 i, scr_no;


	if (xinit())
		return(1);
	if (!s3d_init(&argc, &argv, "compotest")) {
		for (scr_no = 0; scr_no < ScreenCount(dpy); scr_no++) {
			XCompositeRedirectSubwindows(dpy, RootWindow(dpy, scr_no), CompositeRedirectAutomatic);
		    XQueryTree (dpy, RootWindow(dpy, scr_no), &root_return, &parent_return, &children, &nchildren);
		    for (i = 0; i < nchildren; i++)
		        window_add(dpy, children[i]);
		    XFree(children);
		}
		s3d_mainloop(mainloop);
	}


	return(0);
}
