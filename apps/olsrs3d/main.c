#include <stdio.h>
#include <s3d.h>
#include <unistd.h>	/* sleep() */
#include <string.h>	/* strncpy() */
#include <math.h>		/* sqrt() */
#include <getopt.h>	/* getopt() */
#include <stdlib.h>	/* exit() */
#include "olsrs3d.h"
#define SPEED		10.0

int Debug = 0;

char Olsr_host[256];   /* ip or hostname of olsr node with running dot_draw plugin */

struct olsr_con *Con_begin = NULL;   /* begin of connection list */
struct olsr_node *Olsr_root = NULL;   /* top of olsr node tree */
struct Obj_to_ip *Obj_to_ip_head, *Obj_to_ip_end, *List_ptr;   /* needed pointer for linked list */

int node_count=-1;
int alpha=0;
int Olsr_node_obj, Olsr_node_inet_obj, Olsr_node_hna_net, mesh;
float asp=1.0;
float bottom=-1.0;
float left=-1.0;

float CamPosition[2][3];   /* CamPosition[trans|rot][x-z] */
float ZeroPosition[3] = {0,0,0};   /* current position zero position */
int ZeroPoint;   /* object zeropoint */
int Zp_rotate = 0;
int ColorSwitch = 0;   /* enable/disable colored olsr connections */
int RotateSwitch = 0;
int RotateSpeed = 2;


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
	struct Obj_to_ip *Obj_to_ip_curr;
	struct olsr_con_list *olsr_con_list;

	/* no more nodes left */
	if ( olsr_node == NULL ) return;

	/* olsr node shape has been modified */
	if ( olsr_node->node_type_modified ) {

		/* delete old shape */
		if ( olsr_node->obj_id != -1 ) {
			/* remove element from ob2ip list */
			lst_del( olsr_node->obj_id );
			s3d_del_object( olsr_node->obj_id );
		}

		if ( olsr_node->desc_id != -1 ) s3d_del_object( olsr_node->desc_id );

		/* create new shape */
		if ( olsr_node->node_type == 1 ) {
			/* olsr node offers internet access */
			olsr_node->obj_id = s3d_clone( Olsr_node_inet_obj );
		} else if ( olsr_node->node_type == 2 ) {
			/* olsr node offers internet access */
			olsr_node->obj_id = s3d_clone( Olsr_node_hna_net );
		} else {
			/* normal olsr node */
			olsr_node->obj_id = s3d_clone( Olsr_node_obj );
		}

		s3d_flags_on( olsr_node->obj_id, S3D_OF_VISIBLE|S3D_OF_SELECTABLE);

		/* link newly created object to ZeroPoint */
		s3d_link( olsr_node->obj_id, ZeroPoint );
		/* add object_id and olsr_node to linked list */
		lst_add(olsr_node->obj_id,&olsr_node);

		/* create olsr node text and attach (link) it to the node */
		olsr_node->desc_id = s3d_draw_string( olsr_node->ip, &f );
		s3d_link( olsr_node->desc_id, olsr_node->obj_id );
		s3d_translate( olsr_node->desc_id, -f/2,-2,0 );
		s3d_flags_on( olsr_node->desc_id, S3D_OF_VISIBLE );

		olsr_node->node_type_modified = 0;

	}

	/* drift away from unrelated nodes */
	Obj_to_ip_curr = Obj_to_ip_head->next;
	while ( Obj_to_ip_curr != Obj_to_ip_end ) {

		/* myself ... */
/* 		if ( strncmp( Obj_to_ip_curr->olsr_node->ip, olsr_node->ip, NAMEMAX ) != 0 ) {*/

			olsr_con_list = olsr_node->olsr_con_list;
			while ( olsr_con_list != NULL ) {

				/* nodes are related */
				if ( ( strncmp( olsr_con_list->olsr_con->left_olsr_node->ip, Obj_to_ip_curr->olsr_node->ip, NAMEMAX ) == 0 ) || ( strncmp( olsr_con_list->olsr_con->right_olsr_node->ip, Obj_to_ip_curr->olsr_node->ip, NAMEMAX ) == 0 ) ) break;

				olsr_con_list = olsr_con_list->next_olsr_con_list;

			}

			/* nodes are not related - so drift */
			if ( olsr_con_list == NULL ) {

				distance = dirt( olsr_node->pos_vec, Obj_to_ip_curr->olsr_node->pos_vec, tmp_mov_vec );
				if ( distance < 0.1 ) distance = 0.1;
				mov_add( olsr_node->mov_vec, tmp_mov_vec,-100 / ( distance * distance ) );
				mov_add( Obj_to_ip_curr->olsr_node->mov_vec, tmp_mov_vec, 100 / ( distance * distance ) );

			}

			Obj_to_ip_curr = Obj_to_ip_curr->next;

/* 		} */

	}

	handle_olsr_node( olsr_node->left );
	handle_olsr_node( olsr_node->right );

}



