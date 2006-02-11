#include <stdio.h>
#include <s3d.h>
#include <unistd.h>	/* sleep() */
#include <string.h>	/* strncpy() */
#include <math.h>		/* sqrt() */
#include <getopt.h>	/* getopt() */
#include "olsrs3d.h"
#define SPEED		10.0

int Debug = 0;

char Olsr_host[256];   											/* ip or hostname of olsr node with running dot_draw plugin */
struct olsr_node *Root = NULL;   							/* top of olsr node tree */
struct Obj_to_ip *Obj_to_ip_head, *Obj_to_ip_end,*List_ptr;			/* needed pointer for linked list */

int node_count=-1;
int alpha=0;
int Olsr_node_obj,Olsr_node_inet_obj,mesh;
float asp=1.0;
float bottom=-1.0;
float left=-1.0;

float CamPosition[2][3];													/* CamPosition[trans|rot][x-z] */
float ZeroPosition[3] = {0,0,0};										/* current position zero position */
int ZeroPoint;																	/* object zeropoint */




/***
 *
 * print usage info
 *
 ***/

void print_usage( void ) {

	printf( "Usage is olsrs3d [options] [-- [s3d options]]\n" );
	printf( "olsrs3d options:\n" );
	printf( "   -h\tprint this short help\n" );
	printf( "   -d\tenable debug mode\n" );
	printf( "   -H\tconnect to olsr node [default: localhost]\n" );
	s3d_usage();

}



/***
 *
 * print error and exit
 *
 ***/

void out_of_mem( void ) {

	printf( "Sorry - you ran out of memory !\n" );
	exit(8);

}



/***
 *
 * calculate distance between 2 vectors => http://en.wikipedia.org/wiki/Euclidean_distance
 *
 *   p1   =>   vector of node 1
 *   p2   =>   vector of node 2
 *
 *   return distance
 *
 ***/

float dist(float p1[], float p2[])
{
	float p[3];
	p[0]=p1[0]-p2[0];
	p[1]=p1[1]-p2[1];
	p[2]=p1[2]-p2[2];
	return (sqrt(p[0]*p[0]   +  p[1]*p[1]  +  p[2]*p[2]));

}



/***
 *
 * calculate distance between 2 vectors and substract vector1 from vector2
 *  => http://en.wikipedia.org/wiki/Vector_%28spatial%29#Vector_addition_and_subtraction
 *
 *   p1   =>   vector of node 1
 *   p2   =>   vector of node 2
 *
 *   return distance
 *
 ***/

float dirt(float p1[], float p2[], float p3[])
{
	float d;
	d=dist(p1,p2);
	if (d!=0.0)
	{
		p3[0]=p2[0]-p1[0];
		p3[1]=p2[1]-p1[1];
		p3[2]=p2[2]-p1[2];
	} else {
		p3[0]=p2[0]=p1[0]=0.0;
	}
	return(d);
}



/***
 *
 * calculate new movement of node by adding the product of the factor and the vector to the movement vector
 *  => http://en.wikipedia.org/wiki/Vector_%28spatial%29#Scalar_multiplication
 *
 *   mov  =>   current mov vector
 *   p    =>   vector of node
 *   fac  =>   factor which is
 *
 ***/

void mov_add(float mov[], float p[], float fac)
{
/*	if (fac>1000)
		return;
	fac=1000; */
	mov[0]+=fac*p[0];
	mov[1]+=fac*p[1];
	mov[2]+=fac*p[2];
}



/***
 *
 * check whether is a new / modified node and handle it accordingly
 *
 *   *olsr_node =>   pointer to current olsr_node
 *
 ***/

