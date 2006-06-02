/*
 * widgets.c
 * 
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
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
#include <time.h>	/* nanosleep() */

struct s3dw_widget *surface;
static struct timespec t={0,100*1000*1000}; /* 100 mili seconds */
void mainloop()
{
	nanosleep(&t,NULL); 
}
void widget_click(struct s3d_evt *evt)
{
	s3dw_click_event(evt);
/*	s3d_quit();*/
}
void okay_button(struct s3dw_widget *dummy)
{
	s3d_quit();
}

void forward_button(struct s3dw_widget *dummy)
{
	struct s3dw_widget *o;
	s3dw_widget_destroy(surface);
	surface=s3dw_surface_new("Let's go",10,7);
	s3dw_label_new(surface->data.surface,"Fast Forward!!",1,2);
	o=s3dw_button_new(surface->data.surface,"Okay",4,4);
	o->data.button->onclick=okay_button;

}
void high_button(struct s3dw_widget *dummy)
{
	struct s3dw_widget *o;
	s3dw_widget_destroy(surface);
	surface=s3dw_surface_new("Up Up'n Away!",10,7);
	s3dw_label_new(surface->data.surface,"Fly away ...",1,2);
	o=s3dw_button_new(surface->data.surface,"Okay",4,4);
	o->data.button->onclick=okay_button;

}

int main (int argc, char **argv)
{
	struct s3dw_widget *o;
	if (!s3d_init(&argc,&argv,"widgettest"))
	{
		s3d_set_callback(S3D_EVENT_OBJ_CLICK,widget_click);

		surface=s3dw_surface_new("Hello World",20,10);
		s3dw_label_new(surface->data.surface,"Where do you want to fly today?",1,2);
		o=s3dw_button_new(surface->data.surface,"Forward",1,7);
		o->data.button->onclick=forward_button;
		o=s3dw_button_new(surface->data.surface,"Into the Sky",10,7);
		o->data.button->onclick=high_button;
		s3d_mainloop(mainloop);
		s3d_quit();
	}

	return(0);
}

