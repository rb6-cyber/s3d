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
#include <stdlib.h> /* atoi(),malloc(), calloc(), free() */
struct vdata{
	layer_t *layer;
	float lonsum,latsum;
	int n;
	int oid;
	int vnum;
};

void calc_earth_to_eukl(float lat, float lon, float *x)
{
	float la,lo;
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
		float x[3];
		node_t *node=NODE_T(t);
		node->vid=v->vnum;
		calc_earth_to_eukl(node->lat,node->lon,x);
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


static int lastid=-1;
struct waylist {
	int node_from,node_to;
	int seg_id;
	int node_from_l,node_from_r;	/* vertex id's for corners */
	int node_to_l,node_to_r;
};
struct nodelist {
	int node_id;			/* (external counting) */
	float la,lo,alt;		/* earth coords */
	float x[3];				/* euclid coords */
};
struct adjlist {
	int node_id;			/* node to which the segment leads to */
	int seg_id;				/* segment which is involved to the node (both internal counting) */
};
/*
struct nodelist nodelist_p[2];
int				nodelist_n=0;
*/

struct waylist 	*waylist_p=NULL;
struct nodelist	*nodelist_p=NULL;
struct adjlist	*adjlist_p=NULL;
int				nodelist_n=0;
int				adjlist_n=0;
int 			waylist_n=0;
int 			waylist_bufn=0;

/* just fetches node information and puts it in some simple 6x float buffer */
int insert_node(void *data, int argc, char **argv, char **azColName)
{
	struct nodelist *np=data;	/* get the nodepointer */
	int i;
	for(i=0; i<argc; i++){
		if (argv[i]) {
			if (0==strcmp(azColName[i],"longitude"))			np[nodelist_n].lo=strtod(argv[i],NULL);
			else if (0==strcmp(azColName[i],"latitude"))		np[nodelist_n].la=strtod(argv[i],NULL);
			else if (0==strcmp(azColName[i],"altitude"))		np[nodelist_n].alt=strtod(argv[i],NULL);
		}
	}
	nodelist_n++;
	return(0);
}
int select_waytype(void *data, int argc, char **argv, char **azColName)
{
	int i;
	for(i=0; i<argc; i++){
		if (argv[i]) {
			if (0==strcmp(argv[i],"motorway"))				*((int *) data)=1;	
			else if (0==strcmp(argv[i],"motorway_link"))	*((int *) data)=2;	
			else if (0==strcmp(argv[i],"primary"))			*((int *) data)=3;	
			else if (0==strcmp(argv[i],"secondary"))		*((int *) data)=4;	
			else if (0==strcmp(argv[i],"residential"))		*((int *) data)=5;	
		}
	}
	return(0);
}
/* draw waylist, clear the queue */
void waylist_draw()
{
	char query[MAXQ];
	float x[6];
	int i,vert=0;
	int node_id;
	int way_obj;
	int waytype=0;
/*	printf("way: %d - %d segments\n",lastid,waylist_n);*/
	way_obj=s3d_new_object();
	if (lastid!=-1) {
		snprintf(query,MAXQ,"SELECT tagvalue FROM tag WHERE tag_id=(SELECT tag_id FROM way WHERE way_id=%d) AND tagkey='highway';",lastid);
		db_exec(query, select_waytype, &waytype);
	}
	switch (waytype)
	{
		case 1:s3d_push_material(way_obj,0.3,0.3,1,	0.3,0.3,1.0,	0.3,0.3,1.0);	/* motorway */
		case 2:s3d_push_material(way_obj,0.5,0.5,0.8,	0.5,0.5,0.8,	0.5,0.5,0.8);	/* motorway_link*/
		case 3:s3d_push_material(way_obj,1.0,0.6,0.2,	1.0,0.6,0.2, 	1.0,0.6,0.2);	/* primary */
		case 4:s3d_push_material(way_obj,1.0,1.0,0.0,	1.0,1.0,0.0, 	1.0,1.0,0.0);	/* secondary */
		case 5:s3d_push_material(way_obj,1.0,1.0,1.0,	1.0,1.0,1.0, 	1.0,1.0,1.0);	/* residential */
		default:s3d_push_material(way_obj,1,0.5,1,		1,0.5,1,		1,0.5,1); /* default */
	}
	/* put nodes of the graph into a list */
	for (i=0;i<waylist_n*2;i++) {
		if (waylist_n%2)		node_id=waylist_p[i/2].node_from;
		else					node_id=waylist_p[i/2].node_to;
		for (j=0;j<nodelist_n;j++)
			if (nodelist_p[j]==node_id) break;
		if (j==nodelist_n) { /* we still need to add this node */
			nodelist_p[j].node_id=node_id;
			nodelist_n++;
			snprintf(query,MAXQ,"SELECT longitude, latitude, altitude FROM node WHERE node_id=%d;",node_id);
			db_exec(query, insert_node,(void *)(nodelist_p+j));
			calc_earth_to_eukl(nodelist_p[j].la,nodelist_p[j].lo,nodelist_p[j].x);
		}
	}
	/* iterate for all nodes */
	for (i=0;i<nodelist_n;i++)
	{
		/* find adjacent segments */
			/* TODO */
		if (adjlist_n>1)	/* more than one adjacent, need to order and calculate intersections */
		{
			/* TODO: order adjlist */
			for (j=0;j<=adjlist_n;j++)
			{
				int adj_segid;
				/* TODO: calc segpoints for j and j+1%adjlist_n */
				s3d_push_vertices(oid,segpoint,1);
				adj_seg=adjlist[j].seg_id;
				if (nodelist_p[i].node_id==waylist[adj_seg].node_from)		waylist[adj_seg].node_from_r=vert;
					else													waylist[adj_seg].node_to_l=vert;
				vert++;
				adj_seg=adjlist[(j+1)%adjlist_n].seg_id;
				if (nodelist_p[i].node_id==waylist[adj_seg].node_from)		waylist[adj_seg].node_from_l=vert;
					else													waylist[adj_seg].node_to_r=vert;
				vert++;
				}
			}
			if (adjlist_n>3) {
				/* TODO: fill the intersection polygon */
			}
		} else {
			/* endpoint */
			/* TODO calculate segpoint and set to/from pointers appropriate */
			adj_seg=adjlist[j].seg_id;
			if (nodelist_p[i].node_id==waylist[adj_seg].node_from)	{
				waylist[adj_seg].node_from_l=a;
				waylist[adj_seg].node_from_r=b;
			} else {
				waylist[adj_seg].node_to_l=b;
				waylist[adj_seg].node_to_r=a;

			}
		}
	}
	s3d_link(way_obj,oidy);
	s3d_flags_on(way_obj,S3D_OF_VISIBLE);
	waylist_n=0;


/*			
	for (i=0;i<waylist_n;i++)
	{
		float len;
		nodelist_n=0;
		snprintf(query,MAXQ,"SELECT longitude, latitude, altitude FROM node WHERE node_id IN (%d,%d);",waylist_p[i].node_from,waylist_p[i].node_to);
		db_exec(query, insert_node,(void *)nodelist_p);
		calc_earth_to_eukl(nodelist_p[0].la,nodelist_p[0].lo,x);
		calc_earth_to_eukl(nodelist_p[1].la,nodelist_p[1].lo,x+3);
		s3d_push_vertices(way_obj,x,2);
		s3d_push_line(way_obj, vert,vert+1, 0);
		vert+=2;
		len=sqrt( (x[0]-x[3])*(x[0]-x[3]) + (x[1]-x[4])*(x[1]-x[4]) + (x[2]-x[5])*(x[2]-x[5]));
		if (len>1000.0)
		{
			printf("length of segment is %3.3f\n",len);
			printf("segment id %d: from id %d to id %d\n",waylist_p[i].seg_id,waylist_p[i].node_from,waylist_p[i].node_to);
			printf("segment no %d of way %d: %f %f %f -> ",i, lastid,nodelist_p[0].la, nodelist_p[0].lo, nodelist_p[0].alt);
			printf("%f %f %f\n",nodelist_p[1].la, nodelist_p[1].lo, nodelist_p[1].alt);
		}

	}
*/		
}
void waylist_add(struct waylist *p)
{
	if (waylist_n>=waylist_bufn) {
		waylist_bufn+=64;
		waylist_p=realloc(waylist_p,sizeof(struct waylist)*waylist_bufn);
		nodelist_p=realloc(nodelist_p,sizeof(struct nodelist)*waylist_bufn*2); /* we can have twice as many nodes as there are segments in a graph. */
		adjlist_p=realloc(adjlist_p,sizeof(struct nodelist)*waylist_bufn*2);
	}
	waylist_p[waylist_n].node_to= p->node_to;
	waylist_p[waylist_n].node_from= p->node_from;
	waylist_n++;
}

