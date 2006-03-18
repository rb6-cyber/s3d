/*
 * object.c
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


#include "global.h"
#include <stdlib.h>		 /*  malloc(),realloc(),free() */
#include <string.h>		 /*  memcpy() */
#include <GL/gl.h>		 /*  gl*, GL* */
#define _ISOC99_SOURCE
#include <math.h>		 /*  sin(),cos() */

#define MAXLOOP	10
		 /*  if oid is always unsigned, we don't have to check oid>=0 */

extern t_mtrx Identity;
extern int focus_oid;


static void obj_update_tex(struct t_tex *tex,unsigned short x,unsigned short y,unsigned short w,unsigned short h,unsigned char *pixbuf);
void obj_sys_update(struct t_process *p, uint32_t oid);

/*  debugging function for objects, prints out some stuff known about it... */
int obj_debug			(struct t_process *p, uint32_t oid)
{
	struct t_obj *o;
	dprintf(HIGH,"about pid %d/obj %d:",p->id,oid);
	if (obj_valid(p,oid,o))
	{
		dprintf(HIGH,"vertices: %d, polygons: %d, materials: %d, textures: %d, flags: %010x",o->n_vertex,o->n_poly, o->n_mat, o->n_tex,o->oflags);
		dprintf(HIGH,"linkid %d, displaylist %d",o->linkid,o->dplist);
		dprintf(HIGH,"translation: %f %f %f",o->translate.x,o->translate.y,o->translate.z);
		dprintf(HIGH,"rotation: %f %f %f",o->rotate.x,o->rotate.y,o->rotate.z);
		dprintf(HIGH,"scale: %f",o->scale);
		if (o->oflags&OF_SYSTEM)
		{
			dprintf(HIGH,"it's a system object!!");
		}
		else if (o->oflags&OF_CLONE)
		{
			dprintf(HIGH,"it's a clone linking to %d",o->n_vertex);
			obj_debug(p,o->n_vertex);
		}
	} else {
		dprintf(HIGH,"can't get oid %d pid %d",oid,p->id);
	}
	return(0);
}
/*  push a few new vertices onto the stack. */
int obj_push_vertex		(struct t_process *p, uint32_t oid, float *x, uint32_t n)
{
	uint32_t i,m;
	struct t_vertex *p_vertex;
	struct t_vertex *a;
	struct t_obj *obj;
	float *px;
	float r;
	int is_clnsrc;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_push_vertex()","error: no data on object allowed!");
			return(-1);
		}

		m=obj->n_vertex;	 /*  saving the first number of vertices */
		px=x; 				 /*  movable pointer for x, later */
		if (NULL!=(p_vertex=realloc(obj->p_vertex,sizeof(struct t_vertex) * ( n + (obj->n_vertex)))))
		{
			if (obj->dplist)
			{
				dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
				glDeleteLists(obj->dplist,1);
				obj->dplist=0;
			}
			obj->p_vertex=p_vertex;
			for (i=0;i<n;i++)
			{
				obj->p_vertex[m+i].x=*(px++);
				obj->p_vertex[m+i].y=*(px++);
				obj->p_vertex[m+i].z=*(px++);

				a=&obj->p_vertex[m+i];
				r=obj->scale * sqrt(	
						(a->x * a->x ) + 
						(a->y * a->y ) +
						(a->z * a->z ));
				if (r> obj->r) obj->r=r;
/*				dprintf(VLOW,"added following vertex[%d]: %f, %f, %f",i,
								obj->p_vertex[m+i].x,
								obj->p_vertex[m+i].y,
								obj->p_vertex[m+i].z);*/

			}
			if (p->id!=MCP)
			{
			/* this is doing live update which is quite okay, but we need
			 * to check for biggest update and clonesources ... */
				obj_check_biggest_object(p,oid);
			}
			if (p->object[oid]->oflags&OF_CLONE_SRC)
			{
				is_clnsrc=0;
				for (i=0;i<p->n_obj;i++)
				{
					if (p->object[i]!=NULL)
					{
						if ((p->object[i]->oflags&OF_CLONE) && (p->object[i]->n_vertex==oid))
						{ /* if it's pointing to our object ... */
							is_clnsrc=1;
							p->object[i]->r=obj->r*(p->object[i]->r/obj->scale); /* give it the new radius too! */
							obj_check_biggest_object(p,i);
						}
					}
				}
				if (!is_clnsrc)
					p->object[oid]->oflags&=~OF_CLONE_SRC;
			}

			obj->n_vertex+=n;
		}
	} else 
	{
		return(-1);
	}
	return(0);
}


int obj_push_mat		(struct t_process *p, uint32_t oid, float *x, uint32_t n)
{
	uint32_t i,m;
	struct t_mat *p_mat;
	struct t_obj *obj;
	float *px;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_push_mat()","error: no data on object allowed!");
			return(-1);
		}
		m=obj->n_mat;	 /*  saving the first number of materials */
		px=x; 				 /*  movable pointer for x, later */
		if (NULL!=(p_mat=realloc(obj->p_mat,sizeof(struct t_mat) * ( n + (obj->n_mat)))))
		{
			if (obj->dplist)
			{
				dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
				glDeleteLists(obj->dplist,1);
				obj->dplist=0;
			}
			obj->p_mat=p_mat;
			for (i=0;i<n;i++)
			{
				obj->p_mat[m+i].amb_r=*(px++);
				obj->p_mat[m+i].amb_g=*(px++);
				obj->p_mat[m+i].amb_b=*(px++);
				obj->p_mat[m+i].amb_a=*(px++);
				obj->p_mat[m+i].spec_r=*(px++);
				obj->p_mat[m+i].spec_g=*(px++);
				obj->p_mat[m+i].spec_b=*(px++);
				obj->p_mat[m+i].spec_a=*(px++);
				obj->p_mat[m+i].diff_r=*(px++);
				obj->p_mat[m+i].diff_g=*(px++);
				obj->p_mat[m+i].diff_b=*(px++);
				obj->p_mat[m+i].diff_a=*(px++);
				obj->p_mat[m+i].tex=-1;
			}
			obj->n_mat+=n;
		}
	} else 
	{
		return(-1);
	}
	return(0);
}

/*  its always the same ... this time we push some polys on the stack */
int obj_push_poly(struct t_process *p, uint32_t oid, uint32_t *x, uint32_t n)
{
	uint32_t i,m;
	struct t_poly *p_poly;
	struct t_obj *obj;
	uint32_t *px;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_push_poly()","error: no data on object allowed!");
			return(-1);
		}

		m=obj->n_poly;	 /*  saving the first number of polys */
		px=x; 				 /*  movable pointer for x, later */
		if (NULL!=(p_poly=realloc(obj->p_poly,sizeof(struct t_poly) * ( n + (obj->n_poly)))))
		{
			if (obj->dplist)
			{
				dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
				glDeleteLists(obj->dplist,1);
				obj->dplist=0;
			}
			obj->p_poly=p_poly;
			for (i=0;i<n;i++)
			{
				obj->p_poly[m+i].v[0]=*(px++);
				obj->p_poly[m+i].v[1]=*(px++);
				obj->p_poly[m+i].v[2]=*(px++);
				obj->p_poly[m+i].mat=*(px++);
				obj->p_poly[m+i].n[0].x=obj->p_poly[m+i].n[0].y=obj->p_poly[m+i].n[0].z=0;
				obj->p_poly[m+i].tc[0].x=obj->p_poly[m+i].n[0].y=obj->p_poly[m+i].n[0].z=0;
		 /* 		obj->p_poly[m+i].n=NULL;		/ *  no normals yet * / */
			}
			obj->n_poly+=n;
		}
	} else 
	{
		return(-1);
	}
	return(0);
}
/*  its always the same ... this time we push some lines on the stack */
int obj_push_line(struct t_process *p, uint32_t oid, uint32_t *x, uint32_t n)
{
	uint32_t i,m;
	struct t_line *p_line;
	struct t_obj *obj;
	uint32_t *px;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_push_line()","error: no data on object allowed!");
			return(-1);
		}

		m=obj->n_line;	 /*  saving the first number of lines */
		px=x; 				 /*  movable pointer for x, later */
		if (NULL!=(p_line=realloc(obj->p_line,sizeof(struct t_line) * ( n + (obj->n_line)))))
		{
			if (obj->dplist)
			{
				dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
				glDeleteLists(obj->dplist,1);
				obj->dplist=0;
			}
			obj->p_line=p_line;
			for (i=0;i<n;i++)
			{
				obj->p_line[m+i].v[0]=*(px++);
				obj->p_line[m+i].v[1]=*(px++);
				obj->p_line[m+i].mat=*(px++);
			}
			obj->n_line+=n;
		}
	} else 
	{
		return(-1);
	}
	return(0);
}
/* creates n new textures on the texture stack, of object oid, with (w,h)
 * given through *x */
