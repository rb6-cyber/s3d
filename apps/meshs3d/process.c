/*
 * process.c
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



#include <stdio.h>  /* NULL */
#include <string.h>  /* strlen(), memmove() */
#include <stdlib.h>  /* rand(), malloc(), realloc(), free() */
#include <s3d.h>
#include <math.h>       /* sqrt() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdint.h>
#include "allocate.c"
#include "hash.h"
#include "meshs3d.h"

/* extern used in net.c */
char lbuf[MAXLINESIZE];

struct hashtable_t *node_hash;
struct hashtable_t *con_hash;


static int long_comp(const void *data1, const void *data2)
{
	return(memcmp(data1, data2, 8));
}

static int long_choose(const void *data, int32_t size)
{
	unsigned char *key = (unsigned char*)data;
	uint32_t hash = 0;
	size_t i;

	for (i = 0; i < 8; i++) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return (hash % size);
}

static int orig_comp(const void *data1, const void *data2)
{
	return(memcmp(data1, data2, 4));
}

/* hashfunction to choose an entry in a hash table of given size */
/* hash algorithm from http://en.wikipedia.org/wiki/Hash_table */
static int orig_choose(const void *data, int32_t size)
{
	unsigned char *key = (unsigned char*)data;
	uint32_t hash = 0;
	size_t i;

	for (i = 0; i < 4; i++) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return (hash % size);
}

