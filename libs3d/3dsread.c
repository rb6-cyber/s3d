/*
 * 3dsread.c
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


#include "s3d.h"
#include "s3dlib.h"
#include <stdlib.h> 	 /*  exit(), malloc() */
#include <math.h>		 /*  sqrt() */
#include <string.h> 	 /*  strncpy() */
#include <errno.h> 		 /*  errno */
#define MAXSTRN		20
/*  just a helper function for reading from file instead of memory. */
int s3d_import_3ds_file(char *fname)
{
	char *buf,*ptr,*next;
	char searchpath[1024];
	char path[1024];
#ifndef OBJSDIR
#define OBJSDIR 	"./:../:../../:/usr/local/share/s3d/:/usr/share/s3d/"
#endif
	
	strncpy(searchpath,OBJSDIR,1023);
	searchpath[1023]=0;							/* just in case */
	next=ptr=searchpath;
	while (next!=NULL)
	{
		next=NULL;
		
		if (NULL!=(next=strchr(ptr,':')))
		{
			*next=0; 							/* clear the delimiter */
			next+=1;							/* move to the beginner of the next dir */
		}
		if ((strlen(ptr)+strlen(fname))<1024) 	/* only try if this fits */
		{
			strcpy(path,ptr); 					/* can use "unsafe" functions because size was verified above */
			strcat(path,fname);
			if (s3d_open_file(path,&buf)!=-1)  /* found something */
				return(s3d_import_3ds(buf));
		}
		if (next!=NULL)
			ptr=next;							/* move pointer to the next position */
	}
	errds(LOW,"s3d_import_3ds_file()","Could not open %s", fname);
	return(-1); /* nothing in search path ... */
}
static void normal(float *p0, float *p1, float *p2, float *r)
{
	float a[3],b[3],n[3];
	float len;
	a[0]=p1[0]-p0[0];
	a[1]=p1[1]-p0[1];
	a[2]=p1[2]-p0[2];
	b[0]=p2[0]-p0[0];
	b[1]=p2[1]-p0[1];
	b[2]=p2[2]-p0[2];
	n[0]=a[1]*b[2] - a[2]*b[1];
	n[1]=a[2]*b[0] - a[0]*b[2];
	n[2]=a[0]*b[1] - a[1]*b[0];

	len=sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
	if (len!=0.0F)
	{
		r[0]=n[0]/len;
		r[1]=n[1]/len;
		r[2]=n[2]/len;
	} else {
		s3dprintf(VLOW,"normal(): couldn't calc normal");
		r[0]=r[1]=r[2]=0.0F;
	}
}
void sort_poly(unsigned long *smooth_list, unsigned long *poly_buf,int polynum)
{
	unsigned long min,minv,i;
	unsigned long polyel[4];
	minv=-1;
	min=0;
	for (i=0;i<polynum;i++)
	{
		if (smooth_list[i]<minv) 
		{
			min=i;
			minv=smooth_list[i];
		}
	}
	if (min!=0)  /*  swap */
	{
#define EL	4*sizeof(unsigned long)
		memcpy(polyel,poly_buf,EL);  /* save */
		memcpy(poly_buf,poly_buf+4*min,EL);  /* put at first place */
		memcpy(poly_buf+4*min,polyel,EL); 
		 /*  now the same with smooth_list */
		i=smooth_list[0];
		smooth_list[0]=smooth_list[min];
		smooth_list[min]=i;
#undef EL
	}
	if (polynum>1)
	{
		sort_poly(smooth_list+1,poly_buf+4,polynum-1);
	}
	
}
struct t_vertex_normal
{
	float n[3];
	unsigned long g,num;
};
/*  this functions takes a shitload of arguments, but that's because of optimization.  */
/*  we add normals of the polygons's vertices so each vertex will finally have */
/*  the sum of the polygons normals where the vertex is part of. */
static int smooth(float *vbuf,int voff, unsigned long *pbuf, float *pnbuf, float *nbuf, struct t_vertex_normal *v_t_buf, int pnum, int g)
{
	int i,j,n;
	unsigned long k;
	float len;
	 /*  run1: add normals on themselves into the v_t_buf */
	for (i=0;i<pnum;i++)
	{
		for (j=0;j<3;j++)
		{
			k=pbuf[i*4+j]-voff;
			if (v_t_buf[k].g!=g)  /*  not added in this group yet */
			{
				for (n=0;n<3;n++)
					v_t_buf[k].n[n]=(pnbuf+i*3)[n];
				v_t_buf[k].num=1;
				v_t_buf[k].g=g;  /*  now it's  in our group. */
			} else {
				for (n=0;n<3;n++)
					v_t_buf[k].n[n]+=(pnbuf+i*3)[n];
				v_t_buf[k].num++;
			}
		}
	}
	 /*  run2: apply to the final vertex buffer */
	for (i=0;i<pnum;i++)
	{
		for (j=0;j<3;j++)
		{
			k=pbuf[i*4+j]-voff;
			if (v_t_buf[k].num>1)  /*  if more than 1, normalize. */
			{
				len=sqrt(v_t_buf[k].n[0]*v_t_buf[k].n[0]+v_t_buf[k].n[1]*v_t_buf[k].n[1]+v_t_buf[k].n[2]*v_t_buf[k].n[2]);
				if (len==0.0F)   /*  this should not happen. well ... */
				{
					for (n=0;n<3;n++)
						v_t_buf[k].n[n]=0;
					v_t_buf[k].g=-1;  /*  we're telling this by setting group to -1 */
				}
				else 
					for (n=0;n<3;n++)
						v_t_buf[k].n[n]/=len;
				v_t_buf[k].num=1;
			}
			if (v_t_buf[k].g==g)  /*  just making sure, or for the case of bad normals. */
				memcpy(nbuf+i*9+j*3,v_t_buf[k].n,sizeof(float)*3);  /*  finally, we save the normal in our normal buffer */
			else  /*  use the pbuf normal */
			{
				memcpy(nbuf+i*9+j*3,pnbuf+i*3,sizeof(float)*3);
			}
		}
	}
	return(0);
}
/*  calculates the normals: */
static float *calc_normals(float *vertex_buf, int vertexnum, unsigned long *poly_buf,
				int polynum, int voff,unsigned long *smooth_list)
{
	int i,j,n=0;
	float *pnormal_list, *nbuf;  /*  pnormal_list has the space for normals per polygon, */
								 /*  nbuf has 3 normals per polygon for each vertex */
	struct t_vertex_normal *v_t_buf;
								 /*  this buffer will save temporary normals for each vector */
								 /*  along with the information of group (each group might */
								 /*  have another normal for a certain vertex) and of polygons */
								 /*  sharing this vertex. */
	unsigned long v[3];
	unsigned long lg,g;  /*  last group and group */
	lg=-1;g=0;
	pnormal_list=malloc(sizeof(float)*3*polynum);
	nbuf=malloc(sizeof(float)*3*3*polynum);
	v_t_buf=malloc(sizeof(struct t_vertex_normal)*vertexnum);
	memset(v_t_buf,0,sizeof(struct t_vertex_normal)*vertexnum);
	sort_poly(smooth_list, poly_buf, polynum);
	for (i=0;i<polynum;i++)
	{
		g=smooth_list[i];
		if (lg!=g)  /*  a different group */
		{
			if (n>0)  /*  that should only be false in the first loop */
			{
				smooth(vertex_buf,voff,poly_buf+(i-n)*4,pnormal_list+(i-n)*3, nbuf+(i-n)*9,v_t_buf,n,g);
			}
			n=0;  /*  no elements so far in the new group */
		}
		for (j=0;j<3;j++)
		{
			v[j]=poly_buf[i*4+j]-voff;
			if (v[j]>=vertexnum)  /*  bad input */
			{
				errds(VHIGH,"calc_normals()","bad input, polygon vertex index out of range");
				return(NULL);
			}
		}
		 /* s3dprintf(LOW,"polygon [%d/%d]: %d %d %d is in smoothlist %d",i,polynum,v[0],v[1],v[2],g); */
		
		normal(	vertex_buf+v[0]*3,
				vertex_buf+v[1]*3,
				vertex_buf+v[2]*3,
				pnormal_list+i*3);
		lg=g; 	 /*  save the last group */
		n++;	 /*  save the number of how much elements are in the group now. */
	}



	 /* s3dprintf(MED,"processing the final group ... %d (%d members)",g,n); */
	smooth(vertex_buf,voff,poly_buf+(i-n)*4,pnormal_list+(i-n)*3, nbuf+(i-n)*9,v_t_buf,n,g);
/*	for (i=0;i<polynum;i++)
	{
		for (j=0;j<3;j++)
			s3dprintf(MED,"poly[%d/%d],point[%d/3]: %f %f %f",i,polynum,j,
							nbuf[i*9+j*3],
							nbuf[i*9+j*3+1],
							nbuf[i*9+j*3+2]);
	}*/
	free(pnormal_list);
	free(v_t_buf);
	return(nbuf);
}
/* get the intergers in the right order */
unsigned short gints(char *ptr)
{
	register unsigned short i;
	i= ((unsigned char )ptr[0]);
	i+=((unsigned char )ptr[1])*0x100;
	return i;
}
unsigned long gintl(char *ptr)
{
	register unsigned long i;
	i= ((unsigned char )ptr[0]);
	i+=((unsigned char )ptr[1])*0x100;
	i+=((unsigned char )ptr[2])*0x10000;
	i+=((unsigned char )ptr[3])*0x1000000;
	return i;
}
/*  imports a 3ds file as ONE object, even it virtually contains more. */
/*  it returns the object id ... */
int s3d_import_3ds(char *buf)
{
	char *ptr,*ptr2,*mesh_end=NULL;
	int i,j,polynum=0,vertexnum=0;
	char ostr[MAXSTRN+1];
	char materials[256][MAXSTRN+1];
	int clen,cid;
	int filesize=1;  /*  just so it hops above the main chunk ... */
	int vertex_offset=0; 
	int v=0;
	int col_obj=-1;
	unsigned long *poly_buf=NULL,*tpbuf,*smooth_list=NULL;
	unsigned char r1,g1,b1,r2,g2,b2,r3,g3,b3;
	unsigned char r_amb=255,g_amb=255,b_amb=255,
				  r_diff=255,g_diff=255,b_diff=255,
				  r_spec=255,g_spec=255,b_spec=255;
	unsigned char color=0;
	unsigned short nfaces;
	float *vertex_buf=NULL, *nbuf=NULL,*tnbuf;
	int cur_oid=-1;
	if (buf==NULL) return(-1);
	ptr=buf;
	while (((ptr)>=buf) && ((ptr)<(buf+filesize)))
	{
		cid=gints(ptr);
		clen=gintl(ptr+2);
		
		s3dprintf (VLOW,"[pos %x]: \t%04x [len:%d]",(ptr-buf),cid,(clen-6));
		if ((ptr==buf) && (cid!=0x4d4d))
		{
			errs("3d_import_3ds()","file doesn't start with 0x4d4d, maybe file corrupt?");
			return(-1);
		}
		ptr=ptr+6;  /*  point to the data .. */
		switch (cid)
		{
		  case 0x4d4d: 
			  s3dprintf(VLOW,"-- the main chunk!!");
			  filesize=clen;
			  if (cur_oid==-1)
			  {
				  cur_oid=s3d_new_object();
				   /*  standard material for fallback reasons */
			      s3d_push_material(cur_oid,0.2,0.2,0,0.2,0.2,0,0.2,0.2,0);
				  col_obj++;
			  }

           	  break;
		  case 0x3D3D:
			  s3dprintf(VLOW,"-- the 3d editor chunk!");
			  break;
		  case 0x4000:
			  s3dprintf(VLOW,"-- an object block. let's see ...");
			  strncpy((char *)ostr,(char *)ptr,MAXSTRN);
			  vertex_offset+=v;
			  v=0;
			  ptr=(ptr+strlen(ostr)+1);
			  break;
		  case 0x4100:
			  s3dprintf(VLOW,"-- Triangular mesh");
			  smooth_list=NULL;
			  mesh_end=ptr+(clen-6);
			  break;
		  case 0x4110: 
			  vertexnum=gints(ptr);
			  ptr+=sizeof(unsigned short);
			  s3dprintf(VLOW,"-- vertices list!! number of vertices: %d",vertexnum);
			  vertex_buf=malloc(sizeof(float)*3*vertexnum);
			  if (vertex_buf==NULL) break;
/* 			  memcpy(vertex_buf,ptr,sizeof(float)*3*i); */
			  for (j=0; j<vertexnum; j++)
		 	  {
				*(vertex_buf+j*3+0)=*((float *)ptr+0);
				*(vertex_buf+j*3+1)=*((float *)ptr+2);
				*(vertex_buf+j*3+2)=-*((float *)ptr+1);
				ptr+=sizeof(float)*3;
			  }
			  v+=vertexnum;  /*  for the correct vertex offset */
			break;
		  case 0x4120:
			polynum=gints(ptr);
			ptr+=sizeof(unsigned short);
			s3dprintf(VLOW,"-- polygon list!! number of polygons: %d",polynum);
			poly_buf=malloc(sizeof(unsigned long)*4*polynum);
			if (poly_buf==NULL) break;
		    for (j=0; j<polynum; j++)
			{
				poly_buf[j*4+0]=vertex_offset+gints(ptr+0); 
				poly_buf[j*4+1]=vertex_offset+gints(ptr+4);
				poly_buf[j*4+2]=vertex_offset+gints(ptr+2);
				poly_buf[j*4+3]=col_obj;  /*  we should have a default material .... */
				ptr+=sizeof(unsigned short)*4;
		    }
			break;
		  case 0x4130:
			ptr2=(char *)ptr+(clen-6);  /*  backup our endpointer ... */
			s3dprintf(VLOW,"-- material information for faces .....");
			strncpy((char *)ostr,(char *)ptr,MAXSTRN);
			ptr+=strlen(ptr)+1;
			s3dprintf(VLOW,".. material string name is %s",ostr);
			col_obj=0;
			while (col_obj<256 && (strncmp(ostr,materials[col_obj],MAXSTRN)!=0)) col_obj++;
			if (col_obj>=256)
			{
				errds(MED,"s3d_import_3ds()","couldn't find material %s",ostr);
				col_obj=0;
			}
			nfaces=gints(ptr);
			ptr+=2;
			for (i=0;i<nfaces;i++)
			{
				j=gints(ptr+2*i);
				if (j>=0 && j<polynum)
					poly_buf[gints(ptr+2*i)*4+3]=col_obj; 
				else {
					errds(MED,"s3d_import_3ds()","polygon %d out of range!",j);
				}
			}
			ptr=ptr2;
			break;
		  case 0x4150:
			s3dprintf(VLOW,"-- smoothing group information (length %d [%d])", clen,clen/4);
			smooth_list=(unsigned long *)ptr;
			for (j=0;j<(clen/4);j++)
			{
				smooth_list[j]=gintl(ptr+j*4);
			}

			ptr=(char *)ptr+(clen-6);
			break;
		  case 0x4160:
			  s3dprintf(VLOW,"-- translation matrix");
			  for (j=0; j<4; j++)
		 	  {
				s3dprintf(VLOW,"[%f:%f:%f:%f]",
								*((float *)ptr),
								*((float *)ptr+1),
								*((float *)ptr+2),
								((j==3)?1.0:0.0)
								);
				ptr+=sizeof(float)*3;
			  }
			  break;
		  case 0xafff:
			  s3dprintf(VLOW,"-- material chunk O_o");
			  break;
		  case 0xa000:
			  strncpy((char *)ostr,(char *)ptr,MAXSTRN);
			  s3dprintf(VLOW,"-- material string name is %s",ostr);
			  ptr=(char *)ptr+(clen-6);
			  color|=8;
			  break;
		  case 0xa010:
			  r1=(unsigned char)*(ptr);
			  g1=(unsigned char)*(ptr+1);
			  b1=(unsigned char)*(ptr+2);
			  r2=(unsigned char)*(ptr+3);
			  g2=(unsigned char)*(ptr+4);
			  b2=(unsigned char)*(ptr+5);
			  r3=(unsigned char)*(ptr+6);
			  g3=(unsigned char)*(ptr+7);
			  b3=(unsigned char)*(ptr+8);
			  s3dprintf(VLOW,"-- ambient color 3:>> [rgb] [%x %x %x]",r3,g3,b3);
			  r_amb=r3;g_amb=g3;b_amb=b3;
			  color=color|1;
			  ptr=(char *)ptr+(clen-6);
			  break;
		  case 0xa020:
			  r1=(unsigned char)*(ptr);
			  g1=(unsigned char)*(ptr+1);
			  b1=(unsigned char)*(ptr+2);
			  r2=(unsigned char)*(ptr+3);
			  g2=(unsigned char)*(ptr+4);
			  b2=(unsigned char)*(ptr+5);
			  r3=(unsigned char)*(ptr+6);
			  g3=(unsigned char)*(ptr+7);
			  b3=(unsigned char)*(ptr+8);
			  s3dprintf(VLOW,"-- diffuse color 3:>> [rgb] [%x %x %x]",r3,g3,b3);
			  r_diff=r3;g_diff=g3;b_diff=b3;
			  color=color|2;
			  ptr=(char *)ptr+(clen-6);
			  break;
		  case 0xa030:
			  r1=(unsigned char)*(ptr);
			  g1=(unsigned char)*(ptr+1);
			  b1=(unsigned char)*(ptr+2);
			  r2=(unsigned char)*(ptr+3);
			  g2=(unsigned char)*(ptr+4);
			  b2=(unsigned char)*(ptr+5);
			  r3=(unsigned char)*(ptr+6);
			  g3=(unsigned char)*(ptr+7);
			  b3=(unsigned char)*(ptr+8);
			  s3dprintf(VLOW,"-- spec color 3:>> [rgb] [%x %x %x]",r3,g3,b3);
			  r_spec=r3;g_spec=g3;b_spec=b3;
			  color=color|4;
			  ptr=(char *)ptr+(clen-6);
			  break;
		  default:
			ptr=(char *)ptr+(clen-6);
		}
		 /*  color finished? then upload. I know, this is not real implementation ... */
		if (color==15)
		{
			col_obj++;
			if (col_obj<256)  /*  TODO: we currently don't support  */
							  /*  more than 256 materials ...  */
							  /*  that's just because i'm lazy */
			{
				strncpy(materials[col_obj],ostr,MAXSTRN);
				s3dprintf(VLOW,"assigned material %s on position %d",
								materials[col_obj],col_obj);
			}
			s3dprintf(VLOW,"-- [%d]colors... amb: %d %d %d, spec %d %d %d, diff %d %d %d",col_obj,
							r_amb,g_amb,b_amb,
							r_spec,g_spec,b_spec,
							r_diff,g_diff,b_diff
				   );
			s3d_push_material(cur_oid,
							r_amb/255.0,g_amb/255.0,b_amb/255.0,
							r_spec/255.0,g_spec/255.0,b_spec/255.0,
							r_diff/255.0,g_diff/255.0,b_diff/255.0
							);
			color=0;
		}
		if ((mesh_end!=NULL) && (ptr>=mesh_end))
		{
			mesh_end=NULL;
			if ((vertex_buf!=NULL))
			    s3d_push_vertices(cur_oid, vertex_buf, vertexnum); 
			if (poly_buf!=NULL)
			{
				if (smooth_list!=NULL)
					nbuf=calc_normals(vertex_buf,vertexnum,poly_buf,polynum,vertex_offset,smooth_list);

				 /*  do in 1000 chunks */
				s3dprintf(LOW,"committing %d polys",polynum);
				tnbuf=nbuf;
				tpbuf=poly_buf;
#define CSIZE	1000
				while (polynum>CSIZE)
				{
					s3d_push_polygons(cur_oid, tpbuf, CSIZE);
					if (nbuf!=NULL)
						s3d_pep_polygon_normals(cur_oid,tnbuf, CSIZE);
					tnbuf+=	CSIZE*9;
					tpbuf+=	CSIZE*4;
					polynum-=CSIZE;
				}
				s3d_push_polygons(cur_oid, tpbuf, polynum);
				if (nbuf!=NULL)
					s3d_pep_polygon_normals(cur_oid,tnbuf, polynum);
			}


			 /*  clean things up */
			if (vertex_buf!=NULL)
			{
				free(vertex_buf);vertex_buf=NULL;
			}
			if (poly_buf!=NULL)
			{
				free(poly_buf);poly_buf=NULL;
			}
			if (smooth_list!=NULL)
			{
				/*FIXME: free(smooth_list);*/
				smooth_list=NULL;
			}
			if (nbuf!=NULL)
			{
				free(nbuf);nbuf=NULL;
			}
		}
	}
	s3dprintf(VLOW,"-- done [ptr:%010p,buf:%010p]...",ptr,buf);
	free(buf);
	return(cur_oid);
}