void handle_olsr_node( struct olsr_node *olsr_node ) {

	float f, distance;
	float tmp_mov_vec[3];
	struct olsr_con **olsr_con;
	
	/* no more nodes left */
	if ( olsr_node == NULL ) return;

	/* olsr node shape has been modified */
	if ( olsr_node->inet_gw_modified ) {

		/* delete old shape */
		if ( olsr_node->obj_id != -1 ) {
			lst_del(olsr_node->obj_id);
			s3d_del_object( olsr_node->obj_id );
		}

		if ( olsr_node->desc_id != -1 ) s3d_del_object( olsr_node->desc_id );

		/* create new shape */
		if ( olsr_node->inet_gw ) {
			/* olsr node offers internet access */
			olsr_node->obj_id = s3d_clone( Olsr_node_inet_obj );
			s3d_link(olsr_node->obj_id,ZeroPoint);
		} else {
			/* normal olsr node */
			olsr_node->obj_id = s3d_clone( Olsr_node_obj );
			s3d_link(olsr_node->obj_id,ZeroPoint);
		}

		s3d_flags_on( olsr_node->obj_id, S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
		/* add object_id and olsr_node to linked list */
		lst_add(olsr_node->obj_id,&olsr_node);
		
		/* create olsr node text and attach (link) it to the node */
		olsr_node->desc_id = s3d_draw_string( olsr_node->ip, &f );
		s3d_link( olsr_node->desc_id, olsr_node->obj_id );
		s3d_translate( olsr_node->desc_id, -f/2,-2,0 );
		s3d_flags_on( olsr_node->desc_id, S3D_OF_VISIBLE );

		olsr_node->inet_gw_modified = 0;

	}

	/* reset movement vector */
	olsr_node->mov_vec[0] = olsr_node->mov_vec[1] = olsr_node->mov_vec[2] = 0.0;

	/* calculate new movement vector */
	olsr_con = &olsr_node->olsr_con;

	while ( (*olsr_con) != NULL ) {

		distance = dirt( olsr_node->pos_vec, (*olsr_con)->olsr_node->pos_vec, tmp_mov_vec );
		/* f = (*olsr_con)->etx / distance; */
		f = (*olsr_con)->etx * 5.0 / distance;
		if ( f < 0.3 ) f = 0.3;
		mov_add( olsr_node->mov_vec, tmp_mov_vec, 1/f-1);

		olsr_con = &(*olsr_con)->next_olsr_con;

	}

	/* move it */
	mov_add( olsr_node->pos_vec, olsr_node->mov_vec, 0.1 );
	s3d_translate( olsr_node->obj_id, olsr_node->pos_vec[0], olsr_node->pos_vec[1], olsr_node->pos_vec[2] );

	olsr_con = &olsr_node->olsr_con;
	while ( (*olsr_con) != NULL ) {

		s3d_pop_vertex( (*olsr_con)->obj_id, 6 );
		s3d_pop_polygon( (*olsr_con)->obj_id, 2 );

		s3d_push_vertex( (*olsr_con)->obj_id, olsr_node->pos_vec[0]+ ZeroPosition[0], olsr_node->pos_vec[1]+ ZeroPosition[1], olsr_node->pos_vec[2]+ ZeroPosition[2] );
		s3d_push_vertex( (*olsr_con)->obj_id, olsr_node->pos_vec[0]+0.2+ ZeroPosition[0], olsr_node->pos_vec[1]+ ZeroPosition[1], olsr_node->pos_vec[2]+ ZeroPosition[2] );
		s3d_push_vertex( (*olsr_con)->obj_id, olsr_node->pos_vec[0]-0.2+ ZeroPosition[0], olsr_node->pos_vec[1]+ ZeroPosition[1], olsr_node->pos_vec[2]+ ZeroPosition[2] );

		s3d_push_vertex( (*olsr_con)->obj_id, (*olsr_con)->olsr_node->pos_vec[0]+ ZeroPosition[0], (*olsr_con)->olsr_node->pos_vec[1]+ ZeroPosition[1], (*olsr_con)->olsr_node->pos_vec[2]+ ZeroPosition[2] );
		s3d_push_vertex( (*olsr_con)->obj_id, (*olsr_con)->olsr_node->pos_vec[0]+ ZeroPosition[0], (*olsr_con)->olsr_node->pos_vec[1]+0.2+ ZeroPosition[1], (*olsr_con)->olsr_node->pos_vec[2] + ZeroPosition[2]);
		s3d_push_vertex( (*olsr_con)->obj_id, (*olsr_con)->olsr_node->pos_vec[0]+ ZeroPosition[0], (*olsr_con)->olsr_node->pos_vec[1]-0.2+ ZeroPosition[1], (*olsr_con)->olsr_node->pos_vec[2] + ZeroPosition[2]);

		s3d_push_polygon( (*olsr_con)->obj_id, 0,4,5,0);
		s3d_push_polygon( (*olsr_con)->obj_id, 3,1,2,0);

		olsr_con = &(*olsr_con)->next_olsr_con;

	}

	handle_olsr_node( olsr_node->left );
	handle_olsr_node( olsr_node->right );

}

void mainloop()
{
	int i,j,o,r;
	float d,gd,f,m[3];
	float z[3]={0,0,0};
// 	for (i=0;i<max;i++)
// 	{
// 		node[i].mov[0]=
// 		node[i].mov[1]=
// 		node[i].mov[2]=0.0;
// 	}

	// prepare nodes
	handle_olsr_node( Root );

	/*	for (i=0;i<max;i++)*/
	/*	{*/
	/*		for (j=i+1;j<max;j++)*/
	/*		{*/
	/*			if (i!=j)*/
	/*			{*/
	/*				gd=adj[i*max+j];*/
	/*				d=dirt(node[i].pos,node[j].pos,m);*/
	/*				if (gd==0.0)*/	/* points are not connected */
	/*				{*/
/*					printf("distance between i and j: %f\n",d);*/
	/*					if (d<0.1) d=0.1;*/
	/*					mov_add(node[j].mov,m,100/(d*d));*/
	/*					mov_add(node[i].mov,m,-100/(d*d));*/
	/*				} else {*/ /* connected!! */

	/*					f=(gd)/d;*/
	/*					if (f<0.3) f=0.3;*/
	/*					mov_add(node[i].mov,m,1/f-1);*/
	/*					mov_add(node[j].mov,m,-(1/f-1));*/
/*					printf("distance between %d and %d: %f / %f = %f\n",i,j,gd,d,f);*/
	/*				}*/
	/*			}*/
	/*		}*/
	/*		d=dirt(node[i].pos,z,m);*/
/*		mov_add(node[i].mov,m,d/100); * move a little bit to point zero */
/*		mov_add(node[i].mov,m,1); * move a little bit to point zero */
/*	}
	/* move it!! */
// 	for (i=0;i<max;i++)
// 	{
/*		printf("applying move vector for point %d: %f:%f:%f\n",i,node[i].mov[0],node[i].mov[1],node[i].mov[2]);*/
// 		if ((d=dist(node[i].mov,z))>10.0)
// 			mov_add(node[i].pos,node[i].mov,1.0/((float )d)); /* normalize */
// 		else
// 			mov_add(node[i].pos,node[i].mov,0.1);
// 		s3d_translate(node[i].obj,node[i].pos[0],node[i].pos[1],node[i].pos[2]);
// 		for (j=i+1;j<max;j++)
// 			if ((o=adj_obj[max*i+j])!=-1)
// 			{
// 				s3d_pop_vertex(o,6);
/*				s3d_pop_polygon(o,2);*/
// 				s3d_push_vertex(o,node[i].pos[0],	 node[i].pos[1],node[i].pos[2]);
// 				s3d_push_vertex(o,node[i].pos[0]+0.2,node[i].pos[1],node[i].pos[2]);
// 				s3d_push_vertex(o,node[i].pos[0]-0.2,node[i].pos[1],node[i].pos[2]);

// 				s3d_push_vertex(o,node[j].pos[0],	 node[j].pos[1],node[j].pos[2]);
// 				s3d_push_vertex(o,node[j].pos[0],node[j].pos[1]+0.2,node[j].pos[2]);
// 				s3d_push_vertex(o,node[j].pos[0],node[j].pos[1]-0.2,node[j].pos[2]);

/*				s3d_push_polygon(o,0,4,5,0);
				s3d_push_polygon(o,3,1,2,0);*/
// 			}
// 	}
	while (0!=(r=net_main()))
		if (r==-1)
		{
			s3d_quit();
			break;
		}
	alpha=(alpha+5)%360;
	s3d_rotate(mesh,0,alpha,0);
	usleep(100000);
/*	sleep(1);*/
	return;
}

void stop()
{
	s3d_quit();
	net_quit();
	process_quit();
}

/***
 *
 * eventhandler when key pressed
 *
 ***/

void keypress(struct s3d_evt *event) {
	int key;
	key=*((unsigned short *)event->buf);
	/* ESC */
	if(key = 27)
		stop();
}

/***
 *
 * eventhandler when object clicked
 *
 ***/

void object_click(struct s3d_evt *evt) 
{
	int oid;
	oid=(int)*((unsigned long *)evt->buf);
	/*s3d_translate(ZeroPoint,0,50,40);
	ZeroPosition[0] = 0;
	ZeroPosition[1] = 50;
	ZeroPosition[2] = 40;*/
	struct olsr_node *olsr_node;
	olsr_node = *lst_search(oid);
	printf("obj2ip: search return %s\n",olsr_node->ip);
}

/***
 *
 * eventhandler when object change by user
 * such as Cam
 *
 ***/

void object_info(struct s3d_evt *hrmz)
{
	struct s3d_obj_info *inf;
	inf=(struct s3d_obj_info *)hrmz->buf;
	if (inf->object==0)
	{
		CamPosition[0][0] = inf->trans_x;
		CamPosition[0][1] = inf->trans_y;
		CamPosition[0][2] = inf->trans_z;
		CamPosition[1][0] = inf->rot_x;
		CamPosition[1][1] = inf->rot_y;
		CamPosition[1][2] = inf->rot_z;
		asp=inf->scale;
		if (asp>1.0) /* wide screen */
		{
			bottom=-1.0;
			left=-asp;
		} else {  /* high screen */
			bottom=(-1.0/asp);
			left=-1.0;

		}
		s3d_translate(mesh,(-left)*3.0-1.8,bottom*3.0+0.8,-3.0);
		s3d_flags_on(mesh,S3D_OF_VISIBLE);
	}
}

/***
 *
 * initialize the struct for a linked list obj2ip
 *
 ***/

void lst_initialize() {
	Obj_to_ip_head = (struct Obj_to_ip*) malloc(sizeof(struct Obj_to_ip));
	Obj_to_ip_end = (struct Obj_to_ip*) malloc(sizeof(struct Obj_to_ip));
	if(Obj_to_ip_head == NULL || Obj_to_ip_end == NULL) {
		printf("not enough memory to initialize struct list\n");
		exit(8);
	}
	Obj_to_ip_head->id = 0;
	Obj_to_ip_end->id = -1;
	Obj_to_ip_head->prev = Obj_to_ip_end->prev = Obj_to_ip_head;
	Obj_to_ip_head->next = Obj_to_ip_end->next = Obj_to_ip_end;
	List_ptr = Obj_to_ip_head;
}

/***
 *
 * add a link object_id to olsr_node, to get ip adress and coordinates per object_id
 *                 id => object_id, returned from s3d_clone or s3d_new_object
 *  **olsr_node => pointer to pointer of current olsr_node 
 *
 ***/

void lst_add(int id,struct olsr_node **olsr_node) {
	struct Obj_to_ip *new;
	new = (struct Obj_to_ip*) malloc(sizeof(struct Obj_to_ip));
	if(new == NULL) {
		printf("not enough memory to add element to linked list\n");
		exit(8);
	}
	new->id = id;
	new->olsr_node = *olsr_node;
	move_lst_ptr(&id);
	printf("obj2ip: add object %d between %d .. %d ip %s to list\n",new->id,List_ptr->id,List_ptr->next->id,new->olsr_node->ip);
	new->prev = List_ptr;
	new->next = List_ptr->next;
	List_ptr->next->prev = new;
	List_ptr->next = new;
}

/***
 *void move_lst_ptr(int *id)
 * remove element from obj2ip linked list
 * id => object_id, returned from s3d_clone or s3d_new_object
 *	
 ***/

void lst_del(int id) {
	struct Obj_to_ip *del;
	move_lst_ptr(&id);
	if(id != List_ptr->next->id)
		printf("obj2ip: remove id %d failed move_lst_ptr return id %d\n",id,List_ptr->next->id);
	else {
		printf("obj2ip: remove object %d ip %s from list\n",List_ptr->next->id,List_ptr->next->olsr_node->ip);
		del = List_ptr->next;
		List_ptr->next = List_ptr->next->next;
		List_ptr->next->prev = List_ptr;
		free(del);
	}
}

/***
 *
 * move the List_ptr one positon ahead the searched element
 *	*id => pointer of object_id , returned from s3d_clone or s3d_new_object
 * 
 ***/
 
void move_lst_ptr(int *id) {
	printf("obj2ip: move for %d\n",*id);
	/* head to point at end or id lass then first element in linked list*/
 	if(Obj_to_ip_head->next->id == -1 || *id < Obj_to_ip_head->next->id)
		List_ptr = Obj_to_ip_head;
 	/* id is greather then last element in linked list */
	else if(*id > Obj_to_ip_end->prev->id)
		List_ptr = Obj_to_ip_end->prev;
	else {
		printf("obj2ip: ok i search deeper ;-) for id=%d\n",*id);
		if(*id < (int)(Obj_to_ip_end->prev->id - Obj_to_ip_head->next->id) / 2) {
			List_ptr = Obj_to_ip_head->next;
			printf("obj2ip: start at head id %d < %d (last-first)/2\n",*id,(Obj_to_ip_end->prev->id - Obj_to_ip_head->next->id) / 2);
			while(*id > List_ptr->next->id) {
				List_ptr = List_ptr->next;
				printf("obj2ip: move --> %d\n",List_ptr->next->id);
			}
		} else {
			List_ptr = Obj_to_ip_end->prev;
			printf("obj2ip: start at end id %d > %d (last-first)/2\n",*id,(Obj_to_ip_end->prev->id - Obj_to_ip_head->next->id) / 2);
			//do List_ptr = List_ptr->prev; while(*id > List_ptr->prev->id);
			while(*id < List_ptr->prev->id) {
				List_ptr = List_ptr->prev;
				printf("obj2ip: move <-- %d\n",List_ptr->id);
			}
			List_ptr = List_ptr->prev->prev; 
		}
		printf("obj2ip: found id %d--> %d <--%d\n",List_ptr->id,List_ptr->next->id,List_ptr->next->next->id);
	}
}

/***
 *
 * search a object_id in linked list and return pointer on struct olsr_node
 *	id => object_id , returned from s3d_clone or s3d_new_object
 * 
 ***/
 
struct olsr_node **lst_search(int id) {
	move_lst_ptr(&id);
	if(id != List_ptr->next->id)
		printf("obj2ip: search id....id not found\n");
	else
		printf("obj2ip: search found objekt_id=%d objekt_ip=%s\n",id,List_ptr->next->olsr_node->ip);
	return(&List_ptr->next->olsr_node);
}

int main( int argc, char *argv[] ) {

	int optchar;
	strncpy( Olsr_host, "127.0.0.1", 256 );

	while ( ( optchar = getopt ( argc, argv, "dhH:" ) ) != -1 ) {

		switch ( optchar ) {

			case 'd':
				Debug = 1;
				break;

			case 'H':
				strncpy( Olsr_host, optarg, 256 );
				break;

			case 'h':
			default:
				print_usage();
				return (0);

		}

	}

	if ( Debug ) printf( "debug mode enabled ...\n" );
	/* initialize obj2ip linked list */
	/* TODO: next step is a double linked list to search,add, remove faster */
	lst_initialize();
	/* delete olsrs3d options */
	while ( ( optind < argc ) && ( argv[optind][0] != '-' ) ) optind++;   /* optind may point to ip addr of '-H' */
	optind--;
	argv[optind] = argv[0];   /* save program path */
	argc -= optind;   /* jump over olsrs3d options */
	argv += optind;

	/* set extern int optind = 0 for parse_args in io.c */
	optind = 0;

	process_init(Olsr_host);
	if (!net_init(Olsr_host))
	{
		if (!s3d_init(&argc,&argv,"olsrs3d"))
		{
			s3d_set_callback(S3D_EVENT_OBJ_INFO,object_info);
			s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);
			s3d_set_callback(S3D_EVENT_KEY,keypress);
			s3d_set_callback(S3D_EVENT_QUIT,stop);
			if (s3d_select_font("vera"))
				printf("font not found\n");
			Olsr_node_obj=s3d_import_3ds_file("accesspoint.3ds");
			Olsr_node_inet_obj=s3d_import_3ds_file("accesspoint_inet.3ds");
			mesh=s3d_import_3ds_file("meshnode.3ds");
			s3d_link(mesh,0);
			s3d_scale(mesh,0.15);
			ZeroPoint = s3d_new_object();
			s3d_mainloop(mainloop);
			s3d_quit();
			net_quit();
			process_quit();
		}
	}
	return(0);
}

