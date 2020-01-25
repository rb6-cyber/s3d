// SPDX-License-Identifier: GPL-2.0-or-later
/* SPDX-FileCopyrightText: 2006-2015  Marek Lindner <mareklindner@neomailbox.ch>
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

	strncpy(wlan_network->bssid, bssid, sizeof(wlan_network->bssid));
	wlan_network->bssid[sizeof(wlan_network->bssid) - 1] = '\0';

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