int way_group(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
	int id=-1;
	struct waylist p;
	p.node_from=-1;
	p.node_to=-1;
	p.seg_id=-1;
	for(i=0; i<argc; i++){
		if (argv[i]) {
			if (0==strcmp(azColName[i],"way_id"))				id=atoi(argv[i]);
			else if (0==strcmp(azColName[i],"node_from"))		p.node_from=atoi(argv[i]);
			else if (0==strcmp(azColName[i],"node_to"))			p.node_to=atoi(argv[i]);
			else if (0==strcmp(azColName[i],"seg_id"))			p.seg_id=atoi(argv[i]);
		}
		/* 	printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");  */
	}
	if (p.node_from==p.node_to)	/* skip */
		return(0);
	if ((lastid!=id) && (id!=-1)) {
		waylist_draw();
		/* flush/draw the list, add new  */
/*		printf("new list: %d\n",id);*/
		waylist_add(&p);
	} else {
		/* add id to the list */
		waylist_add(&p);
	}
	lastid=id;
		
	return 0;
}

void draw_ways(char *filter)
{
	char query[MAXQ];
	snprintf(query,MAXQ,"SELECT * FROM segment WHERE %s ORDER BY way_id;",filter);
/*	snprintf(query,MAXQ,"SELECT DISTINCT way_id,segment.layer_id,node_id,node_from,node_to,longitude,latitude FROM segment JOIN node WHERE %s AND (node.node_id=segment.node_to OR node.node_id=segment.node_from) ORDER BY way_id;",filter);
	printf("query: %s\n",query);*/
	db_exec(query, way_group,0);
	waylist_draw(); /* last way */
	printf("[done]\n");
}
void draw_translate_icon(int user_icon, float la, float lo)
{
	float x[3];
	calc_earth_to_eukl(la,lo,x);
	s3d_translate(user_icon,x[0],x[1],x[2]);
	s3d_rotate(user_icon,(90-la),lo,0);
}
void draw_osm()
{
	draw_ways("segment.layer_id=(SELECT layer_id FROM layer WHERE name='osm')");
}
void draw_all_layers()
{
	draw_osm();
/*	int i;
	for (i=0;i<layerset.n;i++)
		draw_layer(layerset.p[i]);
	*/
}
