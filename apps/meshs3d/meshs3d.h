/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 * SPDX-FileCopyrightText: 2004-2015  Marek Lindner <mareklindner@neomailbox.ch>
 * SPDX-FileCopyrightText: 2004-2015  Andreas Langer <an.langer@gmx.de>
 */

#ifndef _MESHS3D_H_
#define _MESHS3D_H_

#include <stdint.h>

#define max(x,y) ((x)>(y)?(x):(y))
#define min(x,y) ((x)<(y)?(x):(y))
#define max_id(x,y) (id_comp(&(x), &(y)) > 0?(x):(y))
#define min_id(x,y) (id_comp(&(x), &(y)) <=0?(x):(y))

#define NAMEMAX  128
#define MAXLINESIZE 1000  /* lines in a digraph just shouldn't get that longer ... */
#define MAXDATASIZE 100   /* max number of bytes we can get at once  */

enum node_type {
	node_undefined = 0,
	node_ip,
	node_ip6,
	node_mac,
	node_generic
};

struct node_id {
	union {
		uint32_t ip;
		uint8_t ip6[16];
		uint8_t mac[6];
		char* generic;
	} id;
	enum node_type type;
};

/* linked list for the all connections */
struct node_con {
	struct node_id address[2];
	float etx1;       /* etx of left olsr node */
	float etx2;      /* etx of right olsr node */
	float etx1_sqrt;     /* sqrt of etx of left olsr node */
	float etx2_sqrt;     /* sqrt etx of right olsr node */
	int obj_id;        /* id of connection object in s3d */
	int color;
	int old;   /* check if the node has been announced in the last block */
	float rgb;
};

/* linked list for the neighbours of each olsr node */
struct olsr_neigh_list {
	struct olsr_neigh_list *next_olsr_neigh_list;  /* pointer to next neighbour */
	struct olsr_con *olsr_con;       /* pointer to the connection */
};


/* we contruct a binary tree to handle the nodes */
struct node {
	struct node_id address;
	char name_string[NAMEMAX];  /* host ip */
	int node_type;     /* normal = 0, internet gateway = 1, via tt announced network = 2 */
	int node_type_modified;   /* node_type modified flag */
	int last_seen;     /* last seen counter */
	int visible;     /* is this node visible or vanished */
	float pos_vec[3];    /* position vector in 3d "space" */
	float mov_vec[3];    /* move vector */
	int obj_id;      /* id of node object in s3d */
	int desc_id;     /* id of node description object in s3d */
	float desc_length;    /* length of node description object in s3d */
};

struct glob {
	int debug;
	int obj_node_normal;
	int obj_node_inet;
	int obj_node_tt;
	int obj_btn_close;
	int obj_s3d_url;
	int obj_zero_point;
	int obj_node_count;
	int node_count;
	int color_switch;
	int output_block_counter;
	int output_block_completed;
	float asp;
	float bottom;
	float left;
	float cam_position[2][3];
};

extern char lbuf[MAXLINESIZE];
extern struct glob Global;
extern struct hashtable_t *node_hash;
extern struct hashtable_t *con_hash;

/* process.c */
/*
void lst_initialize(void);
void lst_add(int id,struct olsr_node **olsr_node);
void lst_del(int id);
struct olsr_node *lst_search(int id);
void lst_out(void);
struct olsr_node *move_lst_ptr(int *id);
*/
int id_comp(const struct node_id* id1, const struct node_id* id2);
int process_main(void);
void process_init(void);

/* net.c */
int net_init(char *host);
int net_main(void);
int net_quit(void);

/* main */
/*
void out_of_mem( void );
void print_etx( void );
float dist(float p1[], float p2[]);
void window_error(char *msg);
*/

#endif /* _MESHS3D_H_ */
