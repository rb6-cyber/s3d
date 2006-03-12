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
#include <unistd.h>	/* sleep() */
#include <string.h>	/* strncpy() */
#include <math.h>		/* sqrt() */
#include <getopt.h>	/* getopt() */
#include <stdlib.h>	/* exit() */
#include "olsrs3d.h"
#define SPEED		10.0

int Debug = 0;

char Olsr_host[256];   /* ip or hostname of olsr node with running dot_draw plugin */

struct olsr_con *Con_begin = NULL;   /* begin of connection list */
struct olsr_node *Olsr_root = NULL;   /* top of olsr node tree */
struct Obj_to_ip *Obj_to_ip_head, *Obj_to_ip_end, *List_ptr;   /* needed pointer for linked list */

int Olsr_node_count = 0, Last_olsr_node_count = -1;
int Olsr_node_count_obj = -1;

int Byte_count;

int Olsr_node_obj, Olsr_node_inet_obj, Olsr_node_hna_net;

float Asp = 1.0;
float Bottom = -1.0;
float Left = -1.0;

float CamPosition[2][3];   /* CamPosition[trans|rot][x-z] */
float ZeroPosition[3] = {0,0,0};   /* current position zero position */
int ZeroPoint;   /* object zeropoint */
int Zp_rotate = 0;
int ColorSwitch = 0;   /* enable/disable colored olsr connections */
int RotateSwitch = 0;
int RotateSpeed = 2;


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

	float f, distance;
	float tmp_mov_vec[3];
	struct olsr_node *other_node;
	struct Obj_to_ip *Obj_to_ip_curr;
	struct olsr_neigh_list *olsr_neigh_list, *prev_olsr_neigh_list, *other_node_neigh_list, *tmp_olsr_neigh_list;

	/* no more nodes left */
	if ( olsr_node == NULL ) return;

	/* olsr node vanished */
	if ( olsr_node->last_seen > 0 ) olsr_node->last_seen--;

	if ( olsr_node->last_seen == 0 && olsr_node->visible ) {

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
			olsr_node->desc_id = s3d_draw_string( olsr_node->ip, &f );
			s3d_link( olsr_node->desc_id, olsr_node->obj_id );
			s3d_translate( olsr_node->desc_id, -f/2,-2,0 );
			s3d_flags_on( olsr_node->desc_id, S3D_OF_VISIBLE );

			olsr_node->node_type_modified = 0;

		}

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

	float f, distance;
	float tmp_mov_vec[3];
	struct olsr_con *olsr_con = Con_begin;

	while ( olsr_con != NULL ) {

		if ( ( olsr_con->left_etx != 0.0 ) && ( olsr_con->right_etx != 0.0  ) ) {

			distance = dirt( olsr_con->left_olsr_node->pos_vec, olsr_con->right_olsr_node->pos_vec, tmp_mov_vec );
			f = (( olsr_con->left_etx + olsr_con->right_etx ) / 4.0 ) / distance;
			if ( f < 0.3 ) f = 0.3;

			mov_add( olsr_con->left_olsr_node->mov_vec, tmp_mov_vec, 1 / f - 1 );
			mov_add( olsr_con->right_olsr_node->mov_vec, tmp_mov_vec, - ( 1 / f - 1 ) );

		}

		olsr_con = olsr_con->next_olsr_con;

	}

}



/***
 *
 * move all olsr nodes and their connections
 *
 ***/

