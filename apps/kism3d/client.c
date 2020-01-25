// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2006-2015  Marek Lindner <mareklindner@neomailbox.ch>
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

	strncpy(wlan_client->mac, mac, sizeof(wlan_client->mac));
	wlan_client->mac[sizeof(wlan_client->mac) - 1] = '\0';
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


