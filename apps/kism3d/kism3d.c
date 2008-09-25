/*
 * kism3d.c
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
#include <stdlib.h>   /* malloc() */
#include <stdio.h>    /* printf() */
#include <string.h>   /* memset(), strncmp(), strncpy() */
#include <errno.h>    /* errno */
#include <unistd.h>   /* write() */
#include <pthread.h>
#include <sys/time.h> /* gettimeofday() */

#include <sys/socket.h> /* inet_pton(), inet_aton() */
#include <sys/types.h>
#include <arpa/inet.h>



static DEFINE_LIST_HEAD(kismet_src_list);
DEFINE_LIST_HEAD(Network_list);
DEFINE_LIST_HEAD(Client_list);

static pthread_t s3d_thread_id;
pthread_mutex_t Network_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Client_list_mutex = PTHREAD_MUTEX_INITIALIZER;

static int thread_running = 0;
int Kism3d_aborted = 0;
int Num_networks = 0;



void *alloc_memory(int len)
{

	void *res = malloc(len);

	if (res == NULL) {
		printf("Error - can't allocate memory: %s\n", strerror(errno));
		exit(1);
	}

	memset(res, 0, len);

	return res;

}



unsigned int get_time(void)
{

	struct timeval tv;

	gettimeofday(&tv, NULL);

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;

}



static void parse_buffer(struct kismet_src *kismet_src)
{

	struct wlan_network *wlan_network;
	struct wlan_client *wlan_client;
	char enable_network[] = "!0 ENABLE NETWORK bssid,type,channel,ssid\n", enable_client[] = "!0 ENABLE CLIENT bssid,mac,ip\n", enable_alert[] = "!0 ENABLE ALERT *\n";
	char *read_ptr, *line_ptr, *last_cr_ptr = NULL, *parse_begin_ptr, *parse_end_ptr;
	char *bssid, *channel, *type, *ssid, *mac, *ip;
	int count;

	read_ptr = kismet_src->recv_buff;
	line_ptr = kismet_src->recv_buff;


	while ((*read_ptr) != '\0') {

		if ((*read_ptr) == '\n') {

			last_cr_ptr = read_ptr;
			*last_cr_ptr = '\0';

			bssid = channel = type = ssid = mac = ip = NULL;

			/* printf( "line: %s\n", line_ptr ); */

			if (strncmp(line_ptr, "*TIME: ", strlen("*TIME: ")) == 0) {

				if (kismet_src->enable_level < 4) {

					switch (kismet_src->enable_level) {

					case 0:
						if (write(kismet_src->sock, enable_alert, sizeof(enable_alert)) < 0) {

							printf("Warning - can't send ENABLE ALERT message to kismet server (%s:%i): %s\n", kismet_src->kismet_ip, kismet_src->port, strerror(errno));

						}

						break;

					case 1:
						if (write(kismet_src->sock, enable_client, sizeof(enable_client)) < 0) {

							printf("Warning - can't send ENABLE CLIENT message to kismet server (%s:%i): %s\n", kismet_src->kismet_ip, kismet_src->port, strerror(errno));

						}

						break;

					case 2:

						if (write(kismet_src->sock, enable_network, sizeof(enable_network)) < 0) {

							printf("Warning - can't send ENABLE NETWORK message to kismet server (%s:%i): %s\n", kismet_src->kismet_ip, kismet_src->port, strerror(errno));

						}

						break;

					case 3:

						if (!thread_running) {

							thread_running++;
							pthread_create(&s3d_thread_id, NULL, &gui_main, NULL);

						}

						break;

					}

					kismet_src->enable_level++;

				}

			} else if (strncmp(line_ptr, "*NETWORK: ", strlen("*NETWORK: ")) == 0) {

				/* printf( "line: %s\n", line_ptr ); */

				parse_begin_ptr = parse_end_ptr = line_ptr + strlen("*NETWORK: ");
				count = 0;

				while ((*parse_end_ptr) != '\0') {

					if ((*parse_end_ptr) == ' ') {

						switch (count) {

						case 0:
							bssid = parse_begin_ptr;
							break;

						case 1:
							type = parse_begin_ptr;
							break;

						case 2:
							channel = parse_begin_ptr;
							break;

						case 3:
							/* ssids with spaces are quoted by kismet */
							if (parse_begin_ptr[0] == '\001') {

								parse_begin_ptr++;

								parse_end_ptr = strchr(parse_begin_ptr, '\001');
								*parse_end_ptr = '\0';

								count++;

							}

							ssid = parse_begin_ptr;
							break;

						}

						if (count == 4)
							break;

						*parse_end_ptr = '\0';
						parse_begin_ptr = parse_end_ptr + 1;

						count++;

					}

					parse_end_ptr++;

				}


				pthread_mutex_lock(&Network_list_mutex);

				wlan_network = get_wlan_network(bssid);

				if ((wlan_network->type != -1) && (wlan_network->chan != -1) && (wlan_network->ssid != NULL)) {

					/* network properties have changed - alert user */

				}

				if (channel != NULL)
					wlan_network->chan = atoi(channel);
				else
					wlan_network->chan = 0;

				if (type != NULL)
					wlan_network->type = atoi(type);
				else
					wlan_network->type = 0;

				if (wlan_network->ssid != NULL)
					free(wlan_network->ssid);

				wlan_network->ssid = (char*)alloc_memory(strlen(ssid));
				strcpy(wlan_network->ssid, ssid);

				pthread_mutex_unlock(&Network_list_mutex);

				/* printf( "network found - bssid %s, type %s, channel %s, ssid '%s' <> '%s'\n", bssid, type, channel, ssid, wlan_network->ssid ); */

			} else if (strncmp(line_ptr, "*CLIENT: ", strlen("*CLIENT: ")) == 0) {

				parse_begin_ptr = parse_end_ptr = line_ptr + strlen("*CLIENT: ");
				count = 0;

				while ((*parse_end_ptr) != '\0') {

					if ((*parse_end_ptr) == ' ') {

						switch (count) {

						case 0:
							bssid = parse_begin_ptr;
							break;

						case 1:
							mac = parse_begin_ptr;
							break;

						case 2:
							ip = parse_begin_ptr;
							break;

						}

						if (count == 2)
							break;

						*parse_end_ptr = '\0';
						parse_begin_ptr = parse_end_ptr + 1;

						count++;

					}

					parse_end_ptr++;

				}

				wlan_client = get_wlan_client(mac);

				/*if ( ( wlan_client->type != -1 ) && ( wlan_client->channel != -1 ) && ( wlan_client->ssid != NULL ) ) {

				  client properties have changed - alert user

				}*/

				strncpy(wlan_client->bssid, bssid, 18);
				strncpy(wlan_client->ip, ip, 16);

				wlan_network = find_wlan_network(wlan_client->bssid);

				if (wlan_client->wlan_network != wlan_network) {

					if (wlan_client->wlan_network != NULL) {

						wlan_client->wlan_network->num_wlan_clients--;
						wlan_client->wlan_network->props_changed = 1;

					}

					if (wlan_network != NULL) {

						wlan_network->num_wlan_clients++;
						wlan_network->props_changed = 1;

					}

					wlan_client->wlan_network = wlan_network;

				}


				/* printf( "client found - bssid %s, mac %s, ip %s\n", bssid, mac, ip ); */

			} else if (strncmp(line_ptr, "*ALERT: ", strlen("*ALERT: ")) == 0) {

				/* printf( "alert: %s\n", line_ptr + strlen( "*ALERT: " ) ); */

			}

			*last_cr_ptr = '\n';
			line_ptr = last_cr_ptr + 1;

		}

		read_ptr++;

	}

	if (last_cr_ptr != NULL)
		memmove(kismet_src->recv_buff, last_cr_ptr + 1, strlen(last_cr_ptr));

}



