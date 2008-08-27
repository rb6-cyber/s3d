/*
 * draw.c
 *
 * Copyright (C) 2006-2008 Simon Wunderlich <dotslash@packetmixer.de>
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
#include <math.h> /* sin(), cos() */
#include <stdio.h> /* printf() */
#include <string.h> /* strcmp() */
#include <stdlib.h> /* atoi(),malloc(), calloc(), free() */

static float temp;
#define  V_COPY(a,b)  a[0]=b[0]; a[1]=b[1]; a[2]=b[2];
#define  V_ADD(a,b,c) c[0]=a[0]+b[0]; c[1]=a[1]+b[1]; c[2]=a[2]+b[2];
#define  V_SUB(a,b,c) c[0]=a[0]-b[0]; c[1]=a[1]-b[1]; c[2]=a[2]-b[2];
#define  V_DOT(a,b)  a[0]*b[0] + a[1]*b[1] + a[2] * b[2]
#define  V_CROSS(a,b,c) c[0]=a[1]*b[2] - a[2]*b[1];  c[1]=a[2]*b[0] - a[0]*b[2];  c[2]=a[0]*b[1] - a[1]*b[0];
#define  V_LEN(a)  sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2])
#define  V_SCAL(a,s)  a[0]=s*a[0]; a[1]=s*a[1]; a[2]=s*a[2];
#define  V_NORM(a)  temp=V_LEN(a); V_SCAL(a,1/temp);


static int num_max;  /* how many results in our query? to calculate pecent done ... */
static int num_done; /* how many already done */

static int lastid = -1;
/* list element of a segment which is on our way */
struct waylist {
	int node_from, node_to;
	int node_from_int, node_to_int;
	int seg_id;
	int node_from_l, node_from_r; /* vertex id's for corners */
	int node_to_l, node_to_r;
};
/* list element of a node which is to be drawn */
struct nodelist {
	int node_id;   /* (external counting) */
	float la, lo, alt;  /* earth coords */
	float x[3];    /* euclid coords */
	float normal[3];
	float len;
};
/* list element for adjacent nodes */
struct adjlist {
	int node_id;   /* node to which the segment leads to */
	int seg_id;    /* segment which is involved to the node (both internal counting) */
};

static struct waylist  *waylist_p = NULL;
static struct nodelist *nodelist_p = NULL;
static struct adjlist *adjlist_p = NULL;
static int    nodelist_n = 0;
static int    adjlist_n = 0;
static int    waylist_n = 0;
static int    waylist_bufn = 0;


