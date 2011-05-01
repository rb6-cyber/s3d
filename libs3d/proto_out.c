/*
 * proto_out.c
 *
 * Copyright (C) 2004-2011  Simon Wunderlich <dotslash@packetmixer.de>
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

/** \brief create new object
 *
 * Creates a new object, returning the object id.
 *
 * \remarks Of course, you won't forget to toggle it visible, won't you?
 */
int s3d_new_object(void)
{
	int oid;

	cb_lock++; /* please, no callbacks now. */
	oid = _queue_want_object();
	cb_lock--; /* no new callbacks and nothing happened */
	return oid;
}

/** \brief clone object
 *
 * Clones an already existing object. They get just look the same as the
 * parent-object and will change when the parent-object changes. Cloning
 * especially makes sense if you want to use the same object a lot of times.
 * Move and transform is independent from the parent. The function returns the
 * childs object id.
 */
int s3d_clone(int oid)
{
	uint32_t res;
	res = s3d_new_object();
	s3d_clone_target(res, oid);
	return res;
}

/** \brief changes the target of a clone-object
 *
 * Changes the clone target of oid to another object (toid). This assumes you've
 * got oid from s3d_clone before.
 */
int s3d_clone_target(int oid, int toid)
{
	uint32_t buf[2];
	buf[0] = htonl(oid);
	buf[1] = htonl(toid);
	net_send(S3D_P_C_CLONE, (char *)&buf, 8);
	/*  s3dprintf(MED,"... changed clone-target of object %d to %d", oid, toid); */
	return oid;
}

/** \brief delete an object
 *
 * Deletes the object referenced by oid.
 */
int s3d_del_object(int oid)
{
	uint32_t res = htonl(oid);
	net_send(S3D_P_C_DEL_OBJ, (char *)&res, 4);
	return oid;
}

/** \brief link object to another one
 *
 * A linked object will move along with it's link parent. For example if you
 * have a book on a table, you can link the book to the table so the book will
 * "keep on the table" if you move the table around in space. It will also
 * rotate with the table etc.
 */
int s3d_link(int oid_from, int oid_to)
{
	uint32_t buf[2];
	buf[0] = htonl(oid_from);
	buf[1] = htonl(oid_to);
	net_send(S3D_P_C_LINK, (char *)buf, 8);
	return 0;
}

/** \brief removes link from another object
 *
 * Remove the link of object oid to its target.
 */
int s3d_unlink(int oid)
{
	uint32_t buf;
	buf = htonl(oid);
	net_send(S3D_P_C_LINK, (char *)&buf, 4);
	return 0;
}
/*  pushing functions */

/** \brief push vertex
 *
 * Pushes a vertex onto the vertex stack. Make sure that you count how many
 * vertices you've pushed because you'll need that for referencing when you push
 * your polygons.
 */
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
	return 0;
}

/** \brief push many vertices
 *
 * Push some vertices from an array. that's much better for performing than
 * using s3d_push_vertex() if you have a lot of vertices (and that's probably
 * the usual case).
 * \code
 * float vertices[] = { 0.0, 0.0, 0.0,
 *                      1.0, 2.0, 3.0,
 *                      3.0, 2.0, 1.0};
 * s3d_push_vertices(object, vertices, 3); // pushing 3 vertices
 * \endcode
 */
int s3d_push_vertices(int object, const float *vbuf, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 4 * 3;
	int     flen, stepl;
	if (n < 1)
		return -1;
	stepl = ((int)((MF_LEN - 4) / (4 * 3))) * (4 * 3);
	f = len / (MF_LEN - 4) + 1;  /*  how many fragments? */
	/*  buf=malloc(f>1?MF_LEN:len+4); */
	for (i = 0; i < f; i++) {
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
	return 0;
}

/** \brief push material
 *
 * Pushes a material for an object. you will have to count them yourself too,
 * as polygons will ask for the material index number. The material properties
 * are given in rgb (red/green/blue) color codes, in float. 0.0 is the minimum,
 * 1.0 is the maximum a color value can be. The specular color is the color
 * which is directly reflected from the light source. The diffuse color is the
 * color which can be seen in the bright side of the object, and the ambience
 * color is the color of the shadow side of the object.
 */
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
	return 0;  /*  nothing yet */
}

/** \brief push material with alpha
 *
 * Same as s3d_push_material, but color has alpha value added. Use
 * s3d_push_materials_a() if you have a lot of materials to push.
 */
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
	return 0;  /*  nothing yet */
}

