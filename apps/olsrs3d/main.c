/*
 * main.c
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *                         Marek Lindner <lindner_marek@yahoo.de>
 *                         Andreas Langer <andreas_lbg@gmx.de>
 *
 * This file is part of olsrs3d, an olsr topology visualizer for s3d.
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
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */



#include <stdio.h>
#include <s3d.h>
#include <s3d_keysym.h>
#include <time.h>	      /* nanosleep() */
#include <string.h>	/* strncpy() */
#include <math.h>		/* sqrt() */
#include <getopt.h>	/* getopt() */
#include <stdlib.h>	/* exit() */
#include "olsrs3d.h"
#include "terminal.h"

#define SPEED		10.0

static struct timespec sleep_time = { 0, 100 * 1000 * 1000 };   /* 100 mili seconds */

int Debug = 0;

char Olsr_host[256];   /* ip or hostname of olsr node with running dot_draw plugin */

struct olsr_con *Con_begin = NULL;   /* begin of connection list */
struct olsr_node *Olsr_root = NULL;   /* top of olsr node tree */
struct Obj_to_ip *Obj_to_ip_head, *Obj_to_ip_end, *List_ptr;   /* needed pointer for linked list */

int Olsr_node_count = 0, Last_olsr_node_count = -1;
int Olsr_node_count_obj = -1;
int Olsr_ip_label_obj = -1;
int Output_border[4];
int *Olsr_neighbour_label_obj = NULL;
int Size;
int Move_prepared = 0;

int Net_read_count;
int Output_block_counter = 0;
int Output_block_completed = 0;

int Olsr_node_obj, Olsr_node_inet_obj, Olsr_node_hna_net;

float Asp = 1.0;
float Bottom = -1.0;
float Left = -1.0;

float CamPosition[2][3];   /* CamPosition[trans|rot][x-z] */
float CamPosition2[2][3];   /* CamPosition[trans|rot][x-z] */
float ZeroPosition[3] = {0,0,0};   /* current position zero position */
int ZeroPoint;   /* object zeropoint */
int Zp_rotate = 0;
int ColorSwitch = 0;   /* enable/disable colored olsr connections */
int RotateSwitch = 0;
int RotateSpeed = 2;
float Factor = 0.6;	/* Factor in calc_olsr_node_mov */
struct olsr_node *Olsr_node_pEtx;
int Btn_close_id = -1;
int Btn_close_obj;

float Title_len;
int cam_go=0;

/* object vars */
int obj_term;
int obj_cursor;
extern int obj_cursor_mp;
struct s3d_object **obj;

int move_cam_to = -1;
int oid_focus = -1;
float returnPoint[2][3];

/***
 *
 * print usage info
 *
 ***/

void print_usage( void ) {

	printf( "Usage is olsrs3d [options] [-- [s3d options]]\n" );
	printf( "olsrs3d options:\n" );
	printf( "   -h\tprint this short help\n" );
	printf( "   -d\tenable debug mode\n" );
	printf( "   -H\tconnect to olsr node [default: localhost]\n" );
	s3d_usage();

}



/***
 *
 * print error and exit
 *
 ***/

void out_of_mem( void ) {

	printf( "Sorry - you ran out of memory !\n" );
	exit(8);

}



/***
 *
 * calculate distance between 2 vectors => http://en.wikipedia.org/wiki/Euclidean_distance
 *
 *   p1   =>   vector of node 1
 *   p2   =>   vector of node 2
 *
 *   return distance
 *
 ***/

float dist(float p1[], float p2[])
{
	float p[3];
	p[0]=p1[0]-p2[0];
	p[1]=p1[1]-p2[1];
	p[2]=p1[2]-p2[2];
	return (sqrt(p[0]*p[0]   +  p[1]*p[1]  +  p[2]*p[2]));

}



/***
 *
 * calculate distance between 2 vectors and substract vector1 from vector2
 *  => http://en.wikipedia.org/wiki/Vector_%28spatial%29#Vector_addition_and_subtraction
 *
 *   p1   =>   vector of node 1
 *   p2   =>   vector of node 2
 *
 *   return distance
 *
 ***/

float dirt(float p1[], float p2[], float p3[])
{
	float d;
	d=dist(p1,p2);
	if (d!=0.0)
	{
		p3[0]=p2[0]-p1[0];
		p3[1]=p2[1]-p1[1];
		p3[2]=p2[2]-p1[2];
	} else {
		p3[0]=p2[0]=p1[0]=0.0;
	}
	return(d);
}



/***
 *
 * calculate new movement of node by adding the product of the factor and the vector to the movement vector
 *  => http://en.wikipedia.org/wiki/Vector_%28spatial%29#Scalar_multiplication
 *
 *   mov  =>   current mov vector
 *   p    =>   vector of node
 *   fac  =>   factor which is
 *
 ***/

void mov_add(float mov[], float p[], float fac)
{
/*	if (fac>1000)
		return;
	fac=1000; */
	mov[0]+=fac*p[0];
	mov[1]+=fac*p[1];
	mov[2]+=fac*p[2];
}



/***
 *
 * check whether is a new / modified / vanished node and handle it accordingly
 *
 *   *olsr_node =>   pointer to current olsr_node
 *
 ***/

