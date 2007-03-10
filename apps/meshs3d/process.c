/*
 * process.c
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *                         Marek Lindner <lindner_marek@yahoo.de>
 *                         Andreas Langer <andreas_lbg@gmx.de>
 *
 * This file is part of meshs3d, an olsr/batman topology visualizer for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * olsrs3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * olsrs3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsrs3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */



#include <stdio.h> 	/* NULL */
#include <string.h> 	/* strlen(), memmove() */
#include <stdlib.h> 	/* rand(), malloc(), realloc(), free() */
#include <s3d.h>
#include <math.h>       /* sqrt() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "meshs3d.h"


char lbuf[MAXLINESIZE];



/***
 *
 * create new or alter connection between 2 nodes
 *
 *   con_from =>   current node
 *   con_to   =>   node to connect to
 *   etx      =>   ETX
 *
 ***/

//int add_olsr_con( struct olsr_node *con_from, struct olsr_node *con_to, float etx ) {
//
//	struct olsr_con **olsr_con = &Con_begin;
//	struct olsr_con *prev_olsr_con = NULL;   /* previous olsr connection */
//	struct olsr_neigh_list **olsr_neigh_list;
//
//	while ( (*olsr_con) != NULL ) {
//
//		/* connection already exists */
//		if ( ( strncmp( (*olsr_con)->left_olsr_node->ip, con_from->ip, NAMEMAX ) == 0 ) && ( strncmp( (*olsr_con)->right_olsr_node->ip, con_to->ip, NAMEMAX ) == 0 ) ) {
//			(*olsr_con)->left_etx = etx;
//			(*olsr_con)->left_etx_sqrt = (etx==-1000.00)? 10.0 : sqrt( etx ) ;
//			break;
//
//		} else if ( ( strncmp( (*olsr_con)->right_olsr_node->ip, con_from->ip, NAMEMAX ) == 0 ) && ( strncmp( (*olsr_con)->left_olsr_node->ip, con_to->ip, NAMEMAX ) == 0 ) ) {
//
//			(*olsr_con)->right_etx = etx;
//			(*olsr_con)->right_etx_sqrt = (etx==-1000.00)? 10.0 : sqrt( etx ) ;
//			break;
//
//		}
//
//		/* save previous olsr connection for later use */
//		prev_olsr_con = (*olsr_con);
//
//		olsr_con = &(*olsr_con)->next_olsr_con;
//
//	}
//
//	/* new connection */
//	if ( (*olsr_con) == NULL ) {
//
//		(*olsr_con) = malloc( sizeof( struct olsr_con ) );
//		if ( (*olsr_con) == NULL ) out_of_mem();
//
//		/* create connection object */
//		(*olsr_con)->obj_id = s3d_new_object();
//
//		/* add olsr node to new olsr connection in order to access the nodes from the connection list */
//		(*olsr_con)->left_olsr_node = con_from;
//		(*olsr_con)->right_olsr_node = con_to;
//
//		/* add connection color */
//		(*olsr_con)->color = 0;
//		s3d_push_material( (*olsr_con)->obj_id,
//				  1.0,1.0,1.0,
//				  1.0,1.0,1.0,
//				  1.0,1.0,1.0);
//
//		/* add connection endpoints */
//		s3d_push_vertex( (*olsr_con)->obj_id, (*olsr_con)->left_olsr_node->pos_vec[0], (*olsr_con)->left_olsr_node->pos_vec[1], (*olsr_con)->left_olsr_node->pos_vec[2] );
//		s3d_push_vertex( (*olsr_con)->obj_id, (*olsr_con)->right_olsr_node->pos_vec[0], (*olsr_con)->right_olsr_node->pos_vec[1], (*olsr_con)->right_olsr_node->pos_vec[2] );
//
//		s3d_push_line( (*olsr_con)->obj_id, 0,1,0 );
//
//		s3d_flags_on( (*olsr_con)->obj_id, S3D_OF_VISIBLE );
//
//		s3d_link( (*olsr_con)->obj_id,  ZeroPoint );
//
//		/* HNA */
//		if ( etx == -1000.00 ) {
//
//			(*olsr_con)->left_etx = etx;
//			(*olsr_con)->left_etx_sqrt = 10.0;
//			(*olsr_con)->right_etx = etx;
//			(*olsr_con)->right_etx_sqrt = 10.0;
//
//		} else {
//
//			(*olsr_con)->left_etx = etx;
//			(*olsr_con)->left_etx_sqrt = sqrt( etx );
//			(*olsr_con)->right_etx = 999.0;
//			(*olsr_con)->right_etx_sqrt = sqrt( 999.0 );
//
//		}
//
//		(*olsr_con)->next_olsr_con = NULL;
//		(*olsr_con)->prev_olsr_con = prev_olsr_con;
//
//		/* add new olsr connection to olsr nodes in order to access the connection from the olsr node */
//		olsr_neigh_list = &(*olsr_con)->left_olsr_node->olsr_neigh_list;
//		while ( (*olsr_neigh_list) != NULL ) olsr_neigh_list = &(*olsr_neigh_list)->next_olsr_neigh_list;
//		(*olsr_neigh_list) = malloc( sizeof( struct olsr_neigh_list ) );
//		if ( (*olsr_neigh_list) == NULL ) out_of_mem();
//		(*olsr_neigh_list)->olsr_con = (*olsr_con);
//		(*olsr_neigh_list)->next_olsr_neigh_list = NULL;
//
//		olsr_neigh_list = &(*olsr_con)->right_olsr_node->olsr_neigh_list;
//		while ( (*olsr_neigh_list) != NULL ) olsr_neigh_list = &(*olsr_neigh_list)->next_olsr_neigh_list;
//		(*olsr_neigh_list) = malloc( sizeof( struct olsr_neigh_list ) );
//		if ( (*olsr_neigh_list) == NULL ) out_of_mem();
//		(*olsr_neigh_list)->olsr_con = (*olsr_con);
//		(*olsr_neigh_list)->next_olsr_neigh_list = NULL;
//
//	}
//
//	return(0);
//
//}
//
//
//
///***
// *
// * get pointer to olsr node or create new node if node string could not be found
// *
// *   **node =>   pointer to current olsr_node
// *   *ip    =>   node ip
// *
// *   return olsr node pointer
// *
// ***/
//
//void *get_olsr_node( struct olsr_node **olsr_node, char *ip ) {
//
//	int result;   /* result of strcmp */
//
//	while ( (*olsr_node) != NULL ) {
//
//		result = strncmp( (*olsr_node)->ip, ip, NAMEMAX );
//
//		/* we found the node */
//		if ( result == 0 ) {
//
//			(*olsr_node)->last_seen = Output_block_counter;
//
//			/* former invisble (deleted) node */
//			if ( (*olsr_node)->visible == 0 ) {
//
//				(*olsr_node)->node_type = 0;
//				(*olsr_node)->node_type_modified = 1;
//
//				(*olsr_node)->visible = 1;
//
//				(*olsr_node)->mov_vec[0] = (*olsr_node)->mov_vec[1] = (*olsr_node)->mov_vec[2] = 0.0;
//
//				if ( Debug ) printf( "new olsr node: %s\n", (*olsr_node)->ip );
//
//				Olsr_node_count++;
//
//			}
//
//			return (*olsr_node);
//
//		}
//
//		/* the searched node must be in the subtree */
//		if ( result < 0 ) {
//			olsr_node = &(*olsr_node)->right;
//		} else {
//			olsr_node = &(*olsr_node)->left;
//		}
//
//	}
//
//	/* if node is NULL we reached the end of the tree and must create a new olsr_node */
//	if ( (*olsr_node) == NULL ) {
//
//		(*olsr_node) = malloc( sizeof( struct olsr_node ) );
//		if ( (*olsr_node) == NULL ) out_of_mem();
//
//		(*olsr_node)->left = NULL;
//		(*olsr_node)->right = NULL;
//
//		strncpy( (*olsr_node)->ip, ip, NAMEMAX );
//
//		(*olsr_node)->node_type = 0;
//		(*olsr_node)->node_type_modified = 1;
//
//		(*olsr_node)->last_seen = Output_block_counter;
//		(*olsr_node)->visible = 1;
//
//		if ( Debug ) printf( "new olsr node: %s\n", (*olsr_node)->ip );
//
//		Olsr_node_count++;
//
//		(*olsr_node)->pos_vec[0] = ( ( float ) 2.0 * rand() ) / RAND_MAX - 1.0;
//		(*olsr_node)->pos_vec[1] = ( ( float ) 2.0 * rand() ) / RAND_MAX - 1.0;
//		(*olsr_node)->pos_vec[2] = ( ( float ) 2.0 * rand() ) / RAND_MAX - 1.0;
//		(*olsr_node)->mov_vec[0] = (*olsr_node)->mov_vec[1] = (*olsr_node)->mov_vec[2] = 0.0;
//
//		(*olsr_node)->obj_id = -1;
//		(*olsr_node)->desc_id = -1;
//		(*olsr_node)->olsr_neigh_list = NULL;
//
//		return (*olsr_node);
//
//	}
//	return(0);
//}
//
///*
// *
// * initialize the struct for a linked list obj2ip
// *
// */
//
//void lst_initialize() {
//	Obj_to_ip_head = (struct Obj_to_ip*) malloc(sizeof(struct Obj_to_ip));
//	Obj_to_ip_end = (struct Obj_to_ip*) malloc(sizeof(struct Obj_to_ip));
//	if(Obj_to_ip_head == NULL || Obj_to_ip_end == NULL)
//		out_of_mem();
//	Obj_to_ip_head->id = 0;
//	Obj_to_ip_end->id = 0;
//	Obj_to_ip_head->prev = Obj_to_ip_end->prev = Obj_to_ip_head;
//	Obj_to_ip_head->next = Obj_to_ip_end->next = Obj_to_ip_end;
//	List_ptr = Obj_to_ip_head;
//}
//
///*
// *
// * add a link object_id to olsr_node, to get ip adress and coordinates per object_id
// *                 id => object_id, returned from s3d_clone or s3d_new_object
// *  **olsr_node => pointer to pointer of current olsr_node
// *
// */
//
//void lst_add(int id,struct olsr_node **olsr_node) {
//	struct Obj_to_ip *new;
//	new = (struct Obj_to_ip*) malloc(sizeof(struct Obj_to_ip));
//	if(new == NULL)
//		out_of_mem();
//	new->id = id;
//	new->olsr_node = *olsr_node;
//	move_lst_ptr(&id);
//	new->prev = List_ptr;
//	new->next = List_ptr->next;
//	List_ptr->next->prev = new;
//	List_ptr->next = new;
//	/* printf("obj2ip: add object %d between %d .. %d ip %s to list\n",new->id,new->prev->id,new->next->id,new->olsr_node->ip); */
//}
//
///*
// *void move_lst_ptr(int *id)
// * remove element from obj2ip linked list
// * id => object_id, returned from s3d_clone or s3d_new_object
// *
// */
//
//void lst_del(int id) {
//	struct Obj_to_ip *del;
//	move_lst_ptr(&id);
//	if(id != List_ptr->id)
//	{
//		printf("obj2ip: remove id %d failed move_lst_ptr return id %d\n",id,List_ptr->next->id);
//	} else {
//		del = List_ptr;
//		List_ptr->next->prev = List_ptr->prev;
//		List_ptr->prev->next = List_ptr->next;
//		/* printf("obj2ip: remove object %d --> %d <-- %d ip %s from list\n",List_ptr->prev->id,del->id,List_ptr->next->id,del->olsr_node->ip); */
//		free(del);
//	}
//}
//
///*
// *
// * move the List_ptr one positon ahead the searched element
// *	*id => pointer of object_id , returned from s3d_clone or s3d_new_object
// *
// */
//
//struct olsr_node *move_lst_ptr(int *id) {
//	/* printf("obj2ip: move for %d\n",*id); */
//	/* head to point at end or id lass then first element in linked list*/
//	if(Obj_to_ip_head->next == Obj_to_ip_head || *id < Obj_to_ip_head->next->id) {
//		List_ptr = Obj_to_ip_head;
//		return NULL;
// 	/* id is greather then last element in linked list */
//	} else if(*id > Obj_to_ip_end->prev->id) {
//		List_ptr = Obj_to_ip_end->prev;
//		return NULL;
//	} else {
//		/* printf("obj2ip: ok i search deeper ;-) for id=%d\n",*id); */
//		if((*id - (int) Obj_to_ip_head->next->id) <= ((int)(Obj_to_ip_end->prev->id)-*id)) {
//			List_ptr = Obj_to_ip_head;
//			/* printf("obj2ip: start at head id %d - %d <= %d - %d \n",*id,Obj_to_ip_head->next->id,Obj_to_ip_end->prev->id,*id); */
//			while(*id >= List_ptr->next->id) {
//				/* printf("obj2ip: %d > %d move to ",*id,List_ptr->id); */
//				List_ptr = List_ptr->next;
//				/* printf("%d\n",List_ptr->id); */
//			}
//		} else {
//			List_ptr = Obj_to_ip_end;
//			/* printf("obj2ip: start at end id %d - %d > %d - %d \n",*id,Obj_to_ip_head->next->id,Obj_to_ip_end->prev->id,*id);  */
//			/*  do List_ptr = List_ptr->prev; while(*id > List_ptr->prev->id); */
//			while(*id < List_ptr->prev->id) {
//				/* printf("obj2ip: %d < %d move to ",*id,List_ptr->id); */
//				List_ptr = List_ptr->prev;
//				/* printf("%d\n",List_ptr->id); */
//			}
//			List_ptr = List_ptr->prev;
//		}
//
//		if ( List_ptr->id == *id )
//			return List_ptr->olsr_node;
//		else
//			return NULL;
//
//		/* printf("obj2ip: found id to insert between %d--> .. <--%d to search/delete %d--> .. <--%d\n",List_ptr->id,List_ptr->next->next->id,List_ptr->prev->id,List_ptr->next->id); */
//	}
//}
//
///*
// *
// * search a object_id in linked list and return pointer on struct olsr_node
// *	id => object_id , returned from s3d_clone or s3d_new_object
// *
// * <example>
// *     struct olsr_node *olsr_node;
// *     olsr_node = *lst_search(oid);
// *     printf("obj2ip: search return %s\n",olsr_node->ip);
// * </example>
// *
// */
//
//struct olsr_node *lst_search(int id) {
//
//	return( move_lst_ptr(&id) );
//
//}
//
//void lst_out() {
//	struct Obj_to_ip *ptr;
//	ptr = Obj_to_ip_head;
//	while(ptr != ptr->next) {
//		printf("id-> %d\n",ptr->id);
//		ptr = ptr->next;
//	}
//}
//
int process_main()
{

	int dn;
	float f;
	char *lbuf_ptr, *last_cr_ptr, *con_from, *con_from_end, *con_to, *con_to_end, *etx, *etx_end, *tmpChar;
	struct olsr_node *olsr_node1;   /* pointer to olsr nodes */
	struct olsr_node *olsr_node2;
	int address;
	char hna_name[NAMEMAX];
	char hna_node[NAMEMAX];

	lbuf_ptr = lbuf;
	last_cr_ptr = NULL;

	con_from = con_from_end = con_to = con_to_end = etx = etx_end = NULL;
	dn = 0;

	/*printf("---lbuf-start---\n%s\n---lbuf-end---\n",lbuf);*/

	while ( (*lbuf_ptr) != '\0' )
	{

		/* printf( "%c",(*lbuf_ptr) ); */

		if ( (*lbuf_ptr) == '\n' )
		{

			last_cr_ptr = lbuf_ptr;
			con_from = con_from_end = con_to = con_to_end = etx = etx_end = NULL;
			dn = 0;

		}

		if ( (*lbuf_ptr) == '"' )
		{
			switch ( dn )
			{
				case 0:
					con_from = lbuf_ptr + 1;
					break;
				case 1:
					con_from_end = lbuf_ptr;
					break;
				case 2:
					con_to = lbuf_ptr + 1;
					break;
				case 3:
					con_to_end = lbuf_ptr;
					break;
				case 4:
					etx = lbuf_ptr + 1;
					break;
				case 5:
					etx_end = lbuf_ptr;
					break;
			}

			if ( ++dn == 6 )
			{

				/* terminate strings - but not before 6 times '"' */
				(*con_from_end) = (*con_to_end) = (*etx_end) = '\0';

				if( Global.debug ) printf( "con_from: %s, con_to: %s, etx: %s\n", con_from, con_to, etx );

				/* announced network via HNA */
				if ( strncmp( etx, "HNA", NAMEMAX ) == 0 )
				{

					/* connection to internet */
					if ( strncmp( con_to, "0.0.0.0/0.0.0.0", NAMEMAX ) == 0 )
					{

//						olsr_node1 = get_olsr_node( &Olsr_root, con_from );

						if ( olsr_node1->node_type != 1 )
						{

							olsr_node1->node_type = 1;
							olsr_node1->node_type_modified = 1;
							if ( Global.debug ) printf( "new internet: %s\n", olsr_node1->ip );

						}

					/* normal HNA */
					} else {
						memmove(hna_node,con_to,NAMEMAX);
						if( (tmpChar = strchr(hna_node, (int)'/')))
						{
							tmpChar++;
							address = (int)-inet_network(tmpChar);
							sprintf(hna_name,"%d",(int)(32 - ceil(log(address)/log(2))));
							strcpy(tmpChar,hna_name);
						}

//						olsr_node1 = get_olsr_node( &Olsr_root, con_from );
//						olsr_node2 = get_olsr_node( &Olsr_root, hna_node );

						if ( olsr_node2->node_type != 2 ) {

							olsr_node2->node_type = 2;
							olsr_node2->node_type_modified = 1;
							if ( Global.debug ) printf( "new hna network: %s\n", olsr_node2->ip );

						}
//						if ( olsr_node1->visible && olsr_node2->visible )
//							add_olsr_con( olsr_node1, olsr_node2, -1000.00 );

					}

				/* normal node */
				} else {

//					olsr_node1 = get_olsr_node( &Olsr_root, con_from );
//					olsr_node2 = get_olsr_node( &Olsr_root, con_to );
					f = strtod(etx,NULL);
					if ( f < 1.0 )
						f = 999.0;
//					add_olsr_con( olsr_node1, olsr_node2, f );
				}
				/* remove zerobyte */
				(*con_from_end) = (*con_to_end) = (*etx_end) = '"';

				con_from = con_from_end = con_to = con_to_end = etx = etx_end = NULL;
				dn = 0;
				last_cr_ptr = lbuf_ptr;

			}

		} else if ( ( (*lbuf_ptr) == '}' ) && ( (*(lbuf_ptr + 1)) == '\n' ) ) {

//			Output_block_completed = 1;

		}

		lbuf_ptr++;

	}

	if ( last_cr_ptr != NULL ) memmove( lbuf, last_cr_ptr + 1, strlen( last_cr_ptr ) );
	/*printf("---memmove-lbuf-start---\n%s\n---memmove-lbuf-end---\n",lbuf);*/
	return(0);

}