int main(int argc, char *argv[])
{

	struct in_addr tmp_ip_holder;
	struct kismet_src *kismet_src;
	struct list_head *kismet_pos, *kismet_pos_tmp;
	/* struct wlan_network *wlan_network;
	struct wlan_client *wlan_client; */
	struct timeval tv;
	int num_kismet_sources = 0, found_args = 1, max_sock = -1, res, status;
	char *colon_ptr, buff[1000];
	fd_set wait_sockets, tmp_wait_sockets;


	FD_ZERO(&wait_sockets);


	while (argc > found_args) {

		kismet_src = (struct kismet_src*)alloc_memory(sizeof(struct kismet_src));

		INIT_LIST_HEAD(&kismet_src->list);
		kismet_src->enable_level = 0;

		/* get ip and port from argument */
		if ((colon_ptr = strchr(argv[found_args], ':')) != NULL) {

			*colon_ptr = '\0';
			colon_ptr++;

		}

		if (inet_pton(AF_INET, argv[found_args], &tmp_ip_holder) < 1) {

			printf("Invalid kismet IP specified: %s\n", argv[found_args]);
			free(kismet_src);
			found_args++;
			continue;

		} else {

			kismet_src->ip = tmp_ip_holder.s_addr;

		}

		if (colon_ptr != NULL) {

			kismet_src->port = strtol(colon_ptr, NULL , 10);

			if (kismet_src->port < 1 || kismet_src->port > 65535) {
				printf("Invalid kismet PORT specified: %s\n", colon_ptr);
				free(kismet_src);
				found_args++;
				continue;
			}

		} else {

			kismet_src->port = 2501;

		}


		kismet_src->kismet_addr.sin_family = AF_INET;
		kismet_src->kismet_addr.sin_port = htons(kismet_src->port);
		kismet_src->kismet_addr.sin_addr.s_addr = kismet_src->ip;

		kismet_src->kismet_ip = (char*)alloc_memory(16);
		inet_ntop(AF_INET, &kismet_src->ip, kismet_src->kismet_ip, 16);


		/* connect to kismet server */
		if ((kismet_src->sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {

			printf("Error - can't create tcp socket (%s:%i): %s\n", kismet_src->kismet_ip, kismet_src->port, strerror(errno));
			free(kismet_src->kismet_ip);
			free(kismet_src);
			found_args++;
			continue;

		}

		if (connect(kismet_src->sock, (struct sockaddr *)&kismet_src->kismet_addr, sizeof(struct sockaddr)) < 0) {

			printf("Error - can't connect to kismet server (%s:%i): %s\n", kismet_src->kismet_ip, kismet_src->port, strerror(errno));
			close(kismet_src->sock);
			free(kismet_src->kismet_ip);
			free(kismet_src);
			found_args++;
			continue;

		}


		kismet_src->recv_buff = (char*)alloc_memory(1500);

		FD_SET(kismet_src->sock, &wait_sockets);
		if (kismet_src->sock > max_sock)
			max_sock = kismet_src->sock;

		list_add_tail(&kismet_src->list, &kismet_src_list);


		num_kismet_sources++;
		found_args++;

	}


	if (num_kismet_sources == 0) {

		printf("Exiting - can't find any valid kismet server\n");
		exit(EXIT_FAILURE);

	}


	while ((num_kismet_sources > 0) && !(Kism3d_aborted)) {

		tv.tv_sec = 0;
		tv.tv_usec = 250;

		tmp_wait_sockets = wait_sockets;

		res = select(max_sock + 1, &tmp_wait_sockets, NULL, NULL, &tv);

		if (res > 0) {

			max_sock = -1;

			list_for_each_safe(kismet_pos, kismet_pos_tmp, &kismet_src_list) {

				kismet_src = list_entry(kismet_pos, struct kismet_src, list);

				if (FD_ISSET(kismet_src->sock, &tmp_wait_sockets)) {

					status = read(kismet_src->sock, buff, sizeof(buff));

					if (status > 0) {

						if (kismet_src->sock > max_sock)
							max_sock = kismet_src->sock;

						buff[status] = '\0';

						/* check for potential buffer overflow */
						if ((strlen(kismet_src->recv_buff) + strlen(buff)) < 1500) {

							strncat(kismet_src->recv_buff, buff, 1000);

						} else {

							/* hope that carriage return is now in buffer */
							if (strlen(kismet_src->recv_buff) < 1500) {

								printf("WARNING: receive buffer almost filled without *any* carriage return within that data !\nAppending truncated buffer to receive buffer to prevent buffer overflow.\n");
								strncat(kismet_src->recv_buff, buff, 1500 - strlen(kismet_src->recv_buff));

							} else {

								printf("ERROR: receive buffer filled without *any* carriage return within that data !\nClearing receive buffer to prevent buffer overflow.\n");
								strncpy(kismet_src->recv_buff, buff, 1000);

							}

						}

						parse_buffer(kismet_src);

						/* printf( "buffer length: %i\n", strlen( kismet_src->recv_buff ) ); */

					} else {

						if (status < 0) {

							printf("Error - can't read message from %s:%i: %s\n", kismet_src->kismet_ip, kismet_src->port, strerror(errno));

						} else {

							printf("Kismet server %s:%i closed connection ...\n", kismet_src->kismet_ip, kismet_src->port);

						}

						FD_CLR(kismet_src->sock, &wait_sockets);
						close(kismet_src->sock);

						list_del(kismet_pos);
						free(kismet_src->kismet_ip);
						free(kismet_pos);

						num_kismet_sources--;

					}

				} else {

					if (kismet_src->sock > max_sock)
						max_sock = kismet_src->sock;

				}

			}

		} else if ((res < 0) && (errno != EINTR)) {

			printf("Error - can't select: %s\n", strerror(errno));
			break;

		}

		/* printf( "\nCurrent network list:\n" );

		list_for_each_safe(kismet_pos, kismet_pos_tmp, &Network_list) {

		 wlan_network = list_entry(kismet_pos, struct wlan_network, list);

		 printf( "   => %s\n", wlan_network->bssid );

		}

		printf( "\nCurrent client list:\n" );

		list_for_each_safe(kismet_pos, kismet_pos_tmp, &Client_list) {

		 wlan_client = list_entry(kismet_pos, struct wlan_client, list);

		 printf( "   => %s belonging to %s\n", wlan_client->mac, ( wlan_client->wlan_network == NULL ? "unknown" : wlan_client->wlan_network->bssid ) );

		} */

	}


	if (Kism3d_aborted) {

		printf("Closing all sockets ... \n");
		list_for_each(kismet_pos, &kismet_src_list) {

			kismet_src = list_entry(kismet_pos, struct kismet_src, list);

			close(kismet_src->sock);

		}

	} else {

		Kism3d_aborted = 1;
		pthread_join(s3d_thread_id, NULL);

	}

	return (0);

}
