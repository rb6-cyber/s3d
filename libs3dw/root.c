/*
 * root.c
 *
 * Copyright (C) 2006-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of the s3d Widgets, a Widget Library for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3d Widgets is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * s3d Widgets is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d Widgets; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <s3d.h>
#include <s3dw.h>
#include <s3dw_int.h>
#include <stdlib.h> /* malloc() */
#include <stdio.h>   /* printf() */
s3dw_callback   s3dwcb_show[S3DW_NTYPES];
s3dw_callback   s3dwcb_hide[S3DW_NTYPES];
s3dw_callback   s3dwcb_destroy[S3DW_NTYPES];
s3dw_click_callback s3dwcb_click[S3DW_NTYPES];
s3dw_key_callback s3dwcb_key[S3DW_NTYPES];


static s3dw_widget *root = NULL;

/* just destroy the widget */
void s3dw_root_destroy(s3dw_widget *widget)
{
	free(widget);
}
/*  do plain nothing. interesting, isn't it? ;) */
void s3dw_nothing(s3dw_widget *S3DUNUSED(widget))
{
}
int s3dw_click_nothing(s3dw_widget *S3DUNUSED(widget), uint32_t S3DUNUSED(dummy))
{
	return(0);
}
int s3dw_key_nothing(s3dw_widget *S3DUNUSED(widget), struct s3d_key_event *S3DUNUSED(dummy))
{
	return(0);
}

const char *s3dw_get_type_string(int type)
{
	switch (type) {
	case S3DW_TROOT:
		return("root");
	case S3DW_TCAM:
		return("cam");
	case S3DW_TSURFACE:
		return("surface");
	case S3DW_TBUTTON:
		return("button");
	case S3DW_TLABEL:
		return("label");
	case S3DW_TINPUT:
		return("input");
	case S3DW_TTEXTBOX:
		return("textbox");
	case S3DW_TSCROLLBAR:
		return("scrollbar");
	}
	return(NULL);
}

/* get the root .... if it's NULL, the lib is not initialized, so do this too ... */
s3dw_widget* s3dw_getroot(void)
{
	if (root == NULL) {
		root = (s3dw_widget *)malloc(sizeof(s3dw_widget));
		root = s3dw_widget_new(root);
		root->type = S3DW_TROOT;
		root->oid = s3d_new_object();
		root->style = &def_style;
		root->flags = S3DW_VISIBLE | S3DW_ACTIVE;
		_s3dw_cam = (s3dw_widget *)malloc(sizeof(s3dw_widget));
		s3dw_widget_new(_s3dw_cam);
		_s3dw_cam->type = S3DW_TCAM;
		_s3dw_cam->oid = 0;
		_s3dw_cam->style = &def_style;
		_s3dw_cam->s = 10;
		_s3dw_cam->width = 1;
		_s3dw_cam->height = 0;
		_s3dw_cam->flags = S3DW_VISIBLE | S3DW_ACTIVE;

		s3dw_widget_append(root, _s3dw_cam);
		/* setup callback tables */
		s3dwcb_show[S3DW_TROOT] =  s3dw_nothing;
		s3dwcb_show[S3DW_TCAM] =   s3dw_nothing;
		s3dwcb_show[S3DW_TSURFACE] =  s3dw_surface_show;
		s3dwcb_show[S3DW_TBUTTON] =  s3dw_button_show;
		s3dwcb_show[S3DW_TLABEL] =  s3dw_label_show;
		s3dwcb_show[S3DW_TINPUT] =  s3dw_input_show;
		s3dwcb_show[S3DW_TTEXTBOX] =  s3dw_textbox_show;
		s3dwcb_show[S3DW_TSCROLLBAR] = s3dw_scrollbar_show;

		s3dwcb_hide[S3DW_TROOT] =  s3dw_nothing;
		s3dwcb_hide[S3DW_TSURFACE] =  s3dw_surface_hide;
		s3dwcb_hide[S3DW_TBUTTON] =  s3dw_button_hide;
		s3dwcb_hide[S3DW_TLABEL] =  s3dw_label_hide;
		s3dwcb_hide[S3DW_TINPUT] =  s3dw_input_hide;
		s3dwcb_hide[S3DW_TTEXTBOX] =  s3dw_textbox_hide;
		s3dwcb_hide[S3DW_TSCROLLBAR] = s3dw_scrollbar_hide;

		s3dwcb_destroy[S3DW_TROOT] =  s3dw_root_destroy;
		s3dwcb_destroy[S3DW_TCAM] =  s3dw_root_destroy;
		s3dwcb_destroy[S3DW_TSURFACE] = s3dw_surface_destroy;
		s3dwcb_destroy[S3DW_TBUTTON] = s3dw_button_destroy;
		s3dwcb_destroy[S3DW_TLABEL] = s3dw_label_destroy;
		s3dwcb_destroy[S3DW_TINPUT] = s3dw_input_destroy;
		s3dwcb_destroy[S3DW_TTEXTBOX] = s3dw_textbox_destroy;
		s3dwcb_destroy[S3DW_TSCROLLBAR] = s3dw_scrollbar_destroy;

		s3dwcb_click[S3DW_TROOT] =  s3dw_click_nothing;
		s3dwcb_click[S3DW_TCAM] =  s3dw_click_nothing;
		s3dwcb_click[S3DW_TSURFACE] = s3dw_surface_event_click;
		s3dwcb_click[S3DW_TBUTTON] =  s3dw_button_event_click;
		s3dwcb_click[S3DW_TLABEL] =  s3dw_label_event_click;
		s3dwcb_click[S3DW_TINPUT] =  s3dw_input_event_click;
		s3dwcb_click[S3DW_TTEXTBOX] = s3dw_textbox_event_click;
		s3dwcb_click[S3DW_TSCROLLBAR] = s3dw_scrollbar_event_click;

		s3dwcb_key[S3DW_TROOT] =   s3dw_key_nothing;
		s3dwcb_key[S3DW_TCAM] =   s3dw_key_nothing;
		s3dwcb_key[S3DW_TSURFACE] =  s3dw_surface_event_key;
		s3dwcb_key[S3DW_TBUTTON] =  s3dw_button_event_key;
		s3dwcb_key[S3DW_TLABEL] =  s3dw_label_event_key;
		s3dwcb_key[S3DW_TINPUT] =  s3dw_input_event_key;
		s3dwcb_key[S3DW_TTEXTBOX] =  s3dw_key_nothing;
		s3dwcb_key[S3DW_TSCROLLBAR] = s3dw_key_nothing;

	}
	return root;
}

