/*
 * search.c
 *
 * Copyright (C) 2006-2012  Andreas Langer <an.langer@gmx.de>
 *
 * This file is part of the olsrs3d, an olsr topology visualizer for s3d.
 * See http://s3d.sourceforge.net/ for more updates.
 *
 * olsrs3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * olsrs3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsrs3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <s3d.h>
#include <s3dw.h>
#include <s3d_keysym.h>
#include <math.h>
#include <stdio.h> /* TODO can remove then no more printf needed */
#include <string.h>
#include <stdlib.h>
#include "olsrs3d.h" /* for window_error(), structs */
/* #include "structs.h"  already included by olsrs3d.h */
#include "search.h"

static s3dw_surface *_search_surface;
static s3dw_input  *_search_input;
static s3dw_widget  *_search_widget;

static struct olsr_node *_node_root = NULL;
static struct olsr_node **search_node = NULL;

static float _return_point[2][3];    /* cam position before move to the widget */
static int  _search_status = NOTHING;   /* status of search */

static void _search_node(s3dw_widget *dummy);
static void _new_search_node(s3dw_widget *dummy);
static void _abort_search(s3dw_widget *dummy);


/* public */
void follow_node(float cam_position_t[], float cam_position_r[], float rotate)
{
	float real_node_pos[3],
	      cam_target[3],
	      tmp_vec[3],
	      diff_vec[3],
	      angle;

	real_node_pos[0] = (*search_node)->pos_vec[0] * cos(rotate * M_PI / 180.0) - (*search_node)->pos_vec[2] * -sin(rotate * M_PI / 180.0);
	real_node_pos[1] = (*search_node)->pos_vec[1];
	real_node_pos[2] = (*search_node)->pos_vec[0] * -sin(rotate * M_PI / 180.0) + (*search_node)->pos_vec[2] * cos(rotate * M_PI / 180.0);

	cam_target[0] = (real_node_pos[0] + 7);
	cam_target[1] =   real_node_pos[1];
	cam_target[2] = (real_node_pos[2] + 7);

	cam_position_t[0] = (cam_position_t[0] * 4 + cam_target[0]) / 5;
	cam_position_t[1] = (cam_position_t[1] * 4 + cam_target[1]) / 5;
	cam_position_t[2] = (cam_position_t[2] * 4 + cam_target[2]) / 5;

	tmp_vec[0] =  0.0;
	tmp_vec[1] =  0.0;
	tmp_vec[2] = -1.0;

	diff_vec[0] = cam_position_t[0] - real_node_pos[0];
	diff_vec[1] = 0.0;
	diff_vec[2] = cam_position_t[2] - real_node_pos[2];
	angle = s3d_vector_angle(diff_vec, tmp_vec);
	/* angle = ( real_node_pos[0] > 0) ? ( 180 - ( 180 / M_PI * angle ) ) : ( 180 + ( 180 / M_PI * angle ) ); */
	angle = 180 - (180 / M_PI * angle);
	cam_position_r[1] = (cam_position_r[1] * 4 + angle) / 5;

	s3d_translate(0, cam_position_t[0], cam_position_t[1], cam_position_t[2]);
	s3d_rotate(0, cam_position_r[0], cam_position_r[1], cam_position_r[2]);
}
static void _abort_search_window(s3dw_widget *bwidget)
{
	s3dw_delete(bwidget->parent); /* remove the window cointaining the button */
	_search_surface = NULL;
	_search_input = NULL;
	_search_widget = NULL;
	set_search_status(NOTHING);
}
void show_search_window(void)
{
	s3dw_button *search_button, *abort_button;

	_search_surface = s3dw_surface_new("Node Search", 17, 10);
	_search_input = s3dw_input_new(_search_surface, 15, 1, 4);

	s3dw_label_new(_search_surface, "Enter the IP of the node.", 1, 2);
	s3dw_focus(S3DWIDGET(_search_input));

	search_button = s3dw_button_new(_search_surface, "Search", 11.5, 7);
	abort_button  = s3dw_button_new(_search_surface, "Abort", 1, 7);
	search_button->onclick = _new_search_node;
	abort_button->onclick = _abort_search_window;

	/* TODO calc position for ok button */

	s3dw_focus(S3DWIDGET(_search_input));
	s3dw_focus(S3DWIDGET(_search_surface));
	s3dw_show(S3DWIDGET(_search_surface));

	_search_widget = S3DWIDGET(search_button);
}

