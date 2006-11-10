/*
 * avl.c
 * 
 * Copyright (C) 2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3dosm, a gps card application for s3d.
 * See http://s3d.berlios.de/ for more updates.
 * 
 * s3dosm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * s3dosm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with s3dosm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "s3dosm.h"
#include <stdio.h>  /* printf(), NULL */
#include <stdlib.h> /* malloc(), free() */
		
void avl_tree_trav(object_t *t, avl_func func, void *data)
{
	if (t!=NULL)
	{
		avl_tree_trav(t->left,func,data);
		func(t,data);
		avl_tree_trav(t->right,func,data);
	}
}
/* avl_find a certain value */
object_t *avl_find(object_t *t, int val)
{
	object_t *ret=NULL;
	while (t!=NULL) 
	{
		if (t->id == val)
		{
			ret=t;
			t=NULL;
		}
		else if (val > t->id) t=t->right;
		else if (val < t->id) t=t->left;
	}
	return(ret);
}
static int s;
object_t *avl_rotate_right(object_t *t)
{
	object_t *b;
	b=t->right;
/*	printf(">> right rotate at %d [%d] with %d [%d]\n",t->id,t->bal,b->id,b->bal);*/
	t->right=b->left;
	b->left=t;
	return b;
}
object_t *avl_rotate_left(object_t *t)
{
	object_t *b;
	b=t->left;
/*	printf("<< left rotate at %d [%d] with %d [%d]\n",t->id,t->bal,b->id,b->bal);*/
	t->left=b->right;
	b->right=t;
	return b;
}

object_t *avl_insert(object_t *t, object_t *nn)
{
	int i;

	if (t == NULL) { 
		s=1;
		t=nn;
	} else  {
	    if (nn->id > t->id) 
		{ 
			i = t->bal; /* might be -1 */
			t->right=avl_insert(t->right,nn);
			if ( s == 1) /* we can still move */
			{ 
						if (i == -1) 	{	t->bal=0;	s=0;}
				else	if (i == 0)  		t->bal=1;
				else 	if (i == 1) 
				{
					if (t->right->bal == -1)
					{ /* Doppelrotation */
						t->right=avl_rotate_left(t->right);
						t=avl_rotate_right(t);
						t->left->bal=0;
						t->right->bal=0;
						if (t->bal == -1)  t->right->bal=1;
						if (t->bal == 1)   t->left->bal=-1;
						t->bal=0;
					} else {
						t->bal=0;
						t=avl_rotate_right(t);
						t->bal=t->bal - 1 ;
					}

					s=0;
				}
			}
		}
		else if (nn->id < t->id) 
		{
			i = t->bal; /* might be 1 */
			t->left=avl_insert(t->left,nn);

			if ( s == 1) /* we can still move */
			{ 
				if (i == 1) {
					t->bal=0;
					s=0;
				} else if (i == 0) 
					t->bal=-1;
				else if (i == -1) 
				{
					if (t->left->bal == 1)
					{  /* Doppelrotation */
						t->left=avl_rotate_right(t->left);
						t=avl_rotate_left(t);
						t->left->bal=0;
						t->right->bal=0;
						if (t->bal == -1) t->left->bal=-1;
						if (t->bal == 1)  t->right->bal=1;
						t->bal=0;
					} else {
						t->bal=0;
						t=avl_rotate_left(t);
						t->bal=t->bal + 1 ;
					}
					s=0;
				}
			}
		}
	}
	return(t);
}
object_t *avl_leftmost(object_t *t)
{
	while (t->left!=NULL) 
	{
		if (t->left!=NULL)			t=t->left;
	} 
	
	return(t);
}
object_t *avl_rightmost(object_t *t)
{
	while (t->right!=NULL) 
	{
		if (t->right!=NULL)		t=t->right;
	}
	return(t);
}
object_t *avl_remove(object_t *t, object_t *nn)
{
	int i;
	s=0;
	if (t == NULL) { 
/*		printf("object %d not found\n",nn->id);*/
		t=nn;
	} else {
	    if (nn->id > t->id) 
		{ 
/*			printf("go right at %d\n",t->id);*/
			i = t->bal; /* might be -1 */
			t->right=avl_remove(t->right,nn);
			if ( s == 1)  /* we can still move */
			{ 
					 if (i == 1) {	t->bal=0;	}
				else if (i == 0) {	t->bal=-1;  s=0;	}
				else if (i == -1) { 
/*					printf("[T]rouble left at object_t *%d\n",t->id);*/

					if (t->left->bal == 1)
					{  /* Doppelrotation */
						t->left=avl_rotate_right(t->left);
						t=avl_rotate_left(t);
						t->left->bal=0;
						t->right->bal=0;
						if (t->bal == -1)  t->left->bal=-1;
						if (t->bal == 1)   t->right->bal=1;
						t->bal=0;
					} else {
						t->bal=0;
						t=avl_rotate_left(t);
						t->bal=t->bal + 1 ;
					}
				}

			}
		} else if (nn->id < t->id) 
		{
/*			printf("go left at %d\n",t->id);*/

			i = t->bal; /* might be 1 */
			t->left=avl_remove(t->left,nn);

			if ( s == 1) /* we can still move */
			{ 
					 if (i == -1) 	{ 	t->bal=0;}
				else if (i == 0) 	{	t->bal=1;	s=0;}
				else if (i == 1) { 
/*					printf("[T]rouble right at object_t *%d\n",t->id);*/

					if (t->right->bal == -1)
					{ /* Doppelrotation */
						t->right=avl_rotate_left(t->right);
						t=avl_rotate_right(t);
						t->left->bal=0;
						t->right->bal=0;
						if (t->bal == -1)   t->right->bal=1;
						if (t->bal == 1)    t->left->bal=-1;
						t->bal=0;
					} else {
						t->bal=0;
						t=avl_rotate_right(t);
						t->bal=t->bal - 1 ;
					}

				}
			}
		} if (nn->id == t->id)
		{
/*			printf("found, removing ...\n");*/
			if (t->left==NULL && t->right==NULL)
			{ /* leaf */
				s=1;
				t=NULL;
			} else {
				object_t *xchg=NULL;
				if (t->right!=NULL) xchg=avl_leftmost(t->right);
				else 				xchg=avl_rightmost(t->left);
/*				printf("using %d as exchange node\n",xchg->id);*/
				avl_remove(t, xchg);	/* remove the leaf */
				xchg->left=t->left;
				xchg->right=t->right;
				xchg->bal=t->bal;
				t=xchg; /* don't set s, keep the value from avl_remove() */
			}
		}
	}
/*	if (t!=NULL) printf("balance of %d is now %d\n",t->id, t->bal);*/
	return t;
}

