#include <stdio.h> 	/* NULL */
#include <string.h> 	/* strlen(), memmove() */
#include <stdlib.h> 	/* rand(), malloc(), realloc(), free(), strtof() */
#include <s3d.h>
#include "olsrs3d.h"
char lbuf[MAXLINESIZE];
int max,new_max;
float 	*adj,*new_adj;
int		*adj_obj,*new_adj_obj;


/***
 *
 * create new or alter connection between 2 nodes
 *
 *   con_from =>   current node
 *   con_to   =>   node to connect to
 *   etx      =>   ETX
 *
 ***/

int add_olsr_con( struct olsr_node *con_from, struct olsr_node *con_to, float etx ) {

	struct olsr_con **olsr_con = &Con_begin;

	while ( (*olsr_con) != NULL ) {

		/* connection already exists */
		if ( ( strncmp( (*olsr_con)->left_olsr_node->ip, con_from->ip, NAMEMAX ) == 0 ) && ( strncmp( (*olsr_con)->right_olsr_node->ip, con_to->ip, NAMEMAX ) == 0 ) ) {
			(*olsr_con)->left_etx = etx;
			break;

		} else if ( ( strncmp( (*olsr_con)->right_olsr_node->ip, con_from->ip, NAMEMAX ) == 0 ) && ( strncmp( (*olsr_con)->left_olsr_node->ip, con_to->ip, NAMEMAX ) == 0 ) ) {

			(*olsr_con)->right_etx = etx;
			break;

		}

		olsr_con = &(*olsr_con)->next_olsr_con;

	}

	/* new connection */
	if ( (*olsr_con) == NULL ) {

		(*olsr_con) = malloc( sizeof( struct olsr_con ) );
		if ( (*olsr_con) == NULL ) out_of_mem();

		/* create connection object */
		(*olsr_con)->obj_id = s3d_new_object();
		s3d_push_material( (*olsr_con)->obj_id,
				  1.0,1.0,1.0,
				  1.0,1.0,1.0,
				  1.0,1.0,1.0);

		s3d_flags_on( (*olsr_con)->obj_id, S3D_OF_VISIBLE );

		s3d_push_polygon( (*olsr_con)->obj_id, 0,4,5,0 );
		s3d_push_polygon( (*olsr_con)->obj_id, 3,1,2,0 );

		/* add olsr node to new olsr connection in order to access the nodes from the connection list */
		(*olsr_con)->left_olsr_node = con_from;
		(*olsr_con)->right_olsr_node = con_to;

		(*olsr_con)->left_etx = etx;
		(*olsr_con)->right_etx = 0.0;

		(*olsr_con)->next_olsr_con = NULL;

		/* add new olsr connection to olsr nodes in order to access the connection from the olsr node */
		struct olsr_con_list **olsr_con_list = &(*olsr_con)->left_olsr_node->olsr_con_list;
		while ( (*olsr_con_list) != NULL ) olsr_con_list = &(*olsr_con_list)->next_olsr_con_list;
		(*olsr_con_list) = malloc( sizeof( struct olsr_con_list ) );
		if ( (*olsr_con_list) == NULL ) out_of_mem();
		(*olsr_con_list)->olsr_con = (*olsr_con);
		(*olsr_con_list)->next_olsr_con_list = NULL;

		olsr_con_list = &(*olsr_con)->right_olsr_node->olsr_con_list;
		while ( (*olsr_con_list) != NULL ) olsr_con_list = &(*olsr_con_list)->next_olsr_con_list;
		(*olsr_con_list) = malloc( sizeof( struct olsr_con_list ) );
		if ( (*olsr_con_list) == NULL ) out_of_mem();
		(*olsr_con_list)->olsr_con = (*olsr_con);
		(*olsr_con_list)->next_olsr_con_list = NULL;

	}

}



/***
 *
 * create new or alter connection between 2 nodes
 *
 *   n1   =>   node id 1
 *   n2   =>   node id 2
 *   l    =>   length ? ETX ?
 *
 ***/

int add_adj(int n1, int n2, float l)
{
	int o,i,j;
	i=n1<n2?n1:n2;
	j=n1>n2?n1:n2;
	if ((n1<max) && (n2<max))
		/* connection already exists */
		o=adj_obj[i*max+j];
	else o=-1;
	if (o==-1)
	{ /* need to generate new object .. */
		o=s3d_new_object();
		s3d_push_material(o,1.0,1.0,1.0,
							1.0,1.0,1.0,
							1.0,1.0,1.0);
		s3d_flags_on(o,S3D_OF_VISIBLE);
		s3d_push_polygon(o,0,4,5,0);
		s3d_push_polygon(o,3,1,2,0);

/*		printf("new adjacent object %d between %d and %d\n",o,i,j);*/
	}
	new_adj_obj[i*new_max+j]=o;
	new_adj[i*new_max+j]=l;
	new_adj[j*new_max+i]=l;

	return(0);
}



/***
 *
 * redo all connections between nodes ?
 *
 ***/

