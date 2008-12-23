/*
 * main.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
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


static struct timespec sleep_time = {
	0, 100 * 1000 * 1000
};   /* 100 mili seconds */


static void init_globals(void)
{
	Global.debug = 1;
	Global.obj_node_hna = 0;
	Global.obj_node_inet = 0;
	Global.obj_node_normal = 0;
	Global.obj_btn_close = 0;
	Global.obj_s3d_url = 0;
	Global.obj_zero_point = 0;
	Global.obj_node_count = 0;
	Global.node_count = 0;
	Global.color_switch = 0;
	Global.output_block_counter = 0;
	Global.output_block_completed = 0;
	Global.asp = 1.0;
	Global.left = -1.0;
	Global.bottom = -1.0;

}


static void print_usage(void)
{

	printf("Usage is olsrs3d [options] [-- [s3d options]]\n");
	printf("olsrs3d options:\n");
	printf("   -h\tprint this short help\n");
	printf("   -d\tenable debug mode\n");
	printf("   -H\tconnect to olsr node [default: localhost]\n");
	s3d_usage();
}

static float dist(float p1[], float p2[])
{
	float p[3];
	p[0] = p1[0] - p2[0];
	p[1] = p1[1] - p2[1];
	p[2] = p1[2] - p2[2];
	return (sqrt(p[0]*p[0]   +  p[1]*p[1]  +  p[2]*p[2]));
}

static float dirt(float p1[], float p2[], float p3[])
{
	float d;
	d = dist(p1, p2);
	if (d == 0) {
		p3[0] = ((float) 0.2 * rand()) / RAND_MAX - 0.1;
		p3[1] = ((float) 0.2 * rand()) / RAND_MAX - 0.1;
		p3[2] = ((float) 0.2 * rand()) / RAND_MAX - 0.1;
		d = s3d_vector_length(p3);
	} else {
		p3[0] = p2[0] - p1[0];
		p3[1] = p2[1] - p1[1];
		p3[2] = p2[2] - p1[2];
	}
	return(d);
}

static void handle_node(void)
{
	struct node *node, *tmp_node;
	struct node_con *con;
	struct hash_it_t *hashit, *tmp_hashit = NULL;
	int ip[2];
	float angle, angle_rad;
	float tmp_mov_vec[3], desc_norm_vec[3] = {0, 0, -1};

	if (node_hash->elements == 0)
		return;
	hashit = NULL;
	while (NULL != (hashit = hash_iterate(node_hash, hashit))) {

		node = (struct node *) hashit->bucket->data;

		if (node->node_type_modified) {

			if (node->obj_id > 0) s3d_del_object(node->obj_id);
			if (node->desc_id > 0) s3d_del_object(node->desc_id);

			if (node->node_type == 1)
				node->obj_id = s3d_clone(Global.obj_node_inet);
			else if (node->node_type == 2)
				node->obj_id = s3d_clone(Global.obj_node_hna);
			else
				node->obj_id = s3d_clone(Global.obj_node_normal);

			s3d_flags_on(node->obj_id, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);

			node->desc_id = s3d_draw_string(node->ip_string, &node->desc_length);
			s3d_link(node->desc_id, node->obj_id);
			s3d_translate(node->desc_id, - node->desc_length / 2, -2, 0);
			s3d_flags_on(node->desc_id, S3D_OF_VISIBLE);

			node->node_type_modified = 0;

		}

		if ((node->last_seen < Global.output_block_counter - 1) && (node->visible)) {
			s3d_del_object(node->desc_id);
			s3d_del_object(node->obj_id);
			node->desc_id = -1;
			node->obj_id = -1;
			node->visible = 0;
			Global.node_count--;
			while (NULL != (tmp_hashit = hash_iterate(node_hash, tmp_hashit))) {
				tmp_node = (struct node *) tmp_hashit->bucket->data;


				if (node != tmp_node) {
					ip[0] = max(node->ip, tmp_node->ip);
					ip[1] = min(node->ip, tmp_node->ip);

					if (NULL != (con = (struct node_con*)hash_find(con_hash, ip))) {
						s3d_del_object(con->obj_id);
						con->obj_id = -1;
					}
				}
			}

		}

		if (node->visible) {

			/* rotate node description so that they are always readable */
			tmp_mov_vec[0] = Global.cam_position[0][0] - node->pos_vec[0];
			tmp_mov_vec[1] = 0;   /* we are not interested in the y value */
			tmp_mov_vec[2] = Global.cam_position[0][2] - node->pos_vec[2];

			angle = s3d_vector_angle(desc_norm_vec, tmp_mov_vec);

			/* take care of inverse cosinus */
			if (tmp_mov_vec[0] > 0) {
				angle_rad = 90.0 / M_PI - angle;
				angle = 180 - (180.0 / M_PI * angle);
			} else {
				angle_rad = 90.0 / M_PI + angle;
				angle = 180 + (180.0 / M_PI * angle);
			}

			s3d_rotate(node->desc_id, 0, angle , 0);
			s3d_translate(node->desc_id, -cos(angle_rad)*node->desc_length / 2 , -1.5, sin(angle_rad)*node->desc_length / 2);
		}

	}
	return;
}