/** \brief push many materials
 *
 * Pushes a buffer of materials. Those materials are in the format float[n*12],
 * with
 * - mbuf[n*12 + 0-3] - ambience
 * - mbuf[n*12 + 4-7] - specular
 * - mbuf[n *12 + 8-11] - diffusion values
 *
 * of each entry. n is the number of materials pushed. The values are in the
 * order r,g,b,a. If you only want to push one material, use the more easy
 * s3d_push_material_a() function.
 *
 * \code
 * // each line has r,g,b,a value
 * float bla[24]=
 *         {1, 0, 0, 1,
 *          1, 0, 0, 1,
 *          1, 0, 0, 1,
 *          0, 1, 1, 1,
 *          0, 1, 1, 1,
 *          0, 1, 1, 1};
 *
 * s3d_push_materials_a(object, bla, 2); // push a red and a cyan material
 * \endcode
 */
int s3d_push_materials_a(int object, const float *mbuf, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 4 * 12;
	int     flen, stepl;
	if (n < 1)
		return -1;
	stepl = ((int)((MF_LEN - 4) / (4 * 12))) * (4 * 12);
	f = len / (MF_LEN - 4) + 1;  /*  how many fragments? */
	/*  buf=malloc(f>1?MF_LEN:len+4); */
	for (i = 0; i < f; i++) {
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
	return 0;
}

/** \brief push polygon
 *
 * Push one polygon on the polygon stack of the object. It takes 3 vertex-index
 * numbers and one material material-index-no. as argument.
 *
 * \code
 * int oid = s3d_new_object();   // create a new object
 * s3d_push_vertex(oid, 0.0, 0.0, 0.0);
 * s3d_push_vertex(oid, 0.0, 1.0, 0.0);
 * s3d_push_vertex(oid, 1.0, 0.0, 0.0);
 * s3d_push_material(oid, 0.3, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0);
 * s3d_push_polygon(oid, 0, 1, 2, 0);
 * // this will create a red polygon
 * \endcode
 */
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
	return 0;
}

/** \brief push line
 *
 * Push one line on the line stack of the object. It takes 2 vertex-index-no,
 * and one material material-index-no. as argument. If you have a lot of lines
 * to push, use s3d_push_lines()
 */
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
	return 0;
}

/** \brief push many polygons
 *
 * As for vertices, you can push arrays of polygons to have greater performance.
 * The pbuf should contain n polygons which consist of 4 uint32_t values of 3
 * vertices indices and 1 material index.
 *
 * \code
 * uint32_t pbuf[] = { 0, 1, 2, 0};
 * int oid = s3d_new_object();   // create a new object
 * s3d_push_vertex(oid, 0.0, 0.0, 0.0);
 * s3d_push_vertex(oid, 0.0, 1.0, 0.0);
 * s3d_push_vertex(oid, 1.0, 0.0, 0.0);
 * s3d_push_material(oid, 0.3, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0);
 * s3d_push_polygons(oid, pbuf, 1);
 * // push one polygon with the pbuf data
 * \endcode
 */
int s3d_push_polygons(int object, const uint32_t *pbuf, uint16_t n)
{
	uint32_t  buf[(MF_LEN+4)/4];
	const uint32_t  *s;
	uint32_t  *d;
	int     f, i, j, len = n * 4 * 4;
	int     flen, stepl;
	if (n < 1)
		return -1;
	stepl = ((int)((MF_LEN - 4) / (4 * 4))) * (4 * 4);
	f = len / (MF_LEN - 4) + 1;  /*  how many fragments? */

	buf[0] = htonl(object);
	d = buf + 1;
	for (i = 0; i < f; i++) {
		if (len - i*stepl > stepl)   flen = stepl;
		else       flen = (len - i * stepl);

		s = pbuf + i * stepl / 4;
		for (j = 0; j < flen / 4; j++)
			d[j] = htonl(s[j]);
		net_send(S3D_P_C_PUSH_POLY, (char *)buf, flen + 4);
	}
	return 0;
}

/** \brief push many lines
 *
 * Pushing n lines on the line stack of the object, each lbuf has a size of n*3,
 * each entry has the index number of the first vertex, second vertex and
 * material number just as in s3d_push_line().
 */
