/*
 * s3d_x11gate.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3d_x11gate, a 3d gateway for x11 desktops.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3d_x11gate is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3d_x11gate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3d_x11gate; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <s3d.h>   /*  s3d_*() */
#include <stdlib.h>   /*  getenv() */
#include <stdio.h>   /*  printf() */
#include <X11/Xlib.h>  /*  Ximage, Display, X*() */
#include <X11/Xutil.h>  /*  XDestroyImage() */
#define XK_MISCELLANY
#include <X11/keysymdef.h>  /* keysyms */
#include <X11/extensions/XTest.h> /* keyboard/mouse input via s3d */
#include <X11/extensions/XShm.h> /* */
#include <time.h>  /* nanosleep() */
#include <sys/time.h>  /* gettimeofday */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <config-s3d.h>

#ifndef S3DX11UNUSED
#if defined(UNUSEDPARAM_ATTRIBUTE)
#define S3DX11UNUSED(x) (x)__attribute__((unused))
#elif defined(UNUSEDPARAM_OMIT)
#define S3DX11UNUSED(x) /* x */
#else
#define S3DX11UNUSED(x) x
#endif
#endif

static int oid;
static XImage *image;
static Display *dpy = NULL;
static int window, scr;
static unsigned int width, height, depth;
static Visual *visual;
static XShmSegmentInfo shminfo;
static char *tex_image = NULL, *otex_image = NULL, *img1, *img2;



static struct timeval start, end;
static int iterations;
static float count[3];