void handle_olsr_node( struct olsr_node *olsr_node ) {

	float distance, angle, angle_rad;
	float tmp_mov_vec[3], desc_norm_vec[3] = {0,0,-1};
	struct olsr_node *other_node;
	struct Obj_to_ip *Obj_to_ip_curr;
	struct olsr_neigh_list *olsr_neigh_list, *prev_olsr_neigh_list, *other_node_neigh_list, *tmp_olsr_neigh_list;

	/* no more nodes left */
	if ( olsr_node == NULL ) return;

	/* olsr node vanished */
	if ( ( olsr_node->last_seen < Output_block_counter - 1 ) && ( olsr_node->visible ) ) {

		if ( Debug ) printf( "olsr node vanished: %s\n", olsr_node->ip );

		Olsr_node_count--;

		olsr_node->visible = 0;

		/* delete shape */
		if ( olsr_node->obj_id != -1 ) {

			/* remove element from ob2ip list */
			lst_del( olsr_node->obj_id );
			/* remove object from s3d server */
			s3d_del_object( olsr_node->obj_id );

			olsr_node->obj_id = -1;

		}

		if ( olsr_node->desc_id != -1 ) {

			s3d_del_object( olsr_node->desc_id );
			olsr_node->desc_id = -1;

		}

		/* delete olsr connections of this node */
		olsr_neigh_list = olsr_node->olsr_neigh_list;

		while ( olsr_neigh_list != NULL ) {

			/* get connection list of 'other' node */
			if ( olsr_neigh_list->olsr_con->left_olsr_node == olsr_node ) {
				other_node = olsr_neigh_list->olsr_con->right_olsr_node;
			} else {
				other_node = olsr_neigh_list->olsr_con->left_olsr_node;
			}

			/* find this connection in 'other' nodes connection list ... */
			prev_olsr_neigh_list = NULL;
			other_node_neigh_list = other_node->olsr_neigh_list;

			while ( other_node_neigh_list != NULL ) {

				if ( other_node_neigh_list->olsr_con == olsr_neigh_list->olsr_con ) {

					/* and delete it ! */
					if ( prev_olsr_neigh_list != NULL ) {
						/* is first, any or last element in the list */
						prev_olsr_neigh_list->next_olsr_neigh_list = other_node_neigh_list->next_olsr_neigh_list;
					} else {
						/* the only element in the list */
						other_node->olsr_neigh_list = NULL;
					}

					free( other_node_neigh_list );

					break;

				}

				prev_olsr_neigh_list = other_node_neigh_list;
				other_node_neigh_list = other_node_neigh_list->next_olsr_neigh_list;

			}

			s3d_del_object( olsr_neigh_list->olsr_con->obj_id );

			/* delete connection */
			if ( olsr_neigh_list->olsr_con->prev_olsr_con != NULL ) olsr_neigh_list->olsr_con->prev_olsr_con->next_olsr_con = olsr_neigh_list->olsr_con->next_olsr_con;
			if ( olsr_neigh_list->olsr_con->next_olsr_con != NULL ) olsr_neigh_list->olsr_con->next_olsr_con->prev_olsr_con = olsr_neigh_list->olsr_con->prev_olsr_con;

			tmp_olsr_neigh_list = olsr_neigh_list;

			olsr_neigh_list = olsr_neigh_list->next_olsr_neigh_list;

			free( tmp_olsr_neigh_list->olsr_con );
			free( tmp_olsr_neigh_list );

		}

		olsr_node->olsr_neigh_list = NULL;

	} else if ( olsr_node->visible ) {

		/* olsr node shape has been modified */
		if ( olsr_node->node_type_modified ) {

			/* delete old shape */
			if ( olsr_node->obj_id != -1 ) {
				/* remove element from ob2ip list */
				lst_del( olsr_node->obj_id );
				s3d_del_object( olsr_node->obj_id );
			}

			if ( olsr_node->desc_id != -1 ) s3d_del_object( olsr_node->desc_id );

			/* create new shape */
			if ( olsr_node->node_type == 1 ) {
				/* olsr node offers internet access */
				olsr_node->obj_id = s3d_clone( Olsr_node_inet_obj );
			} else if ( olsr_node->node_type == 2 ) {
				/* via hna announced network */
				olsr_node->obj_id = s3d_clone( Olsr_node_hna_net );
			} else {
				/* normal olsr node */
				olsr_node->obj_id = s3d_clone( Olsr_node_obj );
			}

			s3d_flags_on( olsr_node->obj_id, S3D_OF_VISIBLE|S3D_OF_SELECTABLE);

			/* link newly created object to ZeroPoint */
			s3d_link( olsr_node->obj_id, ZeroPoint );
			/* add object_id and olsr_node to linked list */
			lst_add(olsr_node->obj_id,&olsr_node);

			/* create olsr node text and attach (link) it to the node */
			olsr_node->desc_id = s3d_draw_string( olsr_node->ip, &olsr_node->desc_length );
			s3d_link( olsr_node->desc_id, olsr_node->obj_id );
			s3d_translate( olsr_node->desc_id, - olsr_node->desc_length / 2, -2, 0 );
			s3d_flags_on( olsr_node->desc_id, S3D_OF_VISIBLE );

			olsr_node->node_type_modified = 0;

		}


		/* rotate node description so that they are always readable */
		tmp_mov_vec[0] = CamPosition2[0][0] - olsr_node->pos_vec[0];
		tmp_mov_vec[1] = 0;   /* we are not interested in the y value */
		tmp_mov_vec[2] = CamPosition2[0][2] - olsr_node->pos_vec[2];

		angle = s3d_vector_angle( desc_norm_vec, tmp_mov_vec );

		/* take care of inverse cosinus */
		if ( tmp_mov_vec[0] > 0 ) {
			angle_rad = 90.0/M_PI - angle;
			angle = 180 - ( 180.0/M_PI * angle );
		} else {
			angle_rad = 90.0/M_PI + angle;
			angle = 180 + ( 180.0/M_PI * angle );
		}

		s3d_rotate( olsr_node->desc_id, 0, angle , 0 );
		s3d_translate( olsr_node->desc_id, -cos(angle_rad)*olsr_node->desc_length/2 ,-1.5, sin(angle_rad)*olsr_node->desc_length/2 );


		/* drift away from unrelated nodes */
		Obj_to_ip_curr = Obj_to_ip_head->next;
		while ( Obj_to_ip_curr != Obj_to_ip_end ) {

			/* myself ... */
			if ( olsr_node != Obj_to_ip_curr->olsr_node ) {

				olsr_neigh_list = olsr_node->olsr_neigh_list;
				while ( olsr_neigh_list != NULL ) {

					/* nodes are related */
					if ( ( olsr_neigh_list->olsr_con->left_olsr_node->visible == 1 ) && ( olsr_neigh_list->olsr_con->right_olsr_node->visible == 1 ) ) {

						if ( ( olsr_neigh_list->olsr_con->left_olsr_node == Obj_to_ip_curr->olsr_node ) || (  olsr_neigh_list->olsr_con->right_olsr_node == Obj_to_ip_curr->olsr_node ) ) break;

					}

					olsr_neigh_list = olsr_neigh_list->next_olsr_neigh_list;

				}

				/* nodes are not related - so drift */
				if ( olsr_neigh_list == NULL ) {

					distance = dirt( olsr_node->pos_vec, Obj_to_ip_curr->olsr_node->pos_vec, tmp_mov_vec );
					if ( distance < 0.1 ) distance = 0.1;
					mov_add( olsr_node->mov_vec, tmp_mov_vec,-100 / ( distance * distance ) );
					mov_add( Obj_to_ip_curr->olsr_node->mov_vec, tmp_mov_vec, 100 / ( distance * distance ) );

				}

			}

			Obj_to_ip_curr = Obj_to_ip_curr->next;

		}

	}

	handle_olsr_node( olsr_node->left );
	handle_olsr_node( olsr_node->right );

}



