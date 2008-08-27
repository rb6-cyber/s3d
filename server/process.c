/*
 * process.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
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


#include "global.h"
#include <stdlib.h>  /*  for malloc, free */
#include <string.h>  /*  strncmp(), strncpy() */
/*  this piece of code handles processes */

#define mcp_p (&procs_p[0])
struct t_process  *procs_p = NULL;  /* pointer to the processes */
int       procs_n;      /* number of processes */
static int p_del(struct t_process *p);   /*  local prototype */
static int process_list_rm(int pid);
int process_sys_init(struct t_process *p);

/* protocol request for process initialization */
struct t_process *process_protinit(struct t_process *p, const char *name) {
	int con_type;
	int32_t mcp_oid;
	if ((strncmp(name, "sys_", 4) == 0)) { /* we don't like "sys_"-apps, kicking this */
		errds(VHIGH, "process_protinit()", "appnames starting with 'sys_' not allowed.");
		return(NULL);
	}
	if ((p->id != MCP) && (strncmp(name, "mcp", 3) == 0)) {
		if (procs_p[MCP].con_type == CON_NULL) {
			s3dprintf(MED, "free mcp place, pid %d becoming mcp!", p->id);
			con_type = p->con_type; /* move connection data */
#ifdef TCP
			procs_p[MCP].sockid = p->sockid; /* don't save contype yet,
         or p_del will notify mcp about a deleted
         mcp-object (which is itselfs, actually) */
#endif
#ifdef SHM
			memcpy(&procs_p[MCP].shmsock, &p->shmsock, sizeof(struct t_shmcb));
#endif
			p_del(p); /* deleting data/mcp object */
			procs_p[MCP].con_type = con_type;
			mcp_init();
			process_list_rm(p->id); /* remove old process, but don't kill connection */
			return(&procs_p[MCP]);
		} else {
			s3dprintf(LOW, "the place for the mcp is already taken ...");
			return(NULL);
		}
	} else {
		strncpy(p->name, name, S3D_NAME_MAX);
		process_sys_init(p);

		/* register the new process in the mcp */
		if (-1 != (mcp_oid = obj_new(&procs_p[MCP]))) {
			mcp_p->object[mcp_oid]->oflags |= OF_VIRTUAL | OF_VISIBLE | OF_SELECTABLE;
			mcp_p->object[mcp_oid]->virtual_pid = p->id;

			/*   mcp_p->object[mcp_oid]->p_mat=(struct t_material *)new_p; */
			/*  dirty, but it's just a pointer after all ... */
			p->mcp_oid = mcp_oid;
			s3dprintf(LOW, "process %d now has mcp_oid %d", p->id, mcp_oid);
			mcp_rep_object(mcp_oid);
			if (mcp_p->con_type == CON_NULL) { /*  there is no mcp connected! setting focus to the new program: */
				mcp_focus(mcp_oid);
			}
		} else {
			s3dprintf(LOW, "couldn't add object to mcp ...");
		}
	}
	return(p);
}
/* adds system objects to the app, like camera, pointers etc ... */
int process_sys_init(struct t_process *p)
{
	int cam, ptr;
	struct t_obj *o;
	cam = obj_new(p);
	ptr = obj_new(p);
	if (p->id == MCP) {   /* this is only called once within process_init, later mcp's are
     will be registered as "real" apps first */
		p->object[cam]->translate.z = 5;
		p->object[cam]->oflags = OF_CAM;
		p->object[ptr]->translate.z = -1;
		p->object[ptr]->oflags = OF_POINTER;
		link_insert(p, ptr, cam);
	} else {
		/* TODO: ... get the cam and ptr position of the mcp, somehow */
		p->object[cam]->oflags = OF_CAM;

		if (OBJ_VALID(mcp_p, get_pointer(mcp_p), o)) { /* get parent pointer, copy parent */
			p->object[ptr]->rotate.x = o->rotate.x;
			p->object[ptr]->rotate.y = o->rotate.y;
			p->object[ptr]->rotate.z = o->rotate.z;
			p->object[ptr]->translate.x = o->translate.x;
			p->object[ptr]->translate.y = o->translate.y;
			p->object[ptr]->translate.z = o->translate.z;
		}
		p->object[ptr]->oflags = OF_POINTER;
		link_insert(p, ptr, cam);
	}
	s3dprintf(MED, "process_sys_init(): added object cam0 %d", cam);
	s3dprintf(MED, "process_sys_init(): added object ptr0 %d", ptr);
	obj_pos_update(get_proc_by_pid(MCP), cam, cam);
	obj_pos_update(get_proc_by_pid(MCP), ptr, ptr);
	/* obj_recalc_tmat(p,0);*/
	event_obj_info(p, 0); /* tell the new program about the thing */

	return(0);
}