int obj_push_tex(struct t_process *p, uint32_t oid, uint16_t *x, uint32_t n)
{
	uint32_t i,m;
	double d;
	struct t_tex *p_tex;
	struct t_obj *obj;
	uint16_t *px,hm;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_push_tex()","error: no data on object allowed!");
			return(-1);
		}
		m=obj->n_tex;	     /*  saving the first number of textures */
		px=x; 				 /*  movable pointer for x, later */
		if (NULL!=(p_tex=realloc(obj->p_tex,sizeof(struct t_tex) * ( n + (obj->n_tex)))))
		{
/*			if (obj->dplist)
			{
				dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
				glDeleteLists(obj->dplist,1);
				obj->dplist=0;
			}*/
			obj->p_tex=p_tex;
			for (i=0;i<n;i++)
			{
				obj->p_tex[m+i].gl_texnum=-1;
				obj->p_tex[m+i].tw=*(px++);
				obj->p_tex[m+i].th=*(px++);
				if ((obj->p_tex[m+i].tw<=TEXTURE_MAX_W) && (obj->p_tex[m+i].th<=TEXTURE_MAX_H))
				{
					d=log((double)obj->p_tex[m+i].tw)/log(2.0);
					hm=pow(2,floor(d));
					dprintf(MED,"hm %d, tw %d",hm,obj->p_tex[m+i].tw);
					if (hm!=obj->p_tex[m+i].tw) 	{
						obj->p_tex[m+i].w=hm*2;
						obj->p_tex[m+i].xs=(float)((double)obj->p_tex[m+i].tw)/((double)obj->p_tex[m+i].w);
					} else	{
						obj->p_tex[m+i].xs=1.0;
						obj->p_tex[m+i].w=obj->p_tex[m+i].tw;
					}
					d=log((double)obj->p_tex[m+i].th)/log(2.0);
					hm=pow(2,floor(d));
					dprintf(MED,"hm %d, th %d",hm,obj->p_tex[m+i].th);
					
					if (hm!=obj->p_tex[m+i].th) 	{
						obj->p_tex[m+i].h=hm*2;
						obj->p_tex[m+i].ys=(float)((double)obj->p_tex[m+i].th)/((double)obj->p_tex[m+i].h);
					} else 	{
						obj->p_tex[m+i].ys=1.0;
						obj->p_tex[m+i].h=obj->p_tex[m+i].th;
					}
					obj->p_tex[m+i].buf=malloc(obj->p_tex[m+i].h*obj->p_tex[m+i].w*4);
					memset(obj->p_tex[m+i].buf,0,obj->p_tex[m+i].h*obj->p_tex[m+i].w*4);
					errds(LOW,"obj_push_tex()","setting up %d %d (in mem: %d %d) texture",
									obj->p_tex[m+i].tw,
									obj->p_tex[m+i].th,
									obj->p_tex[m+i].w,
									obj->p_tex[m+i].h);
									
				} else	{
					errds(MED,"obj_push_tex()","bad size for texture %d (requested size: %dx%d, max %dx%d)",m+i,
									obj->p_tex[m+i].tw,obj->p_tex[m+i].th,TEXTURE_MAX_W,TEXTURE_MAX_H);
					obj->p_tex[m+i].buf=NULL;
				}
			}
			obj->n_tex+=n;
		}
	} else 
	{
		return(-1);
	}
	return(0);
}
/*  add some normal information to the polygon buffer */
int obj_pep_poly_normal(struct t_process *p, uint32_t oid, float *x, uint32_t n)
{
	uint32_t i,j,m;
	struct t_obj *obj;
	float *px;
	if (obj_valid(p,oid,obj))
	{
		m=obj->n_poly;
		if (m<n)	 /*  saving the first number of polys */
			n=m;  /*  when more polygons than available should be pepped,  */
				 /*  just pep the first m polygons */
		px=x; 				 /*  movable pointer for x, later */
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_pep_poly_normal()","error: no data on object allowed!");
			return(-1);
		}

		if (obj->dplist)
		{
			dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
			glDeleteLists(obj->dplist,1);
			obj->dplist=0;
		}
		dprintf(VLOW,"pepping poly's %d to %d",(m-n),m);
		for (i=(m-n);i<m;i++)
		{
			for (j=0;j<3;j++)
			{
				obj->p_poly[i].n[j].x=*(px++);
				obj->p_poly[i].n[j].y=*(px++);
				obj->p_poly[i].n[j].z=*(px++);
			}
		}
	} else 
	{
		return(-1);
	}
	return(0);
}

/*  add textures coordinates to each vertex of the polygon(s) */
int obj_pep_poly_texc(struct t_process *p, uint32_t oid, float *x, uint32_t n)
{
	uint32_t i,j,m;
	struct t_obj *obj;
	float *px;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_pep_poly_texc()","error: no data on object allowed!");
			return(-1);
		}

		m=obj->n_poly;
		if (m<n)	 /*  saving the first number of polys */
			n=m;  /*  when more polygons than available should be pepped,  */
				 /*  just pep the first m polygons */
		px=x; 				 /*  movable pointer for x, later */

		if (obj->dplist)
		{
			dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
			glDeleteLists(obj->dplist,1);
			obj->dplist=0;
		}
		dprintf(VLOW,"pepping poly's %d to %d",(m-n),m);
		for (i=(m-n);i<m;i++)
		{
			for (j=0;j<3;j++)
			{
				obj->p_poly[i].tc[j].x=*(px++);
				obj->p_poly[i].tc[j].y=*(px++);
			}
		}
	} else 
	{
		return(-1);
	}
	return(0);
}
/*  overwrite n latest materials with some other materials */
int obj_pep_mat(struct t_process *p, uint32_t oid, float *x, uint32_t n)
{
	uint32_t i,m;
	struct t_obj *obj;
	float *px;
	if (obj_valid(p,oid,obj))
	{
		m=obj->n_mat;	 /*  saving the first number of materials */
		if (m<n)	
			n=m;  /*  when more mats than available should be pepped,  */
				 /*  just pep the first m mats */
		px=x; 				 /*  movable pointer for x, later */
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_pep_mat()","error: no data on object allowed!");
			return(-1);
		}
		if (obj->dplist)
		{
			dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
			glDeleteLists(obj->dplist,1);
			obj->dplist=0;
		}
		dprintf(MED,"pepping mats %d to %d",(m-n),m);
		for (i=(m-n);i<m;i++)
		{
			obj->p_mat[i].amb_r=*(px++);
			obj->p_mat[i].amb_g=*(px++);
			obj->p_mat[i].amb_b=*(px++);
			obj->p_mat[i].amb_a=*(px++);
			obj->p_mat[i].spec_r=*(px++);
			obj->p_mat[i].spec_g=*(px++);
			obj->p_mat[i].spec_b=*(px++);
			obj->p_mat[i].spec_a=*(px++);
			obj->p_mat[i].diff_r=*(px++);
			obj->p_mat[i].diff_g=*(px++);
			obj->p_mat[i].diff_b=*(px++);
			obj->p_mat[i].diff_a=*(px++);
		}
	} else 
	{
		return(-1);
	}
	return(0);
}
/*  overwrite n latest lines with some other lines */
int obj_pep_line(struct t_process *p, uint32_t oid, uint32_t *x, uint32_t n)
{
	uint32_t i,m;
	struct t_obj *obj;
	uint32_t *px;
	if (obj_valid(p,oid,obj))
	{
		m=obj->n_line;	 /*  saving the first number of lines */
		if (m<n)	
			n=m;  /*  when more lines than available should be pepped,  */
				 /*  just pep the first m lines */
		px=x; 				 /*  movable pointer for x, later */
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_pep_line()","error: no data on object allowed!");
			return(-1);
		}
		if (obj->dplist)
		{
			dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
			glDeleteLists(obj->dplist,1);
			obj->dplist=0;
		}
		dprintf(VLOW,"pepping lines %d to %d",(m-n),m);
		for (i=(m-n);i<m;i++)
		{
			obj->p_line[i].v[0]=*(px++);
			obj->p_line[i].v[1]=*(px++);
			obj->p_line[i].mat=*(px++);
		}
	} else 
	{
		return(-1);
	}
	return(0);
}