/***
 *
 * calculate movement vector of all olsr nodes
 *
 ***/

void calc_olsr_node_mov( void ) {

	float distance;
	float tmp_mov_vec[3];
	float f;
	struct olsr_con *olsr_con = Con_begin;

	while ( olsr_con != NULL ) {

		distance = dirt( olsr_con->left_olsr_node->pos_vec, olsr_con->right_olsr_node->pos_vec, tmp_mov_vec );
		f = ( ( olsr_con->left_etx_sqrt + olsr_con->left_etx_sqrt ) / 4.0 ) / distance;

		/***
		 * drift factor - 0.0 < factor < 1.0 ( best results: 0.3 < factor < 0.9
		 * small factor: fast and strong drift to neighbours
		 ***/
		if ( f < Factor ) f = Factor;

		mov_add( olsr_con->left_olsr_node->mov_vec, tmp_mov_vec, 1 / f - 1 );
		mov_add( olsr_con->right_olsr_node->mov_vec, tmp_mov_vec, - ( 1 / f - 1 ) );

		olsr_con = olsr_con->next_olsr_con;

	}

}



/***
 *
 * move all olsr nodes and their connections
 *
 ***/

void move_olsr_nodes( void ) {

	float null_vec[3] = {0,0,0}, vertex_buf[6];
	float tmp_mov_vec[3];
	float distance, etx, rgb;
	struct olsr_con *olsr_con = Con_begin;

	while ( olsr_con != NULL ) {

		/* move left olsr node if it has not been moved yet */
		if ( !( ( olsr_con->left_olsr_node->mov_vec[0] == 0 ) && ( olsr_con->left_olsr_node->mov_vec[1] == 0 ) && ( olsr_con->left_olsr_node->mov_vec[2] == 0 ) ) && olsr_con->left_olsr_node->visible ) {

			distance = dirt( olsr_con->left_olsr_node->pos_vec, null_vec, tmp_mov_vec );
			mov_add( olsr_con->left_olsr_node->mov_vec, tmp_mov_vec, distance / 100 ); /* move a little bit to point zero */
			mov_add( olsr_con->left_olsr_node->mov_vec, tmp_mov_vec, 1 ); /* move a little bit to point zero */

			if ( ( distance = dist( olsr_con->left_olsr_node->mov_vec, null_vec ) ) > 10.0 ) {
				mov_add( olsr_con->left_olsr_node->pos_vec, olsr_con->left_olsr_node->mov_vec, 1.0 / ( ( float ) distance ) );
			} else {
				mov_add( olsr_con->left_olsr_node->pos_vec, olsr_con->left_olsr_node->mov_vec, 0.1 );
			}

			s3d_translate( olsr_con->left_olsr_node->obj_id, olsr_con->left_olsr_node->pos_vec[0], olsr_con->left_olsr_node->pos_vec[1], olsr_con->left_olsr_node->pos_vec[2] );

			/* reset movement vector */
			olsr_con->left_olsr_node->mov_vec[0] = olsr_con->left_olsr_node->mov_vec[1] = olsr_con->left_olsr_node->mov_vec[2] = 0.0;

		}

		/* move right olsr node if it has not been moved yet */
		if ( !( ( olsr_con->right_olsr_node->mov_vec[0] == 0 ) && ( olsr_con->right_olsr_node->mov_vec[1] == 0 ) && ( olsr_con->right_olsr_node->mov_vec[2] == 0 ) ) && olsr_con->right_olsr_node->visible ) {

			distance = dirt( olsr_con->right_olsr_node->pos_vec, null_vec, tmp_mov_vec );
			mov_add( olsr_con->right_olsr_node->mov_vec, tmp_mov_vec, distance / 100 ); /* move a little bit to point zero */
			mov_add( olsr_con->right_olsr_node->mov_vec, tmp_mov_vec, 1 ); /* move a little bit to point zero */

			if ( ( distance = dist( olsr_con->right_olsr_node->mov_vec, null_vec ) ) > 10.0 ) {
				mov_add( olsr_con->right_olsr_node->pos_vec, olsr_con->right_olsr_node->mov_vec, 1.0 / ( ( float ) distance ) );
			} else {
				mov_add( olsr_con->right_olsr_node->pos_vec, olsr_con->right_olsr_node->mov_vec, 0.1 );
			}

			s3d_translate( olsr_con->right_olsr_node->obj_id, olsr_con->right_olsr_node->pos_vec[0], olsr_con->right_olsr_node->pos_vec[1], olsr_con->right_olsr_node->pos_vec[2] );

			/* reset movement vector */
			olsr_con->right_olsr_node->mov_vec[0] = olsr_con->right_olsr_node->mov_vec[1] = olsr_con->right_olsr_node->mov_vec[2] = 0.0;

		}


		/* move connection between left and right olsr node */
		vertex_buf[0] = olsr_con->left_olsr_node->pos_vec[0];
		vertex_buf[1] = olsr_con->left_olsr_node->pos_vec[1];
		vertex_buf[2] = olsr_con->left_olsr_node->pos_vec[2];
		vertex_buf[3] = olsr_con->right_olsr_node->pos_vec[0];
		vertex_buf[4] = olsr_con->right_olsr_node->pos_vec[1];
		vertex_buf[5] = olsr_con->right_olsr_node->pos_vec[2];

		s3d_pep_vertices( olsr_con->obj_id, vertex_buf, 2 );


		if ( ColorSwitch ) {

			/* HNA */
			if ( olsr_con->left_etx == -1000.00 ) {

				if(olsr_con->color != 1) {
					s3d_pep_material( olsr_con->obj_id,
								   0.0,0.0,1.0,
								   0.0,0.0,1.0,
								   0.0,0.0,1.0);
					olsr_con->color = 1;
				}

			} else {

				etx = ( olsr_con->left_etx + olsr_con->right_etx ) / 2.0;

				/* very good link - bright blue */
				if ( ( etx >= 1.0 ) && ( etx < 1.5 ) ) {

					if(olsr_con->color != 2) {
						s3d_pep_material( olsr_con->obj_id,
								0.5,1.0,1.0,
								0.5,1.0,1.0,
								0.5,1.0,1.0);
						olsr_con->color = 2;
					}

				/* good link - bright yellow */
				} else if ( ( etx >= 1.5 ) && ( etx < 2.0 ) ) {

					rgb = 2.0 - etx;
					if( olsr_con->color != 3 || (olsr_con->color == 3 && (int) rintf(olsr_con->rgb * 10) !=  (int) rintf(rgb * 10))) {
						s3d_pep_material( olsr_con->obj_id,
								1.0,1.0,rgb,
								1.0,1.0,rgb,
								1.0,1.0,rgb);
						olsr_con->color = 3;

						olsr_con->rgb =  rgb;
					}

				/* not so good link - orange */
				} else if ( ( etx >= 2.0 ) && ( etx < 3.0 ) ) {

					rgb = 1.5 - ( etx / 2.0 );
					if( olsr_con->color != 4 || (olsr_con->color == 4 && (int) rintf(olsr_con->rgb * 10) !=  (int) rintf(rgb * 10))) {
						s3d_pep_material( olsr_con->obj_id,
								1.0,rgb,0.0,
								1.0,rgb,0.0,
								1.0,rgb,0.0);
						olsr_con->color = 4;

						olsr_con->rgb = rgb;
					}

				/* bad link (almost dead) - brown */
				} else if ( ( etx >= 3.0 ) && ( etx < 5.0 ) ) {

					rgb = 1.75 - ( etx / 4.0 );

					if( olsr_con->color != 5 || (olsr_con->color == 5 && (int) rintf(olsr_con->rgb * 10) !=  (int) rintf(rgb * 10)) ) {

						s3d_pep_material( olsr_con->obj_id,
								rgb,rgb - 0.5,0.0,
								rgb,rgb - 0.5,0.0,
								rgb,rgb - 0.5,0.0);
						olsr_con->color = 5;

						olsr_con->rgb = rgb;
					}

				/* zombie link - grey */
				} else if ( ( etx >= 5.0 ) && ( etx < 1000.0 ) ) {

					rgb = 1000.0 / ( 1500.0 + etx );

					if( olsr_con->color != 6 || (olsr_con->color == 6 && (int) rintf(olsr_con->rgb * 10) !=  (int) rintf(rgb * 10)) ) {

						s3d_pep_material( olsr_con->obj_id,
								rgb,rgb,rgb,
								rgb,rgb,rgb,
								rgb,rgb,rgb);
						olsr_con->color = 6;

						olsr_con->rgb = rgb;
					}

				/* wtf - dark grey */
				} else {

					if(olsr_con->color != 7) {
						s3d_pep_material( olsr_con->obj_id,
								0.3,0.3,0.3,
								0.3,0.3,0.3,
								0.3,0.3,0.3);
						olsr_con->color = 7;
					}

				}

			}

		} else {

			if(olsr_con->color != 0) {
				s3d_pep_material( olsr_con->obj_id,
							1.0,1.0,1.0,
							1.0,1.0,1.0,
							1.0,1.0,1.0);
				olsr_con->color = 0;
			}

		}


		olsr_con = olsr_con->next_olsr_con;

	}

}



