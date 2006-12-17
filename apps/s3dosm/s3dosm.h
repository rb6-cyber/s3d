#include <sqlite3.h>
#include "../../config.h"
#define	ESIZE	637800		/* earth size */
#define	RESCALE	1
#define VIEWHEIGHT 3
#define MAXQ	4096	
#define QBUF	1024*128

/* stack it */
/* #define DB_STACK	1*/

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
#define OBJECT_T(x)		((object_t *)x)
#define NODE_T(x)		((node_t *)x)
#define SEGMENT_T(x)	((segment_t *)x)
#define WAY_T(x)		((way_t *)x)
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
	char *path;
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
	ID_T 		 id;		/* id of this object */
	ID_T		 layerid;
	ID_T		 tagid;
	int 		 oid;		/* s3d oid */
	int 		 type;		/* type of this object */
	/* avl stuff */
	char 		 bal;
	int			 tag_n;
	tag_t		*tag_p;
	object_t 	*left,*right;
};

struct _adj_t {
	ID_T 		 to,seg;	/* destination and segment to use */
};

struct _node_t {
	object_t	 base;
	float 		 lon;		/* longitude */
	float 		 lat;		/* latitude */
	float 		 alt;		/* altitude */
	char 		 visible;	/* node visible? 0 = no, 1 = yes, 2 = some sepcial object */
	int 		 vid;		/* vertex id */
/*	time_t time;*/
	int 		 adj_n;		/* adjacence list */
	adj_t 		*adj_p;
};
struct _segment_t {
	object_t	 base;
	ID_T		 from;
	ID_T		 to;
};
struct _way_t {
	object_t	 base;
	int 		 seg_n;
	ID_T		*seg_p;
};

/* public functions */

/* object.c */
void 		 object_init(object_t *nobj);
void 		 node_init(node_t *nnode);
void 		 segment_init(segment_t *nsegment);
void 		 way_init(way_t *nway);
object_t 	*object_new(int key);
node_t 		*node_new();
segment_t 	*segment_new();
layer_t 	*layer_new();
way_t 		*way_new();
void 		 node_free(node_t *node);
void 		 segment_free(segment_t *segment);
void 		 way_free(way_t *way);
void 		 layerset_add(layer_t *layer);
/* main.c */
void mainloop();
/* osm.c */
void debug_obj(object_t *obj, void *dummy);
layer_t *parse_osm(char *buf, int length);
layer_t *load_osm_file(char *filename);
layer_t *load_osm_web(float minlon, float minlat, float maxlon, float maxlat);
/* kismet.c */
layer_t *parse_kismet(char *buf, int length);
layer_t *load_kismet_file(char *filename);
/* draw.c */
void draw_all_layers();
int draw_layer(layer_t *layer);
void calc_earth_to_eukl(float lat, float lon, float alt, float *x);
void draw_translate_icon(int user_icon, float la, float lo);
/* nav.c */
void nav_main();
void nav_init();
void nav_center(float la, float lo);
void nav_autocenter();
float get_heading(float la1, float lo1, float la2, float lo2);
extern int oidy;
/* tag.c */
void tag_add(object_t *obj,char *k, char *v);
tag_t *tag_get(object_t *obj, char *k);
void tag_free(tag_t *tag);
/* io.c */
char *read_file(char *fname, int *fsize);
int process_args(int argc, char **argv);
/* db.c */
int db_exec(const char *query, sqlite3_callback callback, void *arg);
int db_add_tag(object_t *obj, char *key, char *val);
int db_gettag(int tagid, char *field, char *target);
int db_getint(void *tagid, int argc, char **argv, char **azColName);
int db_insert_node(node_t *node);
int db_insert_segment(segment_t *seg);
int db_insert_way_only(way_t *way);
int db_insert_way_seg(way_t *way, int seg_n);
int db_insert_object(object_t *obj);
int db_init(char *dbFile);
int db_quit();
int db_create();
int db_insert_layer(char *layer_name);
void db_flush();
int callback(void *NotUsed, int argc, char **argv, char **azColName);
/* gps.c */
int gps_init(char *gpshost);
int gps_main();
int gps_quit();
/* ui.c */
int load_window(char *text);
int load_window_remove();
int load_update_status(float percent);