/***
 *
 * calculate movement vector of all olsr nodes
 *
 ***/

void calc_olsr_node_mov( void ) {

	float f, distance;
	float tmp_mov_vec[3];
	struct olsr_con **olsr_con = &Con_begin;

	while ( (*olsr_con) != NULL ) {

		if ( ( (*olsr_con)->left_etx != 0.0 ) && ( (*olsr_con)->right_etx != 0.0  ) ) {

			distance = dirt( (*olsr_con)->left_olsr_node->pos_vec, (*olsr_con)->right_olsr_node->pos_vec, tmp_mov_vec );
			f = ( ( (*olsr_con)->left_etx + (*olsr_con)->right_etx ) / 4.0 ) / distance;
			if ( f < 0.3 ) f = 0.3;

			mov_add( (*olsr_con)->left_olsr_node->mov_vec, tmp_mov_vec, 1 / f - 1 );
			mov_add( (*olsr_con)->right_olsr_node->mov_vec, tmp_mov_vec, - ( 1 / f - 1 ) );

		}

		olsr_con = &(*olsr_con)->next_olsr_con;

	}

}



/***
 *
 * move all olsr nodes and their connections
 *
 ***/

void move_olsr_nodes( void ) {

	float null_vec[3] = {0,0,0};
	float tmp_mov_vec[3];
	float distance, etx, rgb;
	struct olsr_con **olsr_con = &Con_begin;

	while ( (*olsr_con) != NULL ) {

		/* move left olsr node if it has not been moved yet */
		if ( !( ( (*olsr_con)->left_olsr_node->mov_vec[0] == 0 ) && ( (*olsr_con)->left_olsr_node->mov_vec[1] == 0 ) && ( (*olsr_con)->left_olsr_node->mov_vec[2] == 0 ) ) ) {

			distance = dirt( (*olsr_con)->left_olsr_node->pos_vec, null_vec, tmp_mov_vec );
			mov_add( (*olsr_con)->left_olsr_node->mov_vec, tmp_mov_vec, distance / 100 ); /* move a little bit to point zero */
			mov_add( (*olsr_con)->left_olsr_node->mov_vec, tmp_mov_vec, 1 ); /* move a little bit to point zero */

			if ( ( distance = dist( (*olsr_con)->left_olsr_node->mov_vec, null_vec ) ) > 10.0 ) {
				mov_add( (*olsr_con)->left_olsr_node->pos_vec, (*olsr_con)->left_olsr_node->mov_vec, 1.0 / ( (float ) distance ) );
			} else {
				mov_add( (*olsr_con)->left_olsr_node->pos_vec, (*olsr_con)->left_olsr_node->mov_vec, 0.1 );
			}

			s3d_translate( (*olsr_con)->left_olsr_node->obj_id, (*olsr_con)->left_olsr_node->pos_vec[0], (*olsr_con)->left_olsr_node->pos_vec[1], (*olsr_con)->left_olsr_node->pos_vec[2] );

			/* reset movement vector */
			(*olsr_con)->left_olsr_node->mov_vec[0] = (*olsr_con)->left_olsr_node->mov_vec[1] = (*olsr_con)->left_olsr_node->mov_vec[2] = 0.0;

		}

		/* move right olsr node if it has not been moved yet */
		if ( !( ( (*olsr_con)->right_olsr_node->mov_vec[0] == 0 ) && ( (*olsr_con)->right_olsr_node->mov_vec[1] == 0 ) && ( (*olsr_con)->right_olsr_node->mov_vec[2] == 0 ) ) ) {

			distance = dirt( (*olsr_con)->right_olsr_node->pos_vec, null_vec, tmp_mov_vec );
			mov_add( (*olsr_con)->right_olsr_node->mov_vec, tmp_mov_vec, distance / 100 ); /* move a little bit to point zero */
			mov_add( (*olsr_con)->right_olsr_node->mov_vec, tmp_mov_vec, 1 ); /* move a little bit to point zero */

			if ( ( distance = dist( (*olsr_con)->right_olsr_node->mov_vec, null_vec ) ) > 10.0 ) {
				mov_add( (*olsr_con)->right_olsr_node->pos_vec, (*olsr_con)->right_olsr_node->mov_vec, 1.0 / ( (float ) distance ) );
			} else {
				mov_add( (*olsr_con)->right_olsr_node->pos_vec, (*olsr_con)->right_olsr_node->mov_vec, 0.1 );
			}

			s3d_translate( (*olsr_con)->right_olsr_node->obj_id, (*olsr_con)->right_olsr_node->pos_vec[0], (*olsr_con)->right_olsr_node->pos_vec[1], (*olsr_con)->right_olsr_node->pos_vec[2] );

			/* reset movement vector */
			(*olsr_con)->right_olsr_node->mov_vec[0] = (*olsr_con)->right_olsr_node->mov_vec[1] = (*olsr_con)->right_olsr_node->mov_vec[2] = 0.0;

		}

		/* move connection between left and right olsr node */
		s3d_pop_vertex( (*olsr_con)->obj_id, 6 );
		s3d_pop_polygon( (*olsr_con)->obj_id, 2 );
		s3d_pop_material( (*olsr_con)->obj_id, 1 );

		s3d_push_vertex( (*olsr_con)->obj_id, (*olsr_con)->left_olsr_node->pos_vec[0] + ZeroPosition[0], (*olsr_con)->left_olsr_node->pos_vec[1] + ZeroPosition[1], (*olsr_con)->left_olsr_node->pos_vec[2] + ZeroPosition[2] );
		s3d_push_vertex( (*olsr_con)->obj_id, (*olsr_con)->left_olsr_node->pos_vec[0] + 0.2 + ZeroPosition[0], (*olsr_con)->left_olsr_node->pos_vec[1] + ZeroPosition[1], (*olsr_con)->left_olsr_node->pos_vec[2] + ZeroPosition[2] );
		s3d_push_vertex( (*olsr_con)->obj_id, (*olsr_con)->left_olsr_node->pos_vec[0] - 0.2 + ZeroPosition[0], (*olsr_con)->left_olsr_node->pos_vec[1] + ZeroPosition[1], (*olsr_con)->left_olsr_node->pos_vec[2] + ZeroPosition[2] );

		s3d_push_vertex( (*olsr_con)->obj_id, (*olsr_con)->right_olsr_node->pos_vec[0] + ZeroPosition[0], (*olsr_con)->right_olsr_node->pos_vec[1]+ ZeroPosition[1], (*olsr_con)->right_olsr_node->pos_vec[2] + ZeroPosition[2] );
		s3d_push_vertex( (*olsr_con)->obj_id, (*olsr_con)->right_olsr_node->pos_vec[0] + ZeroPosition[0], (*olsr_con)->right_olsr_node->pos_vec[1]+ 0.2 + ZeroPosition[1], (*olsr_con)->right_olsr_node->pos_vec[2] + ZeroPosition[2] );
		s3d_push_vertex( (*olsr_con)->obj_id, (*olsr_con)->right_olsr_node->pos_vec[0] + ZeroPosition[0], (*olsr_con)->right_olsr_node->pos_vec[1]- 0.2 + ZeroPosition[1], (*olsr_con)->right_olsr_node->pos_vec[2] + ZeroPosition[2] );

		if ( ColorSwitch ) {

			/* HNA */
			if ( (*olsr_con)->left_etx == -1000.00 ) {

				s3d_push_material( (*olsr_con)->obj_id,
							   0.0,0.0,1.0,
							   0.0,0.0,1.0,
							   0.0,0.0,1.0);

			} else {

				etx = ( ( ( (*olsr_con)->left_etx + (*olsr_con)->right_etx ) / 2.0 ) - 10.0 ) * 10.0;

				if ( ( etx >= 1.0 ) && ( etx < 2.0 ) ) {

					rgb = etx - 1.0;
					s3d_push_material( (*olsr_con)->obj_id,
								rgb,1.0,0.0,
								rgb,1.0,0.0,
								rgb,1.0,0.0);

				} else if ( ( etx >= 2.0 ) && ( etx < 3.0 ) ) {

					rgb = 3.0 - etx;
					s3d_push_material( (*olsr_con)->obj_id,
								1.0,rgb,0.0,
								1.0,rgb,0.0,
								1.0,rgb,0.0);

				} else {

					s3d_push_material( (*olsr_con)->obj_id,
								1.0,0.0,0.0,
								1.0,0.0,0.0,
								1.0,0.0,0.0);

				}

			}

		} else {

			s3d_push_material( (*olsr_con)->obj_id,
						1.0,1.0,1.0,
						1.0,1.0,1.0,
						1.0,1.0,1.0);

		}

		s3d_push_polygon( (*olsr_con)->obj_id, 0,4,5,0 );
		s3d_push_polygon( (*olsr_con)->obj_id, 3,1,2,0 );

		olsr_con = &(*olsr_con)->next_olsr_con;

	}

}