void mainloop() {

	int net_result;   /* result of function net_main */
	char nc_str[20];
	float strLen;
	float target,current;
	int i;
	/* calculate new movement vector */
	calc_olsr_node_mov();

	/* prepare nodes */
	handle_olsr_node( Olsr_root );

	/* move it */
	move_olsr_nodes();

	/* if we have more or less nodes now - redraw node count */
	if ( Olsr_node_count != Last_olsr_node_count ) {

		if ( Olsr_node_count_obj != -1 ) s3d_del_object( Olsr_node_count_obj );
		snprintf( nc_str, 20, "node count: %d", Olsr_node_count );
		Olsr_node_count_obj = s3d_draw_string( nc_str, &strLen );
		s3d_link( Olsr_node_count_obj, 0 );
		s3d_flags_on( Olsr_node_count_obj, S3D_OF_VISIBLE );
		s3d_scale( Olsr_node_count_obj, 0.2 );
		s3d_translate( Olsr_node_count_obj, -Left*3.0-(strLen * 0.2), -Bottom*3.0-0.5, -3.0 );
		Last_olsr_node_count = Olsr_node_count;

	}

	if ( Output_block_completed ) {

		Output_block_counter++;
		Output_block_completed = 0;

	}

	/* read data from socket */
	Net_read_count = 0;
	while ( ( net_result = net_main() ) != 0 ) {
		if ( net_result == -1 ) {
			s3d_quit();
			break;
		}
	}

	/* move to terminal */
	if(move_cam_to != -1 && move_cam_to == obj[obj_term]->oid)
	{
		oid_focus = obj[obj_term]->oid;
		for( i=0; i<3; i++)
		{
			CamPosition[0][i]=(CamPosition[0][i]*4+obj[obj_term]->poi[i])/5;

			target = obj[obj_term]->rot[i];
			current = CamPosition[1][i];

			if( obj[obj_term]->rot[i] - CamPosition[1][i] > 180 )
				target = obj[obj_term]->rot[i] - 360;
			if( obj[obj_term]->rot[i] - CamPosition[1][i] < -180 )
				current = CamPosition[1][i] - 360;
			CamPosition[1][i]=(CamPosition[1][i]*4+target)/5;
		}
		s3d_translate(0,CamPosition[0][0],CamPosition[0][1],CamPosition[0][2]);
		s3d_rotate(0,CamPosition[1][0],CamPosition[1][1],CamPosition[1][2]);

		if (dist(CamPosition[0],obj[obj_term]->poi)<0.2)
		{
			s3d_translate(0,obj[obj_term]->poi[0],obj[obj_term]->poi[1],obj[obj_term]->poi[2]);
			s3d_rotate(0,obj[obj_term]->rot[0],(obj[obj_term]->rot[1]),obj[obj_term]->rot[2]);
			move_cam_to = -1;
		}
	}
	
	/* move back to returnPoint */
	if(move_cam_to == -2)
	{
		oid_focus = -1;
		for( i=0; i<3; i++)
		{
			CamPosition[0][i]=(CamPosition[0][i]*4+returnPoint[0][i])/5;

			target = returnPoint[1][i];
			current = CamPosition[1][i];

			if( returnPoint[1][i] - CamPosition[1][i] > 180 )
				target = returnPoint[1][i] - 360;
			if( returnPoint[1][i] - CamPosition[1][i] < -180 )
				current = CamPosition[1][i] - 360;
			CamPosition[1][i]=(CamPosition[1][i]*4+target)/5;
		}
		s3d_translate(0,CamPosition[0][0],CamPosition[0][1],CamPosition[0][2]);
		s3d_rotate(0,CamPosition[1][0],CamPosition[1][1],CamPosition[1][2]);

		if (dist(CamPosition[0],returnPoint[0])<0.2)
		{
			s3d_translate(0,returnPoint[0][0],returnPoint[0][1],returnPoint[0][2]);
			s3d_rotate(0,returnPoint[1][0],returnPoint[1][1],returnPoint[1][2]);
			move_cam_to = -1;
		}
	}

	rotate_cursor();

	if( Olsr_ip_label_obj != -1 )
	{
		print_etx();
	}

	if(RotateSwitch) {
		Zp_rotate = (Zp_rotate+RotateSpeed)%360;
		s3d_rotate(ZeroPoint,0,Zp_rotate,0);
	}
	CamPosition2[0][0]=  CamPosition[0][0]*cos(Zp_rotate*M_PI/180.0) - CamPosition[0][2] * sin (Zp_rotate*M_PI/180.0);
	CamPosition2[0][1]=  CamPosition[0][1];
	CamPosition2[0][2]=  CamPosition[0][0]*sin(Zp_rotate*M_PI/180.0) + CamPosition[0][2] * cos (Zp_rotate*M_PI/180.0);

	if (cam_go && move_cam_to == -1)
	{ /* move a little bit closer ... */
		CamPosition[0][0]=(CamPosition[0][0]*9+Olsr_node_pEtx->pos_vec[0])/10;
		CamPosition[0][1]=(CamPosition[0][1]*9+Olsr_node_pEtx->pos_vec[1])/10;
		CamPosition[0][2]=(CamPosition[0][2]*9+Olsr_node_pEtx->pos_vec[2])/10;
		s3d_translate(0,CamPosition[0][0],CamPosition[0][1],CamPosition[0][2]);
		if (dist(CamPosition[0],Olsr_node_pEtx->pos_vec)<5) /* close enough? stop! */
				cam_go=0;
	}

	nanosleep( &sleep_time, NULL );

	return;

}

