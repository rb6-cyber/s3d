/*  this file should render truetype fonts as objects */
#include "s3d.h"
#include "s3dlib.h"
#include <stdlib.h>    /*  malloc(), free() */
#include <netinet/in.h>  /*  htonl(), htons() */
#include "ft2build.h"
#include FT_FREETYPE_H
#include <GL/glu.h>  /*  gluTess* */
#ifndef CALLBACK 
#define CALLBACK
#endif
/*  bad global vars ... */
static FT_Library 	library;
static FT_Face		face;
static unsigned char *memory_font=NULL;		 /*  the font file in memory */
static int memory_font_size=0;	 /*  and it's size, to reduce load times. */
static int ft_init=0;
static int face_init=0;

static GLUtesselator *tobj;
static int v_off; 	 /*  the vertex number offset, to have the right vertex numbers for each character */
static int f_oid;	 /*  the oid of our font string */
static int algo;	 /*  how the order of the vertex points should be interpreted (in the tesselator callbacks) */
static GLdouble *point;	 /*  the point of the outline points */
static int *pk,pn;		 /*  the index buffer and it's size */
static int ch;
struct t_buf tess_buf[256];

int _s3d_init_tessbuf();
int _s3d_clear_tessbuf();
int _s3d_add_tessbuf(unsigned short a);
int _s3d_draw_tessbuf(int oid,unsigned short a,int *voff, float *xoff);
/*  callbacks */
static void CALLBACK cb_vertex(GLdouble *v)
{
/* 	dprintf(LOW,"point is at %010p, while v is %010p, vertex nr. %d",point,v,v_off+(v-point)/3); */
	pk[pn]=v_off+(v-point)/3;
	pn++;
}
/*  taken from tess.c/ redbook source code */
static void CALLBACK cb_combine (GLdouble coords[3], 
                     GLdouble *vertex_data[4],
                     GLfloat weight[4], GLdouble **dataOut )
{
   GLdouble *vertex;
   int i;

   vertex = (GLdouble *) malloc(6 * sizeof(GLdouble));

   vertex[0] = coords[0];
   vertex[1] = coords[1];
   vertex[2] = coords[2];
   for (i = 3; i < 7; i++)
      vertex[i] = weight[0] * vertex_data[0][i] 
                  + weight[1] * vertex_data[1][i]
                  + weight[2] * vertex_data[2][i] 
                  + weight[3] * vertex_data[3][i];
   dprintf(LOW,"combining ...");
   *dataOut = vertex;
}