/*  overwrite n latest vertices with some other vertices */
int obj_pep_vertex(struct t_process *p, uint32_t oid, float *x, uint32_t n)
{
	uint32_t i,m;
	float r;
	struct t_vertex *a;
	struct t_obj *obj;
	float *px;
	int is_clnsrc;
	if (obj_valid(p,oid,obj))
	{
		m=obj->n_vertex;	 /*  saving the first number of vertices */
		if (m<n)	 
			n=m;  /*  when more mats than available should be pepped,  */
				 /*  just pep the first m mats */
		px=x; 				 /*  movable pointer for x, later */
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_pep_vertices()","error: no data on object allowed!");
			return(-1);
		}
		if (obj->dplist)
		{
			dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
			glDeleteLists(obj->dplist,1);
			obj->dplist=0;
		}
		dprintf(MED,"pepping vertices %d to %d",(m-n),m-1);
		for (i=(m-n);i<m;i++)
		{
			obj->p_vertex[i].x=*(px++);
			obj->p_vertex[i].y=*(px++);
			obj->p_vertex[i].z=*(px++);
			a=&obj->p_vertex[i];
			r=obj->scale * sqrt(	
					(a->x * a->x ) + 
					(a->y * a->y ) +
					(a->z * a->z ));
			if (r> obj->r) obj->r=r;
		}
		if (p->id!=MCP)
		{
		/* this is doing live update which is quite okay, but we need
		 * to check for biggest update and clonesources ... */
			obj_check_biggest_object(p,oid);
		}
		if (p->object[oid]->oflags&OF_CLONE_SRC)
		{
			is_clnsrc=0;
			for (i=0;i<p->n_obj;i++)
			{
				if (p->object[i]!=NULL)
				{
					if ((p->object[i]->oflags&OF_CLONE) && (p->object[i]->n_vertex==oid))
					{ /* if it's pointing to our object ... */
						is_clnsrc=1;
						p->object[i]->r=obj->r*(p->object[i]->r/obj->scale); /* give it the new radius too! */
						obj_check_biggest_object(p,i);
					}
				}
			}
			if (!is_clnsrc)
				p->object[oid]->oflags&=~OF_CLONE_SRC;
		}
	} else 
	{
		return(-1);
	}
	return(0);
}
/*  assign textures to the last n materials */
int obj_pep_mat_tex(struct t_process *p, uint32_t oid, uint32_t *x, uint32_t n)
{
	uint32_t i,m;
	struct t_obj *obj;
	uint32_t *px;
	if (obj_valid(p,oid,obj))
	{
		m=obj->n_mat;	 /*  saving the first number of vertices */
		if (m<n)	 /*  saving the first number of polys */
			n=m;  /*  when more mats than available should be pepped,  */
				 /*  just pep the first m mats */
		px=x; 				 /*  movable pointer for x, later */
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_pep_mat_tex()","error: no data on object allowed!");
			return(-1);
		}
		if (obj->dplist)
		{
			dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
			glDeleteLists(obj->dplist,1);
			obj->dplist=0;
		}
		dprintf(MED,"pepping mats %d to %d",(m-n),m);
		for (i=(m-n);i<m;i++)
			obj->p_mat[i].tex=*(px++);
	} else 
	{
		return(-1);
	}
	return(0);
}
/*  add some normal information to the polygon buffer */
int obj_load_poly_normal(struct t_process *p, uint32_t oid, float *x, uint32_t start, uint32_t n)
{
	uint32_t i,j,m;
	struct t_obj *obj;
	float *px;
	if (obj_valid(p,oid,obj))
	{
		m=obj->n_poly;
		if (m<(start+n))	
			n=m-start; 
		px=x;
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_load_poly_normal()","error: no data on object allowed!");
			return(-1);
		}

		if (obj->dplist)
		{
			dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
			glDeleteLists(obj->dplist,1);
			obj->dplist=0;
		}
		for (i=start;i<(start+n);i++)
		{
			for (j=0;j<3;j++)
			{
				obj->p_poly[i].n[j].x=*(px++);
				obj->p_poly[i].n[j].y=*(px++);
				obj->p_poly[i].n[j].z=*(px++);
			}
		}
	} else 
		return(-1);
	return(0);
}
/*  add textures coordinates to each vertex of the polygon(s) */
int obj_load_poly_texc(struct t_process *p, uint32_t oid, float *x, uint32_t start, uint32_t n)
{
	uint32_t i,j,m;
	struct t_obj *obj;
	float *px;
	if (obj_valid(p,oid,obj))
	{
		m=obj->n_poly;
		if (m<(start+n))	
			n=m-start; 
		px=x; 				 /*  movable pointer for x, later */
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_load_poly_texc()","error: no data on object allowed!");
			return(-1);
		}

		if (obj->dplist)
		{
			dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
			glDeleteLists(obj->dplist,1);
			obj->dplist=0;
		}
		for (i=start;i<(start+n);i++)
		{
			for (j=0;j<3;j++)
			{
				obj->p_poly[i].tc[j].x=*(px++);
				obj->p_poly[i].tc[j].y=*(px++);
			}
		}
	} else 
		return(-1);
	return(0);
}


