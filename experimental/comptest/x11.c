/*
 * x11.c
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
#include <stdio.h>		/* printf() */
#include <string.h>		/* memcpy() */
#include <stdlib.h>		/* free() */
#include <X11/Xproto.h>	/* X_* request defines */

#define XCOMPOSITE_VERSION_0_2 200

struct extension {
	int event, error, request;
};
static struct extension    xrender, xcomposite, xdamage, xfixes;
Display       *dpy;
static char    *display = NULL;

int print_event(Display *COMPUNUSED(dpy), XEvent *event)
{
	char *name = "unknown";
	switch (event->type & 0x7f) {
	case CreateNotify:		name = "Create";			break;
	case DestroyNotify:		name = "Destroy";			break;
	case Expose:			name = "Expose";			break;
	case MapNotify:			name = "Map";				break;
	case UnmapNotify:		name = "Unmap";				break;
	case ReparentNotify:	name = "Reparent";			break;
	case CirculateNotify:	name = "Circulate";			break;
	case PropertyNotify:	name = "PropertyNotify";	return(0);	break;
	}
	if (event->type == xdamage.event + XDamageNotify) {
		name = "Damage";
		return(0); /* don't report this. */
	} else if (event->type == ConfigureNotify) {
		name = "Configure";
	}

	printf("Event: %s (%d)\n", name, event->type);
	return(0);
}

int error(Display *COMPUNUSED(dpy), XErrorEvent *event)
{
	char *name;
	char buf[256];
	char buf_req[256];
	char *req;

	XGetErrorText(dpy, event->error_code, buf, 256);
	name = buf;

	switch (event->error_code - xfixes.error) {
	case BadRegion:		name = "BadRegion";		break;
	default:			break;
	}
	switch (event->error_code - xdamage.error) {
	case BadDamage:		name = "BadDamage";		break;
	default:			break;
	}
	switch (event->error_code - xrender.error) {
	case BadPictFormat:	name = "BadPictFormat";		break;
	case BadPicture:	name = "BadPicture";		break;
	case BadPictOp:		name = "BadPictOp";			break;
	case BadGlyphSet:	name = "BadGlyphSet";		break;
	case BadGlyph:		name = "BadGlyph";			break;
	default:			break;
	}

	/* request code */
	sprintf(buf_req, "unknown (request %d, minor %d)", event->request_code, event->minor_code);
	req = buf_req;

	switch (event->request_code) {
	case X_GetWindowAttributes:	req = "XGetWindowAttributes()";	break;
	case X_GetImage:			req = "XGetImage()";			break;
	case X_QueryTree:			req = "XQueryTree()";			break;
	case X_GetProperty:			req = "XGetProperty()";			break;
	default:				break;
	}
	if (event->request_code == xcomposite.request) {
		switch (event->minor_code) {
		case X_CompositeRedirectWindow:			req = "XCompositeRedirectWindow()";			break;
		case X_CompositeNameWindowPixmap:		req = "XCompositeNameWindowPixmap()";		break;
		default:				
			sprintf(buf_req, "XComposite?? unknown (request %d, minor %d)", event->request_code, event->minor_code);
			req = buf_req;
			break;
		}
	}

	if (event->request_code == xdamage.request) {
		switch (event->minor_code) {
		case X_DamageAdd:			req = "XDamageAdd()";			break;
		case X_DamageCreate:		req = "XDamageCreate()";		break;
		case X_DamageDestroy:		req = "XDamageDestroy()";		break;
		case X_DamageSubtract:		req = "XDamageSubtract()";		break;
		default:				
			sprintf(buf_req, "XDamage?? unknown (request %d, minor %d)", event->request_code, event->minor_code);
			req = buf_req;
			break;
		}
	}

	printf("ERROR at request %s [%d]: %s\n", req, event->error_code, name);
	return(0);
}

