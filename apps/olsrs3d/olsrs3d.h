#define NAMEMAX		128	
struct t_node {
	float pos[3], mov[3];
	char name[NAMEMAX];
	int obj,s_obj;
};


/* linked list for the connections */
struct olsr_con {

	struct olsr_con *next_olsr_con;   /* pointer to next connection */
	struct olsr_node *olsr_node;   /* pointer to end point of the connection */
	int obj_id;   /* id of connection object in s3d */
	float etx;

};


/* we contruct a binary tree to handle the nodes */
struct olsr_node {

	struct olsr_node *left;
	struct olsr_node *right;
	char ip[NAMEMAX];   /* host ip */
	int inet_gw;   /* internet gateway flag */
	int inet_gw_modified;   /* internet gateway modified flag */
	float pos_vec[3];   /* position vector in 3d "space" */
	float mov_vec[3];   /* move vector */
	int obj_id;   /* id of node object in s3d */
	int desc_id;   /* id of node description object in s3d */
	struct olsr_con *olsr_con;   /* pointer to first connection */

};

struct Obj_to_ip {
	int id;
	struct olsr_node *olsr_node;
	struct Obj_to_ip *next;
	struct Obj_to_ip *prev;
};



extern struct olsr_node *Root;   																/* top of olsr node tree */
extern struct Obj_to_ip *Obj_to_ip_head, *Obj_to_ip_end,*List_ptr;			/* struct list */
extern int 	max, new_max;
extern float 	*adj;
extern int	*adj_obj;
extern int	Olsr_node_obj;
extern int	Olsr_node_inet_obj;
extern int	node_count;
extern float 	bottom,left;

struct t_node 	*node;
#define MAXLINESIZE 1000 /* lines in a digraph just shouldn't get that longer ... */
#define MAXDATASIZE 100 /* max number of bytes we can get at once  */
extern char lbuf[MAXLINESIZE];
/* process */
int process_init();
int process_main();
int process_quit();
/* net */
int net_init(char *host);
int net_main();
int net_quit();
/* main obj2ip list*/
void lst_add(int id,struct olsr_node **olsr_node);
void lst_del(int id);
void move_lst_ptr(int *id);
struct olsr_node **lst_search(int id);