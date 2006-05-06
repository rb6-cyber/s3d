/*
 * s3dfm.h
 * 
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3dfm, a s3d file manager.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3dfm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3dfm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3dfm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <s3d.h>
#define T_DUNO		0
#define T_LOCALDIR	1
#define T_BACKDIR	2
#define T_FOLDER	3
#define T_GEOMETRY	4
#define T_MUSIC		5

#define M_DIR		512
#define M_NAME		256

#define BOXHEIGHT	1.2
#define BHP			1.001
#define BHH			BOXHEIGHT+0.001

/* maximum size of the animation stack */
#define MAXANI		1024
#define ZOOMS 		10
/* zoomspeed */

struct t_item {
	int str,close;							/* object ids ...*/
	int block;								/* oid of the block */
	int dirs_opened;						/* how many directories are on the block */
	char name[M_NAME];						/* name (e.g. file name) */
	float len;
	struct t_item *parent;					/* parent item */
	struct t_item *list;					/* list of items  (if it's a subdir)*/
	float px ,py ,pz ,scale;				/* state after animation */
	float dpx,dpy,dpz,dscale;				/* current state in animation */
	int n_item;								/* number of items in list ( = -1 for normal or not-expanded files) */
	int type;								/* type, determined by extension or file type like dir, pipe, link etc */
	int disp;
};

extern struct t_item root;
/* main.c */
void get_path(struct t_item *dir, char *path);
int parse_dir(struct t_item *dir);
struct t_item *finditem(struct t_item *t, int oid);
void object_click(struct s3d_evt *evt);
void mainloop();
/* animation.c */
float ani_get_scale(struct t_item *f);
void ani_focus(struct t_item *f);
void ani_mate();
void ani_add(struct t_item *f);
void ani_del(int i);
void ani_doit(struct t_item *f);
void ani_finish(struct t_item *f, int i);
void ani_iterate(struct t_item *f);
int ani_check(struct t_item *f);
int ani_onstack(struct t_item *f);
/* display.c */
int box_collapse(struct t_item *dir);
int box_collapse_grandkids(struct t_item *dir);
int box_expand(struct t_item *dir);
int box_buildblock(struct t_item *dir);
void box_sidelabel(struct t_item *dir);
void box_position_kids(struct t_item *dir);
int box_icon(struct t_item *dir,int i);
int box_init(struct t_item *dir);