/* this is to be called when a new connection appears. a pointer to the added process will be returned */
struct t_process *process_add(void) {
	struct t_process *new_p;
	procs_n++;
	procs_p = (struct t_process *)realloc(procs_p, sizeof(struct t_process)*procs_n); /* increase the block */
	new_p = &procs_p[procs_n-1];

	new_p->id   = procs_n - 1;
	/* if (new_p->id==0)
	  mcp_p=&procs_p[0];*/
	new_p->object = NULL;
	new_p->n_obj  = 0;
	/* new_p->netin  = 0;*/
	new_p->mcp_oid = -1;
	new_p->biggest_obj = -1;
	new_p->con_type = CON_NULL; /* this is to be changed by the caller */
	new_p->name[0] = '\0';
	return(new_p);
}
/* deletes the process with pid */
int process_del(int pid)
{
	if (pid == MCP) {
		n_remove(&procs_p[pid]);
		p_del(&procs_p[pid]);
		return(0);
	}
	if ((pid > 0) && (pid < procs_n)) {
		n_remove(&procs_p[pid]);
		p_del(&procs_p[pid]);
		process_list_rm(pid);
		return(0);
	}
	return(-1);
}
/* just kick process out of the process list, no network/mcp-oid cleanup */
static int process_list_rm(int pid)
{
	if (pid != (procs_n - 1)) { /* copy last block, swap pid */
		memcpy(&procs_p[pid], &procs_p[procs_n-1], sizeof(struct t_process));
		procs_p[pid].id = pid; /* change the pid of the new procs_p */
		if (procs_p[pid].mcp_oid != -1) /* the last process could just appear without initializing yet ... */
			procs_p[0].object[procs_p[pid].mcp_oid]->virtual_pid = pid;
		/* change the mcp-objects pid-pointer to the right position! */
		/* this is kind of pointer madness */
	}
	procs_n--;
	procs_p = (struct t_process*)realloc(procs_p, sizeof(struct t_process) * procs_n); /* decrease the block,
  wipe the last one */
	return(0);
}
struct t_process *get_proc_by_pid(int pid) {
	if ((pid >= 0) && (pid < procs_n))
		return(&procs_p[pid]);
	return(NULL);
}
/*  this actually deleted the process with all it's parts */
/* it's quite the same as the original version, but without free() */
static int p_del(struct t_process *p)
{
	int j, i = p->n_obj;
	if (p->id != MCP) {
		if (p->mcp_oid != -1) {
			for (j = 0;j < mcp_p->n_obj;j++)  /*  remove clones and links pointing on this app-object ... */
				if (mcp_p->object[j] != NULL) {
					if ((mcp_p->object[j]->oflags&OF_CLONE) && (mcp_p->object[j]->clone_ooid == p->mcp_oid)) { /*  it's linking to our object! */
						mcp_p->object[j]->oflags &= ~OF_CLONE;  /*  disable clone flag */
						mcp_p->object[j]->clone_ooid = 0;   /*  and "clone reference" to 0 */
						mcp_p->object[j]->r = 0.0F;   /*  empty object, so radius is zero! */
					}
				}
			obj_free(mcp_p, p->mcp_oid);  /*  free the mcp-app-object. */
			mcp_del_object(p->mcp_oid);   /*  tell MCP that it's object is beeing deleted. */
		} /* else
   errs("p_del()","bad mcp_oid, unable to free mcp object");*/
		if (i > 0) {
			for (i = 0;i < p->n_obj;i++)
				if (p->object[i]) obj_free(p, i);
			free(p->object);
		}
	} else {
		/*  the mcp keeps in our memory ... */
		/*  so we just delete the objects added */
		/*  by the last mcp */
		s3dprintf(MED, "clean up mcp's junk ...");
		for (i = 0;i < p->n_obj;i++) {
			if (p->object[i] != NULL)
				if (!(p->object[i]->oflags&(OF_SYSTEM | OF_VIRTUAL)))
					obj_free(p, i);
		}
	}
	return(0);  /*  successfully deleted */
}
int process_init(void)
{
	procs_n = 0;
	procs_p = NULL;
	process_add();
	/* set up mcp */
	strncpy(mcp_p->name, "mcp", S3D_NAME_MAX);
	mcp_p->con_type = CON_NULL;
	process_sys_init(mcp_p);
	return(0);
}
int process_quit(void)
{
	int i;
	s3dprintf(HIGH, "telling %d processes to go away", procs_n);
	for (i = (procs_n - 1);i >= 0;i--) {
		s3dprintf(HIGH, "[QUIT] for %d", i);
		event_quit(&procs_p[i]);
		/*  process_del(procs_p[i].id);*/
	}
	free(procs_p);
	return(0);
}