static void mov_add(float mov[], float p[], float fac)
{
	/* if (fac>1000)
	  return;
	 fac=1000; */
	mov[0] += fac * p[0];
	mov[1] += fac * p[1];
	mov[2] += fac * p[2];
}

static void move_meshnode(struct node *node)
{
	float null_vec[3] = {0, 0, 0};
	float tmp_mov_vec[3];
	float distance;

	if (!((node->mov_vec[0] == 0) && (node->mov_vec[1] == 0) && (node->mov_vec[2] == 0)) && node->visible) {
		distance = dirt(node->pos_vec, null_vec, tmp_mov_vec);
		mov_add(node->mov_vec, tmp_mov_vec, distance / 100);   /* move a little bit to point zero */

		if ((distance = dist(node->mov_vec, null_vec)) > 10.0)
			mov_add(node->pos_vec, node->mov_vec, 1.0 / ((float) distance));
		else
			mov_add(node->pos_vec, node->mov_vec, 0.1);

		s3d_translate(node->obj_id, node->pos_vec[0], node->pos_vec[1], node->pos_vec[2]);
		/* reset movement vector */
		node->mov_vec[0] = node->mov_vec[1] = node->mov_vec[2] = 0.0;
	}
}

static void color_handler(struct node_con *con)
{
	float rgb = 0.0, r = 0.0, g = 0.0, b = 0.0, etx;
	int c, c1 = 0;

	if (con->etx1 == -1000.00 || con->etx2 == -1000) {

		c = 1;
		b = 1.0;

	} else {

		etx = (con->etx1 + con->etx2) / 2.0;

		/* very good link - bright blue */
		if ((etx >= 1.0) && (etx < 1.5)) {

			c = 2;
			r = 0.5;
			g = 1.0;
			b = 1.0;

			/* good link - bright yellow */
		} else if ((etx >= 1.5) && (etx < 2.0)) {

			rgb = 2.0 - etx;

			c = 3;
			c1 = con->color == 3 && rintf(con->rgb * 10) != rintf(rgb * 10) ? 1 : 0;
			r = 1.0;
			g = 1.0;
			b = rgb;

			/* not so good link - orange */
		} else if ((etx >= 2.0) && (etx < 3.0)) {

			rgb = 1.5 - (etx / 2.0);

			c = 4;
			c1 = con->color == 4 && rintf(con->rgb * 10) != rintf(rgb * 10) ? 1 : 0;
			r = 1.0;
			g = rgb;

			/* bad link (almost dead) - brown */
		} else if ((etx >= 3.0) && (etx < 5.0)) {

			rgb = 1.75 - (etx / 4.0);

			c = 5;
			c1 = con->color == 5 && rintf(con->rgb * 10) != rintf(rgb * 10) ? 1 : 0;

			r = rgb;
			g = rgb - 0.5;


			/* zombie link - grey */
		} else if ((etx >= 5.0) && (etx < 1000.0)) {

			rgb = 1000.0 / (1500.0 + etx);

			c = 6;
			c1 = con->color == 6 && rintf(con->rgb * 10) != rintf(rgb * 10) ? 1 : 0;

			r = g = b = rgb;

			/* wtf - dark grey */
		} else {

			c = 7;
			r = g = b = 0.3;

		}

	}

	if (con->color != c || c1) {
		s3d_pep_material(con->obj_id,
		                 r, g, b,
		                 r, g, b,
		                 r, g, b);

		con->color = c;

		if (rgb != 0.0)
			con->rgb = rgb;
	}

}


