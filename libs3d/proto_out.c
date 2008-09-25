/*
 * proto_out.c
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


#include "s3d.h"
#include "s3dlib.h"
#include <proto.h>
#include <string.h>   /*  memset(),strncpy(),strncmp(),memcpy() */
#include <errno.h>   /*  errno */
#include <netinet/in.h>  /*  htons(),htonl() */
#include <unistd.h>   /*  select() */
#include <stdlib.h>   /*  getenv(),atoi(), malloc() */
#ifdef WIN32
#define uint32_t uint32_t  /*  sohn */
#else
#include <netdb.h>   /*  gethostbyname()  */
#endif

#define MF_LEN 65530  /*  maximum fragmentation length */
/*  creates a new object */
int s3d_new_object(void)
{
	int oid;

	cb_lock++; /* please, no callbacks now. */
	oid = _queue_want_object();
	cb_lock--; /* no new callbacks and nothing happened */
	return(oid);
}
/*  clones an object */
int s3d_clone(int oid)
{
	uint32_t res;
	res = s3d_new_object();
	s3d_clone_target(res, oid);
	return(res);
}

/*  changes the target of a clone-object */
int s3d_clone_target(int oid, int toid)
{
	uint32_t buf[2];
	buf[0] = htonl(oid);
	buf[1] = htonl(toid);
	net_send(S3D_P_C_CLONE, (char *)&buf, 8);
	/*  s3dprintf(MED,"... changed clone-target of object %d to %d", oid, toid); */
	return oid;
}
/*  deletes an object */
int s3d_del_object(int oid)
{
	uint32_t res = htonl(oid);
	net_send(S3D_P_C_DEL_OBJ, (char *)&res, 4);
	return oid;
}
/*  creates a link from object oid_from to object oid_to in order to copy */
/*  translations/rotations */
int s3d_link(int oid_from, int oid_to)
{
	uint32_t buf[2];
	buf[0] = htonl(oid_from);
	buf[1] = htonl(oid_to);
	net_send(S3D_P_C_LINK, (char *)buf, 8);
	return(0);
}
/*  remove the link to another object */
int s3d_unlink(int oid)
{
	uint32_t buf;
	buf = htonl(oid);
	net_send(S3D_P_C_LINK, (char *)&buf, 4);
	return(0);
}
/*  pushing functions */

/*  has float always the same size? i'm not quite sure ... */
int s3d_push_vertex(int object, float x, float y, float z)
{
	char    buf[4+3*4], *ptr;
	int     len = 4 + 3 * 4;

	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((float *)ptr) = x;
	ptr += sizeof(float);
	*((float *)ptr) = y;
	ptr += sizeof(float);
	*((float *)ptr) = z;

	htonfb((float*)(buf + sizeof(uint32_t)), 3);
	net_send(S3D_P_C_PUSH_VERTEX, buf, len);
	return(0);
}
/*  like vertex add, but you can add a lot of vertices with this. */
/*  it's to be used for file readers or fast coders :) */
int s3d_push_vertices(int object, const float *vbuf, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 4 * 3;
	int     flen, stepl;
	if (n < 1)
		return(-1);
	stepl = ((int)((MF_LEN - 4) / (4 * 3))) * (4 * 3);
	f = len / (MF_LEN - 4) + 1;  /*  how many fragments? */
	/*  buf=malloc(f>1?MF_LEN:len+4); */
	for (i = 0;i < f;i++) {
		ptr = buf;
		*((uint32_t *)ptr) = htonl(object);
		ptr += sizeof(uint32_t);  /*  object id */
		if (len - i*stepl > stepl)
			flen = stepl;
		else
			flen = (len - i * stepl);
		memcpy(ptr, (char *)vbuf + i*stepl, flen);
		htonfb((float*)(ptr), flen / 4);
		net_send(S3D_P_C_PUSH_VERTEX, buf, flen + 4);
	}
	/*  free(buf); */
	return(0);
}

