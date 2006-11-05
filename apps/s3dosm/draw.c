#include "s3dosm.h"
#include <s3d.h>
#include <math.h>	/* sin(), cos() */
#include <stdio.h>	/* printf() */
#define	ESIZE	6378

struct vdata{
	layer_t *layer;
	double lonsum,latsum;
	int n;
	int oid;
	int vnum;
};

void calc_earth_to_eukl(double lon, double lat, double *x)
{
	double la,lo;
	la=lat*M_PI/180.0;
	lo=lon*M_PI/180.0;
	x[0]=ESIZE*sin(lo) *cos(la);
	x[1]=ESIZE*			sin(la);
	x[2]=ESIZE*cos(lo) *cos(la);
}
void draw_add_vertices(object_t *t, void *data)
{
	struct vdata *v=data;
	
	if (t->type==T_NODE)
	{
		double x[3];
		node_t *node=NODE_T(t);
		node->vid=v->vnum;
		calc_earth_to_eukl(node->lon,node->lat,x);
		s3d_push_vertex(v->oid,x[0],x[1],x[2]);
		printf("vertex  %d: %f %f %f\n",node->vid,x[0],x[1],x[2]);
		v->vnum++;
		v->lonsum+=node->lon;
		v->latsum+=node->lat;
		v->n++;
	}
}

void draw_add_segments(object_t *t, void *data)
{
	struct vdata *v=data;
	if (t->type==T_SEGMENT)
	{
		node_t *from, *to;
		segment_t *seg=SEGMENT_T(t);
		from=NODE_T(avl_find(v->layer->tree,seg->from));
		to=NODE_T(avl_find(v->layer->tree,seg->to));
		if (from!=NULL && to!=NULL)
			s3d_push_line(v->oid,from->vid,to->vid,0);
	}
}
int oidx, oidy;
int draw_layer(layer_t *layer)
{
	struct vdata v;
	int oid;
	double lo,la,x[3];
	oid=s3d_new_object();
	oidx=s3d_new_object();
	oidy=s3d_new_object();
	s3d_link(oid,oidy);
	s3d_link(oidy,oidx);
	v.layer=layer;
	v.oid=oid;
	v.vnum=0;
	v.n=0;
	v.lonsum=v.latsum=0;
	s3d_push_material(oid,1,1,1,	1,1,1,	1,1,1);
	avl_tree_trav(layer->tree,draw_add_vertices,(void *)&v);
	avl_tree_trav(layer->tree,draw_add_segments,(void *)&v);
	lo=(v.lonsum)/v.n;
	la=(v.latsum)/v.n;
	s3d_rotate(oidy,0,-lo,0);
	s3d_rotate(oidx,-(90-la),0,0);
	calc_earth_to_eukl(lo,la,x);
	s3d_translate(oidx,0,-ESIZE*10,0);
	s3d_scale(oidx,10);
	s3d_flags_on(oid,S3D_OF_VISIBLE);
	return(0);

}