void print_properties(Window win)
{
	Atom *p;
	int num, j;
	char *aname;
	Atom type;
	int format;
	unsigned long nitems, bytes_after;
	unsigned char *ret = NULL;

	p = XListProperties(dpy, win, &num);
	printf("found %d properties for window %d\n", num, (int)win);
	for (j = 0; j < num; j++) {
		aname = XGetAtomName(dpy, p[j]);
		if (aname) {
			if(Success == XGetWindowProperty(dpy, win, XInternAtom(dpy, aname, False),
						0L, ~0L, False, XA_STRING,
						&type, &format, &nitems,
						&bytes_after, &ret))
			{
/*				printf("format = %d, nitems = %d, bytes_after = %d\n", format, nitems, bytes_after);*/
				printf("%s = %s\n", aname, ret);
				XFree(ret);
			}
			XFree(aname);
		} else printf("NULL\n");
	}
	XFree(p);
}
char *x11_get_prop(Window win, char *prop) 
{
	Atom type;
	int format;
	unsigned long nitems, bytes_after;
	unsigned char *reqret = NULL;
	char *ret = NULL;


	if (Success == XGetWindowProperty(dpy, win, XInternAtom(dpy, prop, False),
		0L, ~0L, False, /*XA_STRING*/ AnyPropertyType,
		&type, &format, &nitems,
		&bytes_after, &reqret)) {
		if (reqret != NULL) {
			ret = strdup((char *)reqret);
			XFree(reqret);
		} 
	}
	return(ret);
}
char *x11_get_name(Window win) 
{
	unsigned int nchildren;
	Window *children;
	Window root, parent;
	char *role, *name;
	int j;

	role = x11_get_prop(win, "WM_WINDOW_ROLE");
	if (role != NULL) {
		if (strcmp(role, "decoration widget") == 0){
			free(role);
			return(NULL);
		}
		free(role);
	}
	name = x11_get_prop(win, "WM_NAME");
	if (name != NULL) {
		if (strcmp(name, "S3D") == 0) {
			printf("setting s3d to always on top\n");
			x11_always_on_top(win);
		}
		return(name);
	}

	if (XQueryTree(dpy, win, &root, &parent, &children, &nchildren) == 0)
		return(NULL);

	for (j = 0; j < (int)nchildren; j++) {
		name = x11_get_name(children[j]);
		if (name != NULL)
			break;
	}
	if (children != NULL)
		XFree(children);
	return(name);

}

/* set always on top property to s3d server */
void x11_always_on_top(Window win)
{
	Atom net_wm_state_stays_on_top, net_wm_state;
	XClientMessageEvent xev;
	int scr_no;

	net_wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
	if (net_wm_state == None)
		fprintf(stderr, "No _NET_WM_STATE\n");

	net_wm_state_stays_on_top = XInternAtom(dpy, "_NET_WM_STATE_STAYS_ON_TOP", 0);
	if (net_wm_state_stays_on_top == None)
		fprintf(stderr, "No _NET_WM_STATE_STAYS_ON_TOP\n");

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.message_type = net_wm_state;
	xev.display = dpy;
	xev.window = win;
	xev.format = 32;
	xev.data.l[0] = 1;
	xev.data.l[1] = net_wm_state_stays_on_top;
	xev.data.l[2] = 0;

	for (scr_no = 0; scr_no < ScreenCount(dpy); scr_no++) {
		if (!XSendEvent(dpy, RootWindow(dpy, scr_no), False, SubstructureRedirectMask, (XEvent *) & xev))
			fprintf(stderr, "Error XSendEvent() on type of event: _NET_WM_STATE\n");
	}
	XFlush(dpy);
}


int xinit(void)
{
	int composite_major, composite_minor;

	dpy = XOpenDisplay(display);

	if (!dpy) {
		fprintf(stderr, "Can't open display\n");
		return(1);
	}
	if (!XQueryExtension (dpy, COMPOSITE_NAME, &xcomposite.request, &xcomposite.event, &xcomposite.error)) {
		fprintf(stderr, "No composite extension\n");
	    return(1);
	}
	if (!XQueryExtension (dpy, RENDER_NAME, &xrender.request, &xrender.event, &xrender.error)) {
		fprintf(stderr, "No render extension\n");
	    return(1);
	}
	if (!XQueryExtension (dpy, DAMAGE_NAME, &xdamage.request, &xdamage.event, &xdamage.error)) {
		fprintf(stderr, "No damage extension\n");
	    return(1);
	}
	if (!XQueryExtension (dpy, XFIXES_NAME, &xfixes.request, &xfixes.event, &xfixes.error)) {
		fprintf(stderr, "No fixes extension\n");
	    return(1);
	}


	if (!XCompositeQueryVersion(dpy, &composite_major, &composite_minor)) {
		fprintf(stderr, "Could not check composite version\n");
		return(1);
	}
	if (XCompositeVersion() < XCOMPOSITE_VERSION_0_2) {
		fprintf(stderr, "Could not find composite version 0.2 or better\n");
		return(1);
	}

	XSetErrorHandler(error);
	return(0);
}