/*  pushes a new material onto the stack */
/*  a vectored version of this would be wise ... */
int s3d_push_material(int object,
                      float amb_r, float amb_g, float amb_b,
                      float spec_r, float spec_g, float spec_b,
                      float diff_r, float diff_g, float diff_b
                     )
{
	char    buf[4+4*12];
	char    *ptr;
	int     len = 4 + 4 * 12;
	/*  s3dprintf(LOW, "adding a new material..."); */
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((float *)ptr) = amb_r;
	ptr += sizeof(float);
	*((float *)ptr) = amb_g;
	ptr += sizeof(float);
	*((float *)ptr) = amb_b;
	ptr += sizeof(float);
	*((float *)ptr) = 1.0;
	ptr += sizeof(float);
	*((float *)ptr) = spec_r;
	ptr += sizeof(float);
	*((float *)ptr) = spec_g;
	ptr += sizeof(float);
	*((float *)ptr) = spec_b;
	ptr += sizeof(float);
	*((float *)ptr) = 1.0;
	ptr += sizeof(float);
	*((float *)ptr) = diff_r;
	ptr += sizeof(float);
	*((float *)ptr) = diff_g;
	ptr += sizeof(float);
	*((float *)ptr) = diff_b;
	ptr += sizeof(float);
	*((float *)ptr) = 1.0;

	htonfb((float*)(buf + sizeof(uint32_t)), 12);
	net_send(S3D_P_C_PUSH_MAT, buf, len);
	return(0);  /*  nothing yet */
}
/*  same as s3d_push_material, but with values for alpha */
int s3d_push_material_a(int object,
                        float amb_r, float amb_g, float amb_b, float amb_a,
                        float spec_r, float spec_g, float spec_b, float spec_a,
                        float diff_r, float diff_g, float diff_b, float diff_a
                       )
{
	char    buf[4+4*12];
	char    *ptr;
	int     len = 4 + 4 * 12;
	/*  s3dprintf(LOW, "adding a new material..."); */
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((float *)ptr) = amb_r;
	ptr += sizeof(float);
	*((float *)ptr) = amb_g;
	ptr += sizeof(float);
	*((float *)ptr) = amb_b;
	ptr += sizeof(float);
	*((float *)ptr) = amb_a;
	ptr += sizeof(float);
	*((float *)ptr) = spec_r;
	ptr += sizeof(float);
	*((float *)ptr) = spec_g;
	ptr += sizeof(float);
	*((float *)ptr) = spec_b;
	ptr += sizeof(float);
	*((float *)ptr) = spec_a;
	ptr += sizeof(float);
	*((float *)ptr) = diff_r;
	ptr += sizeof(float);
	*((float *)ptr) = diff_g;
	ptr += sizeof(float);
	*((float *)ptr) = diff_b;
	ptr += sizeof(float);
	*((float *)ptr) = diff_a;

	htonfb((float*)(buf + sizeof(uint32_t)), 12);
	net_send(S3D_P_C_PUSH_MAT, buf, len);
	return(0);  /*  nothing yet */
}
/*  push a material array (with alpha information!) */
int s3d_push_materials_a(int object, const float *mbuf, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 4 * 12;
	int     flen, stepl;
	if (n < 1)
		return(-1);
	stepl = ((int)((MF_LEN - 4) / (4 * 12))) * (4 * 12);
	f = len / (MF_LEN - 4) + 1;  /*  how many fragments? */
	/*  buf=malloc(f>1?MF_LEN:len+4); */
	for (i = 0;i < f;i++) {
		ptr = buf;
		*((uint32_t *)ptr) = htonl(object);
		ptr += sizeof(uint32_t);  /*  object id */
		if (len - i*stepl > stepl)
			flen = stepl;
		else
			flen = (len - i * stepl);
		memcpy(ptr, (char *)mbuf + i*stepl, flen);
		htonfb((float*)(ptr), flen / 4);
		net_send(S3D_P_C_PUSH_MAT, buf, flen + 4);
	}
	/*  free(buf); */
	return(0);
}
int s3d_push_polygon(int object, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t material)
{
	char    buf[4+4*4], *ptr;
	int     len = 4 + 4 * 4;
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((uint32_t *)ptr) = htonl(v1);
	ptr += sizeof(uint32_t);
	*((uint32_t *)ptr) = htonl(v2);
	ptr += sizeof(uint32_t);
	*((uint32_t *)ptr) = htonl(v3);
	ptr += sizeof(uint32_t);
	*((uint32_t *)ptr) = htonl(material);

	net_send(S3D_P_C_PUSH_POLY, buf, len);
	return(0);
}
int s3d_push_line(int object, uint32_t v1, uint32_t v2, uint32_t material)
{
	char    buf[4+3*4], *ptr;
	int     len = 4 + 3 * 4;
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((uint32_t *)ptr) = htonl(v1);
	ptr += sizeof(uint32_t);
	*((uint32_t *)ptr) = htonl(v2);
	ptr += sizeof(uint32_t);
	*((uint32_t *)ptr) = htonl(material);

	net_send(S3D_P_C_PUSH_LINE, buf, len);
	return(0);
}

