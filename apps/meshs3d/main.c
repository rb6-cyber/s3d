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
#include "allocate.h"
#include "hash.h"

/* global vars */
struct glob Global;
static struct timespec sleep_time = { 0, 100 * 1000 * 1000 };   /* 100 mili seconds */


void init_globals( void )
{
	Global.debug = 1;
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

float dist(float p1[], float p2[])
{
	float p[3];
	p[0]=p1[0]-p2[0];
	p[1]=p1[1]-p2[1];
	p[2]=p1[2]-p2[2];
	return (sqrt(p[0]*p[0]   +  p[1]*p[1]  +  p[2]*p[2]));
}

float dirt(float p1[], float p2[], float p3[])
{
	float d;
	d=dist(p1,p2);
	if (d==0) {
		p3[0]=(( float ) 0.2 * rand() ) / RAND_MAX - 0.1;
		p3[1]=(( float ) 0.2 * rand() ) / RAND_MAX - 0.1;
		p3[2]=(( float ) 0.2 * rand() ) / RAND_MAX - 0.1;
		d=s3d_vector_length(p3);
	} else {
		p3[0]=p2[0]-p1[0];
		p3[1]=p2[1]-p1[1];
		p3[2]=p2[2]-p1[2];
	}
	return(d);
}

void handle_node()
{
	struct node *node;
	struct hash_it_t *hashit;

	if( node_hash->elements == 0 )
		return;
	hashit = NULL;
	while ( NULL != ( hashit = hash_iterate( node_hash, hashit ) ) )
	{
		node = (struct node *) hashit->bucket->data;
		if( node->node_type_modified ) {
			
			node->node_type_modified = 0;
			if ( node->obj_id != -1 )
			{
				s3d_del_object( node->obj_id );
			}

			if ( node->desc_id != -1 ) s3d_del_object( node->desc_id );

			if ( node->node_type == 1 ) {
				node->obj_id = s3d_clone( Global.obj_node_inet );
			} else if ( node->node_type == 2 ) {
				node->obj_id = s3d_clone( Global.obj_node_hna );
			} else {
				node->obj_id = s3d_clone( Global.obj_node_normal );
			}

			s3d_flags_on( node->obj_id, S3D_OF_VISIBLE|S3D_OF_SELECTABLE);

		}
	}
	return;
}

void mov_add(float mov[], float p[], float fac)
{
/*	if (fac>1000)
		return;
	fac=1000; */
	mov[0]+=fac*p[0];
	mov[1]+=fac*p[1];
	mov[2]+=fac*p[2];
}

void calc_node_mov( void ) {

	float distance;
	float tmp_mov_vec[3];
	float f;
	struct node_con *con;
	struct node *first_node, *sec_node;
	struct hash_it_t *hashit;

	if( con_hash->elements == 0 )
		return;
	hashit = NULL;
	while ( NULL != ( hashit = hash_iterate( con_hash, hashit ) ) )
	{
		con = (struct node_con *) hashit->bucket->data;
		first_node = hash_find( node_hash, &con->ip[0] );
		sec_node = hash_find( node_hash, &con->ip[1] );
		distance = dirt( first_node->pos_vec, sec_node->pos_vec, tmp_mov_vec );
		f = ( ( con->etx1_sqrt + con->etx2_sqrt ) / 4.0 ) / distance;
// 
// 		/***
// 		 * drift factor - 0.0 < factor < 1.0 ( best results: 0.3 < factor < 0.9
// 		 * small factor: fast and strong drift to neighbours
// 		 ***/
// 		if ( f < Factor ) f = Factor;

		mov_add( first_node->mov_vec, tmp_mov_vec, 1 / f - 1 );
		mov_add( sec_node->mov_vec, tmp_mov_vec, - ( 1 / f - 1 ) );
	}

}

void move_nodes( void ) {

	float null_vec[3] = {0,0,0}, vertex_buf[6];
	float tmp_mov_vec[3];
	float distance, etx, rgb;
	struct node_con *con;
	struct node *first_node, *sec_node;
	struct hash_it_t *hashit;

	if( con_hash->elements == 0 )
		return;
	hashit = NULL;
	while ( NULL != ( hashit = hash_iterate( con_hash, hashit ) ) )
	{
		con = (struct node_con *) hashit->bucket->data;
		first_node = hash_find( node_hash, &con->ip[0] );
		sec_node = hash_find( node_hash, &con->ip[1] );
		/* move left olsr node if it has not been moved yet */
		if ( !( ( first_node->mov_vec[0] == 0 ) && ( first_node->mov_vec[1] == 0 ) && ( first_node->mov_vec[2] == 0 ) ) && first_node->visible ) {
			distance = dirt( first_node->pos_vec, null_vec, tmp_mov_vec );
			mov_add( first_node->mov_vec, tmp_mov_vec, distance / 100 ); /* move a little bit to point zero */
			mov_add( first_node->mov_vec, tmp_mov_vec, 1 ); /* move a little bit to point zero */

			if ( ( distance = dist( first_node->mov_vec, null_vec ) ) > 10.0 ) {
				mov_add( first_node->pos_vec, first_node->mov_vec, 1.0 / ( ( float ) distance ) );
			} else {
				mov_add( first_node->pos_vec, first_node->mov_vec, 0.1 );
			}

			s3d_translate( first_node->obj_id, first_node->pos_vec[0], first_node->pos_vec[1], first_node->pos_vec[2] );

			/* reset movement vector */
			first_node->mov_vec[0] = first_node->mov_vec[1] = first_node->mov_vec[2] = 0.0;

		}

		/* move right olsr node if it has not been moved yet */
		if ( !( ( sec_node->mov_vec[0] == 0 ) && ( sec_node->mov_vec[1] == 0 ) && ( sec_node->mov_vec[2] == 0 ) ) && sec_node->visible ) {

			distance = dirt( sec_node->pos_vec, null_vec, tmp_mov_vec );
			mov_add( sec_node->mov_vec, tmp_mov_vec, distance / 100 ); /* move a little bit to point zero */
			mov_add( sec_node->mov_vec, tmp_mov_vec, 1 ); /* move a little bit to point zero */

			if ( ( distance = dist( sec_node->mov_vec, null_vec ) ) > 10.0 ) {
				mov_add( sec_node->pos_vec, sec_node->mov_vec, 1.0 / ( ( float ) distance ) );
			} else {
				mov_add( sec_node->pos_vec, sec_node->mov_vec, 0.1 );
			}

			s3d_translate( sec_node->obj_id, sec_node->pos_vec[0], sec_node->pos_vec[1], sec_node->pos_vec[2] );

			/* reset movement vector */
			sec_node->mov_vec[0] = sec_node->mov_vec[1] = sec_node->mov_vec[2] = 0.0;

		}


		/* move connection between left and right olsr node */
		vertex_buf[0] = first_node->pos_vec[0];
		vertex_buf[1] = first_node->pos_vec[1];
		vertex_buf[2] = first_node->pos_vec[2];
		vertex_buf[3] = sec_node->pos_vec[0];
		vertex_buf[4] = sec_node->pos_vec[1];
		vertex_buf[5] = sec_node->pos_vec[2];

		s3d_pep_vertices( con->obj_id, vertex_buf, 2 );


// 		if ( ColorSwitch ) {
// 
// 			/* HNA */
// 			if ( olsr_con->left_etx == -1000.00 ) {
// 
// 				if(olsr_con->color != 1) {
// 					s3d_pep_material( olsr_con->obj_id,
// 								   0.0,0.0,1.0,
// 								   0.0,0.0,1.0,
// 								   0.0,0.0,1.0);
// 					olsr_con->color = 1;
// 				}
// 
// 			} else {
// 
// 				etx = ( olsr_con->left_etx + olsr_con->right_etx ) / 2.0;
// 
// 				/* very good link - bright blue */
// 				if ( ( etx >= 1.0 ) && ( etx < 1.5 ) ) {
// 
// 					if(olsr_con->color != 2) {
// 						s3d_pep_material( olsr_con->obj_id,
// 								0.5,1.0,1.0,
// 								0.5,1.0,1.0,
// 								0.5,1.0,1.0);
// 						olsr_con->color = 2;
// 					}
// 
// 				/* good link - bright yellow */
// 				} else if ( ( etx >= 1.5 ) && ( etx < 2.0 ) ) {
// 
// 					rgb = 2.0 - etx;
// 					if( olsr_con->color != 3 || (olsr_con->color == 3 && (int) rintf(olsr_con->rgb * 10) !=  (int) rintf(rgb * 10))) {
// 						s3d_pep_material( olsr_con->obj_id,
// 								1.0,1.0,rgb,
// 								1.0,1.0,rgb,
// 								1.0,1.0,rgb);
// 						olsr_con->color = 3;
// 
// 						olsr_con->rgb =  rgb;
// 					}
// 
// 				/* not so good link - orange */
// 				} else if ( ( etx >= 2.0 ) && ( etx < 3.0 ) ) {
// 
// 					rgb = 1.5 - ( etx / 2.0 );
// 					if( olsr_con->color != 4 || (olsr_con->color == 4 && (int) rintf(olsr_con->rgb * 10) !=  (int) rintf(rgb * 10))) {
// 						s3d_pep_material( olsr_con->obj_id,
// 								1.0,rgb,0.0,
// 								1.0,rgb,0.0,
// 								1.0,rgb,0.0);
// 						olsr_con->color = 4;
// 
// 						olsr_con->rgb = rgb;
// 					}
// 
// 				/* bad link (almost dead) - brown */
// 				} else if ( ( etx >= 3.0 ) && ( etx < 5.0 ) ) {
// 
// 					rgb = 1.75 - ( etx / 4.0 );
// 
// 					if( olsr_con->color != 5 || (olsr_con->color == 5 && (int) rintf(olsr_con->rgb * 10) !=  (int) rintf(rgb * 10)) ) {
// 
// 						s3d_pep_material( olsr_con->obj_id,
// 								rgb,rgb - 0.5,0.0,
// 								rgb,rgb - 0.5,0.0,
// 								rgb,rgb - 0.5,0.0);
// 						olsr_con->color = 5;
// 
// 						olsr_con->rgb = rgb;
// 					}
// 
// 				/* zombie link - grey */
// 				} else if ( ( etx >= 5.0 ) && ( etx < 1000.0 ) ) {
// 
// 					rgb = 1000.0 / ( 1500.0 + etx );
// 
// 					if( olsr_con->color != 6 || (olsr_con->color == 6 && (int) rintf(olsr_con->rgb * 10) !=  (int) rintf(rgb * 10)) ) {
// 
// 						s3d_pep_material( olsr_con->obj_id,
// 								rgb,rgb,rgb,
// 								rgb,rgb,rgb,
// 								rgb,rgb,rgb);
// 						olsr_con->color = 6;
// 
// 						olsr_con->rgb = rgb;
// 					}
// 
// 				/* wtf - dark grey */
// 				} else {
// 
// 					if(olsr_con->color != 7) {
// 						s3d_pep_material( olsr_con->obj_id,
// 								0.3,0.3,0.3,
// 								0.3,0.3,0.3,
// 								0.3,0.3,0.3);
// 						olsr_con->color = 7;
// 					}
// 
// 				}
// 
// 			}
// 
// 		} else {

			if( con->color == 0) {
				s3d_pep_material( con->obj_id,
							1.0,1.0,1.0,
							1.0,1.0,1.0,
							1.0,1.0,1.0);
				con->color = 0;
			}

// 		}

	}

}

void mainloop()
{

	int net_result;   /* result of function net_main */

	calc_node_mov();
	handle_node();
	move_nodes();

	while ( ( net_result = net_main() ) != 0 ) {
		if ( net_result == -1 ) {
			s3d_quit();
			break;
		}
	}
	nanosleep( &sleep_time, NULL );
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

	process_init();

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
