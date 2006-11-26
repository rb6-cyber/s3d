/*
 * draw.c
 * 
 * Copyright (C) 2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3dosm, a gps card application for s3d.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3dosm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3dosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3dosm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "s3dosm.h"
#include <s3d.h>
#include <math.h>	/* sin(), cos() */
#include <stdio.h>	/* printf() */
#include <string.h>	/* strcmp() */

struct vdata{
	layer_t *layer;
	double lonsum,latsum;
	int n;
	int oid;
	int vnum;
};

void calc_earth_to_eukl(double lon, double lat, double *x)
{
	double la,lo;
	la=lat*M_PI/180.0;
	lo=lon*M_PI/180.0;
	x[0]=ESIZE*sin(lo) *cos(la);
	x[1]=ESIZE*			sin(la);
	x[2]=ESIZE*cos(lo) *cos(la);
}
void draw_add_vertices(object_t *t, void *data)
{
	struct vdata *v=data;
	tag_t *tag;
	
	if (t->type==T_NODE)
	{
		double x[3];
		node_t *node=NODE_T(t);
		node->vid=v->vnum;
		calc_earth_to_eukl(node->lon,node->lat,x);
		s3d_push_vertex(v->oid,x[0],x[1],x[2]);
		if (node->visible==2) /* something special */
		{
			if (NULL!=(tag=tag_get(OBJECT_T(node), "amenity")))
			{
				if (0==(strcmp(tag->v,"wifi")))
				{
					tag_t *wtag,*ttag;
					if (NULL!=(ttag=tag_get(OBJECT_T(node),"wifi_type"))) {
						if (0==strcmp(ttag->v,"infrastructure"))
						{	
							if (NULL!=(wtag=tag_get(OBJECT_T(node), "wifi_wep")))
								if (0==strcmp(wtag->v,"true"))
									node->base.oid=s3d_clone(icons[ICON_AP_OPEN].oid);
								else
									node->base.oid=s3d_clone(icons[ICON_AP].oid);
							else
							node->base.oid=s3d_clone(icons[ICON_AP].oid);
							s3d_translate(node->base.oid,x[0],x[1],x[2]);
							s3d_link(node->base.oid,v->oid);
							s3d_flags_on(node->base.oid,S3D_OF_VISIBLE);
							s3d_rotate(node->base.oid,(90-node->lat),node->lon,0);
							v->layer->visible=1;
						} else { /* not an ap */
						}
					}
				}
			}
		}
		v->vnum++;
		v->lonsum+=node->lon;
		v->latsum+=node->lat;
		v->n++;
	}
}

void draw_add_segments(object_t *t, void *data)
{/*
	struct vdata *v=data;
	tag_t *tag;
	int color;
	
	if (t->type==T_SEGMENT)
	{
		node_t *from, *to;
		segment_t *seg=SEGMENT_T(t);
		from=NODE_T(avl_find(v->layer->tree,seg->from));
		to=NODE_T(avl_find(v->layer->tree,seg->to));

		color=0;
		/ * TODO: look at the ways using it, not the segments  * /
		if (NULL!=(tag=tag_get(OBJECT_T(seg), "highway")))
		{
			if (0==(strcmp(tag->v,"motorway"))) color=1;
			else if (0==(strcmp(tag->v,"motorway_link"))) color=2;
			else if (0==(strcmp(tag->v,"primary"))) color=3;
			else if (0==(strcmp(tag->v,"secondary"))) color=4;
			else if (0==(strcmp(tag->v,"residential"))) color=5;
		}
		if (from!=NULL && to!=NULL)
		{
			s3d_push_line(v->oid,from->vid,to->vid,color);
			v->layer->visible=1;
		}
	}*/
}
int draw_layer(layer_t *layer)
{
	struct vdata v;
	int oid;
	oid=s3d_new_object();
	s3d_link(oid,oidy);
	v.layer=layer;
	v.oid=oid;
	v.vnum=0;
	v.n=0;
	v.lonsum=v.latsum=0;
	s3d_push_material(oid,1,1,1,		1,1,1,		1,1,1); /* default */
	s3d_push_material(oid,0.3,0.3,1,	0.3,0.3,1.0,	0.3,0.3,1.0);	/* motorway */
	s3d_push_material(oid,0.5,0.5,0.8,	0.5,0.5,0.8,	0.5,0.5,0.8);	/* motorway_link*/
	s3d_push_material(oid,1.0,1.0,0.0,	1.0,1.0,0.0, 	1.0,1.0,0.0);	/* primary */
	s3d_push_material(oid,0.8,0.8,0.2,	0.8,0.8,0.2, 	0.8,0.8,0.2);	/* secondary */
	s3d_push_material(oid,0.7,0.7,0.4,	0.7,0.7,0.4, 	0.7,0.7,0.4);	/* secondary */
	/*
	avl_tree_trav(layer->tree,draw_add_vertices,(void *)&v);
	avl_tree_trav(layer->tree,draw_add_segments,(void *)&v);*/
	layer->center_lo=(v.lonsum)/v.n;
	layer->center_la=(v.latsum)/v.n;	
	s3d_flags_on(oid,S3D_OF_VISIBLE);
	return(0);
}
void draw_all_layers()
{
	int i;
	for (i=0;i<layerset.n;i++)
		draw_layer(layerset.p[i]);
}
