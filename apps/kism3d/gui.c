/*
 * gui.c
 *
 * Copyright (C) 2006  Marek Lindner <lindner_marek@yahoo.de>
 *
 * This file is part of kism3d, an 802.11 visualizer for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * kism3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * kism3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsrs3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */



#include "kism3d.h"
#include <s3d.h>
#include <math.h>	/* M_PI, cos(), sin() */
#include <stdlib.h>	/* malloc(), free() */
#include <stdio.h>      /* printf() */
#include <time.h>	/* nanosleep()  */
#include <pthread.h>



int oid;
int r;

int wire_sphere(int slices, int stacks)
{
	int x,y,i,o;
	int num_v,num_l;
	float *v,*n;			/* vertices, normals */
	float alpha, beta;
	unsigned int *l;	/* lines */
	num_v=(stacks+1) * slices;
	num_l=stacks * slices+ (stacks-1) * slices; /* vertical + horizontal */
	v=malloc(sizeof(float) * 3 * num_v);
	n=malloc(sizeof(float) * 6 * num_l);
	l=malloc(sizeof(unsigned int) * 3 * num_l);
	i=0;
	for (x=0;x<slices;x++) {
		alpha=(x*360.0/slices)*M_PI/180.0;
		for (y=0;y<(stacks+1);y++) {
			beta=((y*180/slices)-90.0)*M_PI/180.0;
			v[i*3+0]=cos(alpha) * cos(beta);
			v[i*3+1]=sin(beta);
			v[i*3+2]=sin(alpha) * cos(beta);
			i++;
		}
	}
	i=0;
	for (x=0;x<slices;x++) {
		for (y=0;y<stacks;y++) {
			if ((y!=0) && (y!=stacks)) /* no horizontal lines at the poles */
			{
				l[i*3+0]=(x*(stacks+1))+y;
				l[i*3+1]=(((x+1)%slices)*(stacks+1))+y;
				l[i*3+2]=0;
				n[i*6+0]=v[ l[i*3+0]*3 + 0];
				n[i*6+1]=v[ l[i*3+0]*3 + 1];
				n[i*6+2]=v[ l[i*3+0]*3 + 2];
				n[i*6+3]=v[ l[i*3+1]*3 + 0];
				n[i*6+4]=v[ l[i*3+1]*3 + 1];
				n[i*6+5]=v[ l[i*3+1]*3 + 2];

				i++;

			}
			/* vertical lines */
			l[i*3+0]=(x*(stacks+1))+y;
			l[i*3+1]=(x*(stacks+1))+y+1;
			l[i*3+2]=0;
			n[i*6+0]=v[ l[i*3+0]*3 + 0];
			n[i*6+1]=v[ l[i*3+0]*3 + 1];
			n[i*6+2]=v[ l[i*3+0]*3 + 2];
			n[i*6+3]=v[ l[i*3+1]*3 + 0];
			n[i*6+4]=v[ l[i*3+1]*3 + 1];
			n[i*6+5]=v[ l[i*3+1]*3 + 2];
			i++;

		}
	}
	o=s3d_new_object();
	s3d_push_material(o,0,0,1,
			  1,0,0,
			  0,1,0);
	s3d_push_vertices(o,v,num_v);
	s3d_push_lines(o,l,num_l);
	s3d_load_line_normals(o,n,0,num_l);
	free(v);
	free(n);
	free(l);
	return(o);
}



int handle_networks() {

	struct list_head *network_pos;
	struct wlan_network *wlan_network;
	float real_node_pos_x, real_node_pos_z, label_str_len;
	int network_index = 0;
	char *label_str;


	pthread_mutex_lock( &Network_list_mutex );

	list_for_each(network_pos, &Network_list) {

		wlan_network = list_entry(network_pos, struct wlan_network, list);

		if ( wlan_network->visible ) {

			network_index++;

			if ( wlan_network->obj_id == -1 ) {

				wlan_network->obj_id = wire_sphere(30,30);
				s3d_flags_on( wlan_network->obj_id, S3D_OF_VISIBLE );

			}

			wlan_network->scale_factor = wlan_network->num_wlan_clients;
			s3d_scale( wlan_network->obj_id, wlan_network->scale_factor );

			real_node_pos_x = sin( ( 360 / Num_networks ) * network_index ) * ( M_PI / 180 ) * ( ( ( 100 * Num_networks ) / 2 * M_PI ) );
			real_node_pos_z = cos( ( 360 / Num_networks ) * network_index ) * ( M_PI / 180 ) * ( ( ( 100 * Num_networks ) / 2 * M_PI ) );

			if ( ( wlan_network->pos_vec[0] != real_node_pos_x ) || ( wlan_network->pos_vec[2] != real_node_pos_z ) ) {

				if ( wlan_network->pos_vec[0] != real_node_pos_x )
					wlan_network->pos_vec[0] += ( ( real_node_pos_x - wlan_network->pos_vec[0] ) * ( wlan_network->pos_vec[0] / real_node_pos_x ) );

				if ( wlan_network->pos_vec[2] != real_node_pos_z )
					wlan_network->pos_vec[2] += ( ( real_node_pos_z - wlan_network->pos_vec[2] ) * ( wlan_network->pos_vec[2] / real_node_pos_z ) );

				s3d_translate( wlan_network->obj_id, wlan_network->pos_vec[0], wlan_network->pos_vec[1], wlan_network->pos_vec[2] );

			}

			if ( wlan_network->properties_changed ) {

				wlan_network->properties_changed = 0;

				if ( wlan_network->label_id != -1 )
					s3d_del_object( wlan_network->label_id );


				label_str = alloc_memory( 100 );
				snprintf( label_str, 100, "%s\n%s", wlan_network->bssid, wlan_network->ssid );
				label_str_len = 100;

				wlan_network->label_id = s3d_draw_string( label_str, &label_str_len );
				s3d_link( wlan_network->label_id, wlan_network->obj_id );
				s3d_translate( wlan_network->label_id, - label_str_len / 2, -2, 0 );
				s3d_flags_on( wlan_network->label_id, S3D_OF_VISIBLE );

			}

			wlan_network->rotation = ( wlan_network->rotation + 1 ) % 360;
			s3d_rotate( wlan_network->obj_id, 0, wlan_network->rotation, 0 );

		}

	}


	pthread_mutex_unlock( &Network_list_mutex );

	return(0);

}



void mainloop() {

	struct timespec sleeptime = { 0, 100 * 1000 * 1000 };   /* 100 mili seconds */


	handle_networks();

	if ( Kism3d_aborted )
		s3d_quit();

	nanosleep( &sleeptime, NULL );

}



int gui_main( void *unused ) {

	if ( !s3d_init( NULL, NULL, "kism3d" ) ) {

		if ( s3d_select_font( "vera" ) ) {

			printf( "font 'vera' not found\n" );

		} else {

			s3d_mainloop( mainloop );

		}

		s3d_quit();

	}

	Kism3d_aborted = 1;
	return(0);

}