int resize_adj()
{
	int i,ind;
	new_adj=realloc(new_adj,sizeof(float)*new_max*new_max);
	new_adj_obj=realloc(new_adj_obj,sizeof(int)*new_max*new_max);

	if (new_max>1)
	{
		/* leave out one ? */
		for (i=(new_max-2);i>=0;i--)
		{
			memmove(new_adj+new_max*i,new_adj+(new_max-1)*i,sizeof(float)*(new_max-1));
			memmove(new_adj_obj+new_max*i,new_adj_obj+(new_max-1)*i,sizeof(float)*(new_max-1));
			ind=i*new_max+(new_max-1);				/* the right edge */
			new_adj[ind]=0.0f;
			new_adj_obj[ind]=-1;
			ind=(new_max-1)*new_max+i;				/* the bottom edge */
			new_adj[ind]=0.0f;
			new_adj_obj[ind]=-1;
		}
	}
	new_adj[new_max*new_max-1]=0.0f;			/* the right bottom corner */
	new_adj_obj[new_max*new_max-1]=-1;			/* the right bottom corner */
	return(0);
}



/***
 *
 * get pointer to olsr node or create new node if node string could not be found
 *
 *   **node =>   pointer to current olsr_node
 *   *ip    =>   node ip
 *
 *   return olsr node pointer
 *
 ***/

void *get_olsr_node( struct olsr_node **olsr_node, char *ip ) {

	int result;   /* result of strcmp */

	while ( (*olsr_node) != NULL ) {

		result = strncmp( (*olsr_node)->ip, ip, NAMEMAX );

		/* we found the node */
		if ( result == 0 ) return (*olsr_node);

		/* the searched node must be in the subtree */
		if ( result < 0 ) {
			olsr_node = &(*olsr_node)->right;
		} else {
			olsr_node = &(*olsr_node)->left;
		}

	}

	/* if node is NULL we reached the end of the tree and must create a new olsr_node */
	if ( (*olsr_node) == NULL ) {

		(*olsr_node) = malloc( sizeof( struct olsr_node ) );
		if ( (*olsr_node) == NULL ) out_of_mem();

		(*olsr_node)->left = NULL;
		(*olsr_node)->right = NULL;
		strncpy( (*olsr_node)->ip, ip, NAMEMAX );
		(*olsr_node)->inet_gw = 0;
		(*olsr_node)->inet_gw_modified = 1;

		if ( Debug ) printf( "new olsr node: %s\n", (*olsr_node)->ip );

		(*olsr_node)->pos_vec[0] = ( ( float ) 2.0 * rand() ) / RAND_MAX - 1.0;
		(*olsr_node)->pos_vec[1] = ( ( float ) 2.0 * rand() ) / RAND_MAX - 1.0;
		(*olsr_node)->pos_vec[2] = ( ( float ) 2.0 * rand() ) / RAND_MAX - 1.0;
		(*olsr_node)->mov_vec[0] = (*olsr_node)->mov_vec[1] = (*olsr_node)->mov_vec[2] = 0.0;

		(*olsr_node)->obj_id = -1;
		(*olsr_node)->desc_id = -1;
		(*olsr_node)->olsr_con_list = NULL;

		return (*olsr_node);

	}

}



/***
 *
 * get node id or create new node if node string could not be found
 *
 *   *str   =>   node description
 *
 *   return node id
 *
 ***/

int get_node_num(char *str)
{
	int i,j;   /* inc vars */
	float f;   /* where does it get its value from ? */

	for (i=0;i<new_max;i++)
	{
		if (strncmp(node[i].name,str,NAMEMAX)==0)
		{
			return(i); /* return the index */
		}
	}

	/* i==new_max now */
	new_max++;

	node=realloc(node,sizeof(struct t_node)*new_max);

	/* create new node */
	strncpy(node[i].name,str,NAMEMAX);
	node[i].obj=s3d_clone(Olsr_node_obj);
	s3d_flags_on(node[i].obj,S3D_OF_VISIBLE);

	/* create node text and attach (link) it to the node */
	node[i].s_obj=s3d_draw_string(str,&f);
	s3d_link(node[i].s_obj, node[i].obj);
	s3d_translate(node[i].s_obj,-f/2,-2,0);
	/*s3d_rotate(node[i].s_obj,0,180,0);*/
	s3d_flags_on(node[i].s_obj,S3D_OF_VISIBLE);

	printf("new %s [%d], Olsr_node_obj nr. %d - %d\n",str,i,node[i].obj,node[i].s_obj);

	for (j=0;j<3;j++)
	{
		node[i].pos[j]=((float)2.0*rand())/RAND_MAX-1.0;
		node[i].mov[j]=0.0;
	}

	resize_adj();

	return(i);
}

