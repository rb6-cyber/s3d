/*
 * client.c
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
#include <string.h>   /* strncmp(), strncpy() */
#include <stdlib.h>   /* rand() */



struct wlan_client *get_wlan_client(char *mac) {

	struct list_head *client_pos;
	struct wlan_client *wlan_client;


	list_for_each(client_pos, &Client_list) {

		wlan_client = list_entry(client_pos, struct wlan_client, list);

		if (strncmp(wlan_client->mac, mac, 18) == 0)
			return wlan_client;

	}


	/* we reached the end of the list and must create a new wlan_network */
	wlan_client = (struct wlan_client*)alloc_memory(sizeof(struct wlan_client));

	INIT_LIST_HEAD(&wlan_client->list);

	strncpy(wlan_client->mac, mac, 18);
	wlan_client->wlan_network = NULL;

	wlan_client->props_changed = 1;

	wlan_client->visible = 1;

	wlan_client->pos_vec[0] = ((float) 2.0 * rand()) / RAND_MAX - 1.0;
	wlan_client->pos_vec[1] = ((float) 2.0 * rand()) / RAND_MAX - 1.0;
	wlan_client->pos_vec[2] = ((float) 2.0 * rand()) / RAND_MAX - 1.0;
	wlan_client->mov_vec[0] = wlan_client->mov_vec[1] = wlan_client->mov_vec[2] = 0.0;

	wlan_client->obj_id = -1;
	wlan_client->ip_id = -1;

	list_add_tail(&wlan_client->list, &Client_list);

	return wlan_client;

}


