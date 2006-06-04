/*
 * global.h
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3d, a 3d network display server.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "config.h"
#include <stdint.h>		 /*  integer types */
#ifdef SHM
	#include <sys/shm.h> /* key_t */
#endif
/*  variables and defines */
extern int frame_mode; 	 /*  GLUT, SDL, ... ? */
extern int running;		 /*  server running flag */
/*  relevance macros */
#define NAME_MAX	256		 /*  limit for names [e.g. process names] */
#define MCP			0		 /*  the mcp's pid	 */
#define TEXTURE_MAX_W	2048
#define TEXTURE_MAX_H	2048
/*  server version */
#define S3D_SERVER_MAJOR	0
#define S3D_SERVER_MINOR	1
#define S3D_SERVER_PATCH	0
#define S3D_SERVER_NAME 	"dotslash s3d server"

#define MAXPLEN	65536

#define RB_STD_SIZE		1024*512
#define RB_MAX_SIZE		1048*4096
#define SHM_SIZE 	sizeof(key_t)*2  		/* space for the keys */

#define RB_OVERHEAD		sizeof(struct buf_t)

#define obj_valid(p,oid,o)	((oid<p->n_obj) && ((o=p->object[oid])!=NULL))
typedef float t_mtrx[16];

struct buf_t
{
	uint32_t start,end,bufsize;	/* start/end of the data */
};

/*  some graphic simple prototypes, they might get into some headerfile later ... */
/*  our lovely vertex list ... */
struct t_vertex 
{
	float x,y,z;
};
struct t_texc
{
	float x,y;
};
/*  polygon definition; */
/*  it's all handled via list types as usually we have only one surface for many polygons, */
/*  and many vertexes have 2 or more polygons connected. OpenGL will optimize the lists for us */
/*  anyways, so we shouldn't care ... */
struct t_poly
{
	uint32_t v[3]; 				 /*  we define a poly as set of 3 vertexes, as its usual */
	struct t_vertex n[3]; 		 /*  normal vectors */
	uint32_t mat;  				 /*  material index */
	struct t_texc tc[3];		 /*  texture coords */
};
struct t_line
{
	uint32_t v[2];
	struct t_vertex n[2];		 /* normal vectors */
	uint32_t mat;
};
/*  material of surfaces, as it's usual in the OpenGL standard */
struct t_mat
{
	float amb_r,amb_g,amb_b,amb_a, 			 /*  ambience */
		  spec_r,spec_g,spec_b,spec_a,	 	 /*  specualar */
		  diff_r,diff_g,diff_b,diff_a;		 /*  diffusion */
	uint32_t tex;							 /*  texture index, -1 if there is no */
};
/*  this defines a texture */
struct t_tex
{
	uint16_t w,h;		 /*  width and height */
	uint16_t tw,th;		 /*  texture width */
	uint8_t *buf;		 /*  the data */
	float xs,ys;		 /*  scale data for gl-implementations which require 2^x */
						 /*  texture sizes. */
	uint32_t gl_texnum;	 /*  the gl texture number. */
};
/*  the object type */
struct t_obj
{
		uint32_t oflags;			 /*  flags, like this object beeing input etc. */
#define OF_TURN_ON 		1
#define OF_TURN_OFF 	2
#define OF_TURN_SWAP 	3

#define	OF_VISIBLE		0x00000001
#define	OF_SELECTABLE	0x00000002
#define	OF_POINTABLE	0x00000004

#define OF_CLONE_SRC	0x01000000
#define OF_LINK_SRC		0x02000000
#define OF_LINK			0x04000000


#define OF_TYPE			0xF0000000
#define OF_NODATA		0xF0000000 /* no data allowed! */

#define OF_CLONE		0x10000000
#define OF_VIRTUAL		0x20000000
		
#define OF_SYSTEM		0x80000000
#define OF_CAM			0x90000000
#define OF_POINTER		0xA0000000
#define OF_3DPOINTER	0xB0000000

#define OF_MASK			0x00FFFFFF
		uint32_t n_vertex, n_mat, n_poly, n_tex, n_line;
					 /*  if OF_VIRTUAL is set, n_mat contains the pid */
					 /*  if OF_CLONE is set, n_vertex contains the original oid */
					 /*  I know this is dirty, but it would a waste of data if I don't do so ... */
		uint32_t dplist;	 /*  opengl display list number */
		uint32_t linkid;	 /*  linking target, -1 if there is none */
		uint32_t lsub,lnext,lprev;
		 /*  pointer to our objects; */
		struct t_vertex *p_vertex;
		struct t_mat	*p_mat;
		struct t_poly	*p_poly;
		struct t_line   *p_line;
		struct t_tex	*p_tex;
		struct t_vertex translate,rotate;
		float 			scale;
		t_mtrx			m;
		int				m_uptodate;	 
		float r,or;					 /*  radius, object radius */
};
#ifdef SHM
struct t_shmcb {
	int shmid_ctos,shmid_stoc;
	key_t key_ctos,key_stoc;
	char *data_ctos,*data_stoc;
	int size_ctos,size_stoc;
	int idle;
};
#endif