/*  this is the polygon array version */
/*  assumes to have a list of polys which consists of v1,v2,v3,material */
int s3d_push_polygons(int object, const uint32_t *pbuf, uint16_t n)
{
	uint32_t  buf[(MF_LEN+4)/4];
	const uint32_t  *s;
	uint32_t  *d;
	int     f, i, j, len = n * 4 * 4;
	int     flen, stepl;
	if (n < 1)
		return(-1);
	stepl = ((int)((MF_LEN - 4) / (4 * 4))) * (4 * 4);
	f = len / (MF_LEN - 4) + 1;  /*  how many fragments? */

	buf[0] = htonl(object);
	d = buf + 1;
	for (i = 0;i < f;i++) {
		if (len - i*stepl > stepl)   flen = stepl;
		else       flen = (len - i * stepl);

		s = pbuf + i * stepl / 4;
		for (j = 0;j < flen / 4;j++)
			d[j] = htonl(s[j]);
		net_send(S3D_P_C_PUSH_POLY, (char *)buf, flen + 4);
	}
	return(0);
}
int s3d_push_lines(int object, const uint32_t *lbuf, uint16_t n)
{
	uint32_t  buf[(MF_LEN+4)/4];
	const uint32_t  *s;
	uint32_t  *d;
	int     f, i, j, len = n * 4 * 3;
	int     flen, stepl;
	if (n < 1)
		return(-1);
	stepl = ((int)((MF_LEN - 4) / (4 * 3))) * (4 * 3);
	f = len / (MF_LEN - 4) + 1;  /*  how many fragments? */

	buf[0] = htonl(object);
	d = buf + 1;

	for (i = 0;i < f;i++) {
		if (len - i*stepl > stepl)   flen = stepl;
		else       flen = (len - i * stepl);

		s = lbuf + i * stepl / 4;
		for (j = 0;j < flen / 4;j++)
			d[j] = htonl(s[j]);

		net_send(S3D_P_C_PUSH_LINE, (char *)buf, flen + 4);
	}
	return(0);
}
int s3d_push_texture(int object, uint16_t w, uint16_t h)
{
	char    buf[4+2*2], *ptr;
	int     len = 4 + 2 * 2;
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((uint16_t *)ptr) = htons(w);
	ptr += sizeof(uint16_t);
	*((uint16_t *)ptr) = htons(h);

	net_send(S3D_P_C_PUSH_TEX, buf, len);
	return(0);
}
int s3d_push_textures(int object, const uint16_t *tbuf, uint16_t n)
{
	uint32_t  buf[(MF_LEN+4)/4];
	const uint16_t  *s;
	uint16_t *d;

	int     f, i, j, len = n * 2 * 2;
	int     flen, stepl;
	if (n < 1)
		return(-1);
	stepl = ((int)((MF_LEN - 4) / (2 * 2))) * (2 * 2);
	f = len / (MF_LEN - 4) + 1;  /*  how many fragments? */

	*((uint32_t *)buf) = htonl(object);
	d = (uint16_t *)(buf + 1);

	for (i = 0;i < f;i++) {
		if (len - i*stepl > stepl)   flen = stepl;
		else       flen = (len - i * stepl);

		s = tbuf + i * stepl / 2;
		for (j = 0;j < flen / 2;j++)
			d[j] = htons(s[j]);
		net_send(S3D_P_C_PUSH_TEX, (char *)buf, flen + 4);
	}
	return(0);
}
/*  popping functions  */

