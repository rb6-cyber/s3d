/*
 * olsrs3d.h
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *                         Marek Lindner <lindner_marek@yahoo.de>
 *                         Andreas Langer <andreas_lbg@gmx.de>
 *
 * This file is part of olsrs3d, an olsr topology visualizer for s3d.
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
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */



#define NAMEMAX		128
#define MAXLINESIZE 1000 /* lines in a digraph just shouldn't get that longer ... */
#define MAXDATASIZE 100 /* max number of bytes we can get at once  */



/* linked list for the all connections */
struct olsr_con {

	struct olsr_con *next_olsr_con;   /* pointer to next connection */
	struct olsr_con *prev_olsr_con;   /* pointer to previous connection */
	struct olsr_node *left_olsr_node;   /* pointer to left end point of the connection */
	struct olsr_node *right_olsr_node;   /* pointer to right end point of the connection */
	float left_etx;   /* etx of left olsr node */
	float right_etx;   /* etx of right olsr node */
	int obj_id;   /* id of connection object in s3d */
	int color;
	int rgb[3];
};


/* linked list for the neighbours of each olsr node */
struct olsr_neigh_list {

	struct olsr_neigh_list *next_olsr_neigh_list;   /* pointer to next neighbour */
	struct olsr_con *olsr_con;   /* pointer to the connection */

};


/* we contruct a binary tree to handle the nodes */
struct olsr_node {

	struct olsr_node *left;
	struct olsr_node *right;
	char ip[NAMEMAX];   /* host ip */
	int node_type;   /* normal = 0, internet gateway = 1, via hna announced network = 2 */
	int node_type_modified;   /* node_type modified flag */
	int last_seen;   /* last seen counter */
	int visible;   /* is this node visible or vanished */
	float pos_vec[3];   /* position vector in 3d "space" */
	float mov_vec[3];   /* move vector */
	int obj_id;   /* id of node object in s3d */
	int desc_id;   /* id of node description object in s3d */
	struct olsr_neigh_list *olsr_neigh_list;   /* pointer to first neighbour */

};


struct Obj_to_ip {
	int id;
	struct olsr_node *olsr_node;
	struct Obj_to_ip *next;
	struct Obj_to_ip *prev;
};



extern int Debug;

extern struct olsr_con *Con_begin;   /* begin of connection list */
extern struct olsr_node *Olsr_root;   /* top of olsr node tree */
extern struct Obj_to_ip *Obj_to_ip_head, *Obj_to_ip_end,*List_ptr;   /* struct list */

extern int	Olsr_node_obj;
extern int	Olsr_node_inet_obj;
extern int	Olsr_node_hna_net;

extern int Olsr_node_count_obj;
extern int Olsr_node_count;
extern int Last_olsr_node_count;

extern int Net_read_count;

extern int ZeroPoint;

extern float Bottom, Left;

extern char lbuf[MAXLINESIZE];
/* process */
void lst_initialize();
void lst_add(int id,struct olsr_node **olsr_node);
void lst_del(int id);
struct olsr_node **lst_search(int id);
void lst_out();
void move_lst_ptr(int *id);
int process_main();
/* net */
int net_init(char *host);
int net_main();
int net_quit();
/* main */
void out_of_mem( void );
struct olsr_node **lst_search(int id);