void move_olsr_nodes( void ) {

	float null_vec[3] = {0,0,0};
	float tmp_mov_vec[3];
	float distance, factor, etx, rgb;
	struct olsr_con *olsr_con = Con_begin;

	while ( olsr_con != NULL ) {

		/* move left olsr node if it has not been moved yet */
		if ( !( ( olsr_con->left_olsr_node->mov_vec[0] == 0 ) && ( olsr_con->left_olsr_node->mov_vec[1] == 0 ) && ( olsr_con->left_olsr_node->mov_vec[2] == 0 ) ) && olsr_con->left_olsr_node->visible ) {

// 			if ( ( dist( olsr_con->left_olsr_node->mov_vec, olsr_con->left_olsr_node->pos_vec ) > 10.0 ) || ( dist( olsr_con->left_olsr_node->pos_vec, null_vec ) < 15.0 ) ) {

				distance = dirt( olsr_con->left_olsr_node->pos_vec, null_vec, tmp_mov_vec );
				mov_add( olsr_con->left_olsr_node->mov_vec, tmp_mov_vec, distance / 100 ); /* move a little bit to point zero */
				mov_add( olsr_con->left_olsr_node->mov_vec, tmp_mov_vec, 1 ); /* move a little bit to point zero */

				if ( ( distance = dist( olsr_con->left_olsr_node->mov_vec, null_vec ) ) > 10.0 ) {
					factor = 1.0 / ( (float ) distance );
				} else {
					factor = 0.1;
				}

				olsr_con->left_olsr_node->mov_vec[0] *= factor;
				olsr_con->left_olsr_node->mov_vec[1] *= factor;
				olsr_con->left_olsr_node->mov_vec[2] *= factor;

				tmp_mov_vec[0] = olsr_con->left_olsr_node->pos_vec[0];
				tmp_mov_vec[1] = olsr_con->left_olsr_node->pos_vec[1];
				tmp_mov_vec[2] = olsr_con->left_olsr_node->pos_vec[2];

				mov_add( tmp_mov_vec, olsr_con->left_olsr_node->mov_vec, 1.0 );

				/*if ( ( dist( tmp_mov_vec, olsr_con->left_olsr_node->pos_vec ) > 0.75 ) || ( dist( tmp_mov_vec, null_vec ) < 2.0 ) ) {*/

				mov_add( olsr_con->left_olsr_node->pos_vec, olsr_con->left_olsr_node->mov_vec, 1.0 );
				s3d_translate( olsr_con->left_olsr_node->obj_id, olsr_con->left_olsr_node->pos_vec[0], olsr_con->left_olsr_node->pos_vec[1], olsr_con->left_olsr_node->pos_vec[2] );

			/*}*/

			/* reset movement vector */
			olsr_con->left_olsr_node->mov_vec[0] = olsr_con->left_olsr_node->mov_vec[1] = olsr_con->left_olsr_node->mov_vec[2] = 0.0;

		}

		/* move right olsr node if it has not been moved yet */
		if ( !( ( olsr_con->right_olsr_node->mov_vec[0] == 0 ) && ( olsr_con->right_olsr_node->mov_vec[1] == 0 ) && ( olsr_con->right_olsr_node->mov_vec[2] == 0 ) ) && olsr_con->right_olsr_node->visible ) {

// 			if ( ( dist( olsr_con->right_olsr_node->mov_vec, olsr_con->right_olsr_node->pos_vec )> 10.0 ) || ( dist( olsr_con->right_olsr_node->pos_vec, null_vec ) < 15.0 ) ) {

				distance = dirt( olsr_con->right_olsr_node->pos_vec, null_vec, tmp_mov_vec );
				mov_add( olsr_con->right_olsr_node->mov_vec, tmp_mov_vec, distance / 100 ); /* move a little bit to point zero */
				mov_add( olsr_con->right_olsr_node->mov_vec, tmp_mov_vec, 1 ); /* move a little bit to point zero */

				if ( ( distance = dist( olsr_con->right_olsr_node->mov_vec, null_vec ) ) > 10.0 ) {
					factor = 1.0 / ( (float ) distance );
				} else {
					factor = 0.1;
				}

				olsr_con->right_olsr_node->mov_vec[0] *= factor;
				olsr_con->right_olsr_node->mov_vec[1] *= factor;
				olsr_con->right_olsr_node->mov_vec[2] *= factor;

				tmp_mov_vec[0] = olsr_con->right_olsr_node->pos_vec[0];
				tmp_mov_vec[1] = olsr_con->right_olsr_node->pos_vec[1];
				tmp_mov_vec[2] = olsr_con->right_olsr_node->pos_vec[2];

				mov_add( tmp_mov_vec, olsr_con->right_olsr_node->mov_vec, 1.0 );

				/*if ( ( dist( tmp_mov_vec, olsr_con->right_olsr_node->pos_vec )> 0.75 ) || ( dist( tmp_mov_vec, null_vec ) < 2.0 ) ) {*/

					mov_add( olsr_con->right_olsr_node->pos_vec, olsr_con->right_olsr_node->mov_vec, 1.0 );
					s3d_translate( olsr_con->right_olsr_node->obj_id, olsr_con->right_olsr_node->pos_vec[0], olsr_con->right_olsr_node->pos_vec[1], olsr_con->right_olsr_node->pos_vec[2] );

				/*}*/

			/* reset movement vector */
			olsr_con->right_olsr_node->mov_vec[0] = olsr_con->right_olsr_node->mov_vec[1] = olsr_con->right_olsr_node->mov_vec[2] = 0.0;

		}


		/* move connection between left and right olsr node */
		s3d_pop_vertex( olsr_con->obj_id, 6 );
		s3d_pop_polygon( olsr_con->obj_id, 2 );
		s3d_pop_material( olsr_con->obj_id, 1 );
		
		s3d_push_vertex( olsr_con->obj_id, olsr_con->left_olsr_node->pos_vec[0] , olsr_con->left_olsr_node->pos_vec[1] , olsr_con->left_olsr_node->pos_vec[2] );
		s3d_push_vertex( olsr_con->obj_id, olsr_con->left_olsr_node->pos_vec[0] + 0.2 , olsr_con->left_olsr_node->pos_vec[1] , olsr_con->left_olsr_node->pos_vec[2] );
		s3d_push_vertex( olsr_con->obj_id, olsr_con->left_olsr_node->pos_vec[0] - 0.2 , olsr_con->left_olsr_node->pos_vec[1] , olsr_con->left_olsr_node->pos_vec[2] );

		s3d_push_vertex( olsr_con->obj_id, olsr_con->right_olsr_node->pos_vec[0] , olsr_con->right_olsr_node->pos_vec[1] , olsr_con->right_olsr_node->pos_vec[2] );
		s3d_push_vertex( olsr_con->obj_id, olsr_con->right_olsr_node->pos_vec[0] , olsr_con->right_olsr_node->pos_vec[1]+ 0.2 , olsr_con->right_olsr_node->pos_vec[2] );
		s3d_push_vertex( olsr_con->obj_id, olsr_con->right_olsr_node->pos_vec[0] , olsr_con->right_olsr_node->pos_vec[1]- 0.2 , olsr_con->right_olsr_node->pos_vec[2] );
		
		if ( ColorSwitch ) {

			/* HNA */
			if ( olsr_con->left_etx == -1000.00 ) {

				s3d_push_material( olsr_con->obj_id,
							   0.0,0.0,1.0,
							   0.0,0.0,1.0,
							   0.0,0.0,1.0);

			} else {

				etx = ( ( ( olsr_con->left_etx + olsr_con->right_etx ) / 2.0 ) - 10.0 ) * 10.0;

				if ( ( etx >= 1.0 ) && ( etx < 2.0 ) ) {

					rgb = etx - 1.0;
					s3d_push_material( olsr_con->obj_id,
								rgb,1.0,0.0,
								rgb,1.0,0.0,
								rgb,1.0,0.0);

				} else if ( ( etx >= 2.0 ) && ( etx < 3.0 ) ) {

					rgb = 3.0 - etx;
					s3d_push_material( olsr_con->obj_id,
								1.0,rgb,0.0,
								1.0,rgb,0.0,
								1.0,rgb,0.0);

				} else {

					s3d_push_material( olsr_con->obj_id,
								1.0,0.0,0.0,
								1.0,0.0,0.0,
								1.0,0.0,0.0);

				}

			}

		} else {

			s3d_push_material( olsr_con->obj_id,
						1.0,1.0,1.0,
						1.0,1.0,1.0,
						1.0,1.0,1.0);

		}

		s3d_push_polygon( olsr_con->obj_id, 0,4,5,0 );
		s3d_push_polygon( olsr_con->obj_id, 3,1,2,0 );

		olsr_con = olsr_con->next_olsr_con;

	}

}