/* public */
void create_search_widget(float OLSRS3DUNUSED(x), float OLSRS3DUNUSED(y), float OLSRS3DUNUSED(z))
{
	s3dw_button *search_button, *abort_button;

	_search_surface = s3dw_surface_new("Node Search", 17, 10);
	_search_input = s3dw_input_new(_search_surface, 15, 1, 4);

	s3dw_label_new(_search_surface, "Enter the IP of the node.", 1, 2);
	s3dw_focus(S3DWIDGET(_search_input));

	search_button = s3dw_button_new(_search_surface, "Search", 11.5, 7);
	abort_button  = s3dw_button_new(_search_surface, "Abort", 1, 7);
	search_button->onclick = _search_node;
	abort_button->onclick = _abort_search;

	/* TODO calc position for ok button */

	s3dw_focus(S3DWIDGET(_search_input));
	s3dw_focus(S3DWIDGET(_search_surface));
	s3dw_show(S3DWIDGET(_search_surface));

	/* disabled for autofollowing mode */
	/*_search_widget = s3dw_getroot();
	move_search_widget( x, y, z );

	_search_widget->ary = 180;
	s3d_rotate( _search_widget->oid, _search_widget->arx, _search_widget->ary, _search_widget->arz );*/
}

/* public */
void move_search_widget(float x, float y, float z)
{
	_search_widget->x = x;
	_search_widget->y = y;
	_search_widget->z = z;
	s3dw_moveit(_search_widget);
}

/* public */
void move_to_search_widget(float cam_position_t[], float cam_position_r[])
{
	float target;

	set_search_status(WIDGET);
	cam_position_t[0] = (cam_position_t[0] * 4 + _search_widget->x) / 5;
	cam_position_t[1] = (cam_position_t[1] * 4 + _search_widget->y) / 5;
	cam_position_t[2] = (cam_position_t[2] * 4 + (_search_widget->z - 10)) / 5;

	target = _search_widget->arx;

	if (_search_widget->arx - cam_position_r[0] > 180)
		target -= 360;
	cam_position_r[0] = (cam_position_r[0] * 4 + target) / 5;

	target = _search_widget->ary;

	if (_search_widget->ary - cam_position_r[1] > 180)
		target -= 360;
	cam_position_r[1] = (cam_position_r[1] * 4 + target) / 5;

	target = _search_widget->arz;

	if (_search_widget->arz - cam_position_r[2] > 180)
		target -= 360;
	cam_position_r[2] = (cam_position_r[2] * 4 + target) / 5;

	s3d_translate(0, cam_position_t[0], cam_position_t[1], cam_position_t[2]);
	s3d_rotate(0, cam_position_r[0], cam_position_r[1], cam_position_r[2]);

	if (sqrt(((cam_position_t[0] - _search_widget->x)*(cam_position_t[0] - _search_widget->x)) +
	                ((cam_position_t[1] - _search_widget->y)*(cam_position_t[1] - _search_widget->y)) +
	                ((cam_position_t[2] - _search_widget->z)*(cam_position_t[2] - _search_widget->z))) < 0.2) {
		s3d_translate(0, _search_widget->x, _search_widget->y, (_search_widget->z - 10));
		s3d_rotate(0, _search_widget->arx, _search_widget->ary, _search_widget->arz);
	}
}

