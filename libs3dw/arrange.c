/*
 * arrange.c
 *
 * Copyright (C) 2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of the s3d Widgets, a Widget Library for s3d.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3d Widgets is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * s3d Widgets is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d Widgets; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <s3d.h>
#include <s3dw.h>
#include <s3dw_int.h>
#include <stdlib.h> /* rand(), RAND_MAX */
#include <math.h>	/* M_PI */
#define R2D		(180/M_PI)
s3dw_widget *cam=NULL;

void s3dw_arr_widgetcenter(s3dw_widget *widget, float *center)
{
	float x,y,z,xt,yt,zt;
	x=widget->s*widget->width/2;
	y=widget->s*-widget->height/2+0.5;
	z=widget->s*0.5;
	/* calc back rotation */
	/* around the y axis (horizontal direction) */
	xt= cos(widget->ry/R2D)*x + sin(widget->ry/R2D) *z;
	yt=y;
	zt=-sin(widget->ry/R2D)*x + cos(widget->ry/R2D) *z;
	
	/* around the x axis (vertical direction) */
	x=xt;
	y= cos(widget->rx/R2D)*yt + sin(widget->rx/R2D) *zt;
	z=-sin(widget->rx/R2D)*yt + cos(widget->rx/R2D) *zt;
	
	center[0]=x;
	center[1]=y;
	center[2]=z;
}
void s3dw_arr_normdir(float *dir)
{
	float dirlen=s3d_vector_length(dir);
	while ((dirlen=s3d_vector_length(dir))==0)
	{
		/* make up some random direction if they're exactly the same position */
		dir[0]=((float)rand() - RAND_MAX/2.0)/RAND_MAX;
		dir[1]=((float)rand() - RAND_MAX/2.0)/RAND_MAX;
		dir[2]=((float)rand() - RAND_MAX/2.0)/RAND_MAX;
	}
	dir[0]/=dirlen;
	dir[1]/=dirlen;
	dir[2]/=dirlen;
}
int al=0;
void s3dw_turn()
{
	s3dw_widget *w,*root=s3dw_getroot();
	int i;
	float a[3],b[3],rx,ry;
	float op[3],np[3];
	a[0]=0;
	a[1]=0;
	a[2]=1;
	al++;
	for (i=0;i<root->nobj;i++)
	{
		w=root->pobj[i];
		if (w->oid!=0)
		{
			s3dw_arr_widgetcenter(w,op);
			/* horizontal movement */
			b[0]=w->x + op[0]  - cam->x;
			b[1]=0;
			b[2]=w->z + op[2]  - cam->z;
			ry=180*s3d_vector_angle(a,b)/M_PI;
			/* correct acos incompletness */
			if (b[0]<0) ry=180-ry;
			else 		ry=180+ry;
			b[0]=0;
			b[1]=w->y + op[1]   - cam->y;
			b[2]=w->z + op[2]   - cam->z;
			rx=180*s3d_vector_angle(a,b)/M_PI;
			if (b[1]>0) rx=180-rx;
			else 		rx=180+rx;
			if ((rx>90) && (rx<=180)) 
				rx=180 - rx;
			else if ((rx>=180) && (rx<270))  rx=540 - rx ;

			w->rx=rx;
			w->ry=ry;
			s3dw_arr_widgetcenter(w,np);
			w->x-=np[0] - op[0];
			w->y-=np[1] - op[1];
			w->z-=np[2] - op[2];

			s3dw_ani_add(w);
		}
	}
}
void s3dw_arrange()
{
	s3dw_widget *w1,*w2,*root=s3dw_getroot();
	int i,j,arranged;
	float len1,len2, dirlen;
	float tomove, move1, move2;
	float f1[3],f2[3],dir[3];

	/* test if there is anything to arrange ... */
	arranged=1;
	for (i=0;i<root->nobj;i++)
		if (!(root->pobj[i]->flags&S3DW_ARRANGED)) arranged=0;
	if (arranged) return; /* no arrangement necceasary .... */
/*	if (cam->flags|S3DW_*/
		
	if (root->nobj==1)
	{
		w1=root->pobj[0];
		w1->flags|=S3DW_ARRANGED; /* done */
		return;
	}
	for (i=0;i<root->nobj;i++)
	{
		w1=root->pobj[i];
		arranged=1;
		for (j=0;j<root->nobj;j++)
			if (i!=j)
			{
				w2=root->pobj[j];
				s3dw_arr_widgetcenter(w1,f1);
				s3dw_arr_widgetcenter(w2,f2);
				len1=s3d_vector_length(f1);
				len2=s3d_vector_length(f2);
				dir[0]=(w1->x+f1[0]) - (w2->x+f2[0]);
				dir[1]=(w1->y+f1[1]) - (w2->y+f2[1]);
				dir[2]=(w1->z+f1[2]) - (w2->z+f2[2]);
				dirlen=s3d_vector_length(dir);
				if (dirlen<(len1+len2))
				{
					arranged=0;
					w1->flags&=~S3DW_ARRANGED;
					w2->flags&=~S3DW_ARRANGED;
					/* it's not arranged, turn the flags off! */
					s3dw_arr_normdir(dir);
					tomove=((len1+len2+1)-dirlen);
					move1=len1/(len1+len2);
					move2=len2/(len1+len2);
					if (w1->oid!=0)
					{
						w1->x += tomove * move1 *  dir[0];
						w1->y += tomove * move1 *  dir[1];
						w1->z += tomove * move1 *  dir[2];
						s3dw_ani_add(w1);
					} else {
						w2->x += tomove * move2 * -dir[0];
						w2->y += tomove * move2 * -dir[1];
						w2->z += tomove * move2 * -dir[2];

					}
					if (w2->oid!=0)
					{
						w2->x += tomove * move2 * -dir[0];
						w2->y += tomove * move2 * -dir[1];
						w2->z += tomove * move2 * -dir[2];
						s3dw_ani_add(w2);
					} else {
						w1->x += tomove * move1 *  dir[0];
						w1->y += tomove * move1 *  dir[1];
						w1->z += tomove * move1 *  dir[2];
					}
				}
			}
		if (arranged)
			w1->flags|=S3DW_ARRANGED;
	}
}