static void exit_error(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

void process_init(void)
{
	/* initialize hashtable */
	if (NULL == (node_hash = hash_new(1600, orig_comp, orig_choose)))
		exit_error("Can't create hashtable node_hash\n");
	if (NULL == (con_hash = hash_new(1600, long_comp, long_choose)))
		exit_error("Can't create hashtable con_hash\n");
	return;
}

static void handle_con(unsigned int ip1, unsigned int ip2, float etx)
{

	unsigned int ip[2];
	struct node_con *con;
	struct hashtable_t *swaphash;

	ip[0] = max(ip1, ip2);
	ip[1] = min(ip1, ip2);

	con = (struct node_con*) hash_find(con_hash, ip);
	if (con == NULL) {
		con = (struct node_con *) debugMalloc(sizeof(struct node_con), 102);
		con->ip[0] = ip[0];
		con->ip[1] = ip[1];
		con->color = 0;
		/* draw line */
		con->obj_id = s3d_new_object();
		s3d_push_material(con->obj_id,
		                  1.0, 1.0, 1.0,
		                  1.0, 1.0, 1.0,
		                  1.0, 1.0, 1.0);
		s3d_push_vertex(con->obj_id, 0, 0, 0);
		s3d_push_vertex(con->obj_id, 0, 0, 0);
		s3d_push_line(con->obj_id, 0, 1, 0);
		s3d_flags_on(con->obj_id, S3D_OF_VISIBLE);

		con->rgb = 0.00;
		con->etx1 = 1.00;
		con->etx2 = 1.00;
		con->etx1_sqrt = 1.00;
		con->etx2_sqrt = 1.00;
		hash_add(con_hash, con);
	}

	if (con->obj_id == -1) {
		con->obj_id = s3d_new_object();
		s3d_push_material(con->obj_id,
		                  1.0, 1.0, 1.0,
		                  1.0, 1.0, 1.0,
		                  1.0, 1.0, 1.0);
		s3d_push_vertex(con->obj_id, 0, 0, 0);
		s3d_push_vertex(con->obj_id, 0, 0, 0);
		s3d_push_line(con->obj_id, 0, 1, 0);
		s3d_flags_on(con->obj_id, S3D_OF_VISIBLE);
	}

	if (con->ip[0] == ip1) {
		con->etx1 = etx;
		if (etx != -1000.00)
			con->etx1_sqrt = sqrt(etx);
		else
			con->etx1_sqrt = sqrt(2.0);
	} else {
		con->etx2 = etx;
		if (etx != -1000.00)
			con->etx2_sqrt = sqrt(etx);
		else
			con->etx2_sqrt = sqrt(2.0);
	}


	if (con_hash->elements * 4 > con_hash->size) {
		swaphash = hash_resize(con_hash, con_hash->size * 2);
		if (swaphash == NULL)
			exit_error("Couldn't resize hash table \n");
		con_hash = swaphash;
	}

	con->old = 0;

}

static struct node *handle_mesh_node(unsigned int ip, char *ip_string)
{
	struct node *orig_node;
	struct hashtable_t *swaphash;

	if (node_hash->elements * 4 > node_hash->size) {
		swaphash = hash_resize(node_hash, node_hash->size * 2);
		if (swaphash == NULL)
			exit_error("Couldn't resize hash table \n");
		node_hash = swaphash;
	}
	orig_node = (struct node *) hash_find(node_hash, &ip);

	if (NULL == orig_node) {
		orig_node = (struct node *)debugMalloc(sizeof(struct node), 101);
		orig_node->ip = ip;
		strncpy(orig_node->ip_string, ip_string, NAMEMAX);

		orig_node->node_type = 0;
		orig_node->node_type_modified = 1;

		orig_node->visible = 1;

		orig_node->pos_vec[0] = ((float) 2.0 * rand()) / RAND_MAX - 1.0;
		orig_node->pos_vec[1] = ((float) 2.0 * rand()) / RAND_MAX - 1.0;
		orig_node->pos_vec[2] = ((float) 2.0 * rand()) / RAND_MAX - 1.0;
		orig_node->mov_vec[0] = orig_node->mov_vec[1] = orig_node->mov_vec[2] = 0.0;

		orig_node->obj_id = -1;
		orig_node->desc_id = -1;
		hash_add(node_hash, orig_node);
		Global.node_count++;
	}

	if (!orig_node->visible) {
		orig_node->visible = 1;
		orig_node->node_type_modified = 1;
		Global.node_count++;
	}
	orig_node->last_seen = Global.output_block_counter;
	return(orig_node);
}

int process_main(void)
{

	int dn;
	float f;
	char *lbuf_ptr, *last_cr_ptr, *con_from, *con_from_end, *con_to, *con_to_end, *etx, *etx_end, *tmp_char;
	struct hash_it_t *hashit;
	struct node_con *con;
	char hna_name[NAMEMAX];
	char hna_node[NAMEMAX];

	struct node *tmp_node;
	unsigned int int_con_from = 0, int_con_to = 0, address;

	lbuf_ptr = lbuf;
	last_cr_ptr = NULL;

	con_from = con_from_end = con_to = con_to_end = etx = etx_end = NULL;
	dn = 0;


	while ((*lbuf_ptr) != '\0') {
		if ((*lbuf_ptr) == '\n') {
			last_cr_ptr = lbuf_ptr;
			con_from = con_from_end = con_to = con_to_end = etx = etx_end = NULL;
			dn = 0;
		}

		if ((*lbuf_ptr) == '"') {
			switch (dn) {
			case 0:
				con_from = lbuf_ptr + 1;
				break;
			case 1:
				con_from_end = lbuf_ptr;
				break;
			case 2:
				con_to = lbuf_ptr + 1;
				break;
			case 3:
				con_to_end = lbuf_ptr;
				break;
			case 4:
				etx = lbuf_ptr + 1;
				break;
			case 5:
				etx_end = lbuf_ptr;
				break;
			}

			if (++dn == 6) {

				(*con_from_end) = (*con_to_end) = (*etx_end) = '\0';

				/* if( Global.debug ) printf( "con_from: %s, con_to: %s, etx: %s\n", con_from, con_to, etx ); */

				/* announced network via HNA */
				if (strncmp(etx, "HNA", NAMEMAX) == 0) {

					if (strncmp(con_to, "0.0.0.0/0.0.0.0", NAMEMAX) == 0) {

						if (inet_pton(AF_INET, con_from, &int_con_from) < 1) {
							printf("%s is not a valid ip address\n", con_from);
							continue;
						}

						tmp_node = handle_mesh_node(int_con_from, con_from);

						if (tmp_node->node_type != 1) {

							tmp_node->node_type = 1;
							tmp_node->node_type_modified = 1;
							if (Global.debug) printf("new internet: %s\n", tmp_node->ip_string);

						}

					} else {

						memmove(hna_node, con_to, NAMEMAX);
						if ((tmp_char = strchr(hna_node, (int)'/'))) {
							tmp_char++;
							address = (int) - inet_network(tmp_char);
							sprintf(hna_name, "%u", (unsigned int)(32 - ceil(log(address) / log(2))));
							strcpy(tmp_char, hna_name);
						}

						tmp_char--;
						tmp_char[0] = 0;
						if (inet_pton(AF_INET, con_from, &int_con_from) < 1) {
							printf("%s is not a valid ip address\n", con_from);
							continue;
						}
						if (inet_pton(AF_INET, hna_node, &int_con_to) < 1) {
							printf("%s is not a valid ip address\n", hna_node);
							continue;
						}
						tmp_char[0] = '/';

						handle_mesh_node(int_con_from, con_from);
						tmp_node = handle_mesh_node(int_con_to, hna_node);

						if (tmp_node->node_type != 2) {

							tmp_node->node_type = 2;
							tmp_node->node_type_modified = 1;
							if (Global.debug) printf("new hna network: %s\n", tmp_node->ip_string);

						}

						handle_con(int_con_from, int_con_to, -1000);

					}

				} else {

					f = strtod(etx, NULL);
					if (f < 1.0)
						f = 999.0;

					if (inet_pton(AF_INET, con_from, &int_con_from) < 1) {
						printf("%s is not a valid ip address\n", con_from);
						continue;
					}
					if (inet_pton(AF_INET, con_to, &int_con_to) < 1) {
						printf("%s is not a valid ip address\n", con_to);
						continue;
					}

					handle_mesh_node(int_con_from, con_from);
					handle_mesh_node(int_con_to, con_to);
					handle_con(int_con_from, int_con_to, f);

				}
				/* remove zerobyte */
				(*con_from_end) = (*con_to_end) = (*etx_end) = '"';
				int_con_from = 0;
				int_con_to = 0;
				con_from = con_from_end = con_to = con_to_end = etx = etx_end = NULL;
				dn = 0;
				last_cr_ptr = lbuf_ptr;
			}

		} else if (((*lbuf_ptr) == '}') && ((*(lbuf_ptr + 1)) == '\n')) {

			Global.output_block_completed = 1;

			hashit = NULL;
			/* check for old nodes and remove them */
			while (NULL != (hashit = hash_iterate(con_hash, hashit))) {
				con = (struct node_con *) hashit->bucket->data;
				if (con->old) {
					if (con->obj_id != -1)
						s3d_del_object(con->obj_id);
					con->obj_id = -1;
					hash_remove_bucket(con_hash, hashit);
				}
				con->old = 1; /* set con on old. if it's still old in the next iteration,
           we will remove it. */
			}

		}

		lbuf_ptr++;

	}

	if (last_cr_ptr != NULL) memmove(lbuf, last_cr_ptr + 1, strlen(last_cr_ptr));
	return(0);

}