static int get_shift(uint32_t t)
{
	int i = 0;
	while (t) {
		t >>= 1;
		i++;
	}
	return(i);
}
static void mainloop(void)
{
#define MAGIC_CHANGED ((unsigned int)~0)
	unsigned int x, y;
	int rs, gs, bs;
	uint32_t d;
	int bpp;
	char *swap_timg;
	unsigned int last_change, start_change;
	gettimeofday(&end, NULL);
	count[0] += (end.tv_sec - start.tv_sec) * 10000000 + end.tv_usec - start.tv_usec;
	start.tv_sec = end.tv_sec;
	start.tv_usec  = end.tv_usec;

	/* image = XGetImage(dpy,window,0,0,width,height,AllPlanes,ZPixmap);*/
	XShmGetImage(dpy, window, image, 0, 0, 0xffffffff);
	gettimeofday(&end, NULL);
	count[1] += (end.tv_sec - start.tv_sec) * 10000000 + end.tv_usec - start.tv_usec;
	start.tv_sec = end.tv_sec;
	start.tv_usec  = end.tv_usec;
	if (image->format == ZPixmap) {
		printf("Ximage: %dx%d, format %d (%d), bpp: %d, depth %d, pad %d\n", image->width, image->height, image->format, ZPixmap,
		       image->bits_per_pixel, image->depth, image->bitmap_pad);
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
		last_change = MAGIC_CHANGED;
		start_change = MAGIC_CHANGED;
		for (y = 0;y < height;y++) {

			for (x = 0;x < width;x++) {
				d = *((uint32_t *)(image->data + (y * width + x) * bpp));
				((uint32_t *)tex_image)[(y*width+x)] =
				        (rs > 0 ? ((d & image->red_mask) >> rs) : ((d & image->red_mask) << -rs)) |
				        (gs > 0 ? ((d & image->green_mask) >> gs) : ((d & image->green_mask) << -gs)) |
				        (bs > 0 ? ((d & image->blue_mask) >> bs) : ((d & image->blue_mask) << -bs)) |
				        255u << 24;
				if (((uint32_t *)tex_image)[(y*width+x)] !=
				                ((uint32_t *)otex_image)[(y*width+x)])
					last_change = y;
			}
			if (last_change != MAGIC_CHANGED) {
				if (start_change == MAGIC_CHANGED) {
					start_change = y;
					/*      printf("setting start_change to %d\n",start_change); */
				}
				if (last_change != y) {  /*  last change is already over, post it! */
					s3d_load_texture(oid, 0, 0, start_change, width, last_change - start_change + 1, (unsigned char *)tex_image + start_change*width * sizeof(uint32_t));
					start_change = MAGIC_CHANGED;
					last_change = MAGIC_CHANGED;
				}
			}
		}
		/*  posting the last bit, maybe */
		if (last_change != MAGIC_CHANGED) {
			/*   printf("last one: [%d-%d]",start_change,last_change);*/
			s3d_load_texture(oid, 0, 0, start_change, width, last_change - start_change, (unsigned char *)tex_image + start_change*width * sizeof(uint32_t));
		}
		/*   s3d_load_texture(oid,0,0,0,width,height,tex_image); */
		/*  swap images */
		swap_timg = tex_image;
		tex_image = otex_image;
		otex_image = swap_timg;
	}
	gettimeofday(&end, NULL);
	count[2] += (end.tv_sec - start.tv_sec) * 10000000 + end.tv_usec - start.tv_usec;
	start.tv_sec = end.tv_sec;
	start.tv_usec  = end.tv_usec;
	iterations++;
	/* XDestroyImage(image);*/
	/* nanosleep(&t,NULL); */
}
static int keypress(struct s3d_evt *event)
{
	int key;
	int kc;
	key = *((unsigned short *)event->buf);
	printf("received key %d ", key);
	kc = XKeysymToKeycode(dpy, key);
	if (kc == 0) {
		kc = XKeysymToKeycode(dpy, 0xFF00 + key);
		printf(" (%04x) ", 0xFF00 + key);
	}
	if (kc == 0) {
		if (key == 8) {
			kc = 22;
			printf("!backspace!");
		}
	}
	printf("using key: %d, keycode %d (%04x)\n", key, kc, kc);
	if (kc != 0)
		XTestFakeKeyEvent(dpy, kc, 1, 1);
	/*     XTestFakeKeyEvent(dpy, kc, 0, 1);*/
	return(0);

}
static int mouseclick(struct s3d_evt *S3DX11UNUSED(event))
{
	int i;
	printf("thats it, collecting:\n");
	for (i = 0;i < 3;i++)
		printf("[%d] %f\n", i, count[i] / iterations);
	exit(0);
}
int main(int argc, char **argv)
{
	const char *disp = NULL;
	int a, b, c, d;
	int xt;
	if (disp == NULL) disp = getenv("DISPLAY");
	if (disp == NULL) disp = "";  /*  fallback */
	dpy = XOpenDisplay(disp);
	if (!dpy) {
		printf("couldn't open display\n");
		return(-1);
	}
	count[0] = count[1] = count[2] = 0;
	iterations = 0;
	if (!s3d_init(&argc, &argv, "X11-gate")) {
		scr = DefaultScreen(dpy);
		window = RootWindow(dpy, scr);
		width = DisplayWidth(dpy, scr);
		height = DisplayHeight(dpy, scr);
		visual = DefaultVisual(dpy, scr);
		depth = DefaultDepth(dpy, scr);
		XLockDisplay(dpy);
		xt = XTestQueryExtension(dpy, &a, &b, &c, &d);
		XUnlockDisplay(dpy);
		if (xt) {
			printf("having xtest extension ...\n");
		}
		/* X11 shm - http://www.xfree86.org/current/mit-shm.html */

		image = XShmCreateImage(dpy, visual, depth, ZPixmap, NULL, &shminfo, width, height);
		shminfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777);
		shminfo.shmaddr = image->data = (char*)shmat(shminfo.shmid, NULL, 0);
		shmctl(shminfo.shmid, IPC_RMID, NULL);
		shminfo.readOnly = False;
		if (!XShmAttach(dpy, &shminfo))
			printf("cannot use the shared memory segment .. :( \n");
		else
			printf("can use share segment :D\n");
		XSync(dpy, False);

		s3d_set_callback(S3D_EVENT_OBJ_CLICK, mouseclick);
		s3d_set_callback(S3D_EVENT_KEY, keypress);
		printf("screen: %dx%d\n", width, height);
		img1 = (char*)malloc(width * height * sizeof(uint32_t));
		img2 = (char*)malloc(width * height * sizeof(uint32_t));
		tex_image = img1;
		otex_image = img2;
		oid = s3d_new_object();
		s3d_push_vertex(oid, -5, -5, 0);
		s3d_push_vertex(oid, 5, -5, 0);
		s3d_push_vertex(oid, 5, 5, 0);
		s3d_push_vertex(oid, -5, 5, 0);
		s3d_push_material_a(oid,
		                    0.8, 0.0, 0.0 , 1.0,
		                    1.0, 1.0, 1.0 , 1.0,
		                    0.8, 0.0, 0.0 , 1.0);
		s3d_push_polygon(oid, 0, 2, 1, 0);
		s3d_pep_polygon_tex_coord(oid, 0.0, 1.0,
		                          1.0, 0.0,
		                          1.0, 1.0);
		s3d_push_polygon(oid, 0, 3, 2, 0);
		s3d_pep_polygon_tex_coord(oid, 0.0, 1.0,
		                          0.0, 0.0,
		                          1.0, 0.0);
		s3d_push_texture(oid, width, height);
		/*  push data on texture 0 position (0,0) */
		s3d_pep_material_texture(oid, 0); /*  assign texture 0 to material 0 */
		s3d_flags_on(oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
		gettimeofday(&start, NULL);
		s3d_mainloop(mainloop);
		free(img1);
		free(img2);
	}
	s3d_quit();
	return(0);
}
