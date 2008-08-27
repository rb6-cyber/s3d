/*
 * label.c
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
#include <string.h> /* strdup() */

void s3dw_label_draw(s3dw_widget *widget)
{
	s3dw_label *label = (s3dw_label *)widget;
	float length;
	widget->oid = s3d_draw_string(label->text, &length);
	s3d_pep_materials_a(widget->oid, widget->style->text_mat, 1);
	s3d_link(widget->oid, widget->parent->oid);
	s3d_translate(widget->oid, widget->x, -widget->y, 0.1);
	widget->width = length + 1;
	widget->height = 2;
}
/* show the label */
void s3dw_label_show(s3dw_widget *widget)
{
	s3d_flags_on(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
}
/* hides the label */
void s3dw_label_hide(s3dw_widget *widget)
{
	s3d_flags_off(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
}
/* change label text */
void s3dw_label_change_text(s3dw_label *label, const char *text)
{
	s3dw_widget *widget = (s3dw_widget *)label;

	/* redraw the text ... */
	free(label->text);
	label->text = strdup(text);
	s3dw_label_erase(widget);
	s3dw_label_draw(widget);
	if (widget->flags&S3DW_ONSCREEN)
		s3dw_label_show(widget);
}


/* create a new label in the surface */
s3dw_label *s3dw_label_new(const s3dw_surface *surface, const char *text, float posx, float posy)
{
	s3dw_label *label;
	s3dw_widget *widget;
	label = (s3dw_label *)malloc(sizeof(s3dw_label));
	widget = s3dw_widget_new((s3dw_widget *)label);
	widget->type = S3DW_TLABEL;
	widget->x = posx;
	widget->y = posy;
	label->text = strdup(text);
	label->onclick = s3dw_nothing;
	s3dw_widget_append((s3dw_widget *)surface, widget);
	s3dw_label_draw(widget);
	return(label);
}

void s3dw_label_erase(s3dw_widget *widget)
{
	s3d_del_object(widget->oid);
}
/* destroy the label */
void s3dw_label_destroy(s3dw_widget *widget)
{
	s3dw_label *label = (s3dw_label *)widget;
	s3dw_label_erase(widget);
	free(label->text);
	free(label);
}
/* handle key events */
int s3dw_label_event_key(s3dw_widget *S3DUNUSED(widget), struct s3d_key_event *S3DUNUSED(keys))
{
	return(0);
}
/* handle click events */
int s3dw_label_event_click(s3dw_widget *widget, uint32_t oid)
{
	s3dw_label *label = (s3dw_label *)widget;
	if (widget->oid == oid) {
		label->onclick(widget);
		return(1);
	}
	return(0);
}