/*  l_* is a list-type, t_* is the type itself */
struct t_process
{
	char 				  name[NAME_MAX];		 /*  process name */
	struct t_obj		**object;				 /*  initial pointer to object list */
	uint32_t			  n_obj;				 /*  number of objects */
	uint32_t			  biggest_obj;			 /*  the biggest object */
	uint32_t			  mcp_oid;				 /*  oid in mcp */
	int 				  id;					 /*  pid */
	int					  con_type;				 /*  type of connection, one of following: */
#define CON_NULL	0
#define CON_TCP		1
#define CON_SHM		2
#ifdef TCP
	int					  sockid;
#endif
#ifdef SHM
	struct t_shmcb		  shmsock;
#endif
};

struct t_obj_info 
{
	uint32_t object;
	uint32_t flags;
	float trans_x,trans_y,trans_z;
	float rot_x,rot_y,rot_z;
	float scale;
	float r;
	char name[NAME_MAX]; 
};
enum {
	zero,
	FRAME_GLUT,
	FRAME_SDL
};
/*  main.c */
int rc_init(void);
int init(void);
void quit(void);
void one_time(void);
/*  network.c */
void sigpipe_handler(int);
void sigio_handler(int);
int network_init(void);
int network_quit(void);
int network_main(void);
int n_readn(struct t_process *p, uint8_t *str,int s);
int n_writen(struct t_process *p, uint8_t *str,int s);
int n_remove(struct t_process *p);
/* tcp.c */
int tcp_init(void);
int tcp_quit(void);
int tcp_pollport(void);
int tcp_pollproc(void);
int tcp_prot_com_in(struct t_process *p);
int tcp_writen(int sock, uint8_t *str,int s);
int tcp_readn(int sock, uint8_t *str,int s);
int tcp_remove(int sock);
/* shm.c/shm_ringbuf.c */
int shm_init(void);
int shm_quit(void);
int shm_main(void);
int shm_remove(struct t_process *p);
int shm_writen(struct buf_t *rb,uint8_t *buf, int n);
int shm_readn(struct buf_t *rb,uint8_t *buf, int n);
int shm_prot_com_in(struct t_process *p);
/* shm_ringbuf.c */
void ringbuf_init(char *data,unsigned long init_size);
int shm_write(struct buf_t *rb,char *buf, int n);
int shm_read(struct buf_t *rb,char *buf, int n);
/*  proto.c */
int prot_com_in(struct t_process *p, uint8_t *pbuf);
int prot_com_out(struct t_process *p, uint8_t opcode, uint8_t *buf, uint16_t length);
/* event.c */
int event_obj_info(struct t_process *p, uint32_t oid);
int event_obj_click(struct t_process *p, uint32_t oid);
int event_key_pressed(uint16_t key, uint16_t uni, uint16_t mod, int state);
int event_mbutton_clicked(uint8_t button, uint8_t state);
int event_cam_changed(void);
int event_ptr_changed(void);
int event_init(struct t_process *p);
int event_quit(struct t_process *p);
int event_ping_in(struct t_process *p, uint32_t o);
/*   user.c */
int user_init(void);
int user_quit(void);
int user_main(void);
#ifdef G_GLUT
int user_init_glut(void);
int user_quit_glut(void);
#endif
#ifdef G_SDL
int user_init_sdl(void);
int user_quit_sdl(void);
int user_main_sdl(void);
#endif
void user_mouse(int button, int state, int x, int y);
void user_key(unsigned short key, unsigned short unicode, unsigned short mod, int state);

/*  error.c */
void errn(char *func, int en);
void errnf(char *func, int en);
void errs(char *func, char *msg);
void errsf(char *func, char *msg);
#ifdef DEBUG
void errds(int relevance,char *func, const char *fmt, ...);
void dprintf(int relevance, const char *msg, ...);
#endif
/*  graphics.c */
int graphics_quit(void);
void graphics_main(void);
int graphics_pick_obj(int x, int y);
int graphics_init(void);
int render_by_mcp(void);
#ifdef G_GLUT
int graphics_init_glut(void);
int graphics_quit_glut(void);
#endif
#ifdef G_SDL
int graphics_init_sdl(void);
int graphics_quit_sdl(void);
#endif

