/*
 * mod_search.c
 * 
 * Copyright (C) 2006 Andreas Langer <andreas_lbg@gmx.de>
 *
 * This file is part of the olsrs3d, an olsr topology visualizer for s3d.
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
#include <math.h>
#include <stdio.h>	/* TODO can remove then no more printf needed */
#include "mod_search.h"

s3dw_surface	*_search_surface;
s3dw_input		*_search_input;
s3dw_widget		*_search_widget;

float	_return_point[2][3];				/* cam position before move to the widget */
int		_search_status = NOTHING;			/* status of search */
											
void _search_node(s3dw_widget *dummy);
void _abort_search(s3dw_widget *dummy);

/* public */
void create_search_widget(float x, float y, float z)
{
	s3dw_button *search_button, *abort_button;
	
	_search_surface	= s3dw_surface_new( "Node Search", 17, 10 );
	_search_input	= s3dw_input_new( _search_surface, 15, 1, 4 );
	
	s3dw_label_new( _search_surface, "Enter the IP of the node.", 1, 2);
	s3dw_focus( S3DWIDGET( _search_input ) );
	
	search_button = s3dw_button_new( _search_surface, "Search", 11.5, 7 );
	abort_button  = s3dw_button_new( _search_surface, "Abort", 1, 7 );
	search_button->onclick = _search_node;
	abort_button->onclick = _abort_search;
	
	s3dw_focus	( S3DWIDGET( _search_input ) );	
	s3dw_focus	( S3DWIDGET( _search_surface ) );
	s3dw_show	( S3DWIDGET( _search_surface ) );
	
	_search_widget	= s3dw_getroot();
	move_search_widget( x, y, z );

	_search_widget->ary = 180;
	s3d_rotate( _search_widget->oid, _search_widget->arx, _search_widget->ary, _search_widget->arz );
}

/* public */
void move_search_widget(float x, float y, float z)
{
	_search_widget->x = x; _search_widget->y = y; _search_widget->z = z;
	s3dw_moveit( _search_widget );
}

/* public */
void move_to_search_widget(float cam_position_t[], float cam_position_r[])
{
	float target, current;
	
	set_search_status(WIDGET);
	cam_position_t[0] = ( cam_position_t[0] * 4 + _search_widget->x ) / 5;
	cam_position_t[1] = ( cam_position_t[1] * 4 + _search_widget->y ) / 5;
	cam_position_t[2] = ( cam_position_t[2] * 4 + ( _search_widget->z - 10 ) ) / 5;

	target = _search_widget->arx;
	current = cam_position_r[0];

	if( _search_widget->arx - cam_position_r[0] > 180 )
		target -= 360;
	if( _search_widget->arx - cam_position_r[0] < -180 )
		current -= 360;
	cam_position_r[0] = ( cam_position_r[0] * 4 + target ) / 5;

	target = _search_widget->ary;
	current = cam_position_r[1];

	if( _search_widget->ary - cam_position_r[1] > 180 )
		target -= 360;
	if( _search_widget->ary - cam_position_r[1] < -180 )
		current -= 360;
	cam_position_r[1] = ( cam_position_r[1] * 4 + target ) / 5;

	target = _search_widget->arz;
	current = cam_position_r[2];

	if( _search_widget->arz - cam_position_r[2] > 180 )
		target -= 360;
	if( _search_widget->arz - cam_position_r[2] < -180 )
		current -= 360;
	cam_position_r[2] = ( cam_position_r[2] * 4 + target ) / 5;
	
	s3d_translate(0,cam_position_t[0],cam_position_t[1],cam_position_t[2]);
	s3d_rotate(0,cam_position_r[0],cam_position_r[1],cam_position_r[2]);
	
	if ( sqrt(  (( cam_position_t[0] - _search_widget->x)*( cam_position_t[0] - _search_widget->x)) + 
				(( cam_position_t[1] - _search_widget->y)*( cam_position_t[1] - _search_widget->y)) + 
				(( cam_position_t[2] - _search_widget->z)*( cam_position_t[2] - _search_widget->z)) ) < 0.2 )
	{
		s3d_translate( 0, _search_widget->x, _search_widget->y, ( _search_widget->z - 10 ) );
		s3d_rotate( 0, _search_widget->arx, _search_widget->ary, _search_widget->arz );
	}
}

/* public */
void move_to_return_point(float cam_position_t[], float cam_position_r[])
{
	float target, current;

	cam_position_t[0] = ( cam_position_t[0] * 4 + _return_point[0][0] ) / 5;
	cam_position_t[1] = ( cam_position_t[1] * 4 + _return_point[0][1] ) / 5;
	cam_position_t[2] = ( cam_position_t[2] * 4 + _return_point[0][2] ) / 5;

	target = _return_point[1][0];
	current = cam_position_r[0];

	if( _return_point[1][0] - cam_position_r[0] > 180 )
		target -= 360;
	if( _return_point[1][0] - cam_position_r[0] < -180 )
		current -= 360;
	cam_position_r[0] = ( cam_position_r[0] * 4 + target ) / 5;

	target = _return_point[1][1];
	current = cam_position_r[1];

	if( _return_point[1][1] - cam_position_r[1] > 180 )
		target -= 360;
	if( _return_point[1][1] - cam_position_r[1] < -180 )
		current -= 360;
	cam_position_r[1] = ( cam_position_r[1] * 4 + target ) / 5;

	target = _return_point[1][2];
	current = cam_position_r[2];

	if( _return_point[1][2] - cam_position_r[2] > 180 )
		target -= 360;
	if( _return_point[1][2] - cam_position_r[2] < -180 )
		current -= 360;
	cam_position_r[2] = ( cam_position_r[2] * 4 + target ) / 5;
	
	s3d_translate(0,cam_position_t[0],cam_position_t[1],cam_position_t[2]);
	s3d_rotate(0,cam_position_r[0],cam_position_r[1],cam_position_r[2]);
	
	if ( sqrt(  (( cam_position_t[0] - _return_point[0][0])*( cam_position_t[0] - _return_point[0][0])) + 
				(( cam_position_t[1] - _return_point[0][1])*( cam_position_t[1] - _return_point[0][1])) + 
				(( cam_position_t[2] - _return_point[0][2])*( cam_position_t[2] - _return_point[0][2])) ) < 0.2 )
	{
		s3d_translate( 0, _return_point[0][0], _return_point[0][1], _return_point[0][2] );
		s3d_rotate( 0, _return_point[1][0], _return_point[1][1], _return_point[1][2] );
		set_search_status(NOTHING);
	}
}

/* public */
void set_return_point(float cam_position_t[], float cam_position_r[])
{
	int i;
	for( i = 0; i < 3; i++ )
		_return_point[0][i] = cam_position_t[i];
	for( i = 0; i < 3; i++ )
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

/* private */
void _search_node(s3dw_widget *dummy)
{
	char *ip;
	ip = s3dw_input_gettext( _search_input );
	printf("%s\n",ip);
}

/* private */
void _abort_search(s3dw_widget *dummy)
{
	set_search_status(ABORT);
}

	