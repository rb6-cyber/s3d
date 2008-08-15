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
#include <s3dw.h>
#include <math.h> /* M_PI, cos(), sin() */
#include <stdlib.h> /* malloc(), free() */
#include <stdio.h>      /* printf() */
#include <time.h> /* nanosleep()  */
#include <pthread.h>



static float CamPosition[2][3];          /* CamPosition[trans|rot][x-z] */

static int Last_Click_Oid = 0;
static unsigned int Last_Click_Time = 0;

static void *Cam_target = NULL;

static int Client_obj;

static struct timespec sleeptime = {
	0, 100 * 1000 * 1000
};   /* 100 mili seconds */



static int wire_sphere(int slices, int stacks)
{
	int x, y, i, o;
	int num_v, num_l;
	float *v, *n;  /* vertices, normals */
	float alpha, beta;
	unsigned int *l; /* lines */
	num_v = (stacks + 1) * slices;
	num_l = stacks * slices + (stacks - 1) * slices; /* vertical + horizontal */
	v = (float*)malloc(sizeof(float) * 3 * num_v);
	n = (float*)malloc(sizeof(float) * 6 * num_l);
	l = (unsigned int*)malloc(sizeof(unsigned int) * 3 * num_l);
	i = 0;
	for (x = 0;x < slices;x++) {
		alpha = (x * 360.0 / slices) * M_PI / 180.0;
		for (y = 0;y < (stacks + 1);y++) {
			beta = ((y * 180 / slices) - 90.0) * M_PI / 180.0;
			v[i*3+0] = cos(alpha) * cos(beta);
			v[i*3+1] = sin(beta);
			v[i*3+2] = sin(alpha) * cos(beta);
			i++;
		}
	}
	i = 0;
	for (x = 0;x < slices;x++) {
		for (y = 0;y < stacks;y++) {
			if ((y != 0) && (y != stacks)) { /* no horizontal lines at the poles */
				l[i*3+0] = (x * (stacks + 1)) + y;
				l[i*3+1] = (((x + 1) % slices) * (stacks + 1)) + y;
				l[i*3+2] = 0;
				n[i*6+0] = v[ l[i*3+0] * 3 + 0];
				n[i*6+1] = v[ l[i*3+0] * 3 + 1];
				n[i*6+2] = v[ l[i*3+0] * 3 + 2];
				n[i*6+3] = v[ l[i*3+1] * 3 + 0];
				n[i*6+4] = v[ l[i*3+1] * 3 + 1];
				n[i*6+5] = v[ l[i*3+1] * 3 + 2];

				i++;

			}
			/* vertical lines */
			l[i*3+0] = (x * (stacks + 1)) + y;
			l[i*3+1] = (x * (stacks + 1)) + y + 1;
			l[i*3+2] = 0;
			n[i*6+0] = v[ l[i*3+0] * 3 + 0];
			n[i*6+1] = v[ l[i*3+0] * 3 + 1];
			n[i*6+2] = v[ l[i*3+0] * 3 + 2];
			n[i*6+3] = v[ l[i*3+1] * 3 + 0];
			n[i*6+4] = v[ l[i*3+1] * 3 + 1];
			n[i*6+5] = v[ l[i*3+1] * 3 + 2];
			i++;

		}
	}
	o = s3d_new_object();
	s3d_push_material(o, 0, 0, 1,
	                  1, 0, 0,
	                  0, 1, 0);
	s3d_push_vertices(o, v, num_v);
	s3d_push_lines(o, l, num_l);
	s3d_load_line_normals(o, n, 0, num_l);
	free(v);
	free(n);
	free(l);
	return(o);
}



