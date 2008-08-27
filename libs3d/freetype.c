/*
 * freetype.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of the s3d API, the API of s3d (the 3d network display server).
 * See http://s3d.berlios.de/ for more updates.
 *
 * The s3d API is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * The s3d API is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the s3d API; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


/*  this file should render truetype fonts as objects */
#include "s3d.h"
#include "s3dlib.h"
#include "sei_interface.h" /* sei_triangulate_polygon() */
#include <stdlib.h>      /*  malloc(), free() */
#include <math.h>   /*  atan2() */
#include <string.h>   /*  strncmp(), strncpy() */
#include "ft2build.h"
#include FT_FREETYPE_H
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };

static const struct {
	int          err_code;
	const char*  err_msg;
} ft_errors[] =

#include FT_ERRORS_H

#ifndef CALLBACK
#define CALLBACK
#endif
        /*  bad global vars ... */
        static FT_Library  library;
static FT_Face  face;
static char *memory_font = NULL; /*  the font file in memory */
static char oldfontpath[256];
static int memory_font_size = 0;  /*  and it's size, to reduce load times. */
static int ft_init = 0;
static int face_init = 0;

static struct t_buf tess_buf[256];



/*  initialize truetype and tess_buf ... */
static int s3d_ft_init(void)
{
	int error = FT_Init_FreeType(&library);
	int i;
	oldfontpath[0] = 0;
	if (error)
		return (-1);
	ft_init = 1;
	for (i = 0; i < 256;i++) {
		tess_buf[i].vbuf = NULL;
		tess_buf[i].pbuf = NULL;
	}

	return(0);
}

static int s3d_ft_load_font(void)
{
	FT_Error error;
	if ((memory_font == NULL) || (memory_font_size == 0)) {
		errds(HIGH, "s3d_ft_load_font()", "there is no font in memory, breaking");
		return(-1);
	}
	face_init = 0;
	error = FT_New_Memory_Face(library, (uint8_t *)memory_font, memory_font_size, 0, &face);
	if (error) {
		errds(VHIGH, "s3d_ft_load_font():FT_New_Memory_Face", "can't load font : (%d) %s", ft_errors[error].err_code, ft_errors[error].err_msg);
		return(-1);
	}
	s3dprintf(LOW, "Load Font successful ...");
	face_init = 1;
	return(0);
}

static int _s3d_clear_tessbuf(void)
{
	int i;

	for (i = 0; i < 256;i++) {
		if (tess_buf[i].vbuf != NULL) free(tess_buf[i].vbuf);
		if (tess_buf[i].pbuf != NULL) free(tess_buf[i].pbuf);
	}
	for (i = 0; i < 256;i++) {
		tess_buf[i].vbuf = NULL;
		tess_buf[i].pbuf = NULL;
	}
	return(0);
}



/* renders a character with seidels algorithm and stores it in the tess_buf for later
 * usage */