static void CALLBACK cb_error(GLenum errorCode)
{
   const GLubyte *estring;

   estring = gluErrorString(errorCode);
   errds(HIGH,"cb_error()","Tessellation Error: %s\n", estring);
/*    exit(0); */
}
static void CALLBACK cb_begin(GLenum which)
{
	algo=which;
	pn=0;
}
static void CALLBACK cb_end(GLenum which)
{
   int i;
   int poff=tess_buf[ch].pn;
   switch (algo)
   {
		case GL_TRIANGLES:
			tess_buf[ch].pn+=pn/3;
			tess_buf[ch].pbuf=realloc(tess_buf[ch].pbuf,tess_buf[ch].pn*sizeof(unsigned long)*4);
			for (i=0;i<(pn/3);i++)
			{
				tess_buf[ch].pbuf[(poff+i)*4]=	pk[i*3];
				tess_buf[ch].pbuf[(poff+i)*4+1]=pk[i*3+1];
				tess_buf[ch].pbuf[(poff+i)*4+2]=pk[i*3+2];
				tess_buf[ch].pbuf[(poff+i)*4+3]=0;
/* 				s3d_push_polygon(f_oid,pk[i*3],pk[i*3+1],pk[i*3+2],0); */
			}
			break;
		case GL_TRIANGLE_FAN:
			tess_buf[ch].pn+=pn-2;
			tess_buf[ch].pbuf=realloc(tess_buf[ch].pbuf,tess_buf[ch].pn*sizeof(unsigned long)*4);
/* 			for (i=1;i<(pn-1);i++) */
			for (i=0;i<(pn-2);i++)
			{
				tess_buf[ch].pbuf[(poff+i)*4]=	pk[0];
				tess_buf[ch].pbuf[(poff+i)*4+1]=pk[i+1];
				tess_buf[ch].pbuf[(poff+i)*4+2]=pk[i+2];
				tess_buf[ch].pbuf[(poff+i)*4+3]=0;
/* 				s3d_push_polygon(f_oid,pk[0],pk[i],pk[i+1],0); */
			}	
			break;
		case GL_TRIANGLE_STRIP:
			tess_buf[ch].pn+=pn-2;
			tess_buf[ch].pbuf=realloc(tess_buf[ch].pbuf,tess_buf[ch].pn*sizeof(unsigned long)*4);
		   	for (i=0;i<(pn-2);i++)
		   	{
				if (i%2)
				{
					tess_buf[ch].pbuf[(poff+i)*4]=	pk[i];
					tess_buf[ch].pbuf[(poff+i)*4+1]=pk[i+2];
					tess_buf[ch].pbuf[(poff+i)*4+2]=pk[i+1];
					tess_buf[ch].pbuf[(poff+i)*4+3]=0;
/* 					s3d_push_polygon(f_oid,pk[i],pk[i+2],pk[i+1],0); */
				} else {
					tess_buf[ch].pbuf[(poff+i)*4]=	pk[i];
					tess_buf[ch].pbuf[(poff+i)*4+1]=pk[i+1];
					tess_buf[ch].pbuf[(poff+i)*4+2]=pk[i+2];
					tess_buf[ch].pbuf[(poff+i)*4+3]=0;
/* 					s3d_push_polygon(f_oid,pk[i],pk[i+1],pk[i+2],0); */
				}
		   }
		   break;
		default: 
		    errds(MED,"cb_end()","tesselation method not supported");
   }
/*    dprintf(LOW,"character [%c]: %d + %d polys",ch,poff,tess_buf[ch].pn-poff); */
}

/*  that's about the callback functions, now the init for the tesselator */
/*  and the truetype part ... */
int s3d_ft_init()
{
	int error= FT_Init_FreeType( &library);
	if (error)
		return (-1);
   tobj = gluNewTess();
      gluTessProperty(tobj, GLU_TESS_WINDING_RULE,
                   GLU_TESS_WINDING_POSITIVE);
   gluTessCallback(tobj, GLU_TESS_VERTEX,	(GLvoid (*) ())cb_vertex);
   gluTessCallback(tobj, GLU_TESS_BEGIN,	(GLvoid (*) ())cb_begin);
   gluTessCallback(tobj, GLU_TESS_END,		(GLvoid (*) ())cb_end);
   gluTessCallback(tobj, GLU_TESS_ERROR,	(GLvoid (*) ())cb_error);
   gluTessCallback(tobj, GLU_TESS_COMBINE,  (GLvoid (*) ())cb_combine);
   _s3d_init_tessbuf();

   ft_init=1;
	return(0);
}

int s3d_ft_load_font()
{
	FT_Error error;
	if ((memory_font==NULL) || (memory_font_size==0))
	{
		errds(HIGH,"s3d_ft_load_font()","there is no font in memory, breaking");
		return(-1);
	}
	face_init=0;
	error= FT_New_Memory_Face(library,memory_font,memory_font_size,0,&face);
	switch (error)
	{
		case 0:
				face_init=1;
				break;
		case FT_Err_Unknown_File_Format:
				errds(HIGH,"s3d_ft_load_font()","bad font file format");
				return(-1);
				break;
		default:
				errds(HIGH,"s3d_ft_load_font()","couldn't load font for some reason (error %d)",error);
				return(-1);
				break;
	}
	return(0);
}
int _s3d_init_tessbuf()
{
	int i;
	for (i=0; i<256;i++)
	{
		tess_buf[i].vbuf=NULL;
		tess_buf[i].pbuf=NULL;
	}
	return(0);
}