void graphics_reshape( int w, int h);
/*  navigation.c */
void navi_left(void);
void navi_right(void);
void navi_fwd(void);
void navi_back(void);
void navi_rot_left(void);
void navi_rot_right(void);
void navi_rot_up(void);
void navi_rot_down(void);
void navi_pos(int xdif, int ydif);
void navi_rot(int xdif, int ydif);
void ptr_move(int x, int y);
/*  process.c */
struct t_process *process_add(void);
int process_del(int id);
int process_init(void);
int process_quit(void);
struct t_process *process_protinit(struct t_process *p, char *name);
struct t_process *get_proc_by_pid(int pid);
/*  object.c */
int obj_debug			(struct t_process *p, uint32_t oid);
int obj_new				(struct t_process *p);
int obj_clone			(struct t_process *p, uint32_t oid);
int obj_clone_change	(struct t_process *p, uint32_t oid, uint32_t toid);
int obj_link			(struct t_process *p, uint32_t oid_from, uint32_t oid_to);
int obj_unlink			(struct t_process *p, uint32_t oid);
int obj_del				(struct t_process *p, uint32_t oid);
int obj_push_vertex		(struct t_process *p, uint32_t oid, float *x, uint32_t n);
int obj_push_mat		(struct t_process *p, uint32_t oid, float *x, uint32_t n);
int obj_push_poly		(struct t_process *p, uint32_t oid, uint32_t *x, uint32_t n);
int obj_push_line		(struct t_process *p, uint32_t oid, uint32_t *x, uint32_t n);
int obj_push_tex		(struct t_process *p, uint32_t oid, uint16_t *x, uint32_t n);
int obj_pep_poly_normal	(struct t_process *p, uint32_t oid, float *x, uint32_t n);
int obj_pep_line_normal (struct t_process *p, uint32_t oid, float *x, uint32_t n);
int obj_pep_poly_texc	(struct t_process *p, uint32_t oid, float *x, uint32_t  n);
int obj_pep_mat			(struct t_process *p, uint32_t oid, float *x, uint32_t n);
int obj_pep_mat_tex		(struct t_process *p, uint32_t oid, uint32_t *x, uint32_t n);
int obj_pep_vertex		(struct t_process *p, uint32_t oid, float *x, uint32_t n);
int obj_pep_line		(struct t_process *p, uint32_t oid, uint32_t *x, uint32_t n);
int obj_load_poly_normal(struct t_process *p, uint32_t oid, float *x, uint32_t start, uint32_t n);
int obj_load_line_normal(struct t_process *p, uint32_t oid, float *x, uint32_t start, uint32_t n);
int obj_load_poly_texc	(struct t_process *p, uint32_t oid, float *x, uint32_t start, uint32_t n);
int obj_load_mat		(struct t_process *p, uint32_t oid, float *x, uint32_t start, uint32_t n);
int obj_load_tex		(struct t_process *p, uint32_t oid, uint32_t tex, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *pixbuf);
int obj_del_vertex		(struct t_process *p, uint32_t oid, uint32_t n);
int obj_del_mat			(struct t_process *p, uint32_t oid, uint32_t n);
int obj_del_poly		(struct t_process *p, uint32_t oid, uint32_t n);
int obj_del_line		(struct t_process *p, uint32_t oid, uint32_t n);
int obj_del_tex			(struct t_process *p, uint32_t oid, uint32_t n);
int obj_toggle_flags	(struct t_process *p, uint32_t oid, uint8_t type, uint32_t flags);
int obj_translate		(struct t_process *p, uint32_t oid, float *transv);
int obj_rotate			(struct t_process *p, uint32_t oid, float *rotv);
int obj_scale			(struct t_process *p, uint32_t oid, float scav);
int obj_render			(struct t_process *p, uint32_t oid);
int obj_free			(struct t_process *p, uint32_t oid);
void obj_get_maximum	(struct t_process *p, struct t_obj *obj);
void into_position		(struct t_process *p, struct t_obj *obj, int depth);
void obj_recalc_tmat	(struct t_process *p, uint32_t oid);
void obj_size_update	(struct t_process *p, uint32_t oid);
void obj_pos_update(struct t_process *p, uint32_t oid, uint32_t first_oid);
void obj_check_biggest_object(struct t_process *p, uint32_t oid);
uint32_t get_pointer(struct t_process *p);
void link_delete(struct t_process *p, uint32_t oid);
void link_insert(struct t_process *p, uint32_t oid, uint32_t target);
/*  mcp.c */
int mcp_rep_object(uint32_t mcp_oid);
int mcp_del_object(uint32_t mcp_oid);
int mcp_init(void);
int mcp_focus(int oid);
/*  matrix.c */
void myMultMatrix(t_mtrx mat2);
void myGetMatrix(t_mtrx mat);
void mySetMatrix(t_mtrx mat);
void myTranslatef(float x, float y, float z);
void myScalef(float x, float y, float z);
void myRotatef(float angle, float x, float y, float z);
void myLoadIdentity(void);
void myTransform4f(float *v);
void myTransform3f(float *v);
void myTransformV(struct t_vertex *v);
int  myInvert(void);

/* cull.c */
void cull_get_planes(void);
int  cull_sphere_in_frustum(struct t_vertex *center, float radius);