/*  delete n vertices */
int s3d_pop_vertex(int object, uint32_t n)
{
	uint32_t  buf[2];
	buf[0] = htonl(object);
	buf[1] = htonl(n);
	net_send(S3D_P_C_DEL_VERTEX, (char *)buf, 4*2);
	return(0);

}
/*  delete n materials */
int s3d_pop_material(int object, uint32_t n)
{
	uint32_t  buf[2];
	buf[0] = htonl(object);
	buf[1] = htonl(n);
	net_send(S3D_P_C_DEL_MAT, (char *)buf, 4*2);
	return(0);

}
/*  delete n polygons */
int s3d_pop_polygon(int object, uint32_t n)
{
	uint32_t  buf[2];
	buf[0] = htonl(object);
	buf[1] = htonl(n);
	net_send(S3D_P_C_DEL_POLY, (char *)buf, 4*2);
	return(0);

}
/*  delete n lines */
int s3d_pop_line(int object, uint32_t n)
{
	uint32_t  buf[2];
	buf[0] = htonl(object);
	buf[1] = htonl(n);
	net_send(S3D_P_C_DEL_LINE, (char *)buf, 4*2);
	return(0);

}
/*  delete n polygons */
int s3d_pop_texture(int object, uint32_t n)
{
	uint32_t  buf[2];
	buf[0] = htonl(object);
	buf[1] = htonl(n);
	net_send(S3D_P_C_DEL_TEX, (char *)buf, 4*2);
	return(0);

}
/*  pepping/loading functions */

/*  overwrites the last material with this one */
int s3d_pep_material(int object,
                     float amb_r, float amb_g, float amb_b,
                     float spec_r, float spec_g, float spec_b,
                     float diff_r, float diff_g, float diff_b
                    )
{
	char    buf[4+4*12];
	char    *ptr;
	int     len = 4 + 4 * 12;
	/*  s3dprintf(LOW, "adding a new material..."); */
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((float *)ptr) = amb_r;
	ptr += sizeof(float);
	*((float *)ptr) = amb_g;
	ptr += sizeof(float);
	*((float *)ptr) = amb_b;
	ptr += sizeof(float);
	*((float *)ptr) = 1.0;
	ptr += sizeof(float);
	*((float *)ptr) = spec_r;
	ptr += sizeof(float);
	*((float *)ptr) = spec_g;
	ptr += sizeof(float);
	*((float *)ptr) = spec_b;
	ptr += sizeof(float);
	*((float *)ptr) = 1.0;
	ptr += sizeof(float);
	*((float *)ptr) = diff_r;
	ptr += sizeof(float);
	*((float *)ptr) = diff_g;
	ptr += sizeof(float);
	*((float *)ptr) = diff_b;
	ptr += sizeof(float);
	*((float *)ptr) = 1.0;

	htonfb((float*)(buf + sizeof(uint32_t)), 12);
	net_send(S3D_P_C_PEP_MAT, buf, len);
	return(0);  /*  nothing yet */
}
/*  same as above, with alpha */
int s3d_pep_material_a(int object,
                       float amb_r, float amb_g, float amb_b, float amb_a,
                       float spec_r, float spec_g, float spec_b, float spec_a,
                       float diff_r, float diff_g, float diff_b, float diff_a
                      )
{
	char    buf[4+4*12];
	char    *ptr;
	int     len = 4 + 4 * 12;
	/*  s3dprintf(LOW, "adding a new material..."); */
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((float *)ptr) = amb_r;
	ptr += sizeof(float);
	*((float *)ptr) = amb_g;
	ptr += sizeof(float);
	*((float *)ptr) = amb_b;
	ptr += sizeof(float);
	*((float *)ptr) = amb_a;
	ptr += sizeof(float);
	*((float *)ptr) = spec_r;
	ptr += sizeof(float);
	*((float *)ptr) = spec_g;
	ptr += sizeof(float);
	*((float *)ptr) = spec_b;
	ptr += sizeof(float);
	*((float *)ptr) = spec_a;
	ptr += sizeof(float);
	*((float *)ptr) = diff_r;
	ptr += sizeof(float);
	*((float *)ptr) = diff_g;
	ptr += sizeof(float);
	*((float *)ptr) = diff_b;
	ptr += sizeof(float);
	*((float *)ptr) = diff_a;

	htonfb((float*)(buf + sizeof(uint32_t)), 12);
	net_send(S3D_P_C_PEP_MAT, buf, len);
	return(0);  /*  nothing yet */
}
int s3d_pep_materials_a(int object, const float *mbuf, uint16_t n)
{
	char    buf[MF_LEN+4];
	if ((n*12*sizeof(float) + 4) > MF_LEN) {
		errds(MED, "s3d_pep_materials_a()", "too much data");
		return(-1);  /*  impossible */
	}
	*((uint32_t *)buf) = htonl(object);  /*  object id */
	memcpy(buf + 4, mbuf, 12*n*sizeof(float));
	htonfb((float*)(buf + sizeof(uint32_t)), n*12);
	net_send(S3D_P_C_PEP_MAT, buf, n*12*sizeof(float) + 4);
	return(0);
}

