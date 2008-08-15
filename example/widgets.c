/*
 * widgets.c
 *
 * Copyright (C) 2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3d, a 3d network display server.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */




#include <s3d.h>
#include <s3dw.h>
#include <stdio.h>  /* NULL */
#include <time.h> /* nanosleep() */
#include <stdlib.h> /* free() */
#include <string.h> /* strlen() */
#include "example.h"

static s3dw_surface *surface;
static s3dw_input *input;
static struct timespec t = {
	0, 33*1000*1000
}; /* 33 mili seconds */
static void mainloop(void)
{
	/* keep this in your mainloop. this will do smooth animations for you ... */
	s3dw_ani_mate();
	nanosleep(&t, NULL);
}
/* you should always put the s3dw-handler in your own event handler,
 * if you want s3dw to react on clicks or keys ... and i'm sure you
 * want that ... */
static int click(struct s3d_evt *evt)
{
	return(s3dw_handle_click(evt));
}
static void key_button(s3dw_widget *button)
{
	s3dw_delete(button->parent); /* parent =surface. this means close containing window */
}
static int key(struct s3d_evt *evt)
{
	struct s3d_key_event *key = (struct s3d_key_event *)evt->buf;
	char string[8];
	s3dw_surface *miniwin;
	s3dw_button  *button;

	s3dw_handle_key(evt);
	/* okay, that's a little bit insane ... ;)
	 * we create some little windows with the actual key pressed. */

	if (key->unicode != 0) {
		miniwin = s3dw_surface_new("Key", 6, 6);
		sprintf(string, "%c", key->unicode);
		s3dw_label_new(miniwin, string, 1, 2);
		button = s3dw_button_new(miniwin, "OK", 2, 4);
		/* clicking on the button will exit ... */
		button->onclick = key_button;
		/* of couse, show it */
		s3dw_show(S3DWIDGET(miniwin));
	}
	return(0);

}

static void done_button(s3dw_widget *S3DUNUSED(dummy))
{
	s3d_quit();
}

static void okay_button(s3dw_widget *S3DUNUSED(dummy))
{
	s3dw_button *button;
	char string[32];
	char *age;

	/* get the input of the text ... before its destroyed, of course*/
	age = s3dw_input_gettext(input);

	/* delete the old surface with it subwidgets */
	s3dw_delete(S3DWIDGET(surface));

	/* and create a new one ... */
	surface = s3dw_surface_new("Ah!", 10, 7);

	/* just cutting the string if it's too long */
	if (strlen(age) > 8) age[8] = 0;

	/* assemble the string ..*/
	sprintf(string, "I see, %s!!", age);

	s3dw_label_new(surface, string, 1, 2);
	button = s3dw_button_new(surface, "Great", 4, 4);
	/* clicking on the button will exit ... */
	button->onclick = done_button;

	/* of couse, show it */
	s3dw_show(S3DWIDGET(surface));

	/* we don't need it anymore. always free strings, don't leak around */
	free(age);
}
static void no_button(s3dw_widget *S3DUNUSED(dummy))
{
	s3dw_button *button;
	s3dw_delete(S3DWIDGET(surface));
	surface = s3dw_surface_new("Well ...", 10, 7);
	s3dw_label_new(surface, "If you don't want to tell me ...", 1, 2);
	button = s3dw_button_new(surface, "Bye", 4, 4);
	/* clicking on the button will exit ... */

	button->onclick = done_button;
	/* of couse, show it */

	s3dw_show(S3DWIDGET(surface));
}
static const char *text = "okay\nn2\n3\nfooobarfooobar ...\noh no\n its too loooong\n";
int main(int argc, char **argv)
{
	s3dw_button *button;
	if (!s3d_init(&argc, &argv, "widgettest")) {
		s3d_set_callback(S3D_EVENT_OBJ_CLICK, click);
		s3d_set_callback(S3D_EVENT_KEY, key);
		s3d_set_callback(S3D_EVENT_OBJ_INFO, s3dw_object_info);
		/* this creates the "window" */
		surface = s3dw_surface_new("Hello World", 20, 20);

		/* put a label (which is simply text) at position x=1, y=2 */
		s3dw_label_new(surface, "How old are you?", 1, 2);

		/* put an input box right below. we grab the pointer because we want to focus it (need for reference) */
		input = s3dw_input_new(surface, 8, 1, 4);

		/* we want the input-field be focused on our widget */
		s3dw_focus(S3DWIDGET(input));

		/* create the okay button */
		button = s3dw_button_new(surface, "OK", 1, 7);

		/* define the callback when the button is clicked. in our case, okay_button() will handle the event */
		button->onclick = okay_button;

		/* another button  */
		button = s3dw_button_new(surface, "Won't tell you", 10, 7);

		/* we will tell him how sad we are ... */
		button->onclick = no_button;
		/* create some textbox at (1,10) widh width 18 and height 8 */
		s3dw_textbox_new(surface, text, 1, 10, 18, 8);

		/* this widget is focused (of course, it's our only one ... */
		s3dw_focus(S3DWIDGET(surface));

		/* show it. without showing, things would be boring... */
		s3dw_show(S3DWIDGET(surface));

		s3d_mainloop(mainloop);
		s3d_quit();
	}
	return(0);
}