static int handle_networks(void)
{

	struct list_head *network_pos;
	struct wlan_network *wlan_network;
	float real_node_pos_x, real_node_pos_z, angle, angle_rad;
	int network_index = 0;
	char label_str[101]; /* safe to do as long as we use strn* functions */
	float maxlen, templen;


	pthread_mutex_lock(&Network_list_mutex);

	list_for_each(network_pos, &Network_list) {

		wlan_network = list_entry(network_pos, struct wlan_network, list);

		if (wlan_network->visible) {

			network_index++;

			if (wlan_network->obj_id == -1) {

				wlan_network->obj_id = s3d_new_object();
				wlan_network->wrsphr_id = wire_sphere(30, 30);
				s3d_link(wlan_network->wrsphr_id, wlan_network->obj_id);
				s3d_flags_on(wlan_network->wrsphr_id, S3D_OF_VISIBLE);

			}


			wlan_network->scale_fac = wlan_network->num_wlan_clients + 2;
			s3d_translate(wlan_network->wrsphr_id, 0, /*-6 + wlan_network->scale_fac*/ 0, 0);
			s3d_scale(wlan_network->obj_id, wlan_network->scale_fac);

			real_node_pos_x = sin(2.0 * M_PI * network_index / ((float) Num_networks)) * (((1 * Num_networks) / 2 * M_PI));
			real_node_pos_z = cos(2.0 * M_PI * network_index / ((float) Num_networks)) * (((1 * Num_networks) / 2 * M_PI));

			if ((fabs(wlan_network->pos_vec[0] - real_node_pos_x) > 0.5) || (fabs(wlan_network->pos_vec[2] - real_node_pos_z) > 0.5)) {

				if (fabs(wlan_network->pos_vec[0] - real_node_pos_x) > 0.5)
					wlan_network->pos_vec[0] = ((wlan_network->pos_vec[0] * 9 + real_node_pos_x) / 10);

				if (fabs(wlan_network->pos_vec[2] - real_node_pos_z) > 0.5)
					wlan_network->pos_vec[2] = ((wlan_network->pos_vec[2] * 9 + real_node_pos_z) / 10);

				s3d_translate(wlan_network->obj_id, wlan_network->pos_vec[0], wlan_network->pos_vec[1], wlan_network->pos_vec[2]);

			}

			if (wlan_network->props_changed) {
				snprintf(label_str, 100, "Type: %s, CH: %i, Clients: %i", (wlan_network->type == 0 ? "Managed" : (wlan_network->type == 1 ? "Ad-Hoc" : (wlan_network->type == 2 ? "Prober" : "unknown"))), wlan_network->chan, wlan_network->num_wlan_clients);

				/* determine our longest string which we draw */
				maxlen = s3d_strlen(label_str);

				if ((templen = s3d_strlen(wlan_network->ssid)) > maxlen)
					maxlen = templen;

				if ((templen = s3d_strlen(wlan_network->bssid)) > maxlen)
					maxlen = templen;

				wlan_network->props_changed = 0;

				if (wlan_network->ssid_id != -1)
					s3d_del_object(wlan_network->ssid_id);

				if (wlan_network->misc_id != -1)
					s3d_del_object(wlan_network->misc_id);


				if (wlan_network->bssid_id == -1) {

					wlan_network->bssid_id = s3d_draw_string(wlan_network->bssid, NULL);
					wlan_network->text_width = maxlen; /* the other strings might be longer, so we use the longest string for calculating our rotation. */
					s3d_link(wlan_network->bssid_id, wlan_network->obj_id);
					s3d_translate(wlan_network->bssid_id, - maxlen / 2, 2 + wlan_network->scale_fac, 0);
					s3d_scale(wlan_network->bssid_id, NETWORK_TEXT_SCALE);
					s3d_flags_on(wlan_network->bssid_id, S3D_OF_VISIBLE);

					wlan_network->click_id = s3d_new_object();
					s3d_link(wlan_network->click_id, wlan_network->bssid_id);
					s3d_push_material(wlan_network->click_id, 0, 0, 0,  0, 0, 0,  0, 0, 0);
					s3d_push_vertex(wlan_network->click_id, 0, 1, 0.1);
					s3d_push_vertex(wlan_network->click_id, maxlen, 1, 0.1);
					s3d_push_vertex(wlan_network->click_id, maxlen, -2.5, 0.1);   /* 3 lines of text + some mor space for low characters, like g,q,p ... */
					s3d_push_vertex(wlan_network->click_id, 0, -2.5, 0.1);
					s3d_push_polygon(wlan_network->click_id, 0, 1, 2, 0);
					s3d_push_polygon(wlan_network->click_id, 0, 2, 3, 0);

					s3d_flags_on(wlan_network->click_id, S3D_OF_SELECTABLE);

				}

				wlan_network->ssid_id = s3d_draw_string(wlan_network->ssid, NULL);
				s3d_link(wlan_network->ssid_id, wlan_network->bssid_id);
				s3d_translate(wlan_network->ssid_id, 0, -1, 0);
				s3d_flags_on(wlan_network->ssid_id, S3D_OF_VISIBLE);


				wlan_network->misc_id = s3d_draw_string(label_str, NULL);
				s3d_link(wlan_network->misc_id, wlan_network->ssid_id);
				s3d_translate(wlan_network->misc_id, 0, -1, 0);
				s3d_flags_on(wlan_network->misc_id, S3D_OF_VISIBLE);

			}

			angle = s3d_angle_to_cam(wlan_network->pos_vec, CamPosition[0], &angle_rad);
			s3d_rotate(wlan_network->bssid_id, 0, angle , 0);

			s3d_translate(wlan_network->bssid_id, -cos(angle_rad) * NETWORK_TEXT_SCALE * wlan_network->text_width / 2 , 2 , sin(angle_rad) * NETWORK_TEXT_SCALE * wlan_network->text_width / 2);

			wlan_network->rotation = (wlan_network->rotation + 1) % 360;
			s3d_rotate(wlan_network->wrsphr_id, 0, wlan_network->rotation, 0);

		}

	}


	pthread_mutex_unlock(&Network_list_mutex);

	return(0);

}