static int _s3d_add_tessbuf(uint16_t a)
{
	float norm, ar, xa, ya;
	int i, j, k, c, n, outl, s, e;
	int np, pos;
	int triangles[SEI_SS*2][3]; /* more than enough ... */
	int ncontours, ncon;
	int cntr[SEI_SS];
	int ncntr[SEI_SS];
	int csta[SEI_SS], ncsta[SEI_SS];
	int perm[SEI_SS];
	float area[SEI_SS];
	double vertices[SEI_SS+1][2];
	double nvertices[SEI_SS+1][2];
	FT_Error error;

	error = FT_Load_Char(face, a, FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
	if (error) {

		errds(VHIGH, "_s3d_add_tessbuf():FT_Load_Char()", "can't load character %d : (%d) %s", a, ft_errors[error].err_code, ft_errors[error].err_msg);
		return(-1);
	}
	s3dprintf(VLOW, "[T]riangulating character %c", a);
	norm = 1.0 / face->glyph->metrics.vertAdvance;
	if ((face->glyph->outline.n_points > 0) && (face->glyph->outline.n_points < SEI_SS)) {
		tess_buf[a].vn = face->glyph->outline.n_points;
		tess_buf[a].vbuf = (float*)malloc(sizeof(float) * face->glyph->outline.n_points * 3);

		j = 0;
		ncontours = face->glyph->outline.n_contours;
		for (c = 0;c < ncontours;c++) {
			i = 0;
			ncon = face->glyph->outline.contours[c]; /* position of the end of ths contour */
			cntr[c] = ncon - j + 1;  /* how many points do we have here? */
			csta[c] = j + 1;
			ar = 0.0f;
			while (j < (ncon + 1)) {
				/* vertices have reverse order in seidels algorithm, outer contours go anticlockwise, inner contours clockwise */
				/* calculate the area */
				k = ((j + 2 - csta[c]) % (cntr[c])) + csta[c] - 1;
				ar -= face->glyph->outline.points[j].x * face->glyph->outline.points[k].y;
				ar += face->glyph->outline.points[k].x * face->glyph->outline.points[j].y;

				pos = ncon - i;
				vertices[pos+1][0] = face->glyph->outline.points[j].x * norm;
				vertices[pos+1][1] = face->glyph->outline.points[j].y * norm;
				j++;
				i++;
			}
			ar = 0.5f * norm * norm * ar;
			s3dprintf(VLOW, "contour %d has area of %3.3f, cntr is %d, contour starts at %d, ncon %d", c, ar, cntr[c], csta[c], ncon);
			area[c] = ar; /* save the area */
		}
		/* now as we have the areas and sizes of the contours, we need to order our contours so that
		 * the outlines and their holes are grouped together */
		n = ncontours;
		for (i = 0;i < n;i++)
			perm[i] = i; /* initialise permutation */
		while (n != 0) {
			outl = -1;
			/* find an outline */
			for (i = 0;i < n;i++)
				if (area[perm[i]] > 0) {
					outl = i; /* found. that was easy ;) */
					break;
				}
			if (outl == -1) {
				s3dprintf(HIGH, "hole without outline found, exiting ... %c", a);
				return(-1);
			}
			for (i = 0;i < n;i++) {
				if (area[perm[i]] < 0) {
					/* test for a hole inside by taking one (the first) point of the hole and doing the test */
					xa = vertices[csta[perm[i]]][0];
					ya = vertices[csta[perm[i]]][1];
					s = csta[perm[outl]];     /* start point of outline */
					e = (csta[perm[outl]] + cntr[perm[outl]]) - 1;  /* end point */
					ar = 0;
					for (j = s;j < e;j++) { /* for all points of the outline, sum: */
						ar += atan2((vertices[j+1][1] - ya) * (vertices[j][0] - xa) - (vertices[j+1][0] - xa) * (vertices[j][1] - ya),
						            (vertices[j+1][0] - xa) * (vertices[j][0] - xa) + (vertices[j+1][1] - ya) * (vertices[j][1] - ya));
					}
					/* dont forget the start/end-point connection*/
					ar += atan2((vertices[s][1] - ya) * (vertices[e][0] - xa) - (vertices[s][0] - xa) * (vertices[e][1] - ya),
					            (vertices[s][0] - xa) * (vertices[e][0] - xa) + (vertices[s][1] - ya) * (vertices[e][1] - ya));
					if (fabsf(ar) > 1)      /* if ar = 0.0, it's outside, elseway it's a multiple of pi. this check should be
                * very generous to roundoff errors */
					{
						s3dprintf(VLOW, "hole %d (%d) in %d (%d): interior angle sum %f (n=%d)", i, perm[i], outl, perm[outl], ar, n);
						j = perm[n-1]; /* swap our hole to the end */
						perm[n-1] = perm[i];
						perm[i] = j;
						if (outl == n - 1)
							outl = i;  /* outline got swapped */
						n--;   /* we don't care for the hole at the end anymore as it's found */
						i--;   /* check again for the just-swapped value in the next
           * loop iteration */
					}
				}
			}
			/* all the holes should be behind n-i, if so, so we swap our outline to the end now */
			j = perm[n-1]; /* swap our hole to the end */
			perm[n-1] = perm[outl];
			perm[outl] = j;
			n--;   /* we don't care for the hole at the end anymore as it's found */
		}
		/* finished the permutation, now apply the new order .... */
		n = 1;
		for (c = 0;c < ncontours;c++) {
			ncsta[c] = n - 1;
			for (j = csta[perm[c]];j < (csta[perm[c]] + cntr[perm[c]]);j++) {
				nvertices[n][0] = vertices[j][0];
				nvertices[n][1] = vertices[j][1];
				tess_buf[a].vbuf[(n-1)*3] = nvertices[n][0];
				tess_buf[a].vbuf[(n-1)*3+1] = nvertices[n][1];
				tess_buf[a].vbuf[(n-1)*3+2] = 0;
				n++;
			}
			ncntr[c] = cntr[perm[c]];
		}
		n = 0;
		tess_buf[a].pbuf = (uint32_t*)malloc(sizeof(uint32_t) * 4 * (face->glyph->outline.n_points + 2 * face->glyph->outline.n_contours));
		k = 0;
		for (c = ncontours - 1;c >= 0;c--) {
			n++;     /* count out and inlines ... */
			if (area[perm[c]] > 0) { /* outline? start! */
				s3dprintf(VLOW, "[T]riangulation from outline %d (%d contours, area = %f)", perm[c], n, area[perm[c]]);
				np = sei_triangulate_polygon(n, ncntr + c, nvertices + (ncsta[c]), triangles);
				for (i = 0;i < np;i++) {
					tess_buf[a].pbuf[k*4] =  triangles[i][0] + ncsta[c] - 1;
					tess_buf[a].pbuf[k*4+1] = triangles[i][2] + ncsta[c] - 1;
					tess_buf[a].pbuf[k*4+2] = triangles[i][1] + ncsta[c] - 1;
					tess_buf[a].pbuf[k*4+3] = 0;
					k++;
				}
				n = 0;
			}
		}
		tess_buf[a].pn = k;
	}
	tess_buf[a].xoff = 1.0 * face->glyph->metrics.horiAdvance * norm;
	return(0);
}

/* draws one charachter a */
static int _s3d_draw_tessbuf(int oid, uint16_t a, int *voff, float *xoff)
{
	float *vbuf;
	uint32_t *pbuf;
	int i;
	if (!(tess_buf[a].vbuf && tess_buf[a].pbuf))
		_s3d_add_tessbuf(a);
	/* only draw if it has some information in it */
	if ((tess_buf[a].pn != 0) && (tess_buf[a].vn != 0)) {

		vbuf = (float*)malloc(sizeof(float) * 3 * tess_buf[a].vn);
		pbuf = (uint32_t*)malloc(sizeof(uint32_t) * 4 * tess_buf[a].pn);
		memcpy(vbuf, tess_buf[a].vbuf, sizeof(float)*3*tess_buf[a].vn);
		memcpy(pbuf, tess_buf[a].pbuf, sizeof(uint32_t)*4*tess_buf[a].pn);
		/*  prepare the buffs ... */
		/*  s3dprintf(LOW,"drawing [%c] (%d vertices, %d polys",a,tess_buf[a].vn,tess_buf[a].pn); */
		for (i = 0;i < tess_buf[a].vn;i++) {
			vbuf[i*3] += *xoff;
			/*  s3dprintf(LOW,"vertex [%c:%d] %f %f %f",a,i,
			      vbuf[i*3],
			      vbuf[i*3+1],
			      vbuf[i*3+2]);*/
		}
		for (i = 0;i < tess_buf[a].pn;i++) {
			pbuf[i*4] += *voff;
			pbuf[i*4+1] += *voff;
			pbuf[i*4+2] += *voff;
			/*  s3dprintf(LOW,"poly [%c:%d] %d %d %d | %d (voff %d)",a,i,
			      pbuf[i*4],
			      pbuf[i*4+1],
			      pbuf[i*4+2],
			      pbuf[i*4+3],*voff);*/
		}
		s3dprintf(VLOW, "commiting %d vertices, %d polygons", tess_buf[a].vn, tess_buf[a].pn);
		s3d_push_vertices(oid, vbuf, tess_buf[a].vn);
		s3d_push_polygons(oid, pbuf, tess_buf[a].pn);
		*voff += tess_buf[a].vn;
		free(vbuf);
		free(pbuf);
	}
	*xoff += tess_buf[a].xoff;  /*  xoffset */
	return(0);
}
int s3d_select_font(const char *path)
{
	char    *oldfont = memory_font;
	int     oldsize = memory_font_size;
	char *c;
	char **p;
	if (!ft_init)
		if (s3d_ft_init()) {
			errds(VHIGH, "s3d_select_font()", "error in initializtation (ft_init())");
			return(-1);
		}
	if (strncmp(oldfontpath, path, 256) == 0) {
		s3dprintf(VLOW, "font already %s loaded.", path);
		return(-1);
	}
	/*  yse (system-specific?!) font grabber */
	if (((c = s3d_findfont(path)) != NULL)) {
		s3dprintf(LOW, "Loading Font %s ... ", c);
		_s3d_clear_tessbuf(); /* free and clear the tessbuf */
		p = &memory_font;
		if ((memory_font_size = s3d_open_file(c, p)) > 0) {
			if (!s3d_ft_load_font()) { /* success */
				if (oldfont != NULL)    free(oldfont);
				strncpy(oldfontpath, path, 256);
				return(0);
			} else {
				memory_font = oldfont;
				memory_font_size = oldsize;
			}
		} else {
			errds(VHIGH, "s3d_select_font()", "Could not open fontfile %s", c);
		}
	}
	return(-1);
}

/*  draws a simple string. */
int s3d_draw_string(const char *str, float *xlen)
{
	int i;
	float xoff;
	int voff;
	int len;
	uint32_t f_oid;
	if (!ft_init)
		if (s3d_ft_init()) {
			errds(VHIGH, "s3d_draw_string()", "error in initializtation (ft_init())");
			return(-1);
		}
	if (!face_init) {
		errds(VHIGH, "s3d_draw_string()", "no font to draw with");
		return(-1);
	}
	f_oid = s3d_new_object();
	/*  standard material */
	s3d_push_material(f_oid, 1.0, 1.0, 1.0,  1.0, 1.0, 1.0, 1.0, 1.0, 1.0);
	xoff = 0;
	voff = 0;
	len = strlen(str);
	for (i = 0;i < len; i++)
		_s3d_draw_tessbuf(f_oid, (uint8_t)str[i], &voff, &xoff);
	/*  s3d_ft_quit(); */
	if (xlen != NULL) *xlen = xoff;
	return(f_oid);
}
/* get the string length before actually drawing it. */
float s3d_strlen(const char *str)
{
	int i;
	float xoff;
	int len;
	uint16_t a;
	if (!ft_init)
		if (s3d_ft_init()) {
			errds(VHIGH, "s3d_draw_string()", "error in initializtation (ft_init())");
			return(0.0);
		}
	if (!face_init) {
		errds(VHIGH, "s3d_draw_string()", "no font to draw with");
		return(0.0);
	}
	/*  standard material */
	xoff = 0;
	len = strlen(str);
	for (i = 0;i < len; i++) {
		a = (uint8_t)str[i];
		if (!(tess_buf[a].vbuf && tess_buf[a].pbuf))
			_s3d_add_tessbuf(a);
		xoff += tess_buf[a].xoff;  /*  xoffset */
	}
	return(xoff);

}

#if 0
static int s3d_ft_quit(void)
{
	_s3d_clear_tessbuf();
	FT_Done_FreeType(library);
	ft_init = 0;
	return(0);
}
#endif /* 0 */