void stop() {

	s3d_quit();
	net_quit();

}

/***
 *
 * eventhandler when key pressed
 *
 ***/

void keypress(struct s3d_evt *event) {
	int key,i;
	key=*((unsigned short *)event->buf);

	if(oid_focus != obj[obj_term]->oid)
	{
		switch(key) {
			case S3DK_ESCAPE: /* esc -> close olsr */
				stop();
				break;
			case 'c': /* c -> color on/off */
				if(ColorSwitch) ColorSwitch = 0;
				else ColorSwitch = 1;
				break;
			case 'r': /* r -> rotate start/stop*/
				if(RotateSwitch) RotateSwitch = 0;
				else RotateSwitch = 1;
				break;
			case 't': /* t -> move to terminal*/
				for(i=0;i<3;i++)
				{
					returnPoint[0][i] = CamPosition[0][i];
					returnPoint[1][i] = CamPosition[1][i];
				}
				move_cam_to = obj[obj_term]->oid;
				break;
			case '+': /* + -> rotate speed increase*/
				if(RotateSwitch && RotateSpeed < 10)
					RotateSpeed++;
				break;
			case '-': /* - -> rotate speed decrease*/
				if(RotateSwitch && RotateSpeed > 1)
					RotateSpeed--;
				break;
			case 16: /* strg + p -> reset nodes ( zeroPoint to 0,0,0 ) */
				s3d_translate(ZeroPoint,0.0,0.0,0.0);
				ZeroPosition[0] = ZeroPosition[1] = ZeroPosition[2] = 0.0;
				break;
			case S3DK_UP: /* arrow up -> move nodes up */
				ZeroPosition[1]++;
				s3d_translate(ZeroPoint,ZeroPosition[0],ZeroPosition[1],ZeroPosition[2]);
				break;
			case S3DK_DOWN: /* arrow down -> move nodes down */
				ZeroPosition[1]--;
				s3d_translate(ZeroPoint,ZeroPosition[0],ZeroPosition[1],ZeroPosition[2]);
				break;
			case S3DK_PAGEUP: /* page up -> change factor in calc_olsr_node_mov */
				if(Factor < 0.9)
					Factor += 0.1;
				break;
			case S3DK_PAGEDOWN: /* page down -> change factor in calc_olsr_node_mov */
				if(Factor > 0.3)
					Factor -= 0.1;
				break;
		}
	} else {
		if( (key >= 48 && key <= 57) || key == 46 || key == 13 || key == 8 )
			write_terminal(key);
		if ( key == S3DK_ESCAPE )
		{
			oid_focus = -1;
			move_cam_to = -2;
		}
	}
}

