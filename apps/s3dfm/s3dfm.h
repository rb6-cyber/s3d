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
#include <s3dw.h>
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

struct _t_node {
	char 			 name[M_NAME];						/* name (e.g. file name) */
	struct _t_node 	*parent;							/* parent node */
	struct _t_node 	**sub;								/* list of nodes  (if it's a subdir)*/
	int 			 n_sub;								/* number of nodes in list ( = -1 for normal or not-expanded files) */
	int 			 dirs_opened;						/* how many dirs are opened/displayed */
	float 			 px ,py ,pz ,scale;					/* state after animation */
	float 			 dpx,dpy,dpz,dscale;				/* current state in animation */
	int 			 type;								/* type, determined by extension or file type like dir, pipe, link etc */
	int 			 disp;								/* the type of how the node is displayed currently */
#define D_NONE		0
#define D_ICON		1
#define D_DIR		2
	int				 parsed,detached;					/* Flags for parsed/detached (selected) nodes ... */
	int 			 oid;								/* main oid, e.g the block or icons oid */
	float 			 len;								/* lenght ... TODO: of what? */
	struct 			 {	
		/* some objects which might be used ... if not, should be -1 */
		int 		 close,select,title,titlestr; /* box decorations. */
		int 		 str;						  
		
	} 				 objs;
	int 			 check;							    /* check marker, for internal things */
};
struct _filelist {
	char **p;
	int n;
};
typedef struct _filelist filelist;
typedef struct _t_node   t_node;


extern t_node root,cam; /* some global objects */
extern t_node *focus;	/* the focused object */

/* node.c */
t_node 		*node_getbypath(char *path);
void 		 node_path(t_node *dir, char *path);
t_node 		*node_getbyoid(t_node *t, int oid);
int 		 node_init(t_node *dir);
int 		 node_delete(t_node *dir);
int 		 node_undisplay(t_node *dir);
/* animation.c */
float		 ani_get_scale(t_node *f);
void 		 ani_focus(t_node *f);
int 		 ani_onstack(t_node *f);
void 		 ani_add(t_node *f);
void 		 ani_del(int i);
void 		 ani_doit(t_node *f);
void 		 ani_finish(t_node *f, int i);
void 		 ani_iterate(t_node *f);
int 		 ani_check(t_node *f);
void 		 ani_mate();
/* event.c */
void 		 event_click(struct s3d_evt *evt);
void		 event_key(struct s3d_evt *evt);
void		 event_oinfo(struct s3d_evt *hrmz);
/* box.c */
void	 	 box_draw(t_node *dir);
void 	 	 box_draw_icons(t_node *dir);
int 		 box_undisplay(t_node *dir);
void  		 box_order_icons(t_node *dir);
void	 	 box_sidelabel(t_node *dir);
int 		 box_buildblock(t_node *dir);
void		 box_select(t_node *dir);
void 		 box_order_subdirs(t_node *dir);
/* parse.c */
int 		 parse_dir(t_node *dir);
/* icon.c */
int 		 icon_draw(t_node *dir);
int 		 icon_undisplay(t_node *dir);
/* dialog.c */
void		 close_win(s3dw_widget *button);
void		 window_help();
void		 window_fs_another();
void		 window_fs_nothing();
void		 window_fs_errno(char *errmsg);
void		 window_fs_abort(s3dw_widget *button);
void		 window_copy(char *path);
void		 window_fs_mkdir(s3dw_widget *button);
void		 window_mkdir(char *path);
void		 window_move(char *path);
void		 window_info(char *path);
/* string.c */
void 		 dots_at_start(char *s, unsigned int n, t_node *d);
void		 dotted_int(char *s,unsigned int i);
char		 *mstrncat(char *dest, const char *src, int n);
char		 *mstrncpy(char *dest, const char *src, int n);
/* fs.c */
filelist	 *fl_new(char *path);
void		 fl_del(filelist *fl);
void	 	 fs_fl_approx(filelist *fl, int *files, int *dirs, int *bytes);
int			 fs_fl_copy(filelist *fl, char *dest);
int			 fs_fl_move(filelist *fl, char *dest);
int			 fs_fl_unlink(filelist *fl);
void	 	 fs_approx(char *source, int *files, int *dirs, int *bytes);
int			 fs_copy(char *source, char *dest);
int			 fs_move(char *source, char *dest);
int			 fs_unlink(char *dest);