/* find biggest bounding rect of the two given rects, write it into dest */
static void merge_rect( XRectangle *dest, XRectangle *src) {
	int max;
	if (dest->width == 0 || dest->height == 0) {
		memcpy(dest, src, sizeof(*dest));
		return;
	}
	max = MAX(dest->width + dest->x, src->width + src->x);
	dest->x = MIN(dest->x, src->x);
	dest->width = max - dest->x;

	max = MAX(dest->height + dest->y, src->height + src->y);
	dest->y = MIN(dest->y, src->y);
	dest->height = max - dest->y;
}

void event(void)
{
	XEvent event;
	struct window *window;
	int d;

	while (XPending(dpy)) {
		XNextEvent(dpy, &event);
		print_event(dpy, &event);
		switch (event.type - xdamage.event) {
		case XDamageNotify:{
			XDamageNotifyEvent *e = (XDamageNotifyEvent*) & event;
			/*   printf("window = %d, geometry = %d:%d (at %d:%d), area = %d:%d (at %d:%d)\n",
			          (int)e->drawable, e->geometry.width, e->geometry.height, e->geometry.x, e->geometry.y,
			          e->area.width, e->area.height, e->area.x, e->area.y);*/
			XDamageSubtract(dpy, e->damage, None, None);
			window = window_find(e->drawable);
			if (window != NULL) {
				window->content_update_needed = 1;
				merge_rect(&window->content_update, &e->area);
			}
			break;
		   }

		}
		switch (event.type) {
		case ConfigureNotify:{
			XConfigureEvent *e = &event.xconfigure;
			window = window_find(e->window);
			if (window != NULL) {
				/*    printf("Configure: window = %d, geometry = %d:%d (at %d:%d)\n",
				           (int)e->window, e->width, e->height, e->x, e->y);*/
				window_restack(window, e->above);
				window->geometry_update_needed = 1;
			} else {
				printf("Configure: Could not find window to configure.\n");
			}
			break;
			 }
		case MapNotify:{
			XMapEvent *e = &event.xmap;
			window = window_find(e->window);
			if (window != NULL)
				window_map(window);
			break;
			}
		case UnmapNotify:{
			XUnmapEvent *e = &event.xunmap;
			window = window_find(e->window);
			if (window != NULL)
				window_unmap(window);
			break;
			}


		case CreateNotify:{
			XCreateWindowEvent *e = &event.xcreatewindow;
			window_add(e->display, e->window);
			break;
			}
		case DestroyNotify:{
			XDestroyWindowEvent *e = &event.xdestroywindow;
			window_remove(e->window);
			break;
		   }
		}
	}
	d = 0;
	for (window = window_head; window != NULL; window = window->next) {
		if (window->geometry_update_needed) 
			window_update_geometry(window);
		if (window->content_update_needed) 
		{
			window_update_content(window);
			d++;
		}
	}

/*	printf("%d windows updated\n", d);*/

}

void x11_addwindows(void)
{
	Window        root_return, parent_return;
	unsigned int     nchildren;
	Window       *children;
	struct window *win;
	int     i, scr_no;

	for (scr_no = 0; scr_no < ScreenCount(dpy); scr_no++) {
		XCompositeRedirectSubwindows(dpy, RootWindow(dpy, scr_no), CompositeRedirectAutomatic);
		/*   XCompositeRedirectSubwindows(dpy, RootWindow(dpy, scr_no), CompositeRedirectManual);*/
		XSelectInput(dpy, RootWindow(dpy, scr_no),
					 SubstructureNotifyMask | ExposureMask | StructureNotifyMask | PropertyChangeMask);
	
		XQueryTree(dpy, RootWindow(dpy, scr_no), &root_return, &parent_return, &children, &nchildren);
		for (i = 0; i < (int)nchildren; i++) {
			win = window_add(dpy, children[i]);
			if (win != NULL && win->attr.map_state != IsUnmapped)
				window_map(win);

		}
		XFree(children);
	}
}