/*  adds normal information to the last n polygons. */
int s3d_pep_polygon_normals(int object, const float *nbuf, uint16_t n)
{
	uint8_t buf[MF_LEN+4];
	if ((n*9*sizeof(float) + 4) > MF_LEN) {
		errds(MED, "s3d_pep_polygon_normals()", "too much data");
		return(-1);  /*  impossible */
	}
	*((uint32_t *)buf) = htonl(object);
	memcpy(buf + 4, nbuf, 9*n*sizeof(float));
	htonfb((float*)(buf + sizeof(uint32_t)), 9*n);
	net_send(S3D_P_C_PEP_POLY_NORMAL, (char *)buf, n*9*sizeof(float) + 4);
	return(0);

}
/*  adds normal information to the last n line. */
int s3d_pep_line_normals(int object, const float *nbuf, uint16_t n)
{
	uint8_t buf[MF_LEN+4];
	if ((n*9*sizeof(float) + 4) > MF_LEN) {
		errds(MED, "s3d_pep_line_normals()", "too much data");
		return(-1);  /*  impossible */
	}
	*((uint32_t *)buf) = htonl(object);
	memcpy(buf + 4, nbuf, 6*n*sizeof(float));
	htonfb((float*)(buf + sizeof(uint32_t)), 6*n);
	net_send(S3D_P_C_PEP_LINE_NORMAL, (char *)buf, n*6*sizeof(float) + 4);
	return(0);

}
/*  replaces the last vertex. */
int s3d_pep_vertex(int object, float x, float y, float z)
{
	char    buf[4+3*4], *ptr;
	int     len = 4 + 3 * 4;

	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((float *)ptr) = x;
	ptr += sizeof(float);
	*((float *)ptr) = y;
	ptr += sizeof(float);
	*((float *)ptr) = z;

	htonfb((float*)(buf + sizeof(uint32_t)), 3);
	net_send(S3D_P_C_PEP_VERTEX, buf, len);
	return(0);
}
/* replaces the last line */
int s3d_pep_line(int object, int v1, int v2, int material)
{
	char    buf[4+3*4], *ptr;
	int     len = 4 + 3 * 4;
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((uint32_t *)ptr) = htonl(v1);
	ptr += sizeof(uint32_t);
	*((uint32_t *)ptr) = htonl(v2);
	ptr += sizeof(uint32_t);
	*((uint32_t *)ptr) = htonl(material);

	net_send(S3D_P_C_PEP_LINE, buf, len);
	return(0);
}