static void calc_node_mov(void)
{

	float distance;
	float tmp_mov_vec[3], vertex_buf[6];
	float f, wish_distance;
	int ip[2];
	struct node_con *con;
	struct node *first_node, *sec_node;
	struct hash_it_t *hashit1, *hashit2;

	if (con_hash->elements == 0)
		return;

	hashit1 = hashit2 = NULL;

	while (NULL != (hashit1 = hash_iterate(node_hash, hashit1))) {

		first_node = (struct node *) hashit1->bucket->data;
		if (!first_node->visible)
			continue;

		while (NULL != (hashit2 = hash_iterate(node_hash, hashit2))) {

			sec_node = (struct node *) hashit2->bucket->data;
			if (!sec_node->visible)
				continue;

			if (first_node != sec_node && (max(first_node->ip, sec_node->ip) == first_node->ip)) {

				ip[0] = first_node->ip;
				ip[1] = sec_node->ip;
				distance = dirt(first_node->pos_vec, sec_node->pos_vec, tmp_mov_vec);

				if ((NULL != (con = (struct node_con*)hash_find(con_hash, ip)))) {

					/* we have a connection */
					wish_distance = ((con->etx1_sqrt + con->etx2_sqrt)) + 4;
					f = powf(wish_distance / distance, 0.25);
					mov_add(first_node->mov_vec, tmp_mov_vec, (1 / f - 1));
					mov_add(sec_node->mov_vec, tmp_mov_vec, -(1 / f - 1));

					vertex_buf[0] = first_node->pos_vec[0];
					vertex_buf[1] = first_node->pos_vec[1];
					vertex_buf[2] = first_node->pos_vec[2];
					vertex_buf[3] = sec_node->pos_vec[0];
					vertex_buf[4] = sec_node->pos_vec[1];
					vertex_buf[5] = sec_node->pos_vec[2];
					s3d_pep_vertices(con->obj_id, vertex_buf, 2);

					if (Global.color_switch)
						color_handler(con);
					else {

						s3d_pep_material(con->obj_id,
						                 1.0, 1.0, 1.0,
						                 1.0, 1.0, 1.0,
						                 1.0, 1.0, 1.0
						                );
						con->color = 0;

					}

				} else {

					/* we have no connection */
					if (distance < 0.1) distance = 0.1;
					mov_add(first_node->mov_vec, tmp_mov_vec, -100 / (distance * distance));
					mov_add(sec_node->mov_vec, tmp_mov_vec, 100 / (distance * distance));

				}
			}
		}

	}

	while (NULL != (hashit1 = hash_iterate(node_hash, hashit1))) {
		first_node = (struct node *) hashit1->bucket->data;
		move_meshnode(first_node);
	}

}

static void mainloop(void)
{
	static int last_count = 0;
	int net_result;   /* result of function net_main */
	char nc_str[20];
	float str_len;

	calc_node_mov();
	handle_node();


	if (Global.node_count && Global.node_count != last_count) {

		if (Global.obj_node_count) s3d_del_object(Global.obj_node_count);

		snprintf(nc_str, 20, "node count: %d", Global.node_count);
		Global.obj_node_count = s3d_draw_string(nc_str, &str_len);

		s3d_link(Global.obj_node_count, 0);
		s3d_flags_on(Global.obj_node_count, S3D_OF_VISIBLE);
		s3d_scale(Global.obj_node_count, 0.2);
		s3d_translate(Global.obj_node_count, -Global.left * 3.0 - (str_len * 0.2), -Global.bottom * 3.0 - 0.7, -3.0);

		last_count = Global.node_count;

	}

	if (Global.output_block_completed) {

		Global.output_block_counter++;
		Global.output_block_completed = 0;

	}

	while ((net_result = net_main()) != 0) {
		if (net_result == -1) {
			printf("that's it folks\n");
			s3d_quit();
			break;
		}
	}
	nanosleep(&sleep_time, NULL);
	return;
}