/*  load at position start n materials, overwriting old ones */
int obj_load_mat(struct t_process *p, uint32_t oid, float *x, uint32_t start, uint32_t n)
{
	uint32_t i,m;
	struct t_obj *obj;
	float *px;
	if (obj_valid(p,oid,obj))
	{
		m=obj->n_mat;	
		if (m<(start+n))	
			n=m-start; 
		px=x; 				 /*  movable pointer for x, later */
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_pep_mat()","error: no data on object allowed!");
			return(-1);
		}
		if (obj->dplist)
		{
			dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
			glDeleteLists(obj->dplist,1);
			obj->dplist=0;
		}
		dprintf(MED,"pepping %d mats, starting at %d",n,start);
		for (i=start;i<(start+n);i++)
		{
			obj->p_mat[i].amb_r=*(px++);
			obj->p_mat[i].amb_g=*(px++);
			obj->p_mat[i].amb_b=*(px++);
			obj->p_mat[i].amb_a=*(px++);
			obj->p_mat[i].spec_r=*(px++);
			obj->p_mat[i].spec_g=*(px++);
			obj->p_mat[i].spec_b=*(px++);
			obj->p_mat[i].spec_a=*(px++);
			obj->p_mat[i].diff_r=*(px++);
			obj->p_mat[i].diff_g=*(px++);
			obj->p_mat[i].diff_b=*(px++);
			obj->p_mat[i].diff_a=*(px++);
		}
	} else 
		return(-1);
	return(0);
}
/* the interal texture updating function ... this is for opengl*/
static void obj_update_tex(struct t_tex *tex,unsigned short x,unsigned short y,unsigned short w,unsigned short h,unsigned char *pixbuf)
{
	GLuint t;
	if ((t=tex->gl_texnum)!=-1)
	{
/* dprintf(MED,"updating texture %d at [%d %d] with a [%d %d] pixbuf",t,x,y,w,h); */
/* 		glTexSubImage2D(t,0,x,y,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixbuf); */

		glDeleteTextures(1,&t);
		tex->gl_texnum=-1;
	}
}
/*  loads some data into the pixbuf */
int obj_load_tex		(struct t_process *p, uint32_t oid, uint32_t tex, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *pixbuf)
{
	struct t_obj *obj;
	struct t_tex *t;
	uint32_t i,p1,p2,m;
	int16_t mw;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_load_tex()","error: no data on object allowed!");
			return(-1);
		}
		if (tex<obj->n_tex)
		{
			t=&obj->p_tex[tex];
			if (t->buf!=NULL)
			{
				if (obj->dplist)
				{
					dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
					glDeleteLists(obj->dplist,1);
					obj->dplist=0;
				}

				m=(t->w-1)*t->th+t->tw; 			 /*  maximum: position of the last pixel in the buffer */
				if ((x+w)>t->tw) mw=(t->tw-x);
					else mw=w;
				if (mw<=0)	 /*  nothing to do */
					return(-1);
				for (i=0;i<h;i++)
				{
					p1=(y+i)*t->w+x;  /*  scanline start position */
					p2=mw;			 /*  and length */
					if (p1>m)
						return(0);   /*  need to break here. */
					if ((p1+w)>m)
						p2=m-p1;	 /*  only draw a part of the scanline */
					memcpy(	t->buf+	4*p1,			 /*  draw at p1 position ... */
							pixbuf+	4*i*w,			 /*  scanline number i ... */
									4*p2);
				}
				obj_update_tex(t,x,y,w,h,pixbuf);
				return(0);
			} else {
				errds(HIGH,"obj_load_tex()","no buffer to draw to in oid %d, texture %d",oid,tex);
			}
		}
	} 
	return(-1);
}
int obj_toggle_flags(struct t_process *p, uint32_t oid, uint8_t type, uint32_t flags)
{
	struct t_obj *obj;
	uint32_t f;

	f=flags&OF_MASK;
	if (obj_valid(p,oid,obj))
	{
		switch (type)
		{
			case OF_TURN_ON:	obj->oflags|=f;		break;
			case OF_TURN_OFF:	obj->oflags&=~f;	break;
			case OF_TURN_SWAP:	obj->oflags^=f;		break;
			default:return(-1);
		}
/* 		dprintf(VLOW,"toggled %d->oflags=%010x with %010x [%d]",oid,obj->oflags,flags,type); */
	} 
	return(0);
}
/*  deletes the last n vertices of the stack. if n>=n_vertex, delete all vertices */
int obj_del_vertex(struct t_process *p, uint32_t oid, uint32_t n)
{
	uint32_t m;
	struct t_vertex *p_vertex;
	struct t_obj *obj;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_del_vertex()","error: can't delete vertices in this object!");
			return(-1);
		}

		dprintf(VLOW,"deleting %d vertices of pid %d/ oid %d",n,p->id,oid);
		m=obj->n_vertex;	 /*  saving the first number of vertices */
		if (n>=m) 
		{
			if (m>0)
				free(obj->p_vertex);
			obj->n_vertex=0;
			obj->p_vertex=NULL;
		}
		else if (n>0)
		{
			if (NULL!=(p_vertex=realloc(obj->p_vertex,sizeof(struct t_vertex) * ( m - n))))
			{
				if (obj->dplist)
				{
					dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
					glDeleteLists(obj->dplist,1);
					obj->dplist=0;
				}
				obj->p_vertex=p_vertex;
				obj->n_vertex-=n;
			}
		}
		obj_size_update(p,oid);
	} else 
	{
		return(-1);
	}
	return(0);
}
/*  deletes the last n materials of the stack. if n>=n_mat, delete all materials */
int obj_del_mat(struct t_process *p, uint32_t oid, uint32_t n)
{
	uint32_t m;
	struct t_mat *p_mat;
	struct t_obj *obj;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_del_mat()","error: can't delete materials in this object!");
			return(-1);
		}

		dprintf(VLOW,"deleting %d materials of pid %d/ oid %d",n,p->id,oid);
		m=obj->n_mat;	 /*  saving the first number of materials */
		if (n>=m) 
		{
			if (m>0)
				free(obj->p_mat);
			obj->n_mat=0;
			obj->p_mat=NULL;
		}
		else if (n>0)
		if (NULL!=(p_mat=realloc(obj->p_mat,sizeof(struct t_mat) * ( m - n))))
		{
			if (obj->dplist)
			{
				dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
				glDeleteLists(obj->dplist,1);
				obj->dplist=0;
			}
			obj->p_mat=p_mat;
			obj->n_mat-=n;
		}
	} else 
		return(-1);
	return(0);
}
/*  deletes the last n polys of the stack. if n>=n_poly, delete all polys */
int obj_del_poly(struct t_process *p, uint32_t oid, uint32_t n)
{
	uint32_t m;
	struct t_poly *p_poly;
	struct t_obj *obj;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_del_poly()","error: can't delete poly in this object!");
			return(-1);
		}

		dprintf(VLOW,"deleting %d polys of pid %d/ oid %d",n,p->id,oid);
		m=obj->n_poly;	 /*  saving the first number of poly  */
		if (n>=m) 
		{
			if (m>0)
				free(obj->p_poly);
			obj->n_poly=0;
			obj->p_poly=NULL;
		}
		else if (n>0)
		if (NULL!=(p_poly=realloc(obj->p_poly,sizeof(struct t_poly) * ( m - n))))
		{
			if (obj->dplist)
			{
				dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
				glDeleteLists(obj->dplist,1);
				obj->dplist=0;
			}
			obj->p_poly=p_poly;
			obj->n_poly-=n;
		}
	} else 
		return(-1);
	return(0);
}
/*  deletes the last n lines of the stack. if n>=n_line, delete all lines */
int obj_del_line(struct t_process *p, uint32_t oid, uint32_t n)
{
	uint32_t m;
	struct t_line *p_line;
	struct t_obj *obj;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_del_line()","error: can't delete line in this object!");
			return(-1);
		}

		dprintf(VLOW,"deleting %d lines of pid %d/ oid %d",n,p->id,oid);
		m=obj->n_line;	 /*  saving the first number of line  */
		if (n>=m) 
		{
			if (m>0)
				free(obj->p_line);
			obj->n_line=0;
			obj->p_line=NULL;
		}
		else if (n>0)
		if (NULL!=(p_line=realloc(obj->p_line,sizeof(struct t_line) * ( m - n))))
		{
			if (obj->dplist)
			{
				dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
				glDeleteLists(obj->dplist,1);
				obj->dplist=0;
			}
			obj->p_line=p_line;
			obj->n_line-=n;
		}
	} else 
		return(-1);
	return(0);
}
/*  delete texture object */
int obj_del_tex(struct t_process *p, uint32_t oid, uint32_t n)
{
	uint32_t m;
	uint32_t i;
	struct t_tex *p_tex;
	struct t_obj *obj;
	GLuint t;
	if (obj_valid(p,oid,obj))
	{
		if (obj->oflags&OF_NODATA)
		{
			errds(MED,"obj_del_tex()","error: can't delete textures in this object!");
			return(-1);
		}

		dprintf(VLOW,"deleting %d textures of pid %d/ oid %d",n,p->id,oid);
		m=obj->n_tex;	 /*  saving the first number of textures  */
		if (n>=m) 
		{
			for (i=0;i<m;i++)
			{
				if ((obj->p_tex[i].buf)!=NULL)
					free(obj->p_tex[i].buf);
				if (obj->p_tex[i].gl_texnum)
				{
					t=obj->p_tex[i].gl_texnum;
					glDeleteTextures(1,&t);
				}
			}
			if (m>0)
				free(obj->p_tex);
			obj->n_tex=0;
			obj->p_tex=NULL;
		} else if (n>0)
		{
			for (i=(m-n);i<m;i++)
			{
				if (obj->p_tex[i].buf!=NULL)
					free(obj->p_tex[i].buf);
				if (obj->p_tex[i].gl_texnum)
				{
					t=obj->p_tex[i].gl_texnum;
					glDeleteTextures(1,&t);
				}

			}
			if (NULL!=(p_tex=realloc(obj->p_tex,sizeof(struct t_tex) * ( m - n))))
			{
				if (obj->dplist)
				{
					dprintf(VLOW,"freeing display list %d to get new data",obj->dplist);
						glDeleteLists(obj->dplist,1);
					obj->dplist=0;
				}
				obj->p_tex=p_tex;
				obj->n_tex=n;
			}
		}
	} else 
		return(-1);
	return(0);
}