int _s3d_clear_tessbuf()
{
	int i;
	for (i=0; i<256;i++)
	{
		if (tess_buf[i].vbuf) free(tess_buf[i].vbuf);
		if (tess_buf[i].pbuf) free(tess_buf[i].pbuf);
	}
	return(0);
}
/*  tessaltes a character and adds it to the buffer */
int _s3d_add_tessbuf(unsigned short a)
{
	float norm;
	int j,c;
	if (FT_Load_Char(face,a,	FT_LOAD_NO_BITMAP|FT_LOAD_NO_SCALE))
	{
		errds(VHIGH,"s3d_add_tessbuf():FT_Load_Char()","can't load character");
		return(-1);
	} 
	norm=1.0/face->glyph->metrics.vertAdvance;
	ch=a;
	v_off=0;
	if (face->glyph->outline.n_points)
	{
		j=0;
		tess_buf[a].vn=face->glyph->outline.n_points;
		tess_buf[a].vbuf=malloc(sizeof(float)*face->glyph->outline.n_points*3);
		point=(GLdouble *)malloc(sizeof(GLdouble)*face->glyph->outline.n_points*3);
		pk=(int *)malloc(sizeof(int)*face->glyph->outline.n_points); 
			 /*  our list which is to be filled with the array of vertex-indices for each  */
			 /*  convex polygon .... */
		gluTessBeginPolygon(tobj, NULL);
		for (c=0;c<face->glyph->outline.n_contours;c++)
		{
      		gluTessBeginContour(tobj);
			while (j<(face->glyph->outline.contours[c]+1))
			{
				point[j*3]=(GLdouble)(face->glyph->outline.points[j].x)*norm;
				point[j*3+1]=(GLdouble)face->glyph->outline.points[j].y*norm;
				point[j*3+2]=0.0;
				tess_buf[a].vbuf[j*3]=point[j*3];
				tess_buf[a].vbuf[j*3+1]=point[j*3+1];
				tess_buf[a].vbuf[j*3+2]=point[j*3+2];

/* 				s3d_push_vertex(f_oid,point[j*3],point[j*3+1],point[j*3+2]); */
       	 		gluTessVertex(tobj, &point[j*3],&point[j*3]);
				j++;
			}
      		gluTessEndContour(tobj);
		}
   		gluTessEndPolygon(tobj);
		
/* 		v_off+=face->glyph->outline.n_points; */
		free(pk);
		free(point);
	}
	tess_buf[a].xoff=1.0*face->glyph->metrics.horiAdvance*norm;
	return(0);
}
/*  tesselates a character and adds it to the buffer, without glu */
int _s3d_add_tessbuf_new(unsigned short a)
{
	float norm;
	int j,c,start;
	struct tessp_t *tessp;
	if (FT_Load_Char(face,a,	FT_LOAD_NO_BITMAP|FT_LOAD_NO_SCALE))
	{
		errds(VHIGH,"s3d_add_tessbuf():FT_Load_Char()","can't load character");
		return(-1);
	} 
	norm=1.0/face->glyph->metrics.vertAdvance;
	ch=a;
	v_off=0;
	if (face->glyph->outline.n_points)
	{
		j=0;
		tess_buf[a].vn=face->glyph->outline.n_points;
		tess_buf[a].vbuf=malloc(sizeof(float)*face->glyph->outline.n_points*3);
		tess_buf[a].pbuf=malloc(sizeof(unsigned long)*face->glyph->outline.n_points*4); /* should be enough ... */
		tessp=malloc(sizeof(struct tessp_t)*tess_buf[a].vn);
		
			 /*  our list which is to be filled with the array of vertex-indices for each  */
			 /*  convex polygon .... */
		for (c=0;c<face->glyph->outline.n_contours;c++)
		{
			start=j; 	/* first point */
			while (j<(face->glyph->outline.contours[c]+1))
			{
				tess_buf[a].vbuf[j*3]=face->glyph->outline.points[j].x*norm;
				tess_buf[a].vbuf[j*3+1]=face->glyph->outline.points[j].y*norm;
				tess_buf[a].vbuf[j*3+2]=0.0;
				tessp[j].prev=j-1;
				tessp[j].next=j+1;
				tessp[j].done=0;
				j++;
			}
			tessp[j-1].next=start;	/* last one */
			tessp[start].prev=j-1;  /* first one */
		}
		_s3d_tesselate(tessp,&tess_buf[a]);
	}
	tess_buf[a].xoff=1.0*face->glyph->metrics.horiAdvance*norm;
	return(0);
}
int _s3d_draw_tessbuf(int oid,unsigned short a,int *voff, float *xoff)
{
	float *vbuf;
	unsigned long *pbuf;
	int i;
	if (!(tess_buf[a].vbuf && tess_buf[a].pbuf))
		_s3d_add_tessbuf(a);
	vbuf=malloc(sizeof(float)*3*tess_buf[a].vn);
	pbuf=malloc(sizeof(unsigned long)*4*tess_buf[a].pn);
	memcpy(vbuf,tess_buf[a].vbuf,sizeof(float)*3*tess_buf[a].vn);
	memcpy(pbuf,tess_buf[a].pbuf,sizeof(unsigned long)*4*tess_buf[a].pn);
	 /*  prepare the buffs ... */
/* 	dprintf(LOW,"drawing [%c] (%d vertices, %d polys",a,tess_buf[a].vn,tess_buf[a].pn); */
	for (i=0;i<tess_buf[a].vn;i++)
	{
		vbuf[i*3]+=*xoff;
/*		dprintf(LOW,"vertex [%c:%d] %f %f %f",a,i,
						vbuf[i*3],
						vbuf[i*3+1],
						vbuf[i*3+2]);*/
	}
	for (i=0;i<tess_buf[a].pn;i++)
	{
		pbuf[i*4]+=*voff;
		pbuf[i*4+1]+=*voff;
		pbuf[i*4+2]+=*voff;
/*		dprintf(LOW,"poly [%c:%d] %d %d %d | %d (voff %d)",a,i,
						pbuf[i*4],
						pbuf[i*4+1],
						pbuf[i*4+2],
						pbuf[i*4+3],*voff);*/
		pbuf[i*4]=htonl(pbuf[i*4]);
		pbuf[i*4+1]=htonl(pbuf[i*4+1]);
		pbuf[i*4+2]=htonl(pbuf[i*4+2]);
		pbuf[i*4+3]=htonl(pbuf[i*4+3]);

	}
	dprintf(HIGH,"commiting %d vertices, %d polygons",tess_buf[a].vn,tess_buf[a].pn);
	s3d_push_vertices(oid,vbuf,tess_buf[a].vn);
	s3d_push_polygons(oid,pbuf,tess_buf[a].pn);
	*xoff+=tess_buf[a].xoff;  /*  xoffset */
	*voff+=tess_buf[a].vn;
	free(vbuf);
	free(pbuf);
	return(0);
}
int s3d_select_font(char *path)
{
	unsigned char *oldfont=memory_font;
	int oldsize=memory_font_size;
	char *c;
	if (!ft_init)
		if (s3d_ft_init())
		{
			errds(VHIGH,"s3d_select_font()","error in initializtation (ft_init())");
			return(-1);
		}

	 /*  yse (system-specific?!) font grabber */
	if (((c=s3d_findfont(path))!=NULL))
	{
		if ((memory_font_size=s3d_open_file(c,(char **)&memory_font))>0)
		{
			if (!s3d_ft_load_font())
			{
				_s3d_clear_tessbuf();
				if (oldfont!=NULL)
					free(oldfont);
				return(0);
			}
		}
	}

	 /*  failed too. restore and return */
	memory_font_size=	oldsize;
	memory_font=		oldfont;
	return(-1);
}

/*  draws a simple string. */
int s3d_draw_string( char *str,float *xlen)
{
	int i;
	float xoff;
	int voff;
	int len;
	if (!ft_init)
		if (s3d_ft_init())
		{
			errds(VHIGH,"s3d_draw_string()","error in initializtation (ft_init())");
			return(-1);
		}
	if (!face_init)
	{
		errds(VHIGH,"s3d_draw_string()","no font to draw with");
		return(-1);
	}
	f_oid=s3d_new_object(); 
	 /*  standard material */
	s3d_push_material(f_oid,1.0,1.0,1.0,		1.0,1.0,1.0,	1.0,1.0,1.0);
	xoff=0;
	voff=0; 
	len=strlen(str);
	for (i=0;i<len; i++)
		_s3d_draw_tessbuf(f_oid,(unsigned char )str[i],&voff,&xoff);
	 /*  s3d_ft_quit(); */
	if (xlen!=NULL) *xlen=xoff;
	return(f_oid);
}
int s3d_ft_quit()
{
	gluDeleteTess(tobj);
	FT_Done_FreeType(library);
	ft_init=0;
	return(0);
}


