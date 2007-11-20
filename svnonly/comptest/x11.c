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

#define XCOMPOSITE_VERSION_0_2 200

struct extension {
	int event, error;
};
static struct extension    xrender, xcomposite, xdamage, xfixes;
Display       *dpy;
static char    *display = NULL;

int print_event(Display *COMPUNUSED(dpy), XEvent *event)
{
	char *name = "unknown";
	switch (event->type & 0x7f) {
	case CreateNotify:
		name = "Create";
		break;
	case DestroyNotify:
		name = "Destroy";
		break;
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
	case PropertyNotify:
		name = "PropertyNotify";
		return(0);
		break;
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
	char *name = "";
	char buf[256];
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
	switch (event->error_code) {
	case BadWindow:
		name = "BadWindow";
		break;
	case BadDrawable:
		name = "BadDrawable";
		break;
	case BadMatch:
		name = "BadMatch";
		return(0);
		break;
	}
	XGetErrorText(dpy, event->error_code, buf, 256);
	printf("error %d (name: %s) request %d minor %d serial %d: %s\n",
	       event->error_code, name, event->request_code, event->minor_code, (int)event->serial, buf);
	return(0);
}


int xinit(void)
{
	int composite_major, composite_minor;

	dpy = XOpenDisplay(display);
	if (!dpy) {
		fprintf(stderr, "Can't open display\n");
		return(1);
	}
	if (!XRenderQueryExtension(dpy, &xrender.event, &xrender.error)) {
		fprintf(stderr, "No render extension\n");
		return(1);
	}

	if (!XCompositeQueryExtension(dpy, &xcomposite.event, &xcomposite.error)) {
		fprintf(stderr, "No composite extension\n");
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
		switch (event.type - xdamage.event) {
		case XDamageNotify:{
			XDamageNotifyEvent *e = (XDamageNotifyEvent*) & event;
			/*   printf("window = %d, geometry = %d:%d (at %d:%d), area = %d:%d (at %d:%d)\n",
			          (int)e->drawable, e->geometry.width, e->geometry.height, e->geometry.x, e->geometry.y,
			          e->area.width, e->area.height, e->area.x, e->area.y);*/
			XDamageSubtract(dpy, e->damage, None, None);
			window = window_find(e->drawable);
			if (window != NULL)
				window_update_content(window, e->area.x, e->area.y, e->area.width, e->area.height);
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
				window_update_geometry(window, e->x, e->y, e->width, e->height);
			} else {
				printf("Configure: Could not find window to configure.\n");
			}
			break;
			 }
		case CreateNotify:{
			XCreateWindowEvent *e = &event.xcreatewindow;
			printf("override_redirect = %d\n", (int)e->override_redirect);
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
}