/*  from proto.c, translates the object. */
int obj_translate(struct t_process *p, uint32_t oid, float *transv)
{
	struct t_obj *obj;
	if (obj_valid(p,oid,obj))
	{
		if ((p->id==MCP) || (!(obj->oflags&OF_SYSTEM)))
		{
			obj->translate.x=*transv;
			obj->translate.y=*(transv+1);
			obj->translate.z=*(transv+2);
			dprintf(VLOW,"[translate|pid %d] %d: %3.3f %3.3f %3.3f",p->id,oid,obj->translate.x,obj->translate.y,obj->translate.z);
			obj_pos_update(p,oid);
		}
	}
	return(0);
}
/*  set rotate vector .... */
int obj_rotate(struct t_process *p, uint32_t oid, float *rotv)
{
	struct t_obj *obj;
	float f;
	if (obj_valid(p,oid,obj))
	{
		if ((p->id==MCP) || (!(obj->oflags&OF_SYSTEM)))
		{
			f=*rotv;
			if (f<0.0)		f+=(float)((int)-f/360)*360;
			if (f>360.0)	f+=(float)((int)f/360)*-360;
			obj->rotate.x=f;
			f=*(rotv+1);
			if (f<0.0)		f+=(float)((int)-f/360)*360;
			if (f>360.0)	f+=(float)((int)f/360)*-360;
			obj->rotate.y=f;
			f=*(rotv+2);
			if (f<0.0)		f+=(float)((int)-f/360)*360;
			if (f>360.0)	f+=(float)((int)f/360)*-360;
			obj->rotate.z=f;
			dprintf(VLOW,"[rotate|pid %d] %d: %3.3f %3.3f %3.3f",p->id,oid,obj->rotate.x,obj->rotate.y,obj->rotate.z);
			obj_pos_update(p,oid);
		}
	}
	return(0);
}
/*  and scaling ! */
int obj_scale(struct t_process *p, uint32_t oid, float scav)
{
	struct t_obj *obj;
	if (obj_valid(p,oid,obj))
	{
		if ((p->id==MCP) || (!(obj->oflags&OF_SYSTEM)))
		if (!isinf(scav) && !isnan(scav) && !((scav<1.0e-10) && (scav>-1.0e-10))) /* ignore very low values */
		{
			dprintf(VLOW,"[scale|pid %d] obj %d to %f",p->id,oid,scav);
			obj->scale=scav;
	/*		obj->scale.x=*scav;
			obj->scale.y=*(scav+1);
			obj->scale.z=*(scav+2);*/
			obj_size_update(p,oid);
			obj_pos_update(p,oid);
		}
	}
	return(0);
}
/*  a recursive function to move/scale the object before rendering. */
void into_position(struct t_process *p, struct t_obj *obj, int depth)
{
	struct t_obj *on;
	if ((obj->oflags&OF_LINK) && (depth<p->n_obj))
	{
		/* TODO: only MultMatrix if m_uptodate ?! */
		if (obj_valid(p,obj->linkid,on))
		{
			into_position(p,on,depth+1);
		} else {
			obj->oflags&=~OF_LINK;
			dprintf(LOW,"link object is broken, removing link");
		}
	}
	 /* if (depth>=MAXLOOP) */
	if (depth>=p->n_obj)
		dprintf(MED,"too much looping ...");
	glTranslatef(obj->translate.x,obj->translate.y,obj->translate.z);
	glRotatef(obj->rotate.y,0.0,1.0,0.0);
	glRotatef(obj->rotate.x,1.0,0.0,0.0);
	glRotatef(obj->rotate.z,0.0,0.0,1.0);
/*	glScalef(obj->scale.x,obj->scale.y,obj->scale.z);*/
	glScalef(obj->scale,obj->scale,obj->scale);
}

void obj_size_update(struct t_process *p, uint32_t oid)
{
	struct t_obj *o,*o2;
	struct t_vertex *a,*vp;
	float r;
	int vn,is_clnsrc;
	uint32_t i;
	if (p->id==MCP) return; /*  mcp does not need that. */
	if (obj_valid(p,oid,o))
	{
		if (o->oflags&OF_SYSTEM)
		{
			o->r=o->or=0; /* we don't care about system objects */
			return; 
		}
		vp=o->p_vertex;
		vn=o->n_vertex;
		if (o->oflags&OF_CLONE)
		{  
			o2=p->object[o->n_vertex];	 /*  get the target into o2*/
			o->r= o2->r * (o->scale/o2->scale);
			obj_check_biggest_object(p,oid);
			return;
		} else {
/* 			printf(MED,"looking through vertices..."); */
			for (i=0;i<vn;i++)
			{
				a=&(vp[i]);
				r=o->scale * sqrt(
						(a->x  * a->x ) + 
						(a->y  * a->y ) +
						(a->z  * a->z ));
				if (r > o->r) o->r=r;
			}
			obj_check_biggest_object(p,oid);
			if (p->object[oid]->oflags&OF_CLONE_SRC)
			{
				is_clnsrc=0;
				for (i=0;i<p->n_obj;i++)
				{
					if (p->object[i]!=NULL)
					{
						if ((p->object[i]->oflags&OF_CLONE) && (p->object[i]->n_vertex==oid))
						{ /* if it's pointing to our object ... */
							is_clnsrc=1;
							p->object[i]->r=o->r*(p->object[i]->r/o->scale); /* give it the new radius too! */
							obj_check_biggest_object(p,i);
						}
					}
				}
				if (!is_clnsrc)
					p->object[oid]->oflags&=~OF_CLONE_SRC;
			}
		}
	}
}
/*  checks if the object is (still) the biggest object. assumes that oid */
/*  is valid */
void obj_check_biggest_object(struct t_process *p, uint32_t oid)
{
	struct t_obj *o,*mcp_o;
	struct t_process *mcp_p;
	float r,r2;
	uint32_t i;
	int found;
	mcp_p=get_proc_by_pid(MCP);
	mcp_o=mcp_p->object[p->mcp_oid];
	o=p->object[oid];
	if (o->oflags&OF_SYSTEM)
		return; /* we don't care, system objects don't count. */
	r=o->r+o->or;
	if (r>mcp_o->r)
	{	 /*  this is now the biggest object. */
		mcp_o->r=r;
		p->biggest_obj=oid;
/*		dprintf(MED,"there is a new biggest object in [%d:\"\"]",p->id,p->name);*/
		mcp_rep_object(p->mcp_oid);	  /*  and tell the mcp */
	} else {
		if (p->biggest_obj==oid)
		{  /*  oid might now lose the status of the "biggest object". let's check: */
			found=0;
			for (i=0;i<p->n_obj;i++)
				if (p->object[i]!=NULL)
				{
					if ((r2=p->object[i]->r+p->object[i]->or)>r)
					{  /*  this object is bigger than the old biggest one. */
						if (!(p->object[i]->oflags&OF_SYSTEM))
						{
							p->biggest_obj=oid;
							r=r2;
							found=1;
						}
					}
				}
			if (found)
			{
				dprintf(VLOW,"there is a new biggest object in [%d:\"\"]",p->id,p->name);
				mcp_o->r=r;  /*  save the new size */
				mcp_rep_object(p->mcp_oid);	  /*  and tell the mcp */
			}
		}   /*  if it wasn't the biggest object, no one cares if it's smaller than process */
			 /*  radius */
	}
}
/* calculates and saves the transformation matrix, if needed */
void obj_recalc_tmat(struct t_process *p, uint32_t oid)
{
	GLint matrixmode;
	if (!p->object[oid]->m_uptodate)
	{
		glGetIntegerv(GL_MATRIX_MODE,&matrixmode); 		 /*  save matrixmode */
		glMatrixMode(GL_MODELVIEW); 					 /*  go into modelview */
		glPushMatrix();
		glLoadIdentity();
		into_position(p,p->object[oid],0);	
		glGetFloatv( GL_MODELVIEW_MATRIX, p->object[oid]->m );
		glPopMatrix();
		glMatrixMode(matrixmode);	
		p->object[oid]->m_uptodate=1;
	}
}
void obj_sys_update(struct t_process *p, uint32_t oid)
{
	struct t_process *mcp_p=get_proc_by_pid(MCP);
	struct t_obj	 *o;
	struct t_vertex	 fs,fa;
	float 			 ss,sa,v[3];
	fs.x=fs.y=fs.z=0.0F;
	fa.x=fa.y=fa.z=0.0F;
	sa=ss=1.0F;
	/* find the angel of the sys object */
	o=mcp_p->object[oid];
	while (o!=NULL)
	{
		fs.x+=o->rotate.x;
		fs.y+=o->rotate.y;
		fs.z+=o->rotate.z;
		ss*=o->scale;
		if (o->oflags&OF_LINK)
			o=mcp_p->object[o->linkid];
		else o=NULL;
	}
	mySetMatrix(mcp_p->object[oid]->m);
	v[0]=v[1]=v[2]=0.0;
	myTransform3f(v);

	o=mcp_p->object[p->mcp_oid];
	while (o!=NULL)
	{
		fa.x+=o->rotate.x;
		fa.y+=o->rotate.y;
		fa.z+=o->rotate.z;
		sa*=o->scale;
		if (o->oflags&OF_LINK)
			o=mcp_p->object[o->linkid];
		else o=NULL;
	}

	mySetMatrix(mcp_p->object[p->mcp_oid]->m);
	myInvert();
	myTransform3f(v);

	p->object[oid]->rotate.x=fs.x-fa.x;
	p->object[oid]->rotate.y=fs.y-fa.y;
	p->object[oid]->rotate.z=fs.z-fa.z;

	p->object[oid]->translate.x=v[0];
	p->object[oid]->translate.y=v[1];
	p->object[oid]->translate.z=v[2];

	p->object[oid]->scale=ss/sa;

/*	obj_debug(p,oid);*/
	obj_pos_update(p,oid); /* now also update the matrix and the objects linking to our sys-object ... */
}
/*  recalculate the position of an object. this assumes that oid is valid. */
void obj_pos_update(struct t_process *p, uint32_t oid)
{
	float v[3];
	uint32_t i;
	int is_lnksrc;
	struct t_obj 		*ao,*o;
	struct t_process	*ap;
	o=p->object[oid];
	dprintf(VLOW,"[obj_pos_upd|pid %d] %d",p->id, oid);
	o->m_uptodate=0;
	obj_recalc_tmat(p,oid);
	if (p->id!=MCP) 
	{/*  mcp does not need that. */
		 /*  save the matrixmode to reset it later on */
		v[0]=v[1]=v[2]=0.0F;
		mySetMatrix(o->m);
		myTransform3f(v);
			 /*  and get it's destination point. phew */
		o->or=sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	} else 
		if (o->oflags&OF_SYSTEM) /* TODO: what will we do if $sys_object is linked to another? */
		{ /* a system object changed position? let's update the focus'ed sys-objects */
			if (obj_valid(p,focus_oid,ao))
				if (NULL!=(ap=get_proc_by_pid(ao->n_mat)))
				{
					if (OF_POINTER==(o->oflags&0xF0000000))
					{ /* we dont have to do that much in this case ... */
						if (obj_valid(ap,get_pointer(ap),ao)) /* we can redefine ao here -> ao = focused app's pointer*/
						{
							ao->rotate.x=o->rotate.x;
							ao->rotate.y=o->rotate.y;
							ao->rotate.z=o->rotate.z;
							ao->translate.x=o->translate.x;
							ao->translate.y=o->translate.y;
							ao->translate.z=o->translate.z; /* just copy */
						}
						
					} else 
						obj_sys_update(ap,oid);	
				}
			switch (o->oflags&0xF0000000)
			{
				case OF_CAM:
					event_cam_changed();
					dprintf(LOW,"[obj_pos_upd|pid %d] %d event_cam_changed",p->id,oid);
					break;
				case OF_POINTER:
					event_ptr_changed();
					dprintf(LOW,"[obj_pos_upd|pid %d] %d event_ptr_changed",p->id,oid);
					break;
				default:
					dprintf(LOW,"[obj_pos_upd|pid %d] %d unknown systen event",p->id,oid);
					
			}

		}
	if (o->oflags&OF_LINK_SRC)
	{
		is_lnksrc=0;
		for (i=0;i<p->n_obj;i++)  /*  update objects which reference on us. (recursive) */
			if (p->object[i]!=NULL)
				if ((p->object[i]->oflags&OF_LINK) && (p->object[i]->linkid==oid))
				{
					is_lnksrc=1;
					dprintf(VLOW,"[obj_pos_upd|pid %d] % is pointing on %d -> update",p->id,i, oid);
					obj_pos_update(p,i);
				}
		if (!is_lnksrc)	/* it's not! switch out the flag */
		{
			o->oflags&=~OF_LINK_SRC;
			dprintf(VLOW,"obj_pos_update(): %d in process %d is no longer a link-source",oid,p->id);
		}
	}
	if (p->id!=MCP)
		obj_check_biggest_object(p,oid);
}