int commit_input()
{
	int 	i,j;
	float 	*swap_adj;
	int  	*swap_adj_obj;
	char	nc_str[20];
	printf("committing input ... \n");

	/* remove old adjacent objects ... */
// 	for (i=0;i<max;i++)
// 		for (j=i+1;j<max;j++)
// 			if (adj_obj[i*max+j]!=-1)
// 				if (new_adj_obj[i*new_max+j]==-1)
// 				{
// /*					printf("old link does not exist anymore ...\n");*/
// 					/* this link does not exist anymore ... */
// 					s3d_del_object(adj_obj[i*max+j]);
// 				}
	/* swap the matrices */
// 	swap_adj=adj;
// 	swap_adj_obj=adj_obj;
//
// 	adj=new_adj;
// 	adj_obj=new_adj_obj;


	/* if we have more nodes redraw node count */
	/* what if we have less nodes ?? */
	if (new_max>max)
	{
		swap_adj=realloc(swap_adj,sizeof(float)*new_max*new_max);
		swap_adj_obj=realloc(swap_adj_obj,sizeof(int)*new_max*new_max);
		s3d_del_object(node_count);
		snprintf(nc_str,20,"node count: %d",new_max);
		node_count=s3d_draw_string(nc_str,NULL);
		s3d_link(node_count,0);
		s3d_flags_on(node_count,S3D_OF_VISIBLE);
		s3d_scale(node_count,0.2);
		s3d_translate(node_count,left*3.0,-bottom*3.0-0.2,-3.0);

	}


// 	new_adj=swap_adj;
// 	new_adj_obj=swap_adj_obj;
	/* setting new maxsize */
	max=new_max;
	/* resetting the input-matrices*/
// 	for (i=0;i<max;i++)
// 	for (j=0;j<max;j++)
// 	{
// 		new_adj[i*max+j]=0.0;
// 		new_adj_obj[i*max+j]=-1;
// 	}
	return(0);
}
int parse_line(int n)
{
	char *data[3];   // in this order: ip_from, ip_to, label
	struct olsr_node *olsr_node1;   // pointer to olsr nodes
	struct olsr_node *olsr_node2;
	int i,dn,n1,n2;
	float f;
	data[0]=data[1]=data[2]=NULL;
	lbuf[n]='\0'; /* we don't need this one anyway */
	i=dn=0;
	while (i<n)
	{
		switch (lbuf[i])
		{
			case '"':
				if (dn<6)
				{
					if (!(dn%2)) /* starts */
						data[(dn/2)]=lbuf+i+1;
					else /* ends */
						lbuf[i]='\0'; /* string terminator!! */
				}
				dn++;
				break;
			case '}':
				if (!(dn%2))	/* we don't end the input inside of strings ... this won't happen anyway, I guess */
					commit_input();
				break;
		}
		i++;
	}
	if (dn>=6)
	{
/*		printf("######link from [%s] to [%s], label [%s]\n",data[0],data[1],data[2]);*/
		/* announced network via HNA */
		if ( strncmp( data[2], "HNA", NAMEMAX ) == 0 ) {

			/* connection to internet */
			if ( strncmp( data[1], "0.0.0.0/0.0.0.0", NAMEMAX ) == 0 ) {

				olsr_node1 = get_olsr_node( &Olsr_root, data[0] );

				if ( olsr_node1->inet_gw == 0 ) {

					olsr_node1->inet_gw = 1;
					olsr_node1->inet_gw_modified = 1;
					if ( Debug ) printf( "new internet: %s\n", olsr_node1->ip );

				}

			}

			/* TODO: other HNA hast to be done */

		/* normal node */
		} else {
// 			n1=get_node_num(data[0]);
// 			n2=get_node_num(data[1]);
			olsr_node1 = get_olsr_node( &Olsr_root, data[0] );
			olsr_node2 = get_olsr_node( &Olsr_root, data[1] );
			f=10.0+strtod(data[2],NULL)/10.0;
/*		printf("######link from %d to %d, %f, %d\n",n1,n2,f, f>=10);*/
			if (f>=5) /* just to prevent ascii to float converting inconsistency ... */
// 				add_adj(n1,n2,f);
				add_olsr_con( olsr_node1, olsr_node2, f );
		}
	}
	return(0);
}
int process_main()
{
	int i,l;
	i=0;
	l=strlen(lbuf);
	while (i<l)
	{
		if ((lbuf[i])=='\n')
		{
			parse_line(i);
			memmove(lbuf,lbuf+i+1,MAXLINESIZE-i-1);
			process_main(); /* well, we don't have to do this the recursive way here, but who cares ... */
			return(0);
		}
		i++;
	}
	return(0);
}
int process_init()
{
	lbuf[0]='\0';
	max=new_max=0;
	new_adj=adj=NULL;
	new_adj_obj=adj_obj=NULL;
	node=NULL;
	return(0);
}

int process_quit()
{
	if (adj!=NULL) 		free(adj);
	if (adj_obj!=NULL) 	free(adj_obj);
	if (node!=NULL) 	free(node);
	if (new_adj!=NULL) 	free(new_adj);
	if (new_adj_obj!=NULL) 	free(new_adj_obj);
	return(0);
}
