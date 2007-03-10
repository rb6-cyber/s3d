/*
 * main.c
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *                         Marek Lindner <lindner_marek@yahoo.de>
 *                         Andreas Langer <andreas_lbg@gmx.de>
 *
 * This file is part of meshs3d, an olsr/batman topology visualizer for s3d.
 * See http://s3d.berlios.de/ for more updates.
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

#include <stdio.h>
#include <s3d.h>
#include <s3d_keysym.h>
#include <s3dw.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include "meshs3d.h"

/* global vars */
struct glob Global;

void init_globals( void )
{
	Global.debug = 0;
	Global.obj_node_hna = 0;
	Global.obj_node_inet = 0;
	Global.obj_node_normal = 0;
	Global.obj_btn_close = 0;
	Global.obj_s3d_url = 0;
	Global.obj_zero_point = 0;
		
}


void print_usage( void )
{

	printf( "Usage is olsrs3d [options] [-- [s3d options]]\n" );
	printf( "olsrs3d options:\n" );
	printf( "   -h\tprint this short help\n" );
	printf( "   -d\tenable debug mode\n" );
	printf( "   -H\tconnect to olsr node [default: localhost]\n" );
	s3d_usage();
}

void mainloop()
{

	int net_result;   /* result of function net_main */
//	char nc_str[20];
//	float strLen;
//
//	/* calculate new movement vector */
//	calc_olsr_node_mov();
//
//	/* prepare nodes */
//	handle_olsr_node( Olsr_root );
//
//	/* move it */
//	move_olsr_nodes();
//
//	/* if we have more or less nodes now - redraw node count */
//	if ( Olsr_node_count != Last_olsr_node_count ) {
//
//		if ( Olsr_node_count_obj != -1 ) s3d_del_object( Olsr_node_count_obj );
//		snprintf( nc_str, 20, "node count: %d", Olsr_node_count );
//		Olsr_node_count_obj = s3d_draw_string( nc_str, &strLen );
//		s3d_link( Olsr_node_count_obj, 0 );
//		s3d_flags_on( Olsr_node_count_obj, S3D_OF_VISIBLE );
//		s3d_scale( Olsr_node_count_obj, 0.2 );
//		s3d_translate( Olsr_node_count_obj, -Left*3.0-(strLen * 0.2), -Bottom*3.0-0.5, -3.0 );
//		Last_olsr_node_count = Olsr_node_count;
//
//	}
//
//	if ( Output_block_completed ) {
//
//		Output_block_counter++;
//		Output_block_completed = 0;
//
//	}

//	/* read data from socket */
//	Net_read_count = 0;
	while ( ( net_result = net_main() ) != 0 ) {
		if ( net_result == -1 ) {
			s3d_quit();
			break;
		}
	}
//
//	/* rotate modus */
//	if(RotateSwitch) {
//		Zp_rotate = ( Zp_rotate + RotateSpeed ) > 360 ? 0.0 : ( Zp_rotate + RotateSpeed );
//		s3d_rotate(ZeroPoint,0,Zp_rotate,0);
//	}
//
//	/* calc for node description */
//	CamPosition2[0][0]=  CamPosition[0][0]*cos(Zp_rotate*M_PI/180.0) - CamPosition[0][2] * sin (Zp_rotate*M_PI/180.0);
//	CamPosition2[0][1]=  CamPosition[0][1];
//	CamPosition2[0][2]=  CamPosition[0][0]*sin(Zp_rotate*M_PI/180.0) + CamPosition[0][2] * cos (Zp_rotate*M_PI/180.0);
//
//	/* check search status */
///*	if( get_search_status() == WIDGET )
//		move_to_search_widget( CamPosition[0], CamPosition[1] );*/
//	if( get_search_status() == FOLLOW )
//		follow_node( CamPosition[0], CamPosition[1], Zp_rotate );
///*	if( get_search_status() == ABORT )
//		move_to_return_point( CamPosition[0], CamPosition[1] );*/
//
//
//	if( Olsr_ip_label_obj != -1 )
//	{
//		print_etx();
//	}
//	s3dw_ani_mate();
//
//	nanosleep( &sleep_time, NULL );
//
	return;

}


int main( int argc, char *argv[] ) {
	int optchar;
	char olsr_host[256];
	
	init_globals();
	strncpy( olsr_host, "127.0.0.1", 256 );
	lbuf[0] = '\0';

	while ( ( optchar = getopt ( argc, argv, "dhH:" ) ) != -1 ) {

		switch ( optchar ) {

			case 'd':
				Global.debug = 1;
				break;

			case 'H':
				strncpy( olsr_host, optarg, 256 );
				break;

			case 'h':
			default:
				print_usage();
				return (0);

		}

	}

	if ( Global.debug )
		printf( "debug mode enabled ...\n" );

//	/* initialize obj2ip linked list */
//	lst_initialize();

	/* delete olsrs3d options */
	while ( ( optind < argc ) && ( argv[optind][0] != '-' ) ) optind++;		/* optind may point to ip addr of '-H' */
	optind--;
	argv[optind] = argv[0];		/* save program path */
	argc -= optind;				/* jump over olsrs3d options */
	argv += optind;

	/* set extern int optind = 0 for parse_args in io.c */
	optind = 0;


	if (!net_init(olsr_host))
	{
//		s3d_set_callback(S3D_EVENT_OBJ_INFO,object_info);
//		s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);
//		s3d_set_callback(S3D_EVENT_KEY,keypress);
//		s3d_set_callback(S3D_EVENT_QUIT,stop);

		if (!s3d_init(&argc,&argv,"meshs3d"))
		{

			if (s3d_select_font("vera"))
				printf("font not found\n");

			Global.obj_node_normal = s3d_import_model_file( "objs/accesspoint.3ds" );
			Global.obj_node_inet = s3d_import_model_file( "objs/accesspoint_inet.3ds" );
			Global.obj_node_hna = s3d_import_model_file( "objs/internet.3ds" );
			Global.obj_btn_close = s3d_import_model_file( "objs/btn_close.3ds" );

			Global.obj_s3d_url = s3d_import_model_file( "objs/s3d_berlios_de.3ds" );

			s3d_translate( Global.obj_s3d_url, 0.75, -0.75, -1 );
			s3d_scale( Global.obj_s3d_url, 0.07 );
			s3d_link( Global.obj_s3d_url, 0 );
			s3d_flags_on( Global.obj_s3d_url, S3D_OF_VISIBLE );

			/* create_search_widget( 0, 0, 300 ); */

			Global.obj_zero_point = s3d_new_object();
//			Output_border[0] = Output_border[1] = Output_border[2] = Output_border[3] = -1;

			s3d_mainloop(mainloop);
			s3d_quit();
			net_quit();
		}
	}
	return(0);
}