/*  calculates the normal for one polygon, if not present. */
int calc_normal(struct t_obj *obj, uint32_t pn)
{
	struct t_vertex a,b,n;
	struct t_vertex *v[3];
	uint32_t vp,i;
	
	float len;
	for (i=0;i<3;i++)  /*  set and check */
	{
		vp= obj->p_poly[pn].v[i];  /*  ... get the vertices ... */
		if (vp<obj->n_vertex)
			v[i]=&(obj->p_vertex[vp]);
		else return(-1);
	}
	 /*  check for already set normal */
	if ((obj->p_poly[pn].n[0].x+obj->p_poly[pn].n[0].x+obj->p_poly[pn].n[0].x)==0)  /*  normal already defined? */
	{
		a.x=v[1]->x-v[0]->x;
		a.y=v[1]->y-v[0]->y;
		a.z=v[1]->z-v[0]->z;
		b.x=v[2]->x-v[0]->x;
		b.y=v[2]->y-v[0]->y;
		b.z=v[2]->z-v[0]->z;
		n.x=a.y*b.z - a.z*b.y;
		n.y=a.z*b.x - a.x*b.z;
		n.z=a.x*b.y - a.y*b.x;

		len=sqrt(n.x*n.x+n.y*n.y+n.z*n.z);
		if (len==0.0F)
		{
		 /* 		errds(VLOW,"bad polygon (can't normalize ...)"); */
		} else {
			n.x=n.x/len;
			n.y=n.y/len;
			n.z=n.z/len;
			for (i=0;i<3;i++) 
			{
				obj->p_poly[pn].n[i].x=n.x;
				obj->p_poly[pn].n[i].y=n.y;
				obj->p_poly[pn].n[i].z=n.z;
			}
		}
	}
	return(0);
}
static struct t_tex *get_texture(struct t_obj *obj,struct t_mat *m)
{
	GLuint t;
	struct t_tex *tex=NULL;
	GLfloat matgl[4];
/* 	int i,j; */
	if (m->tex<obj->n_tex)
	{
		tex=&obj->p_tex[m->tex];
		if (tex->buf!=NULL)
		{  /*  texture seems to be okay, select it. */
			matgl[0]=0.5f;
			matgl[1]=0.5f;
			matgl[2]=0.5f;
			matgl[3]=1.0f;
			glMaterialfv(GL_FRONT,GL_AMBIENT,matgl);
			glMaterialfv(GL_FRONT,GL_DIFFUSE,matgl);
			glMaterialfv(GL_FRONT,GL_SPECULAR,matgl);
			if (tex->gl_texnum!=-1)
			{
				glBindTexture( GL_TEXTURE_2D,tex->gl_texnum);
			} else {
				glGenTextures(1,&t);
				glBindTexture( GL_TEXTURE_2D, t);
				tex->gl_texnum=t;
				dprintf(HIGH,"generated texture %d [%dx%d, in memory %dx%d]",t,tex->tw,tex->th,tex->w,tex->h);
/*				for (j=0;j<tex->th;j++)
				for (i=0;i<tex->tw;i++)
				{
					dprintf(MED,"pixel[%d,%d], %d %d %d %d",i,j,
									tex->buf[(j*tex->w+i)*4+0],
									tex->buf[(j*tex->w+i)*4+1],
									tex->buf[(j*tex->w+i)*4+2],
									tex->buf[(j*tex->w+i)*4+3]);
				}*/
				glTexImage2D(	GL_TEXTURE_2D,0, GL_RGBA,	
								tex->w,tex->h,0,  /*  no border. */
								GL_RGBA,GL_UNSIGNED_BYTE,tex->buf);
								 /*  texture has to be generated yet ... */
			   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
			                   GL_NEAREST);
			   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
			                   GL_NEAREST);
			   
				}
		}
		else {  /* . can't use a texture  */
			tex=NULL;
		}
	}
	return(tex);
}
/*  finally, the rendering portion. */
int obj_render(struct t_process *p,uint32_t oid)
{
	uint32_t pn;
	uint32_t mat,omat=-1;
	uint32_t v;
/* 	int link_obj; */
	struct t_vertex *on;
	struct t_obj *obj;
	struct t_mat *m;
	struct t_tex *tex=NULL;
	GLfloat matgl[4];
	uint32_t i;

	obj=p->object[oid];
	glPushMatrix();
	glMultMatrixf(obj->m);
/*	into_position(p,obj,0);*/
	if (obj->oflags&OF_SYSTEM)
		return(-1); /* can't render system objects */
	if (obj->oflags&OF_CLONE)
		obj=p->object[obj->n_vertex];
	if (obj->dplist)
	{
		glCallList(obj->dplist);
	} else {
		obj->dplist=glGenLists(1);
		if (obj->dplist)	glNewList(obj->dplist,GL_COMPILE_AND_EXECUTE);
		 else 				dprintf(LOW,"couldn't get a new list :/");
		omat=-1;
		for (pn=0; pn<obj->n_poly; pn++)  /*  cycle throu our polygons ... */
		{
			if (calc_normal(obj,pn))
			{
				dprintf(HIGH,"something is wrong with polygon %d!",pn);
				if (obj->dplist) glEndList();
				glPopMatrix();
				return(-1);
			}
	 /* 		glNormal3f(-n.x,-n.y,-n.z); */
			mat= obj->p_poly[pn].mat;
			if (mat!=omat) {
				tex=NULL;
				if (mat< obj->n_mat) {
					m=&obj->p_mat[mat];
					if (m->tex!=-1)
						tex=get_texture(obj,m);
					if (tex==NULL)
					{
/*						dprintf(VLOW,"no texture, using standard material...");*/
						glBindTexture( GL_TEXTURE_2D,0);
						matgl[0]=m->amb_r/2;		
						matgl[1]=m->amb_g/2;		
						matgl[2]=m->amb_b/2;
						matgl[3]=m->amb_a;
		 /* 				glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,matgl); */
						glMaterialfv(GL_FRONT,GL_AMBIENT,matgl);
						matgl[0]=m->diff_r/2;
						matgl[1]=m->diff_g/2;
						matgl[2]=m->diff_b/2;
						matgl[3]=m->diff_a;
		 /* 				glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,matgl); */
						glMaterialfv(GL_FRONT,GL_DIFFUSE,matgl);
						matgl[0]=m->spec_r/2;
						matgl[1]=m->spec_g/2;
						matgl[2]=m->spec_b/2;
						matgl[3]=m->spec_a;
		 /* 				glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,matgl); */
						glMaterialfv(GL_FRONT,GL_SPECULAR,matgl);
					}
				} else {
					dprintf(MED,"something is wrong with polygon %d! material: [%d,%d]",pn, mat,obj->n_mat);
					if (obj->dplist) glEndList();
					glEnd();
					glPopMatrix();
					return(-1);
				}
			}
			omat=mat;		 /*  saving old material */
			glBegin(GL_TRIANGLES);
			for (i=0; i<3; i++)
				{
					on=&(obj->p_poly[pn].n[i]);
					glNormal3f(-on->x,-on->y,-on->z);
					if (tex!=NULL)
					{
/*						dprintf(MED,"using texture coordinate (%f,%f) for polygon %d point %d",
										obj->p_poly[pn].tc[i].x *tex->xs,
										obj->p_poly[pn].tc[i].y *tex->ys,
										pn,i);*/

						glTexCoord2f(	obj->p_poly[pn].tc[i].x *tex->xs,
										(obj->p_poly[pn].tc[i].y *tex->ys));
					}
					v= obj->p_poly[pn].v[i];  /*  ... get the vertices ... */
					glVertex3f(obj->p_vertex[v].x, obj->p_vertex[v].y, obj->p_vertex[v].z);  /*  ...and draw them */
				}
			glEnd();
		}
		if (tex!=NULL)
			glBindTexture( GL_TEXTURE_2D, 0);  /*  switch back to standard texture */

		for (pn=0;pn<obj->n_line; pn++)
		{
			mat= obj->p_line[pn].mat;
			if (mat!=omat) {
				tex=NULL;
				if (mat< obj->n_mat) {
					m=&obj->p_mat[mat];
					/* dont need to care about textures ...  it's rather impossible
					 * to get some textures on a line. at least it would look ugly ;)*/
					matgl[0]=m->amb_r/2;		
					matgl[1]=m->amb_g/2;		
					matgl[2]=m->amb_b/2;
					matgl[3]=m->amb_a;
		 /* 			glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,matgl); */
					glMaterialfv(GL_FRONT,GL_AMBIENT,matgl);
					matgl[0]=m->diff_r/2;
					matgl[1]=m->diff_g/2;
					matgl[2]=m->diff_b/2;
					matgl[3]=m->diff_a;
		 /* 			glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,matgl); */
					glMaterialfv(GL_FRONT,GL_DIFFUSE,matgl);
					matgl[0]=m->spec_r/2;
					matgl[1]=m->spec_g/2;
					matgl[2]=m->spec_b/2;
					matgl[3]=m->spec_a;
		 /* 			glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,matgl); */
					glMaterialfv(GL_FRONT,GL_SPECULAR,matgl);
				} else {
					dprintf(MED,"something is wrong with line %d! material: [%d,%d]",pn, mat,obj->n_mat);
					if (obj->dplist) glEndList();
					glEnd();
					glPopMatrix();
					return(-1);
				}
			}
			omat=mat;		 /*  saving old material */
			glBegin(GL_LINES);
			for (i=0; i<2; i++)
				{
					v= obj->p_line[pn].v[i];  /*  ... get the vertices ... */
					glVertex3f(obj->p_vertex[v].x, obj->p_vertex[v].y, obj->p_vertex[v].z);  /*  ...and draw them */
				}
			glEnd();
		}
		if (obj->dplist) glEndList();
	}
	glPopMatrix();
	return(0);
}
/*  creates a link from object from an object to another  */
/*  to have a translation or anything move with other things */
int obj_link(struct t_process *p, uint32_t oid_from, uint32_t oid_to)
{
	struct t_obj *o,*o2;
	if (obj_valid(p,oid_from,o) && obj_valid(p,oid_to,o2))
	{
		if (oid_to==oid_from)
		{
			errds(VHIGH,"obj_link()","can't link to itself!!",oid_from,oid_to);
			return(-1);
		}
		while (o2->oflags&OF_LINK)
		{
			if (o2->linkid==oid_from)  /*  circular link!! we can't do that */
			{
				errds(VHIGH,"obj_link()","link from %d to %d would produce a circular link!",oid_from,oid_to);
				return(-1);
			}
			o2=p->object[o2->linkid];  /*   move to the next object in the linkchain */
		}
		if ((o->oflags&OF_SYSTEM) && (p->id==MCP))
		{
			errds(VHIGH,"obj_link()","can't link system-objects in non-mcp-apps!",oid_from,oid_to);
			return(-1);
		}
 		dprintf(LOW,"[link|pid %d] %d -> %d",p->id, oid_from,oid_to); 
		o->oflags|=OF_LINK;
		o->linkid=oid_to;
		p->object[oid_to]->oflags|=OF_LINK_SRC;
		obj_pos_update(p,oid_from);
		return(0);
	}
	return(-1);
}
/*  this unlinks an object ... */
int obj_unlink(struct t_process *p, uint32_t oid)
{
	struct t_obj *o;
	if (obj_valid(p,oid,o))
	{
		o->oflags&=~OF_LINK;
		dprintf(VLOW,"removing link of object %d from pid %d",oid,p->id);
		obj_pos_update(p,oid);
		return(0);
	}
	return(-1);
}
/*  creates a new object, returning the object id */
int obj_new(struct t_process *p)
{
	struct t_obj *obj;
	uint32_t pos,reuse=0;
	obj=malloc(sizeof(struct t_obj));  /*  get an object and define it with our data */
	memset(obj,0,sizeof(struct t_obj));
	obj->linkid=-1;
	obj->rotate.x=obj->rotate.y=obj->rotate.z=0.0F;
	obj->translate.x=obj->translate.y=obj->translate.z=0.0F;
	obj->scale=1.0F;
	obj->n_vertex=obj->n_poly=obj->n_mat=obj->n_tex=0;
	obj->r=obj->or=0.0F;
	obj->m_uptodate=0;
	memcpy(obj->m,Identity,sizeof(t_mtrx));
	 /*  fresh and clean ... */
	if (p!=NULL)
	{
		 /*  look for an old object for reuse ... */
		for (pos=0;pos<p->n_obj;pos++)
		{
			if (p->object[pos]==NULL)
			{
				reuse=1;
				break;
/* 				dprintf(HIGH,"reusing position %d",pos); */
			}
		}
		if (!reuse)
		{
			if (p->n_obj>0)
				p->object=realloc(p->object,sizeof(struct t_obj *)*(p->n_obj+1));
			else p->object=malloc(sizeof(struct t_obj *)*(p->n_obj+1));
			pos=p->n_obj; 				 /*  add object at the end */
			p->n_obj++;					 /*  increment counter */
		}
		p->object[pos]=obj;						
		dprintf(VLOW,"pid %d added new object %d at %010p [pos %d]",p->id,pos,obj,pos);
		return (pos);
	} else {
		dprintf(HIGH,"obj_new(): no such process %d",p->id);	
		return(-1);
	}
}