void calc_earth_to_eukl(float lat, float lon, float alt, float *x)
{
	float la, lo;
	la = lat * M_PI / 180.0;
	lo = lon * M_PI / 180.0;
	x[0] = (ESIZE + alt) * sin(lo) * cos(la);
	x[1] = (ESIZE + alt) *   sin(la);
	x[2] = (ESIZE + alt) * cos(lo) * cos(la);
}
static int draw_icon(void *S3DOSMUNUSED(data), int argc, char **argv, char **S3DOSMUNUSED(azColName))
{
	int i, tagid = -1, oid;
	/* char query[MAXQ];*/
	char s[MAXQ];
	float la, lo;
	float x[3];
	la = lo = 0.0;
	num_done++;
	for (i = 0; i < argc; i++) {
		if (argv[i]) {
			if (0 == strcmp(azColName[i], "longitude"))   lo = strtod(argv[i], NULL);
			else if (0 == strcmp(azColName[i], "latitude"))  la = strtod(argv[i], NULL);
			else if (0 == strcmp(azColName[i], "tag_id"))    tagid = atoi(argv[i]);
		}
	}
	if (0 == db_gettag(tagid, "amenity", s)) {
		oid = -1;
		if (0 == strcmp(s, "wifi")) {  /* some wifi icon */
			if (0 == db_gettag(tagid, "wifi_type", s)) {
				if (0 == strcmp(s, "infrastructure")) { /* access point */
					if (0 == db_gettag(tagid, "wifi_wep", s)) {
						if (0 == strcmp(s, "true")) oid = s3d_clone(icons[ICON_AP_WPA].oid);
						else      oid = s3d_clone(icons[ICON_AP_OPEN].oid);
					} else oid = s3d_clone(icons[ICON_AP_OPEN].oid); /* assuming open ap */
				}
			}
		}
		if (oid != -1) {
			calc_earth_to_eukl(la, lo, 0, x);
			s3d_translate(oid, x[0], x[1], x[2]);
			s3d_rotate(oid, (90 - la), lo, 0);
			s3d_link(oid, oidy);
			s3d_flags_on(oid, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
			load_update_status((100.0*num_done) / (float)num_max);
		}

	}
	return(0);
}
/* just fetches node information and puts in the nodelist */
static int insert_node(void *data, int argc, char **argv, char **azColName)
{
	struct nodelist *np = (struct nodelist *)data; /* get the nodepointer */
	int i;
	for (i = 0; i < argc; i++) {
		if (argv[i]) {
			if (0 == strcmp(azColName[i], "longitude"))   np[nodelist_n].lo = strtod(argv[i], NULL);
			else if (0 == strcmp(azColName[i], "latitude"))  np[nodelist_n].la = strtod(argv[i], NULL);
			else if (0 == strcmp(azColName[i], "altitude"))  np[nodelist_n].alt = strtod(argv[i], NULL);
		}
	}
	return(0);
}
static int select_waytype(void *data, int argc, char **argv, char **S3DOSMUNUSED(azColName))
{
	int i;
	for (i = 0; i < argc; i++) {
		if (argv[i]) {
			if (0 == strcmp(argv[i], "motorway"))    *((int *) data) = 5;
			else if (0 == strcmp(argv[i], "motorway_link")) *((int *) data) = 4;
			else if (0 == strcmp(argv[i], "primary"))   *((int *) data) = 3;
			else if (0 == strcmp(argv[i], "secondary"))  *((int *) data) = 2;
			else if (0 == strcmp(argv[i], "residential"))  *((int *) data) = 1;
		}
	}
	return(0);
}
/* draw waylist, clear the queue */
static void waylist_draw(const char *filter)
{
	float len;
	char query[MAXQ];
	int i, j, k, vert = 0;
	int node_id;
	int way_obj;
	int waytype = 0;
	int adj_seg;
	float a[3], b[3], *left, *right, *swap;
	float street_width; /* dynamically adjust? */
	float an[3];  /* normal on the plane, orthogonal on the right side of the left segment */
	float n[3];   /* the direction vector in which the intersecion should be placed */
	float s[3];   /* intersection point */
	float point_zero[3]; /* we use point_zero so we don't have very big bounding spheres in s3d and speed up picking */
	float n_len, scale;

	if (waylist_n == 0) /* no nodes, no fun */
		return;
	/* printf("way: %d - %d segments\n",lastid,waylist_n);*/
	way_obj = s3d_new_object();
	if (lastid != -1) {
		snprintf(query, MAXQ, "SELECT tagvalue FROM tag WHERE tag_id=(SELECT tag_id FROM way WHERE way_id=%d AND %s) AND tagkey='highway';", lastid, filter);
		db_exec(query, select_waytype, &waytype);
	}
	switch (waytype) {
	case 5:
		s3d_push_material(way_obj, 0.2, 0.2, 0.6,  1.0, 1.0, 1.0, 0.3, 0.3, 1.0); /* motorway */
	case 4:
		s3d_push_material(way_obj, 0.3, 0.3, 0.4,  1.0, 1.0, 1.0, 0.5, 0.5, 0.8); /* motorway_link*/
	case 3:
		s3d_push_material(way_obj, 0.6, 0.3, 0.1,  1.0, 1.0, 1.0,  1.0, 0.6, 0.2); /* primary */
	case 2:
		s3d_push_material(way_obj, 0.6, 0.6, 0.0,  1.0, 1.0, 1.0,  1.0, 1.0, 0.0); /* secondary */
	case 1:
		s3d_push_material(way_obj, 0.6, 0.6, 0.6,  1.0, 1.0, 1.0,  1.0, 1.0, 1.0); /* residential */
	default:
		s3d_push_material(way_obj, 0.6, 0.2, 0.6,  1.0, 1.0, 1.0, 1.0, 0.5, 1.0); /* default */
	}
	street_width = (0.5 + waytype / 10) / RESCALE;
	/* put nodes of the graph into a list */
	nodelist_n = 0;
	for (i = 0;i < waylist_n*2;i++) {
		if (i % 2)    node_id = waylist_p[i/2].node_from;
		else     node_id = waylist_p[i/2].node_to;
		for (j = 0;j < nodelist_n;j++)
			if (nodelist_p[j].node_id == node_id) break;
		if (j == nodelist_n) { /* we still need to add this node */
			/*   printf("[way %d] add node %d to nodelist as %d\n",lastid, node_id, nodelist_n);*/
			nodelist_p[j].node_id = node_id;
			snprintf(query, MAXQ, "SELECT longitude, latitude, altitude FROM node WHERE %s AND node_id=%d;", filter, node_id);
			db_exec(query, insert_node, (void *)(nodelist_p));
			calc_earth_to_eukl(nodelist_p[j].la, nodelist_p[j].lo, 0, nodelist_p[j].x); /* elevate higher priority streets a little bit ... */
			len = sqrt(nodelist_p[j].x[0] * nodelist_p[j].x[0] + nodelist_p[j].x[1] * nodelist_p[j].x[1] + nodelist_p[j].x[2] * nodelist_p[j].x[2]);
			nodelist_p[j].normal[0] = nodelist_p[j].x[0] / len;
			nodelist_p[j].normal[1] = nodelist_p[j].x[1] / len;
			nodelist_p[j].normal[2] = nodelist_p[j].x[2] / len;
			nodelist_n++;
		}
		if (i % 2)    waylist_p[i/2].node_from_int = j;
		else     waylist_p[i/2].node_to_int = j;
	}
	V_COPY(point_zero, nodelist_p[0].x);
	/* iterate for all nodes */
	for (i = 0;i < nodelist_n;i++) {
		/* find adjacent segments */
		adjlist_n = 0;
		node_id = nodelist_p[i].node_id;
		for (j = 0;j <= waylist_n;j++) {
			if (waylist_p[j].node_from == node_id) {
				adjlist_p[adjlist_n].node_id = waylist_p[j].node_to_int;
				adjlist_p[adjlist_n].seg_id = j;
				adjlist_n++;
			} else  if (waylist_p[j].node_to == node_id) {
				adjlist_p[adjlist_n].node_id = waylist_p[j].node_from_int;
				adjlist_p[adjlist_n].seg_id = j;
				adjlist_n++;
			}
		}

		if (adjlist_n > 1) { /* more than one adjacent, need to order and calculate intersections */
			if (adjlist_n > 2) { /* no ordering needed for 2 incoming segments */
				for (j = 0;j < adjlist_n - 2;j++)
					for (k = j + 2;k < adjlist_n;k++) {
						float test[3], normal[3], linevector[3];
						/* (re)calc test direction */
						V_SUB(nodelist_p[adjlist_p[j].node_id].x, nodelist_p[adjlist_p[j+1].node_id].x, linevector);
						V_CROSS(nodelist_p[adjlist_p[j].node_id].normal, linevector,  normal); /* normal should look outside of our circle now. */
						while (k < adjlist_n) {
							/* determine on which side the point is. if its between our testvector, we'll need to swap. */
							V_SUB(nodelist_p[adjlist_p[j].node_id].x, nodelist_p[adjlist_p[k].node_id].x, test);
							if (s3d_vector_dot_product(normal, test) > 0) { /* same side, means adjacent line k is nearer to our point j
                   than our point j+1 which is supposed to be the nearest point,
                   so we swap them and call a break to get the new test-normal */
								struct adjlist swap;
								memcpy(&swap, &(adjlist_p[j+1]), sizeof(struct adjlist));
								memcpy(&(adjlist_p[j+1]), &(adjlist_p[k]), sizeof(struct adjlist));
								memcpy(&(adjlist_p[k]), &swap, sizeof(struct adjlist));
								break;
							}
							k++;
						}
					}
			}
			left = a;
			right = b;
			V_SUB(nodelist_p[adjlist_p[0].node_id].x, nodelist_p[i].x, right);
			V_NORM(right);


			for (j = 0;j < adjlist_n;j++) {
				swap = left;
				left = right; /* use last right segment as new left segment */
				right = swap; /* get space for the next right segment */
				V_SUB(nodelist_p[adjlist_p[(j+1)%adjlist_n].node_id].x, nodelist_p[i].x, right);
				V_NORM(right);
				V_CROSS(nodelist_p[i].normal, left , an); /* an is also normalized, as first and second argument are already length 1 */
				V_ADD(left, right, n);      /* direction which our intersection is */

				V_CROSS(nodelist_p[i].normal, n, s);
				V_CROSS(s, nodelist_p[i].normal, n); /* get n on the plane which is spanned by the points normal */

				n_len = V_LEN(n);

				V_COPY(s, nodelist_p[i].x); /* s = P + (street_width/ ( n * an)) * n */
				V_SCAL(n, 1 / n_len); /* normalize n first! */
				scale = V_DOT(n, an); /* get cos (alpha/2), alpha is opposite angel of left and right segment */

				if ((n_len < 0.1) || (fabs(scale) < 0.1)) { /* too low, don't use, just have intersection 90 degree of it. */
					V_SCAL(an, -street_width);  /* S = P + street_width * an */
					V_ADD(nodelist_p[i].x, an, s);

				} else {
					V_SCAL(n, -street_width / scale);
					V_ADD(s, n, s);
				}


				/*    printf("calc intersection: %3.3f %3.3f %3.3f\n",s[0],s[1],s[2]);*/
				V_SUB(s, point_zero, s);
				s3d_push_vertices(way_obj, s, 1);
				adj_seg = adjlist_p[j].seg_id;  /* left segment */
				if (i == waylist_p[adj_seg].node_from_int) waylist_p[adj_seg].node_from_r = vert;
				else         waylist_p[adj_seg].node_to_l = vert;
				adj_seg = adjlist_p[(j+1)%adjlist_n].seg_id; /* right segment */
				if (i == waylist_p[adj_seg].node_from_int) waylist_p[adj_seg].node_from_l = vert;
				else         waylist_p[adj_seg].node_to_r = vert;
				vert++;
			}
			if (adjlist_n >= 3) {
				/* we know that the last adjlist_n vertices set belong to our intersection here .. */
				for (j = vert - adjlist_n + 1;j < (vert - 1);j++)
					s3d_push_polygon(way_obj, vert - adjlist_n, j, j + 1, 0);
			}
		} else {
			/* endpoint */
			V_SUB(nodelist_p[adjlist_p[0].node_id].x, nodelist_p[i].x, a);
			V_NORM(a);
			V_CROSS(nodelist_p[i].normal, a , an); /* an is also normalized, as first and second argument are already length 1 */
			V_SCAL(an, street_width);

			V_COPY(s, nodelist_p[i].x);
			V_ADD(s, an, s);
			V_SUB(s, point_zero, s);
			s3d_push_vertices(way_obj, s, 1);
			j = vert;
			vert++;
			V_SCAL(an, -1);
			V_COPY(s, nodelist_p[i].x);
			V_ADD(s, an, s);
			V_SUB(s, point_zero, s);
			s3d_push_vertices(way_obj, s, 1);
			k = vert;
			vert++;

			adj_seg = adjlist_p[0].seg_id;
			if (i == waylist_p[adj_seg].node_from_int) {
				waylist_p[adj_seg].node_from_l = j;
				waylist_p[adj_seg].node_from_r = k;
			} else {
				waylist_p[adj_seg].node_to_l = k;
				waylist_p[adj_seg].node_to_r = j;
			}
		}
	}
	for (i = 0;i < waylist_n;i++) {
		uint32_t polys[8];
		/* printf("drawing way from points %d %d %d %d\n",waylist_p[i].node_from_l, waylist_p[i].node_to_l, waylist_p[i].node_to_r,waylist_p[i].node_from_r);*/
		polys[0] = waylist_p[i].node_from_l;
		polys[1] = waylist_p[i].node_to_l;
		polys[2] = waylist_p[i].node_to_r;
		polys[3] = 0;
		polys[4] = waylist_p[i].node_from_l;
		polys[5] = waylist_p[i].node_to_r;
		polys[6] = waylist_p[i].node_from_r;
		polys[7] = 0;

		s3d_push_polygons(way_obj, polys, 2);
	}
	s3d_translate(way_obj, point_zero[0], point_zero[1], point_zero[2]);
	s3d_link(way_obj, oidy);
	s3d_flags_on(way_obj, S3D_OF_VISIBLE | S3D_OF_SELECTABLE);
	snprintf(query, MAXQ, "UPDATE way SET s3doid=%d WHERE way_id=%d AND %s;", way_obj, lastid, filter);
	db_exec(query, NULL, NULL);

	waylist_n = 0;

	load_update_status((100.0*num_done) / (float)num_max);
}
static void waylist_add(struct waylist *p)
{
	if (waylist_n >= waylist_bufn) {
		waylist_bufn += 64;
		waylist_p = (struct waylist *)realloc(waylist_p, sizeof(struct waylist) * waylist_bufn);
		nodelist_p = (struct nodelist *)realloc(nodelist_p, sizeof(struct nodelist) * waylist_bufn * 2); /* we can have twice as many nodes as there are segments in a graph. */
		adjlist_p = (struct adjlist *)realloc(adjlist_p, sizeof(struct nodelist) * waylist_bufn * 2);
	}
	waylist_p[waylist_n].node_to = p->node_to;
	waylist_p[waylist_n].node_from = p->node_from;
	waylist_n++;
}

static int way_group(void *data, int argc, char **argv, char **azColName)
{
	int i;
	int id = -1;
	struct waylist p;
	char *filter = (char *)data;
	num_done++;
	p.node_from = p.node_to = 0;
	p.node_to = -1;
	p.seg_id = -1;
	for (i = 0; i < argc; i++) {
		if (argv[i]) {
			if (0 == strcmp(azColName[i], "way_id"))    id = atoi(argv[i]);
			else if (0 == strcmp(azColName[i], "node_from"))  p.node_from = atoi(argv[i]);
			else if (0 == strcmp(azColName[i], "node_to"))   p.node_to = atoi(argv[i]);
			else if (0 == strcmp(azColName[i], "seg_id"))   p.seg_id = atoi(argv[i]);
		}
	}
	if (p.node_from == p.node_to) /* skip */
		return(0);
	if ((lastid != id) && (id != 0)) {
		waylist_draw(filter);
		/* flush/draw the list, add new  */
		waylist_add(&p);
	} else {
		/* add id to the list */
		waylist_add(&p);
	}
	lastid = id;
	return 0;
}
void draw_translate_icon(int user_icon, float la, float lo)
{
	float x[3];
	calc_earth_to_eukl(la, lo, 1 / RESCALE, x);
	s3d_translate(user_icon, x[0], x[1], x[2]);
	s3d_rotate(user_icon, (90 - la), lo, 0);
}

static void draw_ways(const char *filter)
{
	char query[MAXQ];
	num_done = 0;
	snprintf(query, MAXQ, "SELECT count(seg_id) FROM segment WHERE %s;", filter);
	db_exec(query, db_getint, &num_max);
	snprintf(query, MAXQ, "SELECT * FROM segment WHERE %s ORDER BY way_id;", filter);
	db_exec(query, way_group, filter);
	waylist_draw(filter); /* last way */
}
static void draw_osm(void)
{
	load_window("Drawing Card ...");
	draw_ways("layer_id=(SELECT layer_id FROM layer WHERE name='osm')");
}
static void draw_kismet(void)
{
	char query[MAXQ];
	char filter[] = "layer_id=(SELECT layer_id FROM layer WHERE name='kismet')";
	load_window("Drawing Access Points ...");
	num_done = 0;
	snprintf(query, MAXQ, "SELECT count(node_id) FROM node WHERE %s;", filter);
	db_exec(query, db_getint, &num_max);
	snprintf(query, MAXQ, "SELECT * FROM node WHERE %s;", filter);
	db_exec(query, draw_icon, filter);
}
void draw_all_layers(void)
{
	draw_osm();
	draw_kismet();
	load_window_remove();
}
