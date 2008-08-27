/*
 * olsrs3d.h
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
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
 * along with olsrs3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "structs.h"


#define max(x,y)((x)>(y)?(x):(y))
#define min(x,y)((x)<(y)?(x):(y))



extern int Debug;

extern struct olsr_con *Con_begin;   /* begin of connection list */
extern struct olsr_node *Olsr_root;   /* top of olsr node tree */
extern struct Obj_to_ip *Obj_to_ip_head, *Obj_to_ip_end, *List_ptr;   /* struct list */

extern int Olsr_node_obj;
extern int Olsr_node_inet_obj;
extern int Olsr_node_hna_net;
extern int Btn_close_obj;
extern int S3d_obj;
extern int Btn_close_id;
extern int Olsr_node_count_obj;
extern int Olsr_node_count;
extern int Last_olsr_node_count;
extern int Net_read_count;
extern int Output_block_counter;
extern int Output_block_completed;
extern int ZeroPoint;
extern float CamPosition[2][3];
extern float Bottom, Left;
extern char lbuf[MAXLINESIZE];
extern int Move_cam_target;
extern int move_cam_to;

/* process */
void lst_initialize(void);
void lst_add(int id, struct olsr_node **olsr_node);
void lst_del(int id);
struct olsr_node *lst_search(int id);
void lst_out(void);
struct olsr_node *move_lst_ptr(int *id);
int process_main(void);
/* net */
int net_init(char *host);
int net_main(void);
int net_quit(void);
/* main */
void out_of_mem(void);
void print_etx(void);
float dist(float p1[], float p2[]);
void window_error(char *msg);
