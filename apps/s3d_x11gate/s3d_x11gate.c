/*
 * s3d_x11gate.c
 * 
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
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


#include <s3d.h>		 /*  s3d_*() */
#include <stdlib.h> 	 /*  getenv() */
#include <stdio.h>		 /*  printf() */
#include <X11/Xlib.h>	 /*  Ximage, Display, X*() */
#include <X11/Xutil.h>	 /*  XDestroyImage() */
#define XK_MISCELLANY
#include <X11/keysymdef.h>	 /* keysyms */
#include <X11/extensions/XTest.h>	/* keyboard/mouse input via s3d */
#include <time.h>	/* nanosleep() */
static struct timespec t={0,100*1000*1000}; /* 100 mili seconds */

int oid;
XImage *image;
Display *dpy=0;
int window,scr;
int width,height;
char *tex_image=NULL,*otex_image=NULL,*img1,*img2;
int get_shift(unsigned long t)
{
	int i=0;
	while (t)
	{
		t>>=1;
		i++;
	}
	return(i);
}
void mainloop()
{
	int x,y;
	int rs,gs,bs;
	unsigned long d;
	int bpp;
	char *swap_timg;
	int last_change,start_change;
	image = XGetImage(dpy,window,0,0,width,height,AllPlanes,ZPixmap);
	if (image->format==ZPixmap)
	{
		printf("Ximage: %dx%d, format %d (%d), bpp: %d, depth %d, pad %d\n",image->width,image->height,image->format,ZPixmap,
					image->bits_per_pixel,image->depth,image->bitmap_pad);
		rs=get_shift(image->red_mask)-8;
		gs=get_shift(image->green_mask)-8;
		bs=get_shift(image->blue_mask)-8;

		bpp=(image->bits_per_pixel/8);
		/* rgb is not bgr */
		rs=rs;
		gs=gs-8;
		bs=bs-16;
		printf("Ximage: rgb: %d|%d|%d\n",	rs,gs,bs);;
/*		printf("red: size %d, offset %d\n",rs,roff);
		printf("green: size %d, offset %d\n",gs,goff);
		printf("blue: size %d, offset %d\n",bs,boff);
		printf("bits per pixel:%d\n",bpp);*/
		last_change=-1;
		start_change=-1;
		for (y=0;y<height;y++)
		{
			
			for (x=0;x<width;x++)
			{
				d=*((unsigned long *)(image->data+(y*width+x)*bpp));
				((unsigned long *)tex_image)[(y*width+x)]=
						(rs>0?((d&image->red_mask)>>rs):	((d&image->red_mask)<<-rs))|
						(gs>0?((d&image->green_mask)>>gs):	((d&image->green_mask)<<-gs))|
						(bs>0?((d&image->blue_mask)>>bs):	((d&image->blue_mask)<<-bs))|
						255<<24;
				if (((unsigned long *)tex_image)[(y*width+x)]!=
					((unsigned long *)otex_image)[(y*width+x)])
					last_change=y;
			}
			if (last_change!=-1)
			{
				if (start_change==-1)
				{
					start_change=y;
/* 					printf("setting start_change to %d\n",start_change); */
				}
				if (last_change!=y)
				{	 /*  last change is already over, post it! */
/*					printf("[%d to %d]",start_change,last_change);*/
					s3d_load_texture(oid,0,0,start_change,width,last_change-start_change+1,(unsigned char *)tex_image+start_change*width*4);
					start_change=-1;
					last_change=-1;
				}
			}
		}
		 /*  posting the last bit, maybe */
		if (last_change!=-1)
		{
/*			printf("last one: [%d-%d]",start_change,last_change);*/
			s3d_load_texture(oid,0,0,start_change,width,last_change-start_change,(unsigned char *)tex_image+start_change*width*4);
		}
/* 		s3d_load_texture(oid,0,0,0,width,height,tex_image); */
		 /*  swap images */
		swap_timg=tex_image;
		tex_image=otex_image;
		otex_image=swap_timg;
	}
	XDestroyImage(image);

	nanosleep(&t,NULL); 
}
void keypress(struct s3d_evt *event)
{
	int key;
	int kc;
	key=*((unsigned short *)event->buf);
	printf("received key %d ",key);
    kc = XKeysymToKeycode(dpy, key);
	if (kc==0) 
	{
	    kc = XKeysymToKeycode(dpy, 0xFF00+ key);
		printf(" (%04x) ",0xFF00+key);
	}
	if (kc==0)
	{
		if (key==8)
		{
			kc=22; printf("!backspace!");
		}
	}
	printf("using key: %d, keycode %d (%04x)\n",key,kc,kc);
	if (kc!=0)
	    XTestFakeKeyEvent(dpy, kc, 1, 1);
/*	    XTestFakeKeyEvent(dpy, kc, 0, 1);*/

}
void mouseclick(struct s3d_evt *event)
{
	printf("not processing mouse clicks yet ... \n");
}
int main(int argc, char **argv)
{
	char *disp=NULL;
	int a,b,c,d;
	int xt;
	if (disp==NULL) disp=getenv("DISPLAY");
	if (disp==NULL) disp="";  /*  fallback */
	dpy = XOpenDisplay(disp);
	if (!dpy)
	{
		printf("couldn't open display\n");
		return(-1);
	}
	if (!s3d_init(&argc,&argv,"X11-gate"))
	{
		scr = DefaultScreen(dpy);
		window = RootWindow(dpy, scr);
		width = DisplayWidth(dpy, scr);
		height = DisplayHeight(dpy, scr);
		XLockDisplay(dpy);
		xt=XTestQueryExtension(dpy,&a,&b,&c,&d);
		XUnlockDisplay(dpy);
		if (xt)
		{
			printf("having xtest extension ...\n");
		}
		s3d_set_callback(S3D_EVENT_OBJ_CLICK,mouseclick);
		s3d_set_callback(S3D_EVENT_KEY,keypress);
		printf("screen: %dx%d\n",width,height);
		img1=malloc(width*height*4);
		img2=malloc(width*height*4);
		tex_image=img1;
		otex_image=img2;
		oid=s3d_new_object();
		s3d_push_vertex(oid,-5,-5,0);
		s3d_push_vertex(oid, 5,-5,0);
		s3d_push_vertex(oid, 5, 5,0);
		s3d_push_vertex(oid,-5, 5,0);
		s3d_push_material_a(oid,
						0.8,	0.0,	0.0	,1.0,
						1.0,	1.0,	1.0	,1.0,
						0.8,	0.0,	0.0	,1.0);
		s3d_push_polygon(oid,0,2,1,0);
		s3d_pep_polygon_tex_coord(oid, 0.0,1.0, 
									   1.0,0.0,
									   1.0,1.0);
		s3d_push_polygon(oid,0,3,2,0);
		s3d_pep_polygon_tex_coord(oid, 0.0,1.0, 
									   0.0,0.0,
									   1.0,0.0);
		s3d_push_texture(oid,width,height);
					 /*  push data on texture 0 position (0,0) */
		s3d_pep_material_texture(oid,0);	 /*  assign texture 0 to material 0 */
		s3d_flags_on(oid,S3D_OF_VISIBLE);
		s3d_mainloop(mainloop);
		free(img1);
		free(img2);
	}
	s3d_quit();
	return(0);
}