void mainloop()
{
	int i,j,o,r;
	float d,gd,f,m[3];
	float z[3]={0,0,0};
/* 	for (i=0;i<max;i++)
 	{
 		node[i].mov[0]=
 		node[i].mov[1]=
 		node[i].mov[2]=0.0;
 	} */

	/* calculate new movement vector */
	calc_olsr_node_mov();

	/* prepare nodes */
	handle_olsr_node( Olsr_root );

	/* move it */
	move_olsr_nodes();

	/*	for (i=0;i<max;i++)
		{
			for (j=i+1;j<max;j++)
			{
				if (i!=j)
				{
					gd=adj[i*max+j];
					d=dirt(node[i].pos,node[j].pos,m);
					if (gd==0.0)*/	/* points are not connected 
					{
					printf("distance between i and j: %f\n",d);
						if (d<0.1) d=0.1;
						mov_add(node[j].mov,m,100/(d*d));
						mov_add(node[i].mov,m,-100/(d*d));
					} else { / * connected!! * /

						f=(gd)/d;
						if (f<0.3) f=0.3;
						mov_add(node[i].mov,m,1/f-1);
						mov_add(node[j].mov,m,-(1/f-1));
					printf("distance between %d and %d: %f / %f = %f\n",i,j,gd,d,f);
					}
				}
			}
			d=dirt(node[i].pos,z,m);
		mov_add(node[i].mov,m,d/100); * move a little bit to point zero 
		mov_add(node[i].mov,m,1); * move a little bit to point zero 
	}
	/ * move it!! * /
 	for (i=0;i<max;i++)
 	{
/ *		printf("applying move vector for point %d: %f:%f:%f\n",i,node[i].mov[0],node[i].mov[1],node[i].mov[2]); * /
 		if ((d=dist(node[i].mov,z))>10.0)
 			mov_add(node[i].pos,node[i].mov,1.0/((float )d)); / * normalize * /
 		else
 			mov_add(node[i].pos,node[i].mov,0.1);
 		s3d_translate(node[i].obj,node[i].pos[0],node[i].pos[1],node[i].pos[2]);
 		for (j=i+1;j<max;j++)
 			if ((o=adj_obj[max*i+j])!=-1)
 			{
 				s3d_pop_vertex(o,6);
/ *				s3d_pop_polygon(o,2);* /
 				s3d_push_vertex(o,node[i].pos[0],	 node[i].pos[1],node[i].pos[2]);
 				s3d_push_vertex(o,node[i].pos[0]+0.2,node[i].pos[1],node[i].pos[2]);
 				s3d_push_vertex(o,node[i].pos[0]-->id0.2,node[i].pos[1],node[i].pos[2]);
				
 				s3d_push_vertex(o,node[j].pos[0],	 node[j].pos[1],node[j].pos[2]);
 				s3d_push_vertex(o,node[j].pos[0],node[j].pos[1]+0.2,node[j].pos[2]);
 				s3d_push_vertex(o,node[j].pos[0],node[j].pos[1]-0.2,node[j].pos[2]);

/ *				s3d_push_polygon(o,0,4,5,0);
				s3d_push_polygon(o,3,1,2,0);* /
 			}
 	} */
	while (0!=(r=net_main()))
		if (r==-1)
		{
			s3d_quit();
			break;
		}
	alpha=(alpha+5)%360;
	s3d_rotate(mesh,0,alpha,0);
	if(RotateSwitch) {
		Zp_rotate = (Zp_rotate+RotateSpeed)%360;
		s3d_rotate(ZeroPoint,0,Zp_rotate,0);
	}
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
	switch(key) {
		case 27: /* esc */
			stop();
			break;
			case 15: /* strg + o */
			lst_out(); /* output ob2ip list */
			break;
		case 99: /* c*/
			if(ColorSwitch) ColorSwitch = 0;
			else ColorSwitch = 1;
			break;
		case 114: /* r */
			if(RotateSwitch) RotateSwitch = 0;
			else RotateSwitch = 1;
			break;
		case 43: /* + */
			if(RotateSwitch && RotateSpeed < 10)
				RotateSpeed++;
			break;
		case 45: /* - */
			if(RotateSwitch && RotateSpeed > 1)
				RotateSpeed--;
			break;
	}
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
	/* printf("obj2ip: search return %s\n",olsr_node->ip); */
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
	/* printf("%f %f %f\n",inf->trans_x,inf->trans_y,inf->trans_z); */
}

