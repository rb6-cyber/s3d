/*
 * textbox.c
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

static void s3dw_textbox_draw(s3dw_widget *widget)
{
	float h, w;
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
	w = widget->width - 2;
	h = widget->height - 3;
	if (widget->width < 1) return;
	/* width of the input depends on the length of the text */
	vertices[0*3+0] = 0.0;
	vertices[0*3+1] = 0.0;
	vertices[0*3+2] = 0.0;
	vertices[1*3+0] = 0.0;
	vertices[1*3+1] = -h - 2.0;
	vertices[1*3+2] = 0.0;
	vertices[2*3+0] = w + 1;
	vertices[2*3+1] = -h - 2.0;
	vertices[2*3+2] = 0.0;
	vertices[3*3+0] = w + 1;
	vertices[3*3+1] = 0.0;
	vertices[3*3+2] = 0.0;
	vertices[4*3+0] = 0.125;
	vertices[4*3+1] = -0.125;
	vertices[4*3+2] = 0.25;
	vertices[5*3+0] = 0.125;
	vertices[5*3+1] = -h - 1.875;
	vertices[5*3+2] = 0.25;
	vertices[6*3+0] = w + 0.875;
	vertices[6*3+1] = -h - 1.875;
	vertices[6*3+2] = 0.25;
	vertices[7*3+0] = w + 0.875;
	vertices[7*3+1] = -0.125;
	vertices[7*3+2] = 0.25;
	vertices[8*3+0] = 0.25;
	vertices[8*3+1] = -0.25;
	vertices[8*3+2] = 0.125;
	vertices[9*3+0] = 0.25;
	vertices[9*3+1] = -h - 1.75;
	vertices[9*3+2] = 0.125;
	vertices[10*3+0] = w + 0.75;
	vertices[10*3+1] = -h - 1.75;
	vertices[10*3+2] = 0.125;
	vertices[11*3+0] = w + 0.75;
	vertices[11*3+1] = -0.25;
	vertices[11*3+2] = 0.125;
	widget->oid = s3d_new_object();
	s3d_push_materials_a(widget->oid, widget->style->inputback_mat, 1);
	s3d_push_materials_a(widget->oid, widget->style->input_mat, 1);
	s3d_push_vertices(widget->oid, vertices, 12);
	s3d_push_polygons(widget->oid, polygons, 18);
	s3d_link(widget->oid, widget->parent->oid);
	s3d_translate(widget->oid, widget->x, -widget->y, 0);

	s3dw_textbox_drawtext(widget);

}
void s3dw_textbox_drawtext(s3dw_widget *widget)
{
	s3dw_textbox *textbox = (s3dw_textbox *)widget;
	char *text, *rest;
	char *linefeedpos;
	int i;
	int x, y;
	float width;
	textbox->n_lineoids = widget->height - 2;
	textbox->p_lineoids = (int*)malloc(textbox->n_lineoids * sizeof(int));
	width = widget->width - 1.5;
	y = -textbox->window_y;
	x = textbox->window_x;
	for (i = 0;i < textbox->n_lineoids;i++)
		textbox->p_lineoids[i] = -1;
	rest = text = strdup(textbox->text);
	while (NULL != (linefeedpos = strchr(rest, '\n'))) { /* process every line */
		linefeedpos[0] = 0;
		if ((x < (int)strlen(rest)) && ((y >= 0) && y < textbox->n_lineoids)) { /* don't bother, if it's not visible anyway */
			rest += x; /* ignore the first x chars because we've scrolled a bit right */
			while ((strlen(rest) > 0) && (s3d_strlen(rest) > width))
				rest[strlen(rest)-1] = 0; /* remove last character and try again until it fits */
			if (strlen(rest) > 0) {
				textbox->p_lineoids[y] = s3d_draw_string(rest, NULL);
				s3d_pep_materials_a(textbox->p_lineoids[y], widget->style->text_mat, 1);
				s3d_translate(textbox->p_lineoids[y], 0.5, -y - 1, 0.30);
				s3d_link(textbox->p_lineoids[y], widget->oid);

			}
		}
		rest = linefeedpos + 1;
		y += 1;
	}
	if (y < 0) textbox->window_y -= y;

	free(text);
}
void s3dw_textbox_erasetext(s3dw_widget *widget)
{
	s3dw_textbox *textbox = (s3dw_textbox *)widget;
	int i;


	if (textbox->p_lineoids != NULL) {
		for (i = 0;i < textbox->n_lineoids;i++)
			if (textbox->p_lineoids[i] != -1)
				s3d_del_object(textbox->p_lineoids[i]);
		free(textbox->p_lineoids);
		textbox->p_lineoids = NULL;
		textbox->n_lineoids = 0;

	}
}
/* show the textbox */
void s3dw_textbox_show(s3dw_widget *widget)
{
	s3dw_textbox *textbox = (s3dw_textbox *)widget;
	int i;
	s3d_flags_on(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	for (i = 0;i < textbox->n_lineoids;i++)
		s3d_flags_on(textbox->p_lineoids[i], S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
}
/* hides the textbox */
void s3dw_textbox_hide(s3dw_widget *widget)
{
	s3dw_textbox *textbox = (s3dw_textbox *)widget;
	int i;
	s3d_flags_off(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	for (i = 0;i < textbox->n_lineoids;i++)
		s3d_flags_on(textbox->p_lineoids[i], S3D_OF_VISIBLE | S3D_OF_SELECTABLE);

}
static void _s3dw_textbox_scrollbar_up(s3dw_widget *widget)
{
	s3dw_textbox_scrollup((s3dw_textbox *)widget->parent);
}
static void _s3dw_textbox_scrollbar_down(s3dw_widget *widget)
{
	s3dw_textbox_scrolldown((s3dw_textbox *)widget->parent);
}
static void _s3dw_textbox_scrollbar_left(s3dw_widget *widget)
{
	s3dw_textbox_scrollleft((s3dw_textbox *)widget->parent);
}
static void _s3dw_textbox_scrollbar_right(s3dw_widget *widget)
{
	s3dw_textbox_scrollright((s3dw_textbox *)widget->parent);
}

/* create a new textbox in the surface */
s3dw_textbox *s3dw_textbox_new(const s3dw_surface *surface, const char *text, float posx, float posy, float width, float height)
{
	s3dw_textbox *textbox;
	s3dw_widget *widget;
	textbox = (s3dw_textbox *)malloc(sizeof(s3dw_textbox));
	widget = s3dw_widget_new((s3dw_widget *)textbox);
	widget->type = S3DW_TTEXTBOX;
	widget->x = posx;
	widget->y = posy;
	widget->width = width;
	widget->height = height;
	textbox->window_x = 0;
	textbox->window_y = 0;
	textbox->p_lineoids = NULL;
	textbox->n_lineoids = 0;
	textbox->text = strdup(text);
	textbox->onclick = s3dw_nothing;
	s3dw_widget_append((s3dw_widget *)surface, widget);  /* append first so the scrollbars inherit the style */
	s3dw_textbox_draw(widget);
	textbox->scroll_horizontal = s3dw_scrollbar_new(widget, S3DW_SBAR_HORI,  0, widget->height - 1, widget->width - 1);
	textbox->scroll_vertical = s3dw_scrollbar_new(widget, S3DW_SBAR_VERT,  widget->width - 1, 0, widget->height - 1);
	textbox->scroll_horizontal->lonclick = _s3dw_textbox_scrollbar_left;
	textbox->scroll_horizontal->ronclick = _s3dw_textbox_scrollbar_right;
	textbox->scroll_vertical->lonclick = _s3dw_textbox_scrollbar_up;
	textbox->scroll_vertical->ronclick = _s3dw_textbox_scrollbar_down;

	return(textbox);
}
static void s3dw_textbox_redraw(s3dw_widget *widget)
{
	s3dw_textbox_erasetext(widget);
	s3dw_textbox_drawtext(widget);
	if (widget->flags&S3DW_ONSCREEN)
		s3dw_textbox_show(widget);
}

void s3dw_textbox_scrollup(s3dw_textbox *textbox)
{
	if (textbox->window_y > 0)
		textbox->window_y--;
	s3dw_textbox_redraw(S3DWIDGET(textbox));
}
void s3dw_textbox_scrolldown(s3dw_textbox *textbox)
{
	textbox->window_y++;
	s3dw_textbox_redraw(S3DWIDGET(textbox));

}
void s3dw_textbox_scrollleft(s3dw_textbox *textbox)
{
	if (textbox->window_x > 0)
		textbox->window_x--;
	s3dw_textbox_redraw(S3DWIDGET(textbox));
}
void s3dw_textbox_scrollright(s3dw_textbox *textbox)
{
	textbox->window_x++;
	s3dw_textbox_redraw(S3DWIDGET(textbox));
}
void s3dw_textbox_scrollto(s3dw_textbox *textbox, int x, int y)
{
	s3dw_widget *widget = (s3dw_widget *)textbox;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	textbox->window_x = x;
	textbox->window_y = y;
	s3dw_textbox_redraw(widget);
}

void s3dw_textbox_change_text(s3dw_textbox *textbox, const char *text)
{
	s3dw_widget *widget = (s3dw_widget *)textbox;
	/* redraw the text ... */
	free(textbox->text);
	textbox->text = strdup(text);
	s3dw_textbox_redraw(widget);
}
void s3dw_textbox_erase(s3dw_widget *widget)
{
	s3dw_textbox_erasetext(widget);
	s3d_del_object(widget->oid);
}

/* destroy the textbox */
void s3dw_textbox_destroy(s3dw_widget *widget)
{
	s3dw_textbox *textbox = (s3dw_textbox *)widget;
	s3dw_textbox_erase(widget);
	free(textbox->text);
	free(textbox);
}
/* handle key events */
int s3dw_textbox_event_key(s3dw_widget *S3DUNUSED(widget), struct s3d_key_event *S3DUNUSED(keys))
{
	return(0);
}
/* handle click events */
int s3dw_textbox_event_click(s3dw_widget *widget, uint32_t oid)
{
	s3dw_textbox *textbox = (s3dw_textbox *)widget;
	if (widget->oid == oid) {
		textbox->onclick(widget);
		return(1);
	}
	return(0);
}
