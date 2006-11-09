#define	ESIZE	637800		/* earth size */
#define	RESCALE	1

typedef struct _layer_t layer_t;
typedef struct _adj_t adj_t;
typedef struct _object_t object_t;
typedef struct _node_t node_t;
typedef struct _segment_t segment_t;
typedef struct _way_t way_t;
typedef struct _tag_t tag_t;
typedef struct _icon_t icon_t;
typedef unsigned long ID_T;
#define OBJECT_T(x)		((object_t *)x)
#define NODE_T(x)		((node_t *)x)
#define SEGMENT_T(x)	((segment_t *)x)
#define WAY_T(x)		((way_t *)x)

struct _layer_t {
	object_t *tree;
	float center_lo, center_la;
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
	ICON_AP,
	ICON_AP_OPEN,
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
	double 		 lon;		/* longitude */
	double 		 lat;		/* latitude */
	double 		 alt;		/* altitude */
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
/* main.c */
char *read_file(char *fname, int *fsize);
/* osm.c */
void debug_obj(object_t *obj, void *dummy);
layer_t *parse_osm(char *buf, int length);
layer_t *load_osm_file(char *filename);
layer_t *load_osm_web(float minlon, float minlat, float maxlon, float maxlat);
/* kismet.c */
layer_t *parse_kismet(char *buf, int length);
layer_t *load_kismet_file(char *filename);
void 		 avl_tree_trav(object_t *t, avl_func func, void *data);
object_t 	*avl_find(object_t *t, int val);
object_t 	*avl_rotate_right(object_t *t);
object_t	*avl_rotate_left(object_t *t);
object_t	*avl_insert(object_t *t, object_t *nn);
object_t 	*avl_leftmost(object_t *t);
object_t 	*avl_rightmost(object_t *t);
object_t 	*avl_remove(object_t *t, object_t *nn);
int 		 avl_height(object_t *t);
int 		 draw_layer(layer_t *layer);
/* draw.c */
void calc_earth_to_eukl(double lon, double lat, double *x);
int draw_layer(layer_t *layer);
/* nav.c */
void nav_init();
void nav_center(float la, float lo);
extern int oidy;
/* tag.c */
void tag_add(object_t *obj,char *k, char *v);
tag_t *tag_get(object_t *obj, char *k);