static int handle_clients(void)
{

	struct list_head *client_pos;
	struct wlan_client *wlan_client;
	float angle, angle_rad;


	pthread_mutex_lock(&Client_list_mutex);

	list_for_each(client_pos, &Client_list) {

		wlan_client = list_entry(client_pos, struct wlan_client, list);

		if (wlan_client->visible) {

			if (wlan_client->obj_id == -1) {

				wlan_client->obj_id = s3d_new_object();
				wlan_client->symbol_id = s3d_clone(Client_obj);
				s3d_link(wlan_client->symbol_id, wlan_client->obj_id);
				s3d_flags_on(wlan_client->symbol_id, S3D_OF_VISIBLE);

			}

			if (wlan_client->props_changed) {

				wlan_client->props_changed = 0;

				if (wlan_client->ip_id != -1)
					s3d_del_object(wlan_client->ip_id);

				wlan_client->ip_id = s3d_draw_string(wlan_client->ip, &wlan_client->ip_len);
				s3d_link(wlan_client->ip_id, wlan_client->obj_id);
				s3d_translate(wlan_client->ip_id, - wlan_client->ip_len / 2, 2, 0);
				s3d_scale(wlan_client->ip_id, CLIENT_TEXT_SCALE);
				s3d_flags_on(wlan_client->ip_id, S3D_OF_VISIBLE);

			}

			angle = s3d_angle_to_cam(wlan_client->pos_vec, CamPosition[0], &angle_rad);
			s3d_rotate(wlan_client->ip_id, 0, angle , 0);

			s3d_translate(wlan_client->ip_id, -cos(angle_rad) * CLIENT_TEXT_SCALE * wlan_client->ip_len / 2 , 2 , sin(angle_rad) * CLIENT_TEXT_SCALE * wlan_client->ip_len / 2);

		}

	}


	pthread_mutex_unlock(&Client_list_mutex);

	return(0);

}



/***
 *
 * eventhandler when object clicked
 *
 ***/