/*  replaces the last n lines. */
int s3d_pep_lines(int object, const uint32_t *lbuf, uint16_t n)
{
	uint32_t  buf[MF_LEN+4];
	int    i;
	if ((n*3*4 + 4) > MF_LEN) {
		errds(MED, "s3d_pep_lines()", "too much data");
		return(-1);  /*  impossible */
	}
	buf[0] = htonl(object);
	for (i = 0;i < 3*n;i++)
		buf[i+1] = htonl(lbuf[0]);
	net_send(S3D_P_C_PEP_LINE, (char *)buf, n*3*4 + 4);
	return(0);

}
/*  replaces the last n vertices. */
int s3d_pep_vertices(int object, const float *vbuf, uint16_t n)
{
	uint8_t buf[MF_LEN+4];
	if ((n*3*sizeof(float) + 4) > MF_LEN) {
		errds(MED, "s3d_pep_vertices()", "too much data");
		return(-1);  /*  impossible */
	}
	*((uint32_t *)buf) = htonl(object);
	memcpy(buf + 4, vbuf, 3*n*sizeof(float));
	htonfb((float*)(buf + sizeof(uint32_t)), 3*n);
	net_send(S3D_P_C_PEP_VERTEX, (char *)buf, n*3*sizeof(float) + 4);
	return(0);

}
/*  peps the last polygon with some texture coords */
int s3d_pep_polygon_tex_coord(int object, float x1, float y1, float x2, float y2, float x3, float y3)
{
	char *ptr, buf[4*6+4];
	ptr = buf;
	*((uint32_t *)buf) = htonl(object);
	ptr += sizeof(uint32_t);
	*((float *)ptr) = x1;
	ptr += sizeof(float);
	*((float *)ptr) = y1;
	ptr += sizeof(float);
	*((float *)ptr) = x2;
	ptr += sizeof(float);
	*((float *)ptr) = y2;
	ptr += sizeof(float);
	*((float *)ptr) = x3;
	ptr += sizeof(float);
	*((float *)ptr) = y3;

	htonfb((float*)(buf + sizeof(uint32_t)), 6);
	net_send(S3D_P_C_PEP_POLY_TEXC, (char *)buf, 6*4 + 4);
	return(0);
}
/*  adds texture coordinates to the last n polygons. */
int s3d_pep_polygon_tex_coords(int object, const float *tbuf, uint16_t n)
{
	char buf[MF_LEN+4];
	if ((n*6*sizeof(float)) > MF_LEN) {
		errds(MED, "s3d_pep_polygon_tex_coords()", "too much data");
		return(-1);  /*  impossible */
	}
	*((uint32_t *)buf) = htonl(object);
	memcpy(buf + 4, tbuf, 6*n*sizeof(float));
	htonfb((float*)(buf + sizeof(uint32_t)), 6*n);
	net_send(S3D_P_C_PEP_POLY_TEXC, (char *)buf, n*6*sizeof(float) + 4);
	return(0);
}
/*  adds normal information to the last n polygons. */
int s3d_load_polygon_normals(int object, const float *nbuf, uint32_t start, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 9 * 4;
	int     flen, stepl;
	uint32_t   mstart;
	if (n < 1)
		return(-1);
	mstart = start;
	stepl = ((int)((MF_LEN - 8) / (9 * 4))) * (9 * 4);
	f = len / (MF_LEN - 8) + 1;  /*  how many fragments? */
	for (i = 0;i < f;i++) {
		ptr = buf;
		*((uint32_t *)ptr) = htonl(object);
		ptr += sizeof(uint32_t);  /*  object id */
		*((uint32_t *)ptr) = htonl(mstart);
		ptr += sizeof(uint32_t);  /*  start */
		if (len - i*stepl > stepl)
			flen = stepl;
		else
			flen = (len - i * stepl);
		memcpy(ptr, (char *)nbuf + i*stepl, flen);
		htonfb((float*)(ptr), flen / 4);
		net_send(S3D_P_C_LOAD_POLY_NORMAL, buf, flen + 8);
		mstart += stepl;
	}
	return(0);
}
/*  adds normal information to the last n polygons. */
int s3d_load_line_normals(int object, const float *nbuf, uint32_t start, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 6 * 4;
	int     flen, stepl;
	uint32_t   mstart;
	if (n < 1)
		return(-1);
	mstart = start;
	stepl = ((int)((MF_LEN - 8) / (6 * 4))) * (6 * 4);
	f = len / (MF_LEN - 8) + 1;  /*  how many fragments? */
	for (i = 0;i < f;i++) {
		ptr = buf;
		*((uint32_t *)ptr) = htonl(object);
		ptr += sizeof(uint32_t);  /*  object id */
		*((uint32_t *)ptr) = htonl(mstart);
		ptr += sizeof(uint32_t);  /*  start */
		if (len - i*stepl > stepl)
			flen = stepl;
		else
			flen = (len - i * stepl);
		memcpy(ptr, (char *)nbuf + i*stepl, flen);
		htonfb((float*)(ptr), flen / 4);
		net_send(S3D_P_C_LOAD_LINE_NORMAL, buf, flen + 8);
		mstart += stepl;
	}
	return(0);
}
/*  adds texture coordinates to the last n polygons. */
int s3d_load_polygon_tex_coords(int object, const float *tbuf, uint32_t start, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 6 * 4;
	int     flen, stepl;
	uint32_t   mstart;
	if (n < 1)
		return(-1);
	mstart = start;
	stepl = ((int)((MF_LEN - 8) / (6 * 4))) * (6 * 4);
	f = len / (MF_LEN - 8) + 1;  /*  how many fragments? */
	for (i = 0;i < f;i++) {
		ptr = buf;
		*((uint32_t *)ptr) = htonl(object);
		ptr += sizeof(uint32_t);  /*  object id */
		*((uint32_t *)ptr) = htonl(mstart);
		ptr += sizeof(uint32_t);  /*  start */
		if (len - i*stepl > stepl)
			flen = stepl;
		else
			flen = (len - i * stepl);
		memcpy(ptr, (char *)tbuf + i*stepl, flen);
		htonfb((float*)(ptr), flen / 4);
		net_send(S3D_P_C_LOAD_POLY_TEXC, buf, flen + 8);
		mstart += stepl;
	}
	return(0);
}
/*  load n materials at position start, overwriting old ones */
int s3d_load_materials_a(int object, const float *mbuf, uint32_t start, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 12 * 4;
	int     flen, stepl;
	uint32_t   mstart;
	if (n < 1)
		return(-1);
	mstart = start;
	stepl = ((int)((MF_LEN - 8) / (12 * 4))) * (12 * 4);
	f = len / (MF_LEN - 8) + 1;  /*  how many fragments? */
	for (i = 0;i < f;i++) {
		ptr = buf;
		*((uint32_t *)ptr) = htonl(object);
		ptr += sizeof(uint32_t);  /*  object id */
		*((uint32_t *)ptr) = htonl(mstart);
		ptr += sizeof(uint32_t);  /*  start */
		if (len - i*stepl > stepl)
			flen = stepl;
		else
			flen = (len - i * stepl);
		memcpy(ptr, (char *)mbuf + i*stepl, flen);
		htonfb((float*)(ptr), flen / 4);
		net_send(S3D_P_C_LOAD_MAT, buf, flen + 8);
		mstart += stepl;
	}
	return(0);
}
int s3d_pep_material_texture(int object, uint32_t tex)
{
	char    buf[4*2], *ptr;
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((uint32_t *)ptr) = htonl(tex);

	net_send(S3D_P_C_PEP_MAT_TEX, buf, 8);
	return(0);
}
int _s3d_update_texture(int object, uint32_t tex, uint16_t xpos, uint16_t ypos, uint16_t w, uint16_t h)
{
	char    buf[16], *ptr;
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((uint32_t *)ptr) = htonl(tex);
	ptr += sizeof(uint32_t);  /*  texture number */
	*((uint16_t *)ptr) = htons(xpos);
	ptr += sizeof(uint16_t);  /*  xpos */
	*((uint16_t *)ptr) = htons(ypos);
	ptr += sizeof(uint16_t);  /*  ypos */
	*((uint16_t *)ptr) = htons(w);
	ptr += sizeof(uint16_t);  /*  width */
	*((uint16_t *)ptr) = htons(h);

	net_send(S3D_P_C_UPDATE_TEX, buf, 16);
	return(0);

}
/*  load data (which has width w and height h) into object, texture tex at position (xpos,ypos) */
int s3d_load_texture(int object, uint32_t tex, uint16_t xpos, uint16_t ypos, uint16_t w, uint16_t h, const uint8_t *data)
{
	char    buf[MF_LEN+4], *ptr;
	int     linestep, lines, i;
	if (_s3d_load_texture_shm(object, tex, xpos, ypos, w, h, data) == 0) {
		/* TODO: send update event to server */
		_s3d_update_texture(object, tex, xpos, ypos, w, h);
		return(0); /* did it over shm */
	}
	linestep = (MF_LEN - 16) / (w * 4);
	if (linestep == 0)
		return(-1);  /*  won't do that. .. yet */
	for (i = 0;i < h;i += linestep) {
		ptr = buf;
		*((uint32_t *)ptr) = htonl(object);
		ptr += sizeof(uint32_t);  /*  object id */
		*((uint32_t *)ptr) = htonl(tex);
		ptr += sizeof(uint32_t);  /*  texture number */
		*((uint16_t *)ptr) = htons(xpos);
		ptr += sizeof(uint16_t);  /*  xpos */
		*((uint16_t *)ptr) = htons(ypos + i);
		ptr += sizeof(uint16_t);  /*  ypos */
		*((uint16_t *)ptr) = htons(w);
		ptr += sizeof(uint16_t);  /*  width */
		if ((h - i) > linestep)  lines = linestep;
		else     lines = h - i;
		*((uint16_t *)ptr) = htons(lines);
		ptr += sizeof(uint16_t);  /*  height */
		memcpy(ptr, data + (i*w*4), lines*w*4);
		net_send(S3D_P_C_LOAD_TEX, buf, 16 + lines*w*4);
	}
	return(0);
}
int s3d_flags_on(int object, uint32_t flags)
{
	char    buf[4+1+4], *ptr;
	int     len = 4 + 1 + 4;
	ptr = buf;
	/*  s3dprintf(VLOW, "toggling flags on .. %010x", flags); */
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);
	*ptr = OF_TURN_ON;
	ptr += sizeof(unsigned char);
	*((uint32_t *)ptr) = htonl(flags);

	net_send(S3D_P_C_TOGGLE_FLAGS, buf, len);
	return(0);
}
int s3d_flags_off(int object, uint32_t flags)
{
	char    buf[4+1+4], *ptr;
	int     len = 4 + 1 + 4;
	ptr = buf;
	/*  s3dprintf(VLOW, "toggling flags off .. %010x", flags); */
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);
	*ptr = OF_TURN_OFF;
	ptr += sizeof(unsigned char);
	*((uint32_t *)ptr) = htonl(flags);

	net_send(S3D_P_C_TOGGLE_FLAGS, buf, len);
	return(0);
}
int s3d_translate(int object, float x, float y, float z)
{
	char    buf[4+4*3], *ptr;
	int     len = 4 + 4 * 3;
	ptr = buf;
	/*  s3dprintf(VLOW, "translating object to  .. %f, %f, %f", x,y,z); */
	*((uint32_t *)ptr) = htonl(object);
	ptr += 4;
	*((float *)ptr) = x;
	ptr += 4;
	*((float *)ptr) = y;
	ptr += 4;
	*((float *)ptr) = z;

	htonfb((float*)(buf + sizeof(uint32_t)), 3);
	net_send(S3D_P_C_TRANSLATE, buf, len);
	return(0);

}
/*  rotation about the x-axis, y-axis and z-axis */
int s3d_rotate(int object, float x, float y, float z)
{
	char    buf[4+4*3], *ptr;
	int     len = 4 + 4 * 3;
	ptr = buf;
	/*  s3dprintf(VLOW, "rotating object to  .. %f, %f, %f", x,y,z); */
	*((uint32_t *)ptr) = htonl(object);
	ptr += 4;
	*((float *)ptr) = x;
	ptr += 4;
	*((float *)ptr) = y;
	ptr += 4;
	*((float *)ptr) = z;

	htonfb((float*)(buf + sizeof(uint32_t)), 3);
	net_send(S3D_P_C_ROTATE, buf, len);
	return(0);
}
/*  scale the object .. */

int s3d_scale(int object, float s)
{
	char    buf[4+4], *ptr;
	int     len = 4 + 4;
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += 4;
	*((float *)ptr) = s;

	htonfb((float*)(buf + sizeof(uint32_t)), 1);
	net_send(S3D_P_C_SCALE, buf, len);
	return(0);
}
/*  sets the focused app through it's mcp object number */
int s3d_mcp_focus(int object)
{
	uint32_t buf = htonl(object);
	net_send(S3D_P_MCP_FOCUS, (char *)&buf, 4);
	return(0);
}