int s3d_push_lines(int object, const uint32_t *lbuf, uint16_t n)
{
	uint32_t  buf[(MF_LEN+4)/4];
	const uint32_t  *s;
	uint32_t  *d;
	int     f, i, j, len = n * 4 * 3;
	int     flen, stepl;
	if (n < 1)
		return -1;
	stepl = ((int)((MF_LEN - 4) / (4 * 3))) * (4 * 3);
	f = len / (MF_LEN - 4) + 1;  /*  how many fragments? */

	buf[0] = htonl(object);
	d = buf + 1;

	for (i = 0; i < f; i++) {
		if (len - i*stepl > stepl)   flen = stepl;
		else       flen = (len - i * stepl);

		s = lbuf + i * stepl / 4;
		for (j = 0; j < flen / 4; j++)
			d[j] = htonl(s[j]);

		net_send(S3D_P_C_PUSH_LINE, (char *)buf, flen + 4);
	}
	return 0;
}

/** \brief push texture
 *
 * Adds a new texture with height w and height h on the texture stack.
 */
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
	return 0;
}

/** \brief push many textures
 *
 * As for vertices, you can push arrays of textures on the texture stack to have
 * greater performance. The tbuf should contain n texture sizes which consist of
 * 2 uint16_t values for width and height for each texture.
 */
int s3d_push_textures(int object, const uint16_t *tbuf, uint16_t n)
{
	uint32_t  buf[(MF_LEN+4)/4];
	const uint16_t  *s;
	uint16_t *d;

	int     f, i, j, len = n * 2 * 2;
	int     flen, stepl;
	if (n < 1)
		return -1;
	stepl = ((int)((MF_LEN - 4) / (2 * 2))) * (2 * 2);
	f = len / (MF_LEN - 4) + 1;  /*  how many fragments? */

	*((uint32_t *)buf) = htonl(object);
	d = (uint16_t *)(buf + 1);

	for (i = 0; i < f; i++) {
		if (len - i*stepl > stepl)   flen = stepl;
		else       flen = (len - i * stepl);

		s = tbuf + i * stepl / 2;
		for (j = 0; j < flen / 2; j++)
			d[j] = htons(s[j]);
		net_send(S3D_P_C_PUSH_TEX, (char *)buf, flen + 4);
	}
	return 0;
}
/*  popping functions  */

/** \brief remove vertices
 *
 * Deletes the latest n vertices from the vertex stack of the object.
 */
int s3d_pop_vertex(int object, uint32_t n)
{
	uint32_t  buf[2];
	buf[0] = htonl(object);
	buf[1] = htonl(n);
	net_send(S3D_P_C_DEL_VERTEX, (char *)buf, 4*2);
	return 0;

}

/** \brief remove materials
 *
 * Deletes the latest n material from the material stack of the object.
 */
int s3d_pop_material(int object, uint32_t n)
{
	uint32_t  buf[2];
	buf[0] = htonl(object);
	buf[1] = htonl(n);
	net_send(S3D_P_C_DEL_MAT, (char *)buf, 4*2);
	return 0;

}

/** \brief remove polygons
 *
 * Deletes the latest n polygon from the polygon stack of the object.
 */
int s3d_pop_polygon(int object, uint32_t n)
{
	uint32_t  buf[2];
	buf[0] = htonl(object);
	buf[1] = htonl(n);
	net_send(S3D_P_C_DEL_POLY, (char *)buf, 4*2);
	return 0;

}

/** \brief remove lines
 *
 * Deletes the latest n lines from the line stack of the object.
 */
int s3d_pop_line(int object, uint32_t n)
{
	uint32_t  buf[2];
	buf[0] = htonl(object);
	buf[1] = htonl(n);
	net_send(S3D_P_C_DEL_LINE, (char *)buf, 4*2);
	return 0;

}

/** \brief remove textures
 *
 * Deletes the latest n textures from the texture stack of the object.
 */
int s3d_pop_texture(int object, uint32_t n)
{
	uint32_t  buf[2];
	buf[0] = htonl(object);
	buf[1] = htonl(n);
	net_send(S3D_P_C_DEL_TEX, (char *)buf, 4*2);
	return 0;

}
/*  pepping/loading functions */

/** \brief rewrite material
 *
 * Overwriting the latest pushed material, overwriting the current value with
 * the specified one. See s3d_pep_materials_a if you want to pep more materials.
 */
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
	return 0;  /*  nothing yet */
}

/** \brief rewrite material with alpha
 *
 * Overwriting the latest pushed material, overwriting the current value with
 * the specified one, with alpha value in contrast to s3d_pep_material See
 * s3d_push_materials_a if you want to pep more materials.
 */
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
	return 0;  /*  nothing yet */
}