static int object_click(struct s3d_evt *evt)
{

	struct list_head *network_pos;
	struct wlan_network *wlan_network;
	int clicked_id = (int) * ((uint32_t *)evt->buf);


	s3dw_handle_click(evt);

	/* emulate double click */
	if ((Last_Click_Oid == clicked_id) && (Last_Click_Time + 250 > get_time())) {

		list_for_each(network_pos, &Network_list) {

			wlan_network = list_entry(network_pos, struct wlan_network, list);

			if (wlan_network->click_id == clicked_id) {

				Cam_target = wlan_network;
				break;

			}

		}

	}

	Last_Click_Oid = clicked_id;
	Last_Click_Time = get_time();

	return(0);

}



/***
 *
 * eventhandler when object change by user
 * such as Cam
 *
 ***/

static int object_info(struct s3d_evt *hrmz)
{

	struct s3d_obj_info *inf;


	inf = (struct s3d_obj_info *)hrmz->buf;
	s3dw_object_info(hrmz);

	if (inf->object == 0) {

		CamPosition[0][0] = inf->trans_x;
		CamPosition[0][1] = inf->trans_y;
		CamPosition[0][2] = inf->trans_z;
		CamPosition[1][0] = inf->rot_x;
		CamPosition[1][1] = inf->rot_y;
		CamPosition[1][2] = inf->rot_z;

	}

	return(0);

}



static void mainloop(void)
{

	float angle, diff_vec[3], tmp_vec[3] = { 0.0, 0.0, -1.0 };


	handle_networks();
	handle_clients();

	if (Cam_target != NULL) {

		/* move to target */
		/* printf( "Moving to Network: %s, %s\n", ((struct wlan_network *)Cam_target)->bssid, ((struct wlan_network *)Cam_target)->ssid ); */

		CamPosition[0][0] = (CamPosition[0][0] * 4 + ((struct wlan_network *)Cam_target)->pos_vec[0] + 10) / 5;
		CamPosition[0][1] = (CamPosition[0][1] * 4 + ((struct wlan_network *)Cam_target)->pos_vec[1]) / 5;
		CamPosition[0][2] = (CamPosition[0][2] * 4 + ((struct wlan_network *)Cam_target)->pos_vec[2]) / 5;

		diff_vec[0] = CamPosition[0][0] - ((struct wlan_network *)Cam_target)->pos_vec[0];
		diff_vec[1] = 0.0;
		diff_vec[2] = CamPosition[0][2] - ((struct wlan_network *)Cam_target)->pos_vec[2];

		angle = s3d_vector_angle(diff_vec, tmp_vec);
		angle = 180 - (180 / M_PI * angle);
		CamPosition[1][1] = (CamPosition[1][1] * 4 + angle) / 5;

		s3d_translate(0, CamPosition[0][0], CamPosition[0][1], CamPosition[0][2]);
		s3d_rotate(0, CamPosition[1][0], CamPosition[1][1], CamPosition[1][2]);

		if ((fabs(diff_vec[0]) < 11.0) && (fabs(CamPosition[0][1] - ((struct wlan_network *)Cam_target)->pos_vec[1]) < 1.0) && (fabs(diff_vec[2]) < 1.0))
			Cam_target = NULL;

	}

	if (Kism3d_aborted)
		s3d_quit();

	nanosleep(&sleeptime, NULL);

}



void* gui_main(void *KISM3DUNUSED(unused))
{

	if (!s3d_init(NULL, NULL, "kism3d")) {

		if (s3d_select_font("vera")) {

			printf("font 'vera' not found\n");

		} else {

			s3d_set_callback(S3D_EVENT_OBJ_INFO, object_info);
			s3d_set_callback(S3D_EVENT_OBJ_CLICK, object_click);

			Client_obj = s3d_import_model_file("objs/accesspoint.3ds");

			s3d_mainloop(mainloop);

		}

		s3d_quit();

	}

	Kism3d_aborted = 1;

	return(NULL);
}
