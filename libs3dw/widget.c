/*
 * widget.c
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
#include <string.h> /* strdup() */

struct s3dw_widget *s3dw_widget_new()
{
	struct s3dw_widget *widget=malloc(sizeof(struct s3dw_widget));
	widget->type=-1;
	widget->_x=widget->_dx=0;
	widget->_y=widget->_dy=0;
	widget->_z=widget->_dz=0;
	widget->_rx=widget->_drx=0;
	widget->_ry=widget->_dry=0;
	widget->_rz=widget->_drz=0;
	widget->_s=widget->_ds=1;
	widget->_o=NULL;
	widget->_width=0;
	widget->_height=0;
	widget->_surface=NULL;
	widget->data.surface=NULL;
	return(widget);
}
void s3dw_widget_destroy(struct s3dw_widget *widget)
{
	switch (widget->type)
		{
			case S3DW_TBUTTON:		s3dw_button_destroy(widget->data.button);			break;
			case S3DW_TSURFACE:		s3dw_surface_destroy(widget->data.surface);			break;
			case S3DW_TLABEL:		s3dw_label_destroy(widget->data.label);				break;
			case S3DW_TINPUT:		s3dw_input_destroy(widget->data.input);				break;
			default:
					dprintf(MED,"can't free this type (yet) - memory leak\n");
		}
	free(widget);
}
void s3dw_widget_event_click(struct s3dw_widget *widget, unsigned long oid)
{
	switch (widget->type)
		{
			case S3DW_TBUTTON:
					s3dw_button_event_click(widget,oid);
					break;
			case S3DW_TLABEL:
					s3dw_label_event_click(widget,oid);
					break;
			case S3DW_TINPUT:
					s3dw_input_event_click(widget,oid);
					break;

		}

}