/** \brief rewrite materials with alpha
 *
 * Alters the last n pushed materials. See s3d_push_materials_a() for more
 * information how mbuf should look like. Use s3d_pep_material_a() if you only
 * want to alter the latest material.
 */
int s3d_pep_materials_a(int object, const float *mbuf, uint16_t n)
{
	char    buf[MF_LEN+4];
	if ((n*12*sizeof(float) + 4) > MF_LEN) {
		errds(MED, "s3d_pep_materials_a()", "too much data");
		return -1;  /*  impossible */
	}
	*((uint32_t *)buf) = htonl(object);  /*  object id */
	memcpy(buf + 4, mbuf, 12*n*sizeof(float));
	htonfb((float*)(buf + sizeof(uint32_t)), n*12);
	net_send(S3D_P_C_PEP_MAT, buf, n*12*sizeof(float) + 4);
	return 0;
}

/** \brief add normals to polygon
 *
 * Adds normal information to polygons, giving each vertex of a polygon a normal
 * information. With this, you can achieve smoothed edge effects.
 *
 * nbuf should contain n * 9 float values, for each vertex a normal vector
 * (x,y,z), and you have 3 vertices for each Polygon so that makes 9 float
 * values per Polygon in total. Don't worry if you don't use this, it's kind of
 * hard to calculate and the server will always use some proper normal values
 * (same for every vertex, calculated by the plane which is defined by the 3
 * points of the polygon.
 */
int s3d_pep_polygon_normals(int object, const float *nbuf, uint16_t n)
{
	uint8_t buf[MF_LEN+4];
	if ((n*9*sizeof(float) + 4) > MF_LEN) {
		errds(MED, "s3d_pep_polygon_normals()", "too much data");
		return -1;  /*  impossible */
	}
	*((uint32_t *)buf) = htonl(object);
	memcpy(buf + 4, nbuf, 9*n*sizeof(float));
	htonfb((float*)(buf + sizeof(uint32_t)), 9*n);
	net_send(S3D_P_C_PEP_POLY_NORMAL, (char *)buf, n*9*sizeof(float) + 4);
	return 0;

}

/** \brief add normals to lines
 *
 * Adds normal information to lines, giving each vertex of a line a normal
 * information. This makes lines somewhat nicer, you'll need that especially
 * when you're going to build wireframe models.
 *
 * nbuf should contain n * 6 float values, for each vertex a normal vector
 * (x,y,z), and you have 2 vertices for each line so that makes 6 float values
 * per line in total.
 */
int s3d_pep_line_normals(int object, const float *nbuf, uint16_t n)
{
	uint8_t buf[MF_LEN+4];
	if ((n*9*sizeof(float) + 4) > MF_LEN) {
		errds(MED, "s3d_pep_line_normals()", "too much data");
		return -1;  /*  impossible */
	}
	*((uint32_t *)buf) = htonl(object);
	memcpy(buf + 4, nbuf, 6*n*sizeof(float));
	htonfb((float*)(buf + sizeof(uint32_t)), 6*n);
	net_send(S3D_P_C_PEP_LINE_NORMAL, (char *)buf, n*6*sizeof(float) + 4);
	return 0;

}

/** \brief rewrite vertex
 *
 * Alter the latest pushed vertex, overwriting with the supplied values.
 */
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
	return 0;
}

/** \brief rewrite line
 *
 * Alter the latest pushed line, overwriting with the supplied values.
 */
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
	return 0;
}


/** \brief rewrite lines
 *
 * Alter the latest n pushed lines. lbuf holds the values which are used to
 * overwrite the old data, n entries with each 3 uint32_t specifying first,
 * second vertex and material of each line.
 */
int s3d_pep_lines(int object, const uint32_t *lbuf, uint16_t n)
{
	uint32_t  buf[MF_LEN+4];
	int    i;
	if ((n*3*4 + 4) > MF_LEN) {
		errds(MED, "s3d_pep_lines()", "too much data");
		return -1;  /*  impossible */
	}
	buf[0] = htonl(object);
	for (i = 0; i < 3*n; i++)
		buf[i+1] = htonl(lbuf[0]);
	net_send(S3D_P_C_PEP_LINE, (char *)buf, n*3*4 + 4);
	return 0;

}

