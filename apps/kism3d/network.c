/*
 * network.c
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



struct wlan_network *get_wlan_network(char *bssid) {

	struct wlan_network *wlan_network;


	wlan_network = find_wlan_network(bssid);

	if (wlan_network != NULL)
		return wlan_network;


	/* we reached the end of the list and must create a new wlan_network */
	wlan_network = (struct wlan_network*)alloc_memory(sizeof(struct wlan_network));

	INIT_LIST_HEAD(&wlan_network->list);

	strncpy(wlan_network->bssid, bssid, 18);

	wlan_network->type = -1;
	wlan_network->chan = -1;

	wlan_network->ssid = NULL;

	wlan_network->num_wlan_clients = 0;

	wlan_network->visible = 1;

	wlan_network->pos_vec[0] = wlan_network->pos_vec[1] = wlan_network->pos_vec[2] = 0;

	wlan_network->obj_id = -1;
	wlan_network->bssid_id = -1;
	wlan_network->ssid_id = -1;
	wlan_network->misc_id = -1;

	wlan_network->rotation = 0;
	wlan_network->scale_fac = 0;

	wlan_network->props_changed = 1;

	list_add_tail(&wlan_network->list, &Network_list);

	Num_networks++;

	return wlan_network;

}



struct wlan_network *find_wlan_network(char *bssid) {

	struct list_head *network_pos;
	struct wlan_network *wlan_network;


	list_for_each(network_pos, &Network_list) {

		wlan_network = list_entry(network_pos, struct wlan_network, list);

		if (strncmp(wlan_network->bssid, bssid, 18) == 0)
			return wlan_network;

	}


	return NULL;

}