/* public */
void move_to_return_point(float cam_position_t[], float cam_position_r[])
{
	float target;

	cam_position_t[0] = (cam_position_t[0] * 4 + _return_point[0][0]) / 5;
	cam_position_t[1] = (cam_position_t[1] * 4 + _return_point[0][1]) / 5;
	cam_position_t[2] = (cam_position_t[2] * 4 + _return_point[0][2]) / 5;

	target = _return_point[1][0];

	if (_return_point[1][0] - cam_position_r[0] > 180)
		target -= 360;
	cam_position_r[0] = (cam_position_r[0] * 4 + target) / 5;

	target = _return_point[1][1];

	if (_return_point[1][1] - cam_position_r[1] > 180)
		target -= 360;
	cam_position_r[1] = (cam_position_r[1] * 4 + target) / 5;

	target = _return_point[1][2];

	if (_return_point[1][2] - cam_position_r[2] > 180)
		target -= 360;
	cam_position_r[2] = (cam_position_r[2] * 4 + target) / 5;

	s3d_translate(0, cam_position_t[0], cam_position_t[1], cam_position_t[2]);
	s3d_rotate(0, cam_position_r[0], cam_position_r[1], cam_position_r[2]);

	if (sqrt(((cam_position_t[0] - _return_point[0][0])*(cam_position_t[0] - _return_point[0][0])) +
	                ((cam_position_t[1] - _return_point[0][1])*(cam_position_t[1] - _return_point[0][1])) +
	                ((cam_position_t[2] - _return_point[0][2])*(cam_position_t[2] - _return_point[0][2]))) < 0.2) {
		s3d_translate(0, _return_point[0][0], _return_point[0][1], _return_point[0][2]);
		s3d_rotate(0, _return_point[1][0], _return_point[1][1], _return_point[1][2]);
		set_search_status(NOTHING);
	}
}

/* public */
/* TODO: WTF?!
 * please fix:
 *  - s is not initialized but still strlen() is used?!
 *  - s will vanish after the function is processed. global variable would be better
 *  - don't forget the terminating \0 after writing a key */
void search_widget_write(int key)
{
	static char s[20];
	int ln = strlen(s);

	if (key == S3DK_COMMA) key = S3DK_PERIOD;

	if (key != S3DK_RETURN) {
		if (key == S3DK_BACKSPACE) {
			if (ln > 0)
				s[ln-1] = '\0';
		} else {
			if (ln < 20)
				s[ln] = key;
		}
		s3dw_input_change_text(_search_input, s);
	} else {
		_new_search_node(_search_widget);
	}
}

/* public */
void set_return_point(float cam_position_t[], float cam_position_r[])
{
	int i;
	for (i = 0; i < 3; i++)
		_return_point[0][i] = cam_position_t[i];
	for (i = 0; i < 3; i++)
		_return_point[1][i] = cam_position_r[i];
}

/* public */
int get_search_status(void)
{
	return _search_status;
}

/* public */
void set_search_status(int stat)
{
	/* TODO check if stat between 0-3 else debug printf */
	_search_status = stat;
}

/* public */
void set_node_root(struct olsr_node *root)
{
	_node_root = root;
}
/* private */
static void _new_search_node(s3dw_widget *dummy)
{
	char *ip;
	int result;

	search_node = &_node_root;

	ip = s3dw_input_gettext(_search_input);

	while ((*search_node) != NULL) {

		result = strncmp((*search_node)->ip, ip, NAMEMAX);

		if (result == 0)
			break;

		if (result < 0)
			(*search_node) = (*search_node)->right;
		else
			(*search_node) = (*search_node)->left;
	}
	free(ip);
	s3dw_delete(dummy->parent); /* remove the window cointaining the button */
	_search_surface = NULL;
	_search_input = NULL;
	_search_widget = NULL;


	if ((*search_node) != NULL) {
		set_search_status(FOLLOW);
	} else {
		window_error("Sorry, could not find...");
		set_search_status(NOTHING);
	}
}
/* public */
void follow_node_by_click(struct olsr_node *olsr_node)
{
	search_node = &_node_root;
	(*search_node) = olsr_node;
	set_search_status(FOLLOW);
}

/* private */
static void _search_node(s3dw_widget *OLSRS3DUNUSED(dummy))
{
	char *ip;
	int result;

	search_node = &_node_root;

	ip = s3dw_input_gettext(_search_input);

	while ((*search_node) != NULL) {

		result = strncmp((*search_node)->ip, ip, NAMEMAX);

		if (result == 0)
			break;

		if (result < 0)
			(*search_node) = (*search_node)->right;
		else
			(*search_node) = (*search_node)->left;
	}
	free(ip);

	if ((*search_node) != NULL)
		set_search_status(FOLLOW);
}

/* private */
void _abort_search(s3dw_widget *OLSRS3DUNUSED(dummy))
{
	set_search_status(ABORT);
}