/** \brief rewrite lines
 *
 * Alter the latest n pushed vertex. vbuf holds the values which are used to
 * overwrite the old data, n entries with each 3 floats specifying x,y,z of the
 * vertices.
 */
int s3d_pep_vertices(int object, const float *vbuf, uint16_t n)
{
	uint8_t buf[MF_LEN+4];
	if ((n*3*sizeof(float) + 4) > MF_LEN) {
		errds(MED, "s3d_pep_vertices()", "too much data");
		return -1;  /*  impossible */
	}
	*((uint32_t *)buf) = htonl(object);
	memcpy(buf + 4, vbuf, 3*n*sizeof(float));
	htonfb((float*)(buf + sizeof(uint32_t)), 3*n);
	net_send(S3D_P_C_PEP_VERTEX, (char *)buf, n*3*sizeof(float) + 4);
	return 0;

}

/** \brief add texture coordinates to polygon
 *
 * Pimp the last polygon pushed with some textures coordinates, x and y values
 * for each vertex point respectively. Those values may be between 0 and 1 and
 * are vertex points on the  texture defined in the material of the polygon. If
 * you have more polygons which should get a texture, use
 * s3d_pep_polygon_tex_coords()
 */
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
	return 0;
}

/** \brief add texture coordinates to polygons
 *
 * Pimp the latest n polygons with texture coordinates. tbuf has 6*n float
 * values for its entries, which are supplied in the order as in
 * s3d_pep_polygon_tex_coord()
 */
int s3d_pep_polygon_tex_coords(int object, const float *tbuf, uint16_t n)
{
	char buf[MF_LEN+4];
	if ((n*6*sizeof(float)) > MF_LEN) {
		errds(MED, "s3d_pep_polygon_tex_coords()", "too much data");
		return -1;  /*  impossible */
	}
	*((uint32_t *)buf) = htonl(object);
	memcpy(buf + 4, tbuf, 6*n*sizeof(float));
	htonfb((float*)(buf + sizeof(uint32_t)), 6*n);
	net_send(S3D_P_C_PEP_POLY_TEXC, (char *)buf, n*6*sizeof(float) + 4);
	return 0;
}

/** \brief add normals to polygon
 *
 * Just as s3d_pep_polygon_normals(), with the difference you won't alter the
 * latest n polygons but n polygons starting with index start.
 */
