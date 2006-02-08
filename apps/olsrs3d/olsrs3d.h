#define NAMEMAX		128
struct t_node {
	float pos[3], mov[3];
	char name[NAMEMAX];
	int obj,s_obj;
};

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