int avl_height(object_t *t)
{
	int h1,h2;
	if (t == NULL) 	return(0);
	else {
		h1=avl_height(t->left);
		h2=avl_height(t->right);
		if (h1>h2)	if (h1 == 0) return(0); else return(h1+1);
		else 		if (h2 == 0) return(0); else return(h2+1);
	}
}
/*
void avl_test()
{
	object_t *tree;

	tree=NULL;
	tree=avl_insert(tree,object_new(5));
	avl_print_tree(tree);
	printf("\n");

	tree=avl_insert(tree,object_new(1));
	avl_print_tree(tree);
	printf("\n");

	tree=avl_insert(tree,object_new(2));
	avl_print_tree(tree);
	printf("\n");
	tree=avl_insert(tree,object_new(7));
	avl_print_tree(tree);
	printf("\n");

	tree=avl_insert(tree,object_new(9));
	avl_print_tree(tree);
	printf("\n");

	tree=avl_insert(tree,object_new(3));
	avl_print_tree(tree);
	printf("\n");

	tree=avl_insert(tree,object_new(10));
	avl_print_tree(tree);
	printf("\n");

	tree=avl_insert(tree,object_new(12));
	avl_print_tree(tree);
	printf("\n");


	tree=avl_remove(tree,avl_find(tree,3));
	tree=avl_remove(tree,avl_find(tree,1));
	tree=avl_remove(tree,avl_find(tree,9));
	tree=avl_remove(tree,avl_find(tree,5));
	tree=avl_remove(tree,avl_find(tree,7));

	printf("my tree:\n");
	avl_print_tree(tree);
	printf("\n");
}
void avl_print_tree(object_t *n)
{ 
	if (n != NULL) 
	{ 
		printf(" (");
		avl_print_tree(n->left);
		printf("%d [%d]",n->id, n->bal);
		avl_print_tree(n->right);
		printf(") ");
	}
}
object_t *avl_findbest(object_t *t,int n)
{
	object_t *ret=NULL;
	while (t != NULL) 
	{
		ret=t;
		if (n >= t->id) t=t->right;
		else if (n < t->id) t=t->left;
	}
	return(ret);
}
*/