/*  this changes the clone-target or sets up a new clone link. */
/*  this will check and supress looplinks and clonechains */
int obj_clone_change(struct t_process *p, uint32_t oid, uint32_t toid)
{
	struct t_obj *o,*no;
	int already_clone,is_clnsrc;
	uint32_t i;
	if (obj_valid(p,oid,o) && obj_valid(p,toid,no))
	{
		if ((o->oflags&OF_SYSTEM) || (no->oflags&OF_SYSTEM))
		{
			dprintf(MED,"can't clone from or to system objects");
		}
		 /*  get obj pointer and check for availability of the other object. */
		if (((already_clone=(o->oflags&OF_CLONE)) || (!(o->n_vertex|o->n_mat|o->n_poly|o->n_tex))) && (!(o->oflags&OF_VIRTUAL)))
		{
			if (no->oflags&OF_CLONE)
			{	 /*  target is clone */
				errds(VHIGH,"obj_clone_change()","couldn't clone %d from %d (on pid %d): clone target is already clone.",oid,toid,p->id,oid);
				return(-1);
			}
			if (!already_clone)  /*  some other object could link to us, so we check the other objects and forward them just in case. */
			{
				if (p->object[oid]->oflags&OF_CLONE_SRC)
				{
					is_clnsrc=0;
					for (i=0;i<p->n_obj;i++)
						if (p->object[i]!=NULL)
							if ((p->object[i]->oflags&OF_CLONE) && (p->object[i]->n_vertex==oid))  /*  it's linking to our object! */
							{
								errds(VHIGH,"obj_clone_change()","couldn't clone %d from %d (on pid %d): object %d is already cloning from object %d.",
												oid,toid,p->id,oid,i,oid);
								return(-1);
							}
					if (!is_clnsrc)
					{
						dprintf(MED,"obj_clone_change(): %d in process %d is no longer a clone-source",oid,p->id);
						p->object[oid]->oflags&=~OF_CLONE_SRC;
					}
				}
			}
			if (oid!=toid)
			{  /*  don't looplink */
				o->oflags|=OF_CLONE;
				no->oflags|=OF_CLONE_SRC;
				o->n_vertex=toid;  /*  n_vertex is not used for this as it's just cloned, so we can use it ... */
				obj_size_update(p,oid);
				dprintf(LOW,"changed clone-target of obj %d to %d of process %d",oid,toid,p->id);
				if (p->id!=MCP) obj_check_biggest_object(p,oid);
			} else {
				errds(MED,"obj_clone_change()","couldn't clone %d from %d (on pid %d): cloning from itself doesn't make sense!",oid,toid,p->id,oid);
				return(-1);
			}
		} else {
			errds(MED,"obj_clone_change()","couldn't clone %d from %d (on pid %d): object %d is not empty",oid,toid,p->id,oid);
			obj_debug(p,oid);
			obj_debug(p,toid);
			return(-1);
		}
	}
	return (0);
}


