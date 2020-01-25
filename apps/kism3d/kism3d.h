/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2006-2015  Marek Lindner <mareklindner@neomailbox.ch>
 */

#ifndef _KISM3D_H_
#define _KISM3D_H_

#include "list.h"
#include <pthread.h>
#include <netinet/in.h>   /* sockaddr_in */
#include <config-s3d.h>

#ifndef KISM3DUNUSED
#if defined(UNUSEDPARAM_ATTRIBUTE)
#define KISM3DUNUSED(x) (x)__attribute__((unused))
#elif defined(UNUSEDPARAM_OMIT)
#define KISM3DUNUSED(x) /* x */
#else
#define KISM3DUNUSED(x) x
#endif
#endif



#define NETWORK_TEXT_SCALE 0.2
#define CLIENT_TEXT_SCALE 0.2



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
	int chan;
	int num_wlan_clients;
	int props_changed;
	int visible;
	float pos_vec[3];
	int obj_id;
	int wrsphr_id;
	int bssid_id;
	int click_id;
	int ssid_id;
	int misc_id;
	int rotation;
	int scale_fac;
	float text_width;

};


struct wlan_client {

	struct list_head list;
	char bssid[18];
	char mac[18];
	char ip[16];
	struct wlan_network *wlan_network;
	int props_changed;
	int visible;
	float pos_vec[3];
	float mov_vec[3];
	int obj_id;
	int symbol_id;
	int ip_id;
	float ip_len;

};



void *alloc_memory(int len);
unsigned int get_time(void);
struct wlan_network *get_wlan_network(char *bssid);
struct wlan_network *find_wlan_network(char *bssid);
struct wlan_client *get_wlan_client(char *mac);
void* gui_main(void *unused);



extern struct list_head Network_list;
extern struct list_head Client_list;
extern pthread_mutex_t Network_list_mutex;
extern pthread_mutex_t Client_list_mutex;
extern int Kism3d_aborted;
extern int Num_networks;

#endif /* _KISM3D_H_ */
