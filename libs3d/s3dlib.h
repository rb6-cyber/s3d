/*
 * s3dlib.h
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of the s3d API, the API of s3d (the 3d network display server).
 * See http://s3d.berlios.de/ for more updates.
 * 
 * The s3d API is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * The s3d API is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d API; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <stdint.h>
#include "config.h"
#ifdef __APPLE__ 
#ifdef SHM
#undef SHM
#endif
#endif 

#ifndef S3DUNUSED
	#if defined(UNUSEDPARAM_ATTRIBUTE)
		#define S3DUNUSED(x) (x)__attribute__((unused))
	#elif defined(UNUSEDPARAM_OMIT)
		#define S3DUNUSED(x) /* x */
	#else
		#define S3DUNUSED(x) x
	#endif
#endif

#define VLOW	1
#define	LOW		2
#define MED		3
#define HIGH	4
#define	VHIGH	5

#define DBM_MAX			1024	 /*  maximum size of a debug message string */
#define S3D_NAME_MAX	256		 /*  limit for names [e.g. process names] */
#define OF_TURN_ON 		1		 /*  logical or */
#define OF_TURN_OFF 	2		 /*  logical ?! */
#define OF_TURN_SWAP 	3		 /*  logical ?! */
#define TIMEOUT			100000
#define MAX_CB			256		 /*  as much as there are callbacks */
#ifndef NULL
	#define NULL	0
#endif
#define CON_NULL	0
#define CON_SHM		1
#define CON_TCP		2
#define SHM_SIZE 	sizeof(key_t)*2  		/* space for the keys */
#define RB_STD_SIZE		1024*512
#define RB_OVERHEAD		sizeof(struct buf_t)
/*  the callback buiffer: */
extern int cb_lock;	/* holds the recursion depth */

extern int _s3d_ready; /* is 1 after s3d_init() was sucessful */
extern s3d_cb s3d_cb_list[MAX_CB];
/*  some local prototypes: */
/*  char *s3d_open_file(char *fname); */
int net_prot_in(uint8_t opcode, uint16_t length, char *buf);

#ifdef DEBUG
#ifdef HAVE_GCCVISIBILITY
#pragma GCC visibility push(default) /* Only export following functions */
#endif
void s3dprintf(int relevance, const char *fmt, ...);
void errdn(int relevance, char *func,int en); 
void errds(int relevance,char *func, const char *fmt, ...);
#ifdef HAVE_GCCVISIBILITY
#pragma GCC visibility pop
#endif

#else 
static __inline__ void s3dprintf(int S3DUNUSED(relevance),
                                 const char *S3DUNUSED(fmt), ...) {}
static __inline__ void errdn(int S3DUNUSED(relevance),
                             char *S3DUNUSED(func),
                             int S3DUNUSED(en)) {}
static __inline__ void errds(int S3DUNUSED(relevance),
                             char *S3DUNUSED(func),
                             const char *S3DUNUSED(fmt), ...) {}
#endif

void errn(char *func,int en);
void errs(char *func, char *msg);

/*  fontselect.c */
char *s3d_findfont(char *mask);
/*  object_queue.c */
int _queue_init();
int _queue_fill();
int _queue_new_object(unsigned int oid);
unsigned int _queue_want_object();
int _queue_quit();
/*  network.c */
int net_send(uint8_t opcode, char *buf, uint16_t length);
int s3d_net_init(char *urlc);
#ifdef TCP
/* tcp.c */
int _tcp_init();
int _tcp_quit();
int _s3d_tcp_net_receive();
int tcp_writen(char *str,int s);
int tcp_readn(char *str,int s);
#endif
/* shm_ringbuf.c */
#ifdef SHM
struct buf_t
{
	uint32_t start,end,bufsize;	/* start/end of the data */
};
int shm_write(struct buf_t *rb,char *buf, int n);
int shm_read(struct buf_t *rb,char *buf, int n);
/* shm.c */
int _shm_init(char *ftoken);
int _shm_quit();
int _shm_net_receive();
int shm_writen(char *str,int s);
int shm_readn(char *str,int s);
#endif
/* freetype.c */
struct t_buf
{
	float *vbuf;
	uint32_t *pbuf;
	int pn,vn;
	float xoff;
};

/* tesselate.c */
struct tessp_t
{
	int next,prev,done;
};
int _s3d_tesselate(struct tessp_t *t,struct t_buf *b);