/***
 *
 * initialize the struct for a linked list obj2ip
 *
 ***/

void lst_initialize() {
	Obj_to_ip_head = (struct Obj_to_ip*) malloc(sizeof(struct Obj_to_ip));
	Obj_to_ip_end = (struct Obj_to_ip*) malloc(sizeof(struct Obj_to_ip));
	if(Obj_to_ip_head == NULL || Obj_to_ip_end == NULL)
		out_of_mem();
	Obj_to_ip_head->id = 0;
	Obj_to_ip_end->id = 0;
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
	if(new == NULL)
		out_of_mem();
	new->id = id;
	new->olsr_node = *olsr_node;
	move_lst_ptr(&id);
	new->prev = List_ptr;
	new->next = List_ptr->next;
	List_ptr->next->prev = new;
	List_ptr->next = new;
	/* printf("obj2ip: add object %d between %d .. %d ip %s to list\n",new->id,new->prev->id,new->next->id,new->olsr_node->ip); */
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
	if(id != List_ptr->id)
	{
		/* printf("obj2ip: remove id %d failed move_lst_ptr return id %d\n",id,List_ptr->next->id); */
	} else {
		del = List_ptr;
		List_ptr->next->prev = List_ptr->prev;
		List_ptr->prev->next = List_ptr->next;
		/* printf("obj2ip: remove object %d --> %d <-- %d ip %s from list\n",List_ptr->prev->id,del->id,List_ptr->next->id,del->olsr_node->ip); */
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
	/* printf("obj2ip: move for %d\n",*id); */
	/* head to point at end or id lass then first element in linked list*/
	if(Obj_to_ip_head->next == Obj_to_ip_head || *id < Obj_to_ip_head->next->id)
		List_ptr = Obj_to_ip_head;
 	/* id is greather then last element in linked list */
	else if(*id > Obj_to_ip_end->prev->id)
		List_ptr = Obj_to_ip_end->prev;
	else {
		/* printf("obj2ip: ok i search deeper ;-) for id=%d\n",*id); */
		if((*id - (int) Obj_to_ip_head->next->id) <= ((int)(Obj_to_ip_end->prev->id)-*id)) {
			List_ptr = Obj_to_ip_head;
			/* printf("obj2ip: start at head id %d - %d <= %d - %d \n",*id,Obj_to_ip_head->next->id,Obj_to_ip_end->prev->id,*id); */
			while(*id >= List_ptr->next->id) {
				/* printf("obj2ip: %d > %d move to ",*id,List_ptr->id); */
				List_ptr = List_ptr->next;
				/* printf("%d\n",List_ptr->id); */
			}
		} else {
			List_ptr = Obj_to_ip_end;
			/* printf("obj2ip: start at end id %d - %d > %d - %d \n",*id,Obj_to_ip_head->next->id,Obj_to_ip_end->prev->id,*id);  */
			/*  do List_ptr = List_ptr->prev; while(*id > List_ptr->prev->id); */
			while(*id < List_ptr->prev->id) {
				/* printf("obj2ip: %d < %d move to ",*id,List_ptr->id); */
				List_ptr = List_ptr->prev;
				/* printf("%d\n",List_ptr->id); */
			}
			List_ptr = List_ptr->prev;
		}
		/* printf("obj2ip: found id to insert between %d--> .. <--%d to search/delete %d--> .. <--%d\n",List_ptr->id,List_ptr->next->next->id,List_ptr->prev->id,List_ptr->next->id); */
	}
}

/***
 *
 * search a object_id in linked list and return pointer on struct olsr_node
 *	id => object_id , returned from s3d_clone or s3d_new_object
 *
 * <example>
 *     struct olsr_node *olsr_node;
 *     olsr_node = *lst_search(oid);
 *     printf("obj2ip: search return %s\n",olsr_node->ip);
 * </example>
 *
 ***/

struct olsr_node **lst_search(int id) {
	move_lst_ptr(&id);
	/* TODO: return NULL when no node found */
	/* if(id != List_ptr->id) */
		/* printf("obj2ip: search id....id not found\n"); */
	/* else */
		/* printf("obj2ip: search found objekt_id=%d objekt_ip=%s\n",List_ptr->id,List_ptr->olsr_node->ip); */
	return(&List_ptr->olsr_node);
}

void lst_out() {
	struct Obj_to_ip *ptr;
	ptr = Obj_to_ip_head;
	while(ptr != ptr->next) {
		/* printf("List--------------------------\n"); */
		printf("id-> %d\n",ptr->id);
		ptr = ptr->next;
	}
	/* printf("List--------------------------\n"); */
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
			Olsr_node_obj = s3d_import_3ds_file( "accesspoint.3ds" );
			Olsr_node_inet_obj = s3d_import_3ds_file( "accesspoint_inet.3ds" );
			Olsr_node_hna_net = s3d_import_3ds_file( "internet.3ds" );
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