static int object_info(struct s3d_evt *hrmz)
{
	struct s3d_obj_info *inf;
	inf = (struct s3d_obj_info *)hrmz->buf;
	s3dw_object_info(hrmz);

	if (inf->object == 0) {
		Global.cam_position[0][0] = inf->trans_x;
		Global.cam_position[0][1] = inf->trans_y;
		Global.cam_position[0][2] = inf->trans_z;
		Global.cam_position[1][0] = inf->rot_x;
		Global.cam_position[1][1] = inf->rot_y;
		Global.cam_position[1][2] = inf->rot_z;

		Global.asp = inf->scale;

		if (Global.asp > 1.0) {

			Global.bottom = -1.0;
			Global.left = -Global.asp;

		} else {

			Global.bottom = (-1.0 / Global.asp);
			Global.left = -1.0;

		}

	}

	return(0);
}

static int keypress(struct s3d_evt *event)
{

	struct s3d_key_event *key = (struct s3d_key_event *)event->buf;

	switch (key->keysym) {

	case S3DK_c: /* color on/off */

		Global.color_switch =  Global.color_switch ? 0 : 1;
		break;

	}

	return(0);

}

int main(int argc, char *argv[])
{
	int optchar;
	char olsr_host[256];

	init_globals();
	strncpy(olsr_host, "127.0.0.1", 256);
	lbuf[0] = '\0';

	while ((optchar = getopt(argc, argv, "dhH:")) != -1) {

		switch (optchar) {

		case 'd':
			Global.debug = 1;
			break;

		case 'H':
			strncpy(olsr_host, optarg, 256);
			break;

		case 'h':
		default:
			print_usage();
			return (0);

		}

	}

	if (Global.debug)
		printf("debug mode enabled ...\n");

	/* initialize obj2ip linked list */
	/* lst_initialize(); */

	/* delete olsrs3d options */
	while ((optind < argc) && (argv[optind][0] != '-')) optind++;        /* optind may point to ip addr of '-H' */
	optind--;
	argv[optind] = argv[0];  /* save program path */
	argc -= optind;    /* jump over olsrs3d options */
	argv += optind;

	/* set extern int optind = 0 for parse_args in io.c */
	optind = 0;

	process_init();

	if (!net_init(olsr_host)) {

		s3d_set_callback(S3D_EVENT_OBJ_INFO, object_info);
		s3d_set_callback(S3D_EVENT_KEY, keypress);


		/*s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);
		s3d_set_callback(S3D_EVENT_QUIT,stop); */

		if (!s3d_init(&argc, &argv, "meshs3d")) {

			if (s3d_select_font("vera"))
				printf("font not found\n");

			Global.obj_node_normal = s3d_import_model_file("objs/accesspoint.3ds");
			Global.obj_node_inet = s3d_import_model_file("objs/accesspoint_inet.3ds");
			Global.obj_node_hna = s3d_import_model_file("objs/internet.3ds");
			Global.obj_btn_close = s3d_import_model_file("objs/btn_close.3ds");

			Global.obj_s3d_url = s3d_import_model_file("objs/s3d_berlios_de.3ds");

			s3d_translate(Global.obj_s3d_url, 0.75, -0.75, -1);
			s3d_scale(Global.obj_s3d_url, 0.07);
			s3d_link(Global.obj_s3d_url, 0);
			s3d_flags_on(Global.obj_s3d_url, S3D_OF_VISIBLE);

			/* create_search_widget( 0, 0, 300 ); */

			Global.obj_zero_point = s3d_new_object();
			/* Output_border[0] = Output_border[1] = Output_border[2] = Output_border[3] = -1; */

			printf("go, s3d\n");
			s3d_mainloop(mainloop);
			s3d_quit();
			net_quit();
			printf("that's it\n");
		} else
			printf("s3d init failed\n");
	}
	return(0);
}
