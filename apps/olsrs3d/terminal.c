#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <s3d.h>
#include "terminal.h"
#include "olsrs3d.h"

#define BLOCK_SIZE 10

/* index of s3d_object array for objects */
int obj_term = 0;
int obj_cursor = 0;

/* object_id of middlepoint cursor */
int obj_cursor_mp = 0;

int obj_cnt=0;
int obj_max = 0;

struct s3d_object **obj = NULL;
struct olsr_node *search_node;

int create_rectangle(struct rectangle *rect,float mat[]);
int find_olsr_node( char *ip );

int create_rectangle(struct rectangle *rect,float mat[])
{
	int oid=s3d_new_object();
	float vertices[24];
	/*
	unsigned long pbuf[] = { 1,2,0,0,
											1,3,2,0,
											5,3,1,0,
											5,4,3,0,
											6,5,4,0,
											6,4,7,0,
											0,6,7,0,
											0,7,2,0,
											3,2,7,0,
											3,7,4,0,
											5,6,0,0,
											5,0,1,0};*/
	unsigned long pbuf[] = { 1,0,2,0,
											1,2,3,0,
											5,1,3,0,
											5,3,4,0,
											6,4,5,0,
											6,7,4,0,
											0,7,6,0,
											0,2,7,0,
											3,7,2,0,
											3,4,7,0,
											5,0,6,0,
											5,1,0,0};

	vertices[0]  = (*rect).front.x; vertices[1] = (*rect).front.y; vertices[2] = (*rect).front.z;
	vertices[3]  = (*rect).rear.x;vertices[4] = (*rect).front.y; vertices[5] = (*rect).front.z;
	vertices[6]  = (*rect).front.x; vertices[7] = (*rect).rear.y; vertices[8] = (*rect).front.z;
	vertices[9]  = (*rect).rear.x; vertices[10] = (*rect).rear.y; vertices[11] = (*rect).front.z;
	vertices[12] = (*rect).rear.x; vertices[13] = (*rect).rear.y; vertices[14] = (*rect).rear.z;
	vertices[15] = (*rect).rear.x; vertices[16] = (*rect).front.y; vertices[17] = (*rect).rear.z;
	vertices[18] = (*rect).front.x; vertices[19] = (*rect).front.y; vertices[20] = (*rect).rear.z;
	vertices[21] = (*rect).front.x; vertices[22] = (*rect).rear.y; vertices[23] = (*rect).rear.z;

	s3d_push_vertices(oid,vertices,8);
	s3d_push_material(oid,mat[0],mat[1],mat[2],mat[3],mat[4],mat[5],mat[6],mat[7],mat[8]);
	s3d_push_polygons(oid,pbuf,12);
	return(oid);
}

int create_object(char name[],float sp[], float ep[],float pos[],float poi[],float mat[])
{
	int oid;

	struct rectangle rect;
	rect.front.x = sp[0]; rect.front.y = sp[1]; rect.front.z = sp[2];
	rect.rear.x = ep[0]; rect.rear.y = ep[1]; rect.rear.z = ep[2];
	oid = create_rectangle(&rect,mat);

	if( obj_cnt >= obj_max )
	{
		obj_max += BLOCK_SIZE;
		obj = (struct s3d_object **) realloc(obj, obj_max*sizeof(struct s3d_object *));
		if(obj == NULL)
			out_of_mem();
	}
	if( (obj[obj_cnt] = (struct s3d_object *) malloc( sizeof(struct s3d_object ) )) == NULL)
		out_of_mem();

	obj[obj_cnt]->oid = oid;
	obj[obj_cnt]->pos[0] = pos[0]; obj[obj_cnt]->pos[1] = pos[1]; obj[obj_cnt]->pos[2] = pos[2];
	obj[obj_cnt]->rot[0] = 0; obj[obj_cnt]->rot[1] = 180; obj[obj_cnt]->rot[2] = 0;

	s3d_translate(oid,pos[0],pos[1],pos[2]);

	strncpy(obj[obj_cnt]->name,name,20);
	obj[obj_cnt]->poi[0] = poi[0]; obj[obj_cnt]->poi[1] = poi[1]; obj[obj_cnt]->poi[2] = poi[2];
	obj_cnt++;
	return(	obj_cnt-1);
}

void create_cursor()
{
	float sp[] = {-0.15,-0.15,-0.05};
	float ep[] = {0.15,0.15,0.05};
	float pos[] = {9.15, 4.25, 299.95};
	float poi[] = {9.0,-17.5,295.0};
	float mat[] = {1.0,1.0,0.7,1.0,1.0,0.7,1.0,1.0,0.7};
	obj_cursor = create_object("cursor",sp,ep,pos,poi,mat);
	s3d_flags_on(obj[obj_cursor]->oid, S3D_OF_VISIBLE);
	return;
}

void create_terminal()
{
	float sp[] = {0.0,0.0,0.0};
	float ep[] = {10.0,5.0,2.0};
	float pos[] = {0.0, 0.0, 300.0};
	float poi[] = {5,2.5,295.0};
	float mat[] = {0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6,0.6};
	obj_term = create_object("terminal",sp,ep,pos,poi,mat);
	s3d_flags_on(obj[obj_term]->oid, S3D_OF_VISIBLE|S3D_OF_SELECTABLE);
	return;
}

void rotate_cursor()
{
	static int y = 0;
	y = (y+20)%360;
	s3d_rotate(obj[obj_cursor]->oid,0,y,0);
	return;
}

/* mod_search
void write_terminal(int key)
{
	static char s[20];
	int ln = strlen(s);
	float draw_length;
	float tmp;
	static int str_id = -1;

	if( key == 266) key = 46;
	if( key >= 256 && key <= 265) key = key - 208;
	
	if(key != 13 && key != 271)
	{
		if(key == 8)
		{
			if(ln > 0)
				s[ln-1] = '\0';
		} else {
			if(ln < 20)
				s[ln] = key;
		}
		if(str_id != -1)
			s3d_del_object(str_id);
		str_id = s3d_draw_string( s, &draw_length );
		s3d_flags_on( str_id, S3D_OF_VISIBLE );
		s3d_pep_material(str_id,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0);
		s3d_scale( str_id, 0.4 );
		s3d_translate( str_id,9,4,-0.45);
		s3d_rotate(str_id,0,180,0);
		s3d_link( str_id, obj[obj_term]->oid );
		tmp = obj[obj_cursor]->pos[0] - (draw_length*0.43);
		s3d_translate( obj[obj_cursor]->oid , tmp, obj[obj_cursor]->pos[1], obj[obj_cursor]->pos[2]);
	} else {
		if(!find_olsr_node(s))
		{
			printf("no node found\n");
		} else {
			move_cam_to = search_node->obj_id;
		}
	}
}
*/
int find_olsr_node( char *ip )
{
	int result;
	search_node = Olsr_root;
	
	while ( search_node != NULL )
	{

		result = strncmp( search_node->ip, ip, NAMEMAX );

		/* we found the node */
		if ( result == 0 ) 
			break;

		/* the searched node must be in the subtree */
		if ( result < 0 )
			search_node = search_node->right;
		else
			search_node = search_node->left;
	}
	
	if( search_node != NULL )
	{
		return(1);
	}
	return(0);
}