void mainloop() {

	int net_result;   /* result of function net_main */
	char nc_str[20];

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
		Olsr_node_count_obj = s3d_draw_string( nc_str, NULL );
		s3d_link( Olsr_node_count_obj, 0 );
		s3d_flags_on( Olsr_node_count_obj, S3D_OF_VISIBLE );
		s3d_scale( Olsr_node_count_obj, 0.2 );
		s3d_translate( Olsr_node_count_obj, Left*3.0, -Bottom*3.0-0.2, -3.0 );

		Last_olsr_node_count = Olsr_node_count;

	}

	/* read data from socket */
	Byte_count = 0;
	while ( ( net_result = net_main() ) != 0 ) {
		if ( net_result == -1 ) {
			s3d_quit();
			break;
		}
	}

	if(RotateSwitch) {
		Zp_rotate = (Zp_rotate+RotateSpeed)%360;
		s3d_rotate(ZeroPoint,0,Zp_rotate,0);
	}
	usleep(100000);
/*	sleep(1);*/
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

	int key;
	key=*((unsigned short *)event->buf);
	switch(key) {
		case 27: /* esc */
			stop();
			break;
		case 15: /* strg + o */
			lst_out(); /* output ob2ip list */
			break;
		case 99: /* c */
			if(ColorSwitch) ColorSwitch = 0;
			else ColorSwitch = 1;
			break;
		case 114: /* r */
			if(RotateSwitch) RotateSwitch = 0;
			else RotateSwitch = 1;
			break;
		case 43: /* + */
			if(RotateSwitch && RotateSpeed < 10)
				RotateSpeed++;
			break;
		case 45: /* - */
			if(RotateSwitch && RotateSpeed > 1)
				RotateSpeed--;
			break;
		case 16: /* strg + p */
			s3d_translate(ZeroPoint,0.0,0.0,0.0);
			ZeroPosition[0] = ZeroPosition[1] = ZeroPosition[2] = 0.0; 
			break;
		case 101: /* arrow up */
			ZeroPosition[1]++;
			s3d_translate(ZeroPoint,ZeroPosition[0],ZeroPosition[1],ZeroPosition[2]);
			break;
		case 103: /* arrow down */
			ZeroPosition[1]--;
			s3d_translate(ZeroPoint,ZeroPosition[0],ZeroPosition[1],ZeroPosition[2]);
			break;
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
	float distance,tmp_vector[3];
	oid=(int)*((unsigned long *)evt->buf);
	struct olsr_node *olsr_node;
	olsr_node = *lst_search(oid);
	/* printf("obj2ip: search return %s\n",olsr_node->ip); */
	/*
	distance = dirt(CamPosition[0],olsr_node->pos_vec,tmp_vector);
	mov_add(ZeroPosition,tmp_vector,1.0);
	s3d_translate(ZeroPoint,ZeroPosition[0] * -1,ZeroPosition[1] * -1,ZeroPosition[2] * -1);
	*/
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
			s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);
			s3d_set_callback(S3D_EVENT_KEY,keypress);
			s3d_set_callback(S3D_EVENT_QUIT,stop);
			if (s3d_select_font("vera"))
				printf("font not found\n");
			Olsr_node_obj = s3d_import_3ds_file( "objs/accesspoint.3ds" );
			Olsr_node_inet_obj = s3d_import_3ds_file( "objs/accesspoint_inet.3ds" );
			Olsr_node_hna_net = s3d_import_3ds_file( "objs/internet.3ds" );
			ZeroPoint = s3d_new_object();
			s3d_mainloop(mainloop);
			s3d_quit();
			net_quit();
		}
	}
	return(0);
}

