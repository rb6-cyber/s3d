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

/* total height of the block */
#define BOXHEIGHT	1.2
#define BHP			1.001
#define BHH			BOXHEIGHT+0.001
/* how much should detached dirs move away from parent block */
#define DETHEIGHT	0.3

/* maximum size of the animation stack */
#define MAXANI		1024
#define ZOOMS 		10
/* zoomspeed */

struct _t_item {
	int str,close,select,title,titlestr;	/* object ids ...*/
	int block;								/* oid of the block */
	int dirs_opened;						/* how many directories are on the block */
	int detached;							/* if it's detached ... */
	char name[M_NAME];						/* name (e.g. file name) */
	float len;
	struct _t_item *parent;					/* parent item */
	struct _t_item *list;					/* list of items  (if it's a subdir)*/
	float px ,py ,pz ,scale;				/* state after animation */
	float dpx,dpy,dpz,dscale;				/* current state in animation */
	int n_item;								/* number of items in list ( = -1 for normal or not-expanded files) */
	int type;								/* type, determined by extension or file type like dir, pipe, link etc */
	int disp,parsed;						/* Flags for displayed/parsed items ... */
#define D_ICON	1
#define D_DIR	2
};
struct _filelist {
	char **p;
	int n;
};
typedef struct _filelist filelist;
typedef struct _t_item   t_item;


extern t_item root;
/* main.c */
void get_path(t_item *dir, char *path);
t_item *get_item(char *path);
t_item *finditem(t_item *t, int oid);
void mainloop();
/* parse.c */
int node_init(t_item *dir);
void node_free(t_item *t);
int parse_dir(t_item *dir);
void parse_again(t_item *t);
/* animation.c */
float ani_get_scale(t_item *f);
void ani_focus(t_item *f);
void ani_mate();
void ani_add(t_item *f);
void ani_del(int i);
void ani_doit(t_item *f);
void ani_finish(t_item *f, int i);
void ani_iterate(t_item *f);
int ani_check(t_item *f);
int ani_onstack(t_item *f);
/* box.c */
int box_collapse(t_item *dir,int force);
int box_collapse_grandkids(t_item *dir);
int box_expand(t_item *dir);
int box_buildblock(t_item *dir);
void box_dissolve(t_item *dir);
int  box_undisplay(t_item *dir);
void box_sidelabel(t_item *dir);
void box_position_kids(t_item *dir);
void box_select(t_item *dir);
/* icon.c */
int icon_draw(t_item *dir,int i);
int icon_undisplay(t_item *dir);
/* fs.c */
filelist *fl_new(char *path);
void fl_del(filelist *fl);
int fs_copy(char *source, char *dest);
int fs_move(char *source, char *dest);
int fs_unlink(char *dest);
void fs_approx(char *source, int *files, int *dirs, int *bytes);
int fs_fl_copy(filelist *fl, char *dest);
int fs_fl_move(filelist *fl, char *dest);
int fs_fl_unlink(filelist *fl);
void fs_fl_approx(filelist *fl, int *files, int *dirs, int *bytes);
/* dialog.c */
void key_handler(struct s3d_evt *evt);
void object_click(struct s3d_evt *evt);
void window_info(char *path);
void window_help();
void window_copy(char *path);
void window_move(char *path);
void window_mkdir(char *path);
