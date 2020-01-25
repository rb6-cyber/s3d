/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 * SPDX-FileCopyrightText: 2004-2015  Marek Lindner <mareklindner@neomailbox.ch>
 * SPDX-FileCopyrightText: 2004-2015  Andreas Langer <an.langer@gmx.de>
 */

#ifndef _OLSRS3D_H_
#define _OLSRS3D_H_

#include "structs.h"
#include <config-s3d.h>

#define max(x,y)((x)>(y)?(x):(y))
#define min(x,y)((x)<(y)?(x):(y))

#ifndef OLSRS3DUNUSED
#if defined(UNUSEDPARAM_ATTRIBUTE)
#define OLSRS3DUNUSED(x) (x)__attribute__((unused))
#elif defined(UNUSEDPARAM_OMIT)
#define OLSRS3DUNUSED(x) /* x */
#else
#define OLSRS3DUNUSED(x) x
#endif
#endif
#ifdef __GNUC_MINOR__
#define NO_RETURN  __attribute__ ((__noreturn__))
#else
#define NO_RETURN
#endif

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
void out_of_mem(void) NO_RETURN;
void print_etx(void);
float dist(float p1[], float p2[]);
void window_error(const char *msg);

#endif /* _OLSRS3D_H_ */
