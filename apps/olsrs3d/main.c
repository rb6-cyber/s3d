#include <stdio.h>
#include <s3d.h>
#include <unistd.h>		/* sleep() */
#include <string.h> 		/* strncpy() */
#include <math.h>		/* sqrt() */
#include <getopt.h>		/* getopt() */
#include "olsrs3d.h"
#define SPEED		10.0

int Debug = 0;

char Olsr_host[256];   // ip or hostname of olsr node with running dot_draw plugin

struct olsr_node *Root = NULL;   // top of olsr node tree


int node_count=-1;
int alpha=0;
int Olsr_node_obj,Olsr_node_inet_obj,mesh;
float asp=1.0;
float bottom=-1.0;
float left=-1.0;



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
	exit( 8 );

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
		if ( olsr_node->obj_id != -1 ) s3d_del_object( olsr_node->obj_id );
		if ( olsr_node->desc_id != -1 ) s3d_del_object( olsr_node->desc_id );

		/* create new shape */
		if ( olsr_node->inet_gw ) {
			/* olsr node offers internet access */
			olsr_node->obj_id = s3d_clone( Olsr_node_inet_obj );
		} else {
			/* normal olsr node */
			olsr_node->obj_id = s3d_clone( Olsr_node_obj );
		}

		s3d_flags_on( olsr_node->obj_id, S3D_OF_VISIBLE );

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
	(*olsr_con) = &olsr_node->olsr_con;

// 	while ( (*olsr_con) != NULL ) {

// 		printf( "distance: %i\n", olsr_con->olsr_node );
// 		distance = dirt( olsr_node->pos_vec, (*olsr_con)->olsr_node->pos_vec, tmp_mov_vec );
// 		f = (*olsr_con)->etx / distance;
// 		if ( f < 0.3 ) f = 0.3;
// 		mov_add( olsr_node->mov_vec, tmp_mov_vec, 1/f-1);

// 		(*olsr_con) = &olsr_con->next_olsr_con;

// 	}

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
void object_info(struct s3d_evt *hrmz)
{
	struct s3d_obj_info *inf;
	inf=(struct s3d_obj_info *)hrmz->buf;
	if (inf->object==0)
	{
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
			if (s3d_select_font("vera"))
				printf("font not found\n");

			Olsr_node_obj=s3d_import_3ds_file("accesspoint.3ds");
			Olsr_node_inet_obj=s3d_import_3ds_file("internet.3ds");
			mesh=s3d_import_3ds_file("meshnode.3ds");
			s3d_link(mesh,0);
			s3d_scale(mesh,0.15);

			s3d_mainloop(mainloop);
			s3d_quit();
			net_quit();
			process_quit();
		}
	}
	return(0);
}

