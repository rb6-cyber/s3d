/*
 * scrollbar.c
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

static void s3dw_scrollbar_draw(s3dw_widget *widget)
{
	s3dw_scrollbar *scrollbar = (s3dw_scrollbar *)widget;
	float back_vertices[3*32];
	int i, j;
	unsigned int back_polygons[44*4] = {
		/* box outside */
		0, 4, 5, 0,       0, 5, 1, 0,
		1, 5, 6, 0,       1, 6, 2, 0,
		3, 2, 7, 0,       7, 2, 6, 0,
		3, 7, 4, 0,       3, 4, 0, 0,

		/* sides of big box */
		12, 13, 9, 0,       12, 9, 8, 0,
		14, 15, 11, 0,       14, 11, 10, 0,

		/* inlay box */
		9, 16, 17, 0,       9, 17, 10, 0,
		17, 18, 14, 0,       17, 14, 10, 0,
		13, 14, 18, 0,       13, 18, 19, 0,
		13, 19, 16, 0,       13, 16, 9, 0,

		/* inlay box back */
		19, 18, 17, 0,       19, 17, 16, 0,

		/* up arrow background surface */
		7, 6, 15, 0,
		7, 15, 14, 0,
		7, 14, 13, 0,
		7, 13, 12, 0,

		/* up arrow face */
		20, 23, 24, 0,       20, 24, 21, 0,
		21, 24, 25, 0,       21, 25, 22, 0,
		20, 22, 25, 0,       20, 25, 23, 0,
		23, 25, 24, 0,

		/* down arrow background surface */
		4, 8, 9, 0,
		4, 9, 10, 0,
		4, 10, 11, 0,
		4, 11, 5, 0,

		/* down arrow face */
		26, 27, 31, 0,       26, 31, 30, 0,
		31, 27, 28, 0,       31, 28, 29, 0,
		26, 30, 29, 0,       26, 29, 28, 0,
		30, 31, 29, 0

	};
	unsigned int bar_polygons[14*4] = {
		/* front */
		0, 4, 5, 0,       0, 5, 1, 0,
		1, 5, 6, 0,       1, 6, 2, 0,
		3, 2, 6, 0,       3, 6, 7, 0,
		3, 7, 4, 0,       3, 4, 0, 0,

		7, 6, 4, 0,       4, 6, 5, 0,

		/* back, only visible sides */
		2, 3, 11, 0,       2, 11, 10, 0,
		1, 9, 8, 0,       1, 8, 0, 0
	};
	float bar_vertices[12*3];
	float w, h;
	float temp;


	w = (scrollbar->type == S3DW_SBAR_VERT) ? widget->width : widget->height;
	h = (scrollbar->type == S3DW_SBAR_VERT) ? widget->height : widget->width;
	/* outside/big  box */
	back_vertices[0*3+0] = w * 0.0;
	back_vertices[0*3+1] = 0.0 - h;
	back_vertices[0*3+2] = 0.0;
	back_vertices[1*3+0] = w * 1.0;
	back_vertices[1*3+1] = 0.0 - h;
	back_vertices[1*3+2] = 0.0;
	back_vertices[2*3+0] = w * 1.0;
	back_vertices[2*3+1] = 0.0;
	back_vertices[2*3+2] = 0.0;
	back_vertices[3*3+0] = w * 0.0;
	back_vertices[3*3+1] = 0.0;
	back_vertices[3*3+2] = 0.0;
	back_vertices[4*3+0] = w * 0.125;
	back_vertices[4*3+1] = w * 0.125 - h;
	back_vertices[4*3+2] = 0.25;
	back_vertices[5*3+0] = w * 0.875;
	back_vertices[5*3+1] = w * 0.125 - h;
	back_vertices[5*3+2] = 0.25;
	back_vertices[6*3+0] = w * 0.875;
	back_vertices[6*3+1] = w * -0.125;
	back_vertices[6*3+2] = 0.25;
	back_vertices[7*3+0] = w * 0.125;
	back_vertices[7*3+1] = w * -0.125;
	back_vertices[7*3+2] = 0.25;

	/* downside inlay */
	back_vertices[8*3+0] = w * 0.125;
	back_vertices[8*3+1] = w - h;
	back_vertices[8*3+2] = 0.25;
	back_vertices[9*3+0] = w * 0.25;
	back_vertices[9*3+1] = w - h;
	back_vertices[9*3+2] = 0.25;
	back_vertices[10*3+0] = w * 0.75;
	back_vertices[10*3+1] = w - h;
	back_vertices[10*3+2] = 0.25;
	back_vertices[11*3+0] = w * 0.875;
	back_vertices[11*3+1] = w - h;
	back_vertices[11*3+2] = 0.25;

	/* upside inlay */
	back_vertices[12*3+0] = w * 0.125;
	back_vertices[12*3+1] = -w;
	back_vertices[12*3+2] = 0.25;
	back_vertices[13*3+0] = w * 0.25;
	back_vertices[13*3+1] = -w;
	back_vertices[13*3+2] = 0.25;
	back_vertices[14*3+0] = w * 0.75;
	back_vertices[14*3+1] = -w;
	back_vertices[14*3+2] = 0.25;
	back_vertices[15*3+0] = w * 0.875;
	back_vertices[15*3+1] = -w;
	back_vertices[15*3+2] = 0.25;

	/* inlay box */
	back_vertices[16*3+0] = w * 0.25;
	back_vertices[16*3+1] = w * 0.125 - h + w;
	back_vertices[16*3+2] = 0.125;
	back_vertices[17*3+0] = w * 0.75;
	back_vertices[17*3+1] = w * 0.125 - h + w;
	back_vertices[17*3+2] = 0.125;
	back_vertices[18*3+0] = w * 0.75;
	back_vertices[18*3+1] = w * -0.125 - w;
	back_vertices[18*3+2] = 0.125;
	back_vertices[19*3+0] = w * 0.25;
	back_vertices[19*3+1] = w * -0.125 - w;
	back_vertices[19*3+2] = 0.125;

	/* arrow up */
	back_vertices[20*3+0] = w * 0.25;
	back_vertices[20*3+1] = w * -0.875;
	back_vertices[20*3+2] = 0.25;
	back_vertices[21*3+0] = w * 0.75;
	back_vertices[21*3+1] = w * -0.875;
	back_vertices[21*3+2] = 0.25;
	back_vertices[22*3+0] = w * 0.5;
	back_vertices[22*3+1] = w * -0.25;
	back_vertices[22*3+2] = 0.25;
	back_vertices[23*3+0] = w * 0.375;
	back_vertices[23*3+1] = w * -0.75;
	back_vertices[23*3+2] = 0.375;
	back_vertices[24*3+0] = w * 0.625;
	back_vertices[24*3+1] = w * -0.75;
	back_vertices[24*3+2] = 0.375;
	back_vertices[25*3+0] = w * 0.5;
	back_vertices[25*3+1] = w * -0.375;
	back_vertices[25*3+2] = 0.375;

	/* arrow down */
	back_vertices[26*3+0] = w * 0.25;
	back_vertices[26*3+1] = -h + w * 0.875;
	back_vertices[26*3+2] = 0.25;
	back_vertices[27*3+0] = w * 0.75;
	back_vertices[27*3+1] = -h + w * 0.875;
	back_vertices[27*3+2] = 0.25;
	back_vertices[28*3+0] = w * 0.5;
	back_vertices[28*3+1] = -h + w * 0.25;
	back_vertices[28*3+2] = 0.25;
	back_vertices[29*3+0] = w * 0.375;
	back_vertices[29*3+1] = -h + w * 0.75;
	back_vertices[29*3+2] = 0.375;
	back_vertices[30*3+0] = w * 0.625;
	back_vertices[30*3+1] = -h + w * 0.75;
	back_vertices[30*3+2] = 0.375;
	back_vertices[31*3+0] = w * 0.5;
	back_vertices[31*3+1] = -h + w * 0.375;
	back_vertices[31*3+2] = 0.375;

	/* scrollbar bar poinst */
	bar_vertices[0*3+0] = w * 0.25;
	bar_vertices[0*3+1] = -(h - 3) + w * 0.125;
	bar_vertices[0*3+2] = 0.25;
	bar_vertices[1*3+0] = w * 0.75;
	bar_vertices[1*3+1] = -(h - 3) + w * 0.125;
	bar_vertices[1*3+2] = 0.25;
	bar_vertices[2*3+0] = w * 0.75;
	bar_vertices[2*3+1] = -w * 0.125;
	bar_vertices[2*3+2] = 0.25;
	bar_vertices[3*3+0] = w * 0.25;
	bar_vertices[3*3+1] = -w * 0.125;
	bar_vertices[3*3+2] = 0.25;

	bar_vertices[4*3+0] = w * 0.125;
	bar_vertices[4*3+1] = -(h - 3) + w * 0.25;
	bar_vertices[4*3+2] = 0.375;
	bar_vertices[5*3+0] = w * 0.875;
	bar_vertices[5*3+1] = -(h - 3) + w * 0.25;
	bar_vertices[5*3+2] = 0.375;
	bar_vertices[6*3+0] = w * 0.875;
	bar_vertices[6*3+1] = -w * 0.25;
	bar_vertices[6*3+2] = 0.375;
	bar_vertices[7*3+0] = w * 0.125;
	bar_vertices[7*3+1] = -w * 0.25;
	bar_vertices[7*3+2] = 0.375;

	bar_vertices[8*3+0] = w * 0.125;
	bar_vertices[8*3+1] = -(h - 3) + w * 0.25;
	bar_vertices[8*3+2] = 0.175;
	bar_vertices[9*3+0] = w * 0.875;
	bar_vertices[9*3+1] = -(h - 3) + w * 0.25;
	bar_vertices[9*3+2] = 0.175;
	bar_vertices[10*3+0] = w * 0.875;
	bar_vertices[10*3+1] = -w * 0.25;
	bar_vertices[10*3+2] = 0.175;
	bar_vertices[11*3+0] = w * 0.125;
	bar_vertices[11*3+1] = -w * 0.25;
	bar_vertices[11*3+2] = 0.175;




	if (scrollbar->type == S3DW_SBAR_HORI) { /* rotate x and y for horizontal scrollbar */
		for (i = 0;i < 32;i++) {
			temp = back_vertices[i*3];
			back_vertices[i*3] = -back_vertices[i*3+1];
			back_vertices[i*3+1] = -temp;
		}
		for (i = 0;i < 44;i++) { /* change clockwiseness */
			j = back_polygons[i*4];
			back_polygons[i*4] = back_polygons[i*4+1];
			back_polygons[i*4+1] = j;
		}
		for (i = 0;i < 12;i++) {
			temp = bar_vertices[i*3];
			bar_vertices[i*3] = -bar_vertices[i*3+1];
			bar_vertices[i*3+1] = -temp;
		}
		for (i = 0;i < 14;i++) { /* change clockwiseness */
			j = bar_polygons[i*4];
			bar_polygons[i*4] = bar_polygons[i*4+1];
			bar_polygons[i*4+1] = j;
		}
	}

	widget->oid = s3d_new_object();
	scrollbar->loid = s3d_new_object();
	scrollbar->roid = s3d_new_object();
	scrollbar->baroid = s3d_new_object();
	s3d_push_materials_a(widget->oid, widget->style->input_mat, 1);
	s3d_push_materials_a(scrollbar->loid, widget->style->input_mat, 1);
	s3d_push_materials_a(scrollbar->roid, widget->style->input_mat, 1);
	s3d_push_materials_a(scrollbar->baroid, widget->style->input_mat, 1);
	s3d_push_vertices(widget->oid, back_vertices, 20);
	s3d_push_vertices(scrollbar->loid, back_vertices, 32);
	s3d_push_vertices(scrollbar->roid, back_vertices, 32);
	s3d_push_vertices(scrollbar->baroid, bar_vertices, 12);

	s3d_push_polygons(widget->oid, back_polygons , 22);
	s3d_push_polygons(scrollbar->loid, back_polygons + 4*22, 11);
	s3d_push_polygons(scrollbar->roid, back_polygons + 4*33, 11);
	s3d_push_polygons(scrollbar->baroid, bar_polygons , 14);
	s3d_link(widget->oid, widget->parent->oid);
	s3d_link(scrollbar->loid, widget->oid);
	s3d_link(scrollbar->roid, widget->oid);
	s3d_link(scrollbar->baroid, widget->oid);
	s3d_translate(widget->oid, widget->x, -widget->y, 0);
	if (scrollbar->type == S3DW_SBAR_VERT)
		s3d_translate(scrollbar->baroid, 0, -1.5, 0);
	else
		s3d_translate(scrollbar->baroid, 1.5, 0, 0);

}
/* show the scrollbar */
void s3dw_scrollbar_show(s3dw_widget *widget)
{
	s3dw_scrollbar *scrollbar = (s3dw_scrollbar *)widget;
	s3d_flags_on(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(scrollbar->loid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(scrollbar->roid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_on(scrollbar->baroid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);

}
/* hides the scrollbar */
void s3dw_scrollbar_hide(s3dw_widget *widget)
{
	s3dw_scrollbar *scrollbar = (s3dw_scrollbar *)widget;
	s3d_flags_off(widget->oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_off(scrollbar->loid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_off(scrollbar->roid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	s3d_flags_off(scrollbar->baroid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
}

/* create a new scrollbar in the surface */
s3dw_scrollbar *s3dw_scrollbar_new(s3dw_widget *parent, int type, float posx, float posy, float length)
{
	s3dw_scrollbar *scrollbar;
	s3dw_widget *widget;
	scrollbar = (s3dw_scrollbar *)malloc(sizeof(s3dw_scrollbar));
	widget = s3dw_widget_new((s3dw_widget *)scrollbar);
	widget->type = S3DW_TSCROLLBAR;
	if ((scrollbar->type = type) == S3DW_SBAR_HORI) {
		widget->height = 1;
		widget->width = length;
	} else {
		widget->width = 1;
		widget->height = length;
	}
	widget->x = posx;
	widget->y = posy;
	widget->oid = -1;
	scrollbar->loid = -1;
	scrollbar->roid = -1;
	scrollbar->baroid = -1;
	scrollbar->lonclick = s3dw_nothing;
	scrollbar->ronclick = s3dw_nothing;
	s3dw_widget_append(parent, widget);
	s3dw_scrollbar_draw(widget);
	return(scrollbar);
}

void s3dw_scrollbar_erase(s3dw_widget *widget)
{
	s3dw_scrollbar *scrollbar = (s3dw_scrollbar *)widget;
	s3d_del_object(widget->oid);
	s3d_del_object(scrollbar->loid);
	s3d_del_object(scrollbar->roid);
	s3d_del_object(scrollbar->baroid);
}
/* destroy the scrollbar */
void s3dw_scrollbar_destroy(s3dw_widget *widget)
{
	s3dw_scrollbar *scrollbar = (s3dw_scrollbar *)widget;
	s3dw_scrollbar_erase(widget);
	free(scrollbar);
}
/* handle key events */
int s3dw_scrollbar_event_key(s3dw_widget *S3DUNUSED(widget), struct s3d_key_event *S3DUNUSED(keys))
{
	return(0);
}
/* handle click events */
int s3dw_scrollbar_event_click(s3dw_widget *widget, uint32_t oid)
{
	s3dw_scrollbar *scrollbar = (s3dw_scrollbar *)widget;
	if (scrollbar->loid == (int)oid) {
		scrollbar->lonclick(widget);
		return(1);
	}
	if (scrollbar->roid == (int)oid) {
		scrollbar->ronclick(widget);
		return(1);
	}

	return(0);
}
