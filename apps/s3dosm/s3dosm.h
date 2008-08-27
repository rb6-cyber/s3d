/*
 * s3dosm.h
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

#include <sqlite3.h>
#include <s3d.h> /* s3devt structure */
#include <config-s3d.h>
#define ESIZE 637800  /* earth size */
#define RESCALE 1
#define VIEWHEIGHT 3
#define MAXQ 4096
#define QBUF 1024*128

#ifndef S3DOSMUNUSED
#if defined(UNUSEDPARAM_ATTRIBUTE)
#define S3DOSMUNUSED(x) (x)__attribute__((unused))
#elif defined(UNUSEDPARAM_OMIT)
#define S3DOSMUNUSED(x) /* x */
#else
#define S3DOSMUNUSED(x) x
#endif
#endif

/* stack it */
/* #define DB_STACK 1*/

typedef struct _layer_t layer_t;
typedef struct _adj_t adj_t;
typedef struct _object_t object_t;
typedef struct _node_t node_t;
typedef struct _segment_t segment_t;
typedef struct _layerset_t layerset_t;
typedef struct _way_t way_t;
typedef struct _tag_t tag_t;
typedef struct _icon_t icon_t;
typedef unsigned long ID_T;
#define OBJECT_T(x)  ((object_t *)x)
#define NODE_T(x)  ((node_t *)x)
#define SEGMENT_T(x) ((segment_t *)x)
#define WAY_T(x)  ((way_t *)x)
struct _layerset_t {
	int n;
	layer_t **p;
};
extern layerset_t layerset;

struct _layer_t {
	object_t *tree;
	float center_lo, center_la;
	int visible;
};

struct _icon_t {
	const char *path;
	int oid;
};
enum {
	T_OBJECT,
	T_NODE,
	T_SEGMENT,
	T_WAY
};
enum {
	ICON_AP_OPEN,
	ICON_AP_WEP,
	ICON_AP_WPA,
	ICON_ARROW,
	ICON_NUM
};
enum {
	TAG_UNKNOWN,
	TAG_NAME,
	TAG_N
};
extern icon_t icons[];
typedef void (*avl_func)(object_t *, void *);
struct _tag_t {
	char *k, *v;
	char ttype;
	union {
		char *s;
		int num;
		int b;
		float f;
	} d;
};

struct _object_t {
	ID_T    id;  /* id of this object */
	ID_T   layerid;
	ID_T   tagid;
	int    oid;  /* s3d oid */
	int    type;  /* type of this object */
	/* avl stuff */
	char    bal;
	int    tag_n;
	tag_t  *tag_p;
	object_t  *left, *right;
};

struct _adj_t {
	ID_T    to, seg; /* destination and segment to use */
};

struct _node_t {
	object_t  base;
	float    lon;  /* longitude */
	float    lat;  /* latitude */
	float    alt;  /* altitude */
	char    visible; /* node visible? 0 = no, 1 = yes, 2 = some sepcial object */
	int    vid;  /* vertex id */
	/* time_t time;*/
	int    adj_n;  /* adjacence list */
	adj_t   *adj_p;
};
struct _segment_t {
	object_t  base;
	ID_T   from;
	ID_T   to;
};
struct _way_t {
	object_t  base;
	int    seg_n;
	ID_T  *seg_p;
};

/* public functions */

/* object.c */
void    object_init(object_t *nobj);
void    object_free(object_t *obj);
void    node_init(node_t *nnode);
void    segment_init(segment_t *nsegment);
void    way_init(way_t *nway);
object_t  *object_new(int key);
node_t   *node_new(void);
segment_t  *segment_new(void);
layer_t  *layer_new(void);
way_t   *way_new(void);
void    node_free(node_t *node);
void    segment_free(segment_t *segment);
void    way_free(way_t *way);
void    layerset_add(layer_t *layer);
/* main.c */
void mainloop(void);
/* osm.c */
void debug_obj(object_t *obj, void *dummy);
layer_t *parse_osm(const char *buf, int length);
layer_t *load_osm_file(const char *filename);
layer_t *load_osm_web(float minlon, float minlat, float maxlon, float maxlat);
/* kismet.c */
layer_t *parse_kismet(const char *buf, int length);
layer_t *load_kismet_file(const char *filename);
/* draw.c */
void draw_all_layers(void);
int draw_layer(layer_t *layer);
void calc_earth_to_eukl(float lat, float lon, float alt, float *x);
void draw_translate_icon(int user_icon, float la, float lo);
/* nav.c */
void nav_main(void);
void nav_init(void);
void nav_center(float la, float lo);
void nav_autocenter(void);
void nav_campos(float campos[3], float earthpos[3]);
float get_heading(float la1, float lo1, float la2, float lo2);
extern int oidy;
/* tag.c */
void tag_add(object_t *obj, const char *k, char *v);
tag_t *tag_get(object_t *obj, const char *k);
void tag_free(tag_t *tag);
/* io.c */
char *read_file(const char *fname, int *fsize);
int process_args(int argc, char **argv);
/* db.c */
int db_olsr_node_init(float *pos);
int db_olsr_check(const char *ip, float *pos);
int db_exec(const char *query, sqlite3_callback callback, const void *arg);
int db_add_tag(object_t *obj, const char *key, const char *val);
int db_gettag(int tagid, const char *field, char *target);
int db_getint(void *tagid, int argc, char **argv, char **azColName);
int db_getpoint(void *data, int argc, char **argv, char **azColName);
int db_insert_node(node_t *node);
int db_insert_segment(segment_t *seg);
int db_insert_way_only(way_t *way);
int db_insert_way_seg(way_t *way, int seg_n);
int db_insert_object(object_t *obj);
int db_init(const char *dbFile);
int db_quit(void);
int db_create(void);
int db_insert_layer(const char *layer_name);
void db_flush(void);
int callback(void *NotUsed, int argc, char **argv, char **azColName);
/* gps.c */
int gps_init(const char *gpshost);
int gps_main(void);
int gps_quit(void);
/* ui.c */
int ui_init(void);
int load_window(const char *text);
int load_window_remove(void);
int load_update_status(float percent);
/* olsrs3d.c */
#define NODEHEIGHT 10
int olsr_object_click(struct s3d_evt *evt);
int olsr_object_info(struct s3d_evt *hrmz);
int olsr_parse_args(int argc, char **argv);
int olsr_keypress(struct s3d_evt *event);
void olsr_main(void);
int olsr_init(void);
int olsr_quit(void);
int olsr_parse_args(int argc, char **argv);
