/*
 * input.c
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
#include <s3d_keysym.h>
#include <s3dw.h>
#include <s3dw_int.h>
#include <stdlib.h> /* malloc() */
#include <string.h> /* strdup(),strlen() */
#include <ctype.h> /* isprint */

uint32_t s3dw_input_draw_string(s3dw_widget *widget)
{
	s3dw_input *input = (s3dw_input *)widget;
	uint32_t oid_text;
	int i;
	float tlen;
	if (widget->width < 1) return(-1);
	i = 0;
	while (s3d_strlen(input->text + i) > (widget->width - 1)) i++;
	oid_text = s3d_draw_string(input->text + i, &tlen);
	s3d_pep_materials_a(oid_text, widget->style->text_mat, 1);
	s3d_translate(oid_text, 0.5, -1.5, 0.30);
	s3d_link(oid_text, widget->oid);
	return (oid_text);
}
void s3dw_input_draw(s3dw_widget *widget)
{
	s3dw_input *input = (s3dw_input *)widget;
	float length;
	float vertices[12*3];
	uint32_t polygons[18*4] = {
		0, 4, 5, 1,
		0, 5, 1, 1,
		1, 5, 6, 1,
		1, 6, 2, 1,
		2, 6, 7, 1,
		2, 7, 3, 1,
		3, 7, 4, 1,
		3, 4, 0, 1,

		4, 8, 9, 1,
		4, 9, 5, 1,
		5, 9, 10, 1,
		5, 10, 6, 1,
		6, 10, 11, 1,
		6, 11, 7, 1,
		7, 11, 8, 1,
		7, 8, 4, 1,


		8, 11, 10, 0,
		8, 10, 9, 0
	};
	length = widget->width - 1;
	if (widget->width < 1) return;
	widget->height = 2;
	/* width of the input depends on the length of the text */
	vertices[0*3+0] = 0.0;
	vertices[0*3+1] = 0.0;
	vertices[0*3+2] = 0.0;
	vertices[1*3+0] = 0.0;
	vertices[1*3+1] = -2.0;
	vertices[1*3+2] = 0.0;
	vertices[2*3+0] = length + 1;
	vertices[2*3+1] = -2.0;
	vertices[2*3+2] = 0.0;
	vertices[3*3+0] = length + 1;
	vertices[3*3+1] = 0.0;
	vertices[3*3+2] = 0.0;
	vertices[4*3+0] = 0.125;
	vertices[4*3+1] = -0.125;
	vertices[4*3+2] = 0.25;
	vertices[5*3+0] = 0.125;
	vertices[5*3+1] = -1.875;
	vertices[5*3+2] = 0.25;
	vertices[6*3+0] = length + 0.875;
	vertices[6*3+1] = -1.875;
	vertices[6*3+2] = 0.25;
	vertices[7*3+0] = length + 0.875;
	vertices[7*3+1] = -0.125;
	vertices[7*3+2] = 0.25;
	vertices[8*3+0] = 0.25;
	vertices[8*3+1] = -0.25;
	vertices[8*3+2] = 0.125;
	vertices[9*3+0] = 0.25;
	vertices[9*3+1] = -1.75;
	vertices[9*3+2] = 0.125;
	vertices[10*3+0] = length + 0.75;
	vertices[10*3+1] = -1.75;
	vertices[10*3+2] = 0.125;
	vertices[11*3+0] = length + 0.75;
	vertices[11*3+1] = -0.25;
	vertices[11*3+2] = 0.125;
	widget->oid = s3d_new_object();
	s3d_push_materials_a(widget->oid, widget->style->inputback_mat, 1);
	s3d_push_materials_a(widget->oid, widget->style->input_mat, 1);
	s3d_push_vertices(widget->oid, vertices, 12);
	s3d_push_polygons(widget->oid, polygons, 18);
	s3d_link(widget->oid, widget->parent->oid);
	s3d_translate(widget->oid, widget->x, -widget->y, 0);

	input->oid_text = s3dw_input_draw_string(widget);
}
/* show the input */
void s3dw_input_show(s3dw_widget *widget)
{
	s3dw_input *input = (s3dw_input *)widget;
	s3d_flags_on(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(input->oid_text, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
}
/* hides the input */
void s3dw_input_hide(s3dw_widget *widget)
{
	s3dw_input *input = (s3dw_input *)widget;
	s3d_flags_off(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_off(input->oid_text, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
}
/* create a new input in the surface */
s3dw_input *s3dw_input_new(const s3dw_surface *surface, float width, float posx, float posy)
{
	s3dw_input *input;
	s3dw_widget *widget;
	input = (s3dw_input *)malloc(sizeof(s3dw_input));
	input->text = strdup("");
	input->onclick = s3dw_nothing;
	input->onedit = s3dw_nothing;
	widget = s3dw_widget_new((s3dw_widget *)input);
	widget->type = S3DW_TINPUT;
	widget->x = posx;
	widget->y = posy;
	widget->width = width;
	widget->height = 2;

	s3dw_widget_append((s3dw_widget *)surface, widget);
	s3dw_input_draw(widget);
	return(input);
}
void s3dw_input_erase(s3dw_widget *widget)
{
	s3dw_input *input = (s3dw_input *)widget;
	s3d_del_object(input->oid_text);
	s3d_del_object(widget->oid);

}
/* destroy the input */
void s3dw_input_destroy(s3dw_widget *widget)
{
	s3dw_input *input = (s3dw_input *)widget;
	s3dw_input_erase(widget);
	free(input->text);
	free(input);
}
/* changes the text of the input */
void s3dw_input_change_text(s3dw_input *input, const char *text)
{
	s3dw_widget *widget = (s3dw_widget *)input;
	uint32_t oid_text;
	/* redraw the text ... */
	free(input->text);
	input->text = strdup(text);
	oid_text = s3dw_input_draw_string(widget);
	if (widget->flags&S3DW_ONSCREEN)
		s3d_flags_on(oid_text, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_del_object(input->oid_text);
	input->oid_text = oid_text;
}
/* handle key events */
int s3dw_input_event_key(s3dw_widget *widget, struct s3d_key_event *keys)
{
	s3dw_input *input = (s3dw_input *)widget;
	char *newtext;
	char key = keys->unicode; /* unicode support so far ... :/ */
	int len;
	s3dprintf(MED, "edit field got key %d!!", key);
	switch (keys->keysym) {
	case S3DK_BACKSPACE:
		if ((len = strlen(input->text)) > 0) {
			newtext = (char *)malloc(len + 0); /* +1 for the terminating byte, -1 for the deleted character */
			strncpy(newtext, input->text, len);
			newtext[len-1] = 0;
			s3dw_input_change_text(input, newtext);
			free(newtext);
			return(1);
		}
		break;
	default:
		if (isprint(key)) {
			len = strlen(input->text);
			newtext = (char *)malloc(len + 2); /* +1 for the terminating byte, +1 for the new character */
			strcpy(newtext, input->text);
			newtext[len] = key;
			newtext[len+1] = 0;
			s3dw_input_change_text(input, newtext);
			free(newtext);
			return(1);
		}
	}

	return(0);
}


int s3dw_input_event_click(s3dw_widget *widget, uint32_t oid)
{
	s3dw_input *input = (s3dw_input *)widget;
	if ((input->oid_text == oid) || (widget->oid == oid)) {
		s3dw_focus(widget);
		input->onclick(widget);
		return(1);
	}
	return(0);
}
char *s3dw_input_gettext(s3dw_input *input)
{
	return(strdup(input->text));
}