int s3d_load_polygon_normals(int object, const float *nbuf, uint32_t start, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 9 * 4;
	int     flen, stepl;
	uint32_t   mstart;
	if (n < 1)
		return -1;
	mstart = start;
	stepl = ((int)((MF_LEN - 8) / (9 * 4))) * (9 * 4);
	f = len / (MF_LEN - 8) + 1;  /*  how many fragments? */
	for (i = 0; i < f; i++) {
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
	return 0;
}

/** \brief add normals to line
 *
 * Just as s3d_pep_line_normals(), with the difference you won't alter the
 * latest n lines but n lines starting with index start.
 */
int s3d_load_line_normals(int object, const float *nbuf, uint32_t start, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 6 * 4;
	int     flen, stepl;
	uint32_t   mstart;
	if (n < 1)
		return -1;
	mstart = start;
	stepl = ((int)((MF_LEN - 8) / (6 * 4))) * (6 * 4);
	f = len / (MF_LEN - 8) + 1;  /*  how many fragments? */
	for (i = 0; i < f; i++) {
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
	return 0;
}

/** \brief add texture coordinates to polygons
 *
 * Just as s3d_pep_polygon_tex_coords(), with the difference you won't alter the
 * latest n polygons but n polygons starting with index start.
 */
int s3d_load_polygon_tex_coords(int object, const float *tbuf, uint32_t start, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 6 * 4;
	int     flen, stepl;
	uint32_t   mstart;
	if (n < 1)
		return -1;
	mstart = start;
	stepl = ((int)((MF_LEN - 8) / (6 * 4))) * (6 * 4);
	f = len / (MF_LEN - 8) + 1;  /*  how many fragments? */
	for (i = 0; i < f; i++) {
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
	return 0;
}

/** \brief add materials with alpha to polygons
 *
 * Loads n materials starting from index position start into the material stack.
 * See s3d_push_materials_a for more information about the values in mbuf.
 */
int s3d_load_materials_a(int object, const float *mbuf, uint32_t start, uint16_t n)
{
	char    buf[MF_LEN+4], *ptr;
	int     f, i, len = n * 12 * 4;
	int     flen, stepl;
	uint32_t   mstart;
	if (n < 1)
		return -1;
	mstart = start;
	stepl = ((int)((MF_LEN - 8) / (12 * 4))) * (12 * 4);
	f = len / (MF_LEN - 8) + 1;  /*  how many fragments? */
	for (i = 0; i < f; i++) {
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
	return 0;
}

/** \brief add texture to material
 *
 * Assign the latest material a texture referenced by the index tex. Of course,
 * you will have pushed this texture with s3d_push_texture()
 */
int s3d_pep_material_texture(int object, uint32_t tex)
{
	char    buf[4*2], *ptr;
	ptr = buf;
	*((uint32_t *)ptr) = htonl(object);
	ptr += sizeof(uint32_t);  /*  object id */
	*((uint32_t *)ptr) = htonl(tex);

	net_send(S3D_P_C_PEP_MAT_TEX, buf, 8);
	return 0;
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
	return 0;

}

/** \brief load texture from memory
 *
 * This will load an 32bit rgba image supplied in data at position xpos,ypos of
 * the texture tex. The image has the width w and height h. This can be used to
 * update only parts of the texture. It's no problem to supply big textures,
 * as the image will be sent to server in fragments. Of course, you will have
 * created the texture with s3d_push_texture, have an material assigned to the
 * texture with s3d_pep_material_texture() and have your polygons set sane
 * polygon texture coords using s3d_pep_polygon_tex_coord().
 */
int s3d_load_texture(int object, uint32_t tex, uint16_t xpos, uint16_t ypos, uint16_t w, uint16_t h, const uint8_t *data)
{
	char    buf[MF_LEN+4], *ptr;
	int     linestep, lines, i;
	if (_s3d_load_texture_shm(object, tex, xpos, ypos, w, h, data) == 0) {
		/* TODO: send update event to server */
		_s3d_update_texture(object, tex, xpos, ypos, w, h);
		return 0; /* did it over shm */
	}
	linestep = (MF_LEN - 16) / (w * 4);
	if (linestep == 0)
		return -1;  /*  won't do that. .. yet */
	for (i = 0; i < h; i += linestep) {
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
	return 0;
}

/** \brief enable flags of object
 *
 * Turn some flags on for object.
 *
 * \remarks If you don't toggle OF_VISIBLE on, you won't see your object. usually
 * you want this. (at least after you *push()d all your content)
 */
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
	return 0;
}

/** \brief disable flags of object
 *
 * Turn some flags off for object.
 */
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
	return 0;
}

/** \brief move object to absolute position
 *
 * Move the object to some position in space. when you create an object, it's
 * always located at 0.0 , 0.0, 0.0.
 *
 * \remarks Translation is absolute, not relative!
 *
 * \code
 * s3d_translate(object, 2, 0, 0);
 * s3d_translate(object, 4, 0, 0);
 * // object will end up at 4,0,0 and not 6,0,0!!
 * \endcode
 */
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
	return 0;

}

/** \brief rotate object
 *
 * Rotate an object around the x, y and z-axis respectively. x,y,z may have
 * values between [0,360] degrees.
 *
 * You will usually only rotate around one axis, leaving the unused fields on 0,
 * I guess. If you want to rotate around more than one axis, please note: The
 * order of the rotation applies is y-axis, x-axis, and then z-axis. You can
 * think of it as the earth position coordinates: x is the longitude, y is the
 * latitude, and z is the rotation at this point of the earth around your bodies
 * axis. (I wonder if that makes it any clearer ;)
 *
 * \remarks Rotate is absolute, not relative!
 *
 * \code
 * s3d_rotate(object, 90,  0, 0);
 * s3d_rotate(object, 180, 0, 0);
 * // object will be rotated 180 degrees around the x-axis, not 270 degress!
 * \endcode
 */
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
	return 0;
}

/** \brief scale object
 *
 * Scales the object. about factor s. s=1 will be the original size, -1 will
 * mirror it.
 *
 * \remarks s=0 is forbidden and will be ignored! s3d_scale is also absolute,
 * not relative!
 */
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
	return 0;
}

/** \brief focus mcp object
 *
 * This is an mcp-only function. It gives focus (for receiving key-strokes etc.)
 * to an app referenced by it's mcp-object-id.
 */
int s3d_mcp_focus(int object)
{
	uint32_t buf = htonl(object);
	net_send(S3D_P_MCP_FOCUS, (char *)&buf, 4);
	return 0;
}
