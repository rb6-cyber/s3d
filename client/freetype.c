/*  this file should render truetype fonts as objects */
#include "s3d.h"
#include "s3dlib.h"
#include <stdlib.h>    /*  malloc(), free() */
#include <netinet/in.h>  /*  htonl(), htons() */
#include "ft2build.h"
#include FT_FREETYPE_H
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

static int v_off; 	 /*  the vertex number offset, to have the right vertex numbers for each character */
static int f_oid;	 /*  the oid of our font string */
static int ch;
struct t_buf tess_buf[256];



/*  initialize truetype and tess_buf ... */
int s3d_ft_init()
{
	int error= FT_Init_FreeType( &library);
	int i;
	if (error)
		return (-1);
    ft_init=1;
	for (i=0; i<256;i++)
	{
		tess_buf[i].vbuf=NULL;
		tess_buf[i].pbuf=NULL;
	}

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

/* renders a character with seidels algorithm and stores it in the tess_buf for later
 * usage */
int _s3d_add_tessbuf(unsigned short a)
{
	float norm;
	int i,j,k,c,start;
	int np,pos,diff,cpos,mpos;
	double vertices[SEI_SS+1][2];
	int triangles[SEI_SS*2][3]; /* more than enough ... */
	int ncontours,ncon;
	int cntr[SEI_SS];
	char used[SEI_SS];
	int map[SEI_SS+1];
	

	if (FT_Load_Char(face,a,	FT_LOAD_NO_BITMAP|FT_LOAD_NO_SCALE))
	{
		errds(VHIGH,"s3d_add_tessbuf():FT_Load_Char()","can't load character");
		return(-1);
	} 
	if (a=='%') return(-1);
	dprintf(LOW,"triangulating character %c",a);
	norm=1.0/face->glyph->metrics.vertAdvance;
	ch=a;
	v_off=0;
	if ((face->glyph->outline.n_points>0) && (face->glyph->outline.n_points<SEI_SS))
	{
		tess_buf[a].vn=face->glyph->outline.n_points;
		tess_buf[a].vbuf=malloc(sizeof(float)*face->glyph->outline.n_points*3);
		
		j=0;
		ncontours=face->glyph->outline.n_contours;
		for (c=0;c<ncontours;c++)
		{
			start=j; 	/* first point */
			i=0;
			ncon=face->glyph->outline.contours[c]; /* position of the end of ths contour */
			cntr[c]=ncon-j+1;
			while (j<(ncon+1))
			{
				/* vertices have reverse order in seidels algorithm, outer contours go anticlockwise, inner contours clockwise */
				pos=ncon-i;
				tess_buf[a].vbuf[pos*3]		=vertices[pos+1][0]=face->glyph->outline.points[j].x*norm;
				tess_buf[a].vbuf[pos*3+1]	=vertices[pos+1][1]=face->glyph->outline.points[j].y*norm;
				map[pos+1]=pos;
				tess_buf[a].vbuf[pos*3+2]	=0;
				j++;
				i++;
			}
		}
		k=0; /* polygon counter */
		/* iterate while there are untriangulated outlines left. this is neccesary
		 * because seidel will only operate on ONE outline at once (number of holes is not 
		 * limited though) */
		tess_buf[a].pbuf=malloc(sizeof(unsigned long)*4*(face->glyph->outline.n_points+2*face->glyph->outline.n_contours)); 
		do {
			dprintf(LOW,"triangulating %d contours", ncontours);
			for (i=0;i<ncontours;i++)
				dprintf(LOW,"[%d]: %d points ", i, cntr[i]);
			np=sei_triangulate_polygon(ncontours, cntr, vertices, triangles);
			dprintf(LOW,"[F]ound %d polygons",np);
			memset(used,0,ncontours);
			for (i=0;i<np;i++)
			{
				tess_buf[a].pbuf[k*4]=  map[triangles[i][0]];
				tess_buf[a].pbuf[k*4+1]=map[triangles[i][1]];
				tess_buf[a].pbuf[k*4+2]=map[triangles[i][2]];
				tess_buf[a].pbuf[k*4+3]=0;
				dprintf(LOW,"TRIANG: %d %d %d = %d %d %d",	triangles[i][0],triangles[i][1],triangles[i][2], 
															map[triangles[i][0]], map[triangles[i][1]], map[triangles[i][2]]);
				for (j=0;j<3;j++)
				{
					cpos=1;
					for (c=0;c<ncontours;c++)
					{
						cpos+=cntr[c];
						if (triangles[i][j]<cpos)
						{
/*							dprintf(LOW,"point %d in contour line %d (cpos = %d) used",triangles[i][j],c,cpos);*/
							used[c]=1;
							break;
						}
					}
				}
				k++;
			}
			j=1;
			for (c=0;c<ncontours;c++)
			{
				j&=used[c];
			}
			if (j) 
				dprintf(LOW,"all contours used");
			else 
			{
				dprintf(LOW,"not all contours used, restarting");
				diff=0;
				ncon=0; /* number of actually unused contours */
				cpos=1; /* position of source vertices */
				mpos=1; /* position of dest vertices */
				for (c=0;c<ncontours;c++)
				{
					if (!used[c])
					{
					  /* not used, move it to new end */
						dprintf(LOW,"contour %d (%d) not used!!",c,cntr[c]);
						cntr[ncon]=cntr[c];
						ncon++;
						if (cpos!=mpos)
						{
							for (i=0;i<cntr[c];i++)
							{
								vertices[mpos+i][0]=vertices[cpos+i][0];
								vertices[mpos+i][1]=vertices[cpos+i][1];
								map[mpos+i]=map[cpos+i];
							}
						}
					}
					cpos+=cntr[c];
				}
			}
			ncontours=ncon;
		} while (!j);
		tess_buf[a].pn=k;
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
	dprintf(VLOW,"commiting %d vertices, %d polygons",tess_buf[a].vn,tess_buf[a].pn);
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
	_s3d_clear_tessbuf();
	FT_Done_FreeType(library);
	ft_init=0;
	return(0);
}


