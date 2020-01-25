/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 * SPDX-FileCopyrightText: 2004-2015  Marek Lindner <mareklindner@neomailbox.ch>
 * SPDX-FileCopyrightText: 2004-2015  Andreas Langer <an.langer@gmx.de>
 */

#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#define NAMEMAX  128
#define MAXLINESIZE 1000  /* lines in a digraph just shouldn't get that longer ... */
#define MAXDATASIZE 100   /* max number of bytes we can get at once  */

/* linked list for the all connections */
struct olsr_con {
	struct olsr_con *next_olsr_con;   /* pointer to next connection */
	struct olsr_con *prev_olsr_con;   /* pointer to previous connection */
	struct olsr_node *left_olsr_node;  /* pointer to left end point of the connection */
	struct olsr_node *right_olsr_node;  /* pointer to right end point of the connection */
	float left_etx;       /* etx of left olsr node */
	float right_etx;      /* etx of right olsr node */
	float left_etx_sqrt;     /* sqrt of etx of left olsr node */
	float right_etx_sqrt;     /* sqrt etx of right olsr node */
	int obj_id;        /* id of connection object in s3d */
	int color;
	float rgb;
};


/* linked list for the neighbours of each olsr node */
struct olsr_neigh_list {
	struct olsr_neigh_list *next_olsr_neigh_list;  /* pointer to next neighbour */
	struct olsr_con *olsr_con;       /* pointer to the connection */
};


/* we contruct a binary tree to handle the nodes */
struct olsr_node {
	struct olsr_node *left;
	struct olsr_node *right;
	char ip[NAMEMAX];    /* host ip */
	int static_node;    /* static nodes have some known geolocation and won't move */
	int node_type;     /* normal = 0, internet gateway = 1, via hna announced network = 2 */
	int node_type_modified;   /* node_type modified flag */
	int last_seen;     /* last seen counter */
	int visible;     /* is this node visible or vanished */
	float pos_vec[3];    /* position vector in 3d "space" */
	float mov_vec[3];    /* move vector */
	int obj_id;      /* id of node object in s3d */
	int desc_id;     /* id of node description object in s3d */
	float desc_length;    /* length of node description object in s3d */
	struct olsr_neigh_list *olsr_neigh_list; /* pointer to first neighbour */
};


struct Obj_to_ip {
	int id;
	struct olsr_node *olsr_node;
	struct Obj_to_ip *next;
	struct Obj_to_ip *prev;
};

#endif /* _STRUCTS_H_ */