/***
 *
 * eventhandler when object clicked
 *
 ***/

void object_click(struct s3d_evt *evt)
{
	int oid;
	/* float distance,tmp_vector[3]; */
	char ip_str[50];

	oid=(int)*((unsigned long *)evt->buf);

	if( oid == Btn_close_id )
	{
		int i;
		s3d_del_object(Btn_close_id);
		s3d_del_object(Olsr_ip_label_obj);
		Btn_close_id = Olsr_ip_label_obj = -1;
		for(i=0; i < Size; i++)
			s3d_del_object( Olsr_neighbour_label_obj[i] );
		free(Olsr_neighbour_label_obj);
		Olsr_neighbour_label_obj = NULL;
		for(i = 0; i < 4; i++)
		{
			if(Output_border[i] != -1)
				s3d_del_object(Output_border[i]);
			Output_border[i] = -1;
		}
		return;
	}

	Olsr_node_pEtx = *lst_search(oid);

	if( Btn_close_id == -1 && Olsr_node_pEtx != NULL )
	{
		Btn_close_id = s3d_clone( Btn_close_obj );
		s3d_link(Btn_close_id,0);
		s3d_flags_on(Btn_close_id,S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
		s3d_scale( Btn_close_id, 0.05 );
		s3d_translate( Btn_close_id,-Left*3.0-0.125, -Bottom*3.0-0.7, -3.0 );
		if ( Olsr_ip_label_obj != -1 ) s3d_del_object( Olsr_ip_label_obj );
		snprintf( ip_str, 35, "ip: %s", Olsr_node_pEtx->ip );
		Olsr_ip_label_obj = s3d_draw_string( ip_str, &Title_len );
		s3d_link( Olsr_ip_label_obj, 0 );
		s3d_flags_on( Olsr_ip_label_obj, S3D_OF_VISIBLE );
		s3d_scale( Olsr_ip_label_obj, 0.2 );
		s3d_translate( Olsr_ip_label_obj,-Left*3.0-(Title_len * 0.2)-0.15, -Bottom*3.0-1.0, -3.0 );
		cam_go=1;
		if ( Olsr_ip_label_obj != -1 ) s3d_del_object( Olsr_ip_label_obj );
		snprintf( ip_str, 35, "ip: %s", Olsr_node_pEtx->ip );
		Olsr_ip_label_obj = s3d_draw_string( ip_str, &Title_len );
		s3d_link( Olsr_ip_label_obj, 0 );
		s3d_flags_on( Olsr_ip_label_obj, S3D_OF_VISIBLE );
		s3d_scale( Olsr_ip_label_obj, 0.2 );
		s3d_translate( Olsr_ip_label_obj,-Left*3.0-(Title_len * 0.2)-0.15, -Bottom*3.0-1.0, -3.0 );
	} else {
		/* object_handler(oid); */
	}
}

void print_etx()
{
	struct olsr_neigh_list *tmpNeighbour;
	float p = 1.2;
	int i;
	float len = 0.0, max_len=0.0;

	if( Olsr_neighbour_label_obj != NULL )
	{
		/* int n = sizeof(Olsr_neighbour_label_obj) / sizeof(int);*/
		for(i=0; i < Size; i++)
			s3d_del_object( Olsr_neighbour_label_obj[i] );
		free(Olsr_neighbour_label_obj);
		Olsr_neighbour_label_obj = NULL;
	}

	tmpNeighbour = Olsr_node_pEtx->olsr_neigh_list;

	Size = 0;
	while(tmpNeighbour != NULL)
	{
		Size++;
		tmpNeighbour = tmpNeighbour->next_olsr_neigh_list;
	}

	Olsr_neighbour_label_obj = malloc(Size*sizeof(int));
	tmpNeighbour = Olsr_node_pEtx->olsr_neigh_list;

	for(i = 0; i < Size ;i++)
	{
		char nIpStr[60];
		float mEtx = ( tmpNeighbour->olsr_con->left_etx + tmpNeighbour->olsr_con->right_etx ) / 2;

		if( mEtx != -1000 )
			snprintf(nIpStr, 60, "%15s --> %.2f",(strcmp(Olsr_node_pEtx->ip,tmpNeighbour->olsr_con->right_olsr_node->ip)?tmpNeighbour->olsr_con->right_olsr_node->ip:tmpNeighbour->olsr_con->left_olsr_node->ip),mEtx);
		else
			snprintf(nIpStr, 60, "%15s --> HNA",(strcmp(Olsr_node_pEtx->ip,tmpNeighbour->olsr_con->right_olsr_node->ip)?tmpNeighbour->olsr_con->right_olsr_node->ip:tmpNeighbour->olsr_con->left_olsr_node->ip));

		Olsr_neighbour_label_obj[i] = s3d_draw_string( nIpStr, &len );
		s3d_link(Olsr_neighbour_label_obj[i], 0);
		s3d_flags_on(Olsr_neighbour_label_obj[i], S3D_OF_VISIBLE );
		s3d_scale(Olsr_neighbour_label_obj[i], 0.2 );
		s3d_translate(Olsr_neighbour_label_obj[i], -Left*3.0-(len * 0.2)-0.15, -Bottom*3.0-p, -3.0 );
		tmpNeighbour = tmpNeighbour->next_olsr_neigh_list;
		p += 0.2;
		max_len = (len > max_len - 0.2)?len+0.2:max_len;
		max_len = (Title_len > max_len - 0.2)?len+0.2:max_len;
		/* printf("title: %f len: %f maxlen: %f %s\n",Title_len,len,max_len-0.2,nIpStr);*/
	}

	if( Btn_close_id != -1)
	{
		if( Output_border[0] == -1 )
		{
			for(i = 0; i < 4; i++)
			{
				Output_border[i] = s3d_new_object();
				s3d_push_material( Output_border[i],
					1.0,1.0,1.0,
					1.0,1.0,1.0,
					1.0,1.0,1.0);
			}
			s3d_push_vertex(Output_border[0], -Left*3.0-0.2,			-Bottom*3.0-0.7, -3.0);
			s3d_push_vertex(Output_border[0], -Left*3.0-(max_len*0.2),	-Bottom*3.0-0.7, -3.0);

			s3d_push_vertex(Output_border[1], -Left*3.0-0.1,			-Bottom*3.0-0.8, -3.0);
			s3d_push_vertex(Output_border[1], -Left*3.0-0.1,			-Bottom*3.0-p, 	-3.0);

			s3d_push_vertex(Output_border[2], -Left*3.0-0.1,			-Bottom*3.0-p, 	-3.0);
			s3d_push_vertex(Output_border[2], -Left*3.0-(max_len*0.2),	-Bottom*3.0-p, 	-3.0);

			s3d_push_vertex(Output_border[3], -Left*3.0-(max_len*0.2),	-Bottom*3.0-0.7, -3.0);
			s3d_push_vertex(Output_border[3], -Left*3.0-(max_len*0.2),	-Bottom*3.0-p, 	-3.0);

			s3d_push_line( Output_border[0], 0,1,0);
			s3d_push_line( Output_border[1], 0,1,0);
			s3d_push_line( Output_border[2], 0,1,0);
			s3d_push_line( Output_border[3], 0,1,0);

			s3d_flags_on( Output_border[0], S3D_OF_VISIBLE);
			s3d_flags_on( Output_border[1], S3D_OF_VISIBLE);
			s3d_flags_on( Output_border[2], S3D_OF_VISIBLE);
			s3d_flags_on( Output_border[3], S3D_OF_VISIBLE);

			s3d_link( Output_border[0], 0);
			s3d_link( Output_border[1], 0);
			s3d_link( Output_border[2], 0);
			s3d_link( Output_border[3], 0);
		} else {
			s3d_pop_vertex(Output_border[0], 2);
			s3d_pop_vertex(Output_border[1], 2);
			s3d_pop_vertex(Output_border[2], 2);
			s3d_pop_vertex(Output_border[3], 2);
			s3d_push_vertex(Output_border[0], -Left*3.0-0.2,				-Bottom*3.0-0.7, -3.0);
			s3d_push_vertex(Output_border[0], -Left*3.0-(max_len*0.2),	-Bottom*3.0-0.7, -3.0);

			s3d_push_vertex(Output_border[1], -Left*3.0-0.1,				-Bottom*3.0-0.8, -3.0);
			s3d_push_vertex(Output_border[1], -Left*3.0-0.1,				-Bottom*3.0-p,	 -3.0);

			s3d_push_vertex(Output_border[2], -Left*3.0-0.1,				-Bottom*3.0-p,	 -3.0);
			s3d_push_vertex(Output_border[2], -Left*3.0-(max_len*0.2),	-Bottom*3.0-p,	 -3.0);

			s3d_push_vertex(Output_border[3], -Left*3.0-(max_len*0.2),	-Bottom*3.0-0.7, -3.0);
			s3d_push_vertex(Output_border[3], -Left*3.0-(max_len*0.2),	-Bottom*3.0-p, 	 -3.0);
		}
	}
}



/***
 *
 * eventhandler when object change by user
 * such as Cam
 *
 ***/

void object_info(struct s3d_evt *hrmz)
{
	struct s3d_obj_info *inf;
	inf=(struct s3d_obj_info *)hrmz->buf;
	if (inf->object==0)
	{
		CamPosition[0][0] = inf->trans_x;
		CamPosition[0][1] = inf->trans_y;
		CamPosition[0][2] = inf->trans_z;
		CamPosition[1][0] = inf->rot_x;
		CamPosition[1][1] = inf->rot_y;
		CamPosition[1][2] = inf->rot_z;
		Asp=inf->scale;
		if (Asp>1.0) /* wide screen */
		{
			Bottom=-1.0;
			Left=-Asp;
		} else {  /* high screen */
			Bottom=(-1.0/Asp);
			Left=-1.0;

		}

	}
	/* printf("%f %f %f\n",inf->trans_x,inf->trans_y,inf->trans_z); */
}

void mbutton_press(struct s3d_evt *hrmz)
{
	struct s3d_but_info *inf;
	inf=(struct s3d_but_info *)hrmz->buf;
	printf("button %d, state %d\n", inf->button,inf->state);
	return;
}

void initialize_objects()
{
	create_terminal();
	create_cursor();
}

int main( int argc, char *argv[] ) {

	int optchar;
	strncpy( Olsr_host, "127.0.0.1", 256 );
	lbuf[0] = '\0';   /* init lbuf */

	while ( ( optchar = getopt ( argc, argv, "dhH:" ) ) != -1 ) {

		switch ( optchar ) {

			case 'd':
				Debug = 1;
				break;

			case 'H':
				strncpy( Olsr_host, optarg, 256 );
				break;

			case 'h':
			default:
				print_usage();
				return (0);

		}

	}

	if ( Debug ) printf( "debug mode enabled ...\n" );

	/* initialize obj2ip linked list */
	lst_initialize();

	/* delete olsrs3d options */
	while ( ( optind < argc ) && ( argv[optind][0] != '-' ) ) optind++;   /* optind may point to ip addr of '-H' */
	optind--;
	argv[optind] = argv[0];   /* save program path */
	argc -= optind;   /* jump over olsrs3d options */
	argv += optind;

	/* set extern int optind = 0 for parse_args in io.c */
	optind = 0;


	if (!net_init(Olsr_host))
	{
		if (!s3d_init(&argc,&argv,"olsrs3d"))
		{
			s3d_set_callback(S3D_EVENT_OBJ_INFO,object_info);
			/* s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click); */
			s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);
			s3d_set_callback(S3D_EVENT_MBUTTON,mbutton_press);
			s3d_set_callback(S3D_EVENT_KEY,keypress);
			s3d_set_callback(S3D_EVENT_QUIT,stop);
			/* s3d_set_callback(S3D_EVENT_MBUTTON,mbutton_click); */
			if (s3d_select_font("vera"))
				printf("font not found\n");
			Olsr_node_obj = s3d_import_3ds_file( "objs/accesspoint.3ds" );
			Olsr_node_inet_obj = s3d_import_3ds_file( "objs/accesspoint_inet.3ds" );
			Olsr_node_hna_net = s3d_import_3ds_file( "objs/internet.3ds" );
			Btn_close_obj = s3d_import_3ds_file("objs/btn_close.3ds");
			ZeroPoint = s3d_new_object();
			Output_border[0] = Output_border[1] = Output_border[2] = Output_border[3] = -1;

			/* create system objects */
			initialize_objects();

			s3d_mainloop(mainloop);
			s3d_quit();
			net_quit();
		}
	}
	return(0);
}

