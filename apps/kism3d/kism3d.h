/*
 * kism3d.h
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



#include "list.h"
#include <netinet/in.h>   /* sockaddr_in */



struct kismet_src {

	struct list_head list;
	unsigned int ip;
	int port;
	int sock;
	char *kismet_ip;
	char *recv_buff;
	struct sockaddr_in kismet_addr;
	int enable_level;

};


struct wlan_network {

	struct list_head list;
	char bssid[18];
	char *ssid;
	int type;
	int channel;
	int visible;
	float pos_vec[3];
	float mov_vec[3];
	int obj_id;
	int desc_id;

};


struct wlan_client {

	struct list_head list;
	char bssid[18];
	char mac[18];
	int ip[16];
	struct wlan_network *wlan_network;
	int visible;
	float pos_vec[3];
	float mov_vec[3];
	int obj_id;
	int desc_id;

};



void *alloc_memory( int len );
struct wlan_network *get_wlan_network( char *bssid );
struct wlan_network *find_wlan_network( char *bssid );
struct wlan_client *get_wlan_client( char *mac );



extern struct list_head Network_list;
extern struct list_head Client_list;