/*  object-deletion request from proto.c */
int obj_del(struct t_process *p, uint32_t oid)
{
	struct t_process *mcp_p;
	struct t_obj *o;
	float r,mr;
	uint32_t i;
	uint32_t mcp_oid=-1;
	mcp_p=get_proc_by_pid(MCP);
	if ((p->id==MCP) && (obj_valid(p,oid,o)))
	{
		if (o->oflags&OF_VIRTUAL)  /*  only delete if virtual */
		{
			dprintf(HIGH,"the mcp wants %d to be closed",o->n_mat);
			event_quit(get_proc_by_pid(o->n_mat));
			return(0);
		}
		if (o->oflags&OF_SYSTEM)
		{
			dprintf(HIGH,"can't delete system object!");
			return(0);
		}
	} else 
		mcp_oid=p->mcp_oid;
	if (obj_valid(p,oid,o))
	{
		obj_free(p,oid);
		if ((p->id!=MCP) && (p->biggest_obj==oid))
		{  /*  if object was the biggest object, find a new one. */
			mr=-1;
			p->biggest_obj=-1;
			for (i=0;i<p->n_obj;i++)
				if (p->object[i]!=NULL)
				{
					r=p->object[i]->r+p->object[i]->or;
					if (r>mr)
					{
						if (!(p->object[i]->oflags&OF_SYSTEM)) 
						{
							p->biggest_obj=i;
							mr=r;
						}
					}
				}
			mcp_p->object[mcp_oid]->r=mr;
			dprintf(MED,"new biggest object is :%d (size: %f)",p->biggest_obj,mr);
		}
		/*  check if someone depended on this object as clone.... */
		if (o->oflags&OF_CLONE_SRC)
			for (i=0;i<p->n_obj;i++)
				if (p->object[i]!=NULL)
					if ((p->object[i]->oflags&OF_CLONE) && (p->object[i]->n_vertex==oid))  /*  it's linking to our object! */
					{
						p->object[i]->oflags&=~OF_CLONE;  	 /*  disable clone flag */
						p->object[i]->n_vertex=0; 			 /*  and "clone reference" to 0 */
						p->object[i]->r=0.0F;				 /*  empty object, so radius is zero! */
						if (p->id!=MCP) obj_check_biggest_object(p,i);
					}
		/* check if we were a link source for anyone ... */
		if (o->oflags&OF_LINK_SRC)
			for (i=0;i<p->n_obj;i++)
				if (p->object[i]!=NULL)
					if ((p->object[i]->oflags&OF_LINK) && (p->object[i]->linkid==oid))
					{
						p->object[i]->linkid=-1;			 /*  lost our link target! */
						if (mcp_oid>-1)
							obj_pos_update(p,i);
					}
		return(0);
	}
	return(-1);
}

/*  this is the "direct" freeing function, without checking for perfomance */
int obj_free(struct t_process *p,uint32_t oid)
{
	uint32_t i;
	GLuint t;
	struct t_obj *o=p->object[oid];
		dprintf(HIGH,"deleting object %d of process %d",oid,p->id);
		if (!(o->oflags&OF_NODATA))
		{
			if (o->n_vertex>0) free(o->p_vertex);
			if (o->n_poly>0) free(o->p_poly);
			if (o->n_mat>0) free(o->p_mat);
			for (i=0;i<o->n_tex;i++)
			{
				if (o->p_tex[i].buf!=NULL)
					free(o->p_tex[i].buf);
				if (o->p_tex[i].gl_texnum)
				{
					t=o->p_tex[i].gl_texnum;
					glDeleteTextures(1,&t);
				}
			}
			if (o->n_tex>0) free(o->p_tex);

		}
		if (o->dplist)
		{
			if (!(o->oflags&(OF_CLONE|OF_SYSTEM))) 
			{
				dprintf(VLOW,"freeing display list %d",o->dplist);
				glDeleteLists(o->dplist,1);
			}
		}
		free(o);
		p->object[oid]=NULL;
		if (oid==(p->n_obj-1))
		{
			i=oid;
			while ((i!=-1) && (p->object[i]==NULL)) i--;
			p->n_obj=i+1;
			p->object=realloc(p->object,sizeof(struct t_obj *)*(p->n_obj));
		}
	return(0);
}
/* get the object of the pointer (that's 1, usually */
uint32_t get_pointer(struct t_process *p)
{
	uint32_t i;
	for (i=0;i<p->n_obj;i++)
	{

		if (OF_POINTER==(p->object[i]->oflags&0xF0000000))
		{
			return(i);
		}
	}
	return(-1);
}
