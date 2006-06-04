/*
 * root.c
 *
 * Copyright (C) 2006 Simon Wunderlich <dotslash@packetmixer.de>
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
static s3dw_widget *root;

/* just destroy the widget */
void s3dw_root_destroy(s3dw_widget *widget)
{
	free(widget);
}
/*  do plain nothing. interesting, isn't it? ;) */
void s3dw_nothing(s3dw_widget *widget)
{
}
/* dummy handler */
int s3dw_root_event_click(s3dw_widget *widget,unsigned long oid)
{
	return(0);
}
/* dummy handler */
int s3dw_root_event_key(s3dw_widget *widget,struct s3d_key_event *keys)
{
	return(0);
}
/* get the root .... if it's NULL, the lib is not initialized, so do this too ... */
s3dw_widget *s3dw_getroot()
{
	if (root==NULL)
	{
		root=(s3dw_widget *)malloc(sizeof(s3dw_widget));
		root=s3dw_widget_new(root);
		root->type=S3DW_TROOT;
		root->oid=s3d_new_object();
		root->style=&def_style;
		root->flags=S3DW_VISIBLE|S3DW_ACTIVE;
		/* setup callback tables */
		s3dwcb_show[S3DW_TROOT]=		s3dw_nothing;
		s3dwcb_show[S3DW_TSURFACE]=		s3dw_surface_show;
		s3dwcb_show[S3DW_TBUTTON]=		s3dw_button_show;
		s3dwcb_show[S3DW_TLABEL]=		s3dw_label_show;
		s3dwcb_show[S3DW_TINPUT]=		s3dw_input_show;
		s3dwcb_hide[S3DW_TROOT]=		s3dw_nothing;
		s3dwcb_hide[S3DW_TSURFACE]=		s3dw_surface_hide;
		s3dwcb_hide[S3DW_TBUTTON]=		s3dw_button_hide;
		s3dwcb_hide[S3DW_TLABEL]=		s3dw_label_hide;
		s3dwcb_hide[S3DW_TINPUT]=		s3dw_input_hide;
		
		s3dwcb_destroy[S3DW_TROOT]=		s3dw_root_destroy;
		s3dwcb_destroy[S3DW_TSURFACE]=	s3dw_surface_destroy;
		s3dwcb_destroy[S3DW_TBUTTON]=	s3dw_button_destroy;
		s3dwcb_destroy[S3DW_TLABEL]=	s3dw_label_destroy;
		s3dwcb_destroy[S3DW_TINPUT]=	s3dw_input_destroy;

		s3dwcb_click[S3DW_TROOT]=		s3dw_root_event_click;
		s3dwcb_click[S3DW_TSURFACE]=	s3dw_surface_event_click;
		s3dwcb_click[S3DW_TBUTTON]=		s3dw_button_event_click;
		s3dwcb_click[S3DW_TLABEL]=		s3dw_label_event_click;
		s3dwcb_click[S3DW_TINPUT]=		s3dw_input_event_click;

		s3dwcb_key[S3DW_TROOT]=			s3dw_root_event_key;
		s3dwcb_key[S3DW_TSURFACE]=		s3dw_surface_event_key;
		s3dwcb_key[S3DW_TBUTTON]=		s3dw_button_event_key;
		s3dwcb_key[S3DW_TLABEL]=		s3dw_label_event_key;
		s3dwcb_key[S3DW_TINPUT]=		s3dw_input_event_key;

	} 
	return root;
}

