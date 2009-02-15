/*
 * modelread.c
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
#include <g3d/g3d.h>
#include <g3d/matrix.h>
#include <stdlib.h>   /*  exit(), malloc() */
#include <math.h>   /*  sqrt() */
#include <string.h>   /*  strncpy() */
#include <errno.h>    /*  errno */


struct material2texture {
	struct material2texture *next_ptr;   /* pointer to next */
	void *material_ptr;
	void *texture_ptr;
	int material_id;
	int texture_id;
};
static struct material2texture *mat2tex_root = NULL;

#define MAXSTRN  20
static int model_load(char *file);
/*  just a helper function for reading from file instead of memory. */
int s3d_import_model_file(const char *fname)
{
	char *buf, *ptr, *next;
	char searchpath[1024];
	char path[1024];
	int oid;
	if (fname == NULL) return(-1);
#ifndef OBJSDIR
#define OBJSDIR  ":./:../:../../:/usr/local/share/s3d/:/usr/share/s3d/"
#endif

	strncpy(searchpath, OBJSDIR, 1023);
	searchpath[1023] = 0;     /* just in case */
	next = ptr = searchpath;
	while (next != NULL) {
		next = NULL;

		if (NULL != (next = strchr(ptr, ':'))) {
			*next = 0;      /* clear the delimiter */
			next += 1;     /* move to the beginner of the next dir */
		}
		if ((strlen(ptr) + strlen(fname)) < 1024) { /* only try if this fits */
			strcpy(path, ptr);     /* can use "unsafe" functions because size was verified above */
			strcat(path, fname);
			if (s3d_open_file(path, &buf) != -1) { /* found something */
				free(buf); /* TODO: badbadbad ... */
				if (-1 != (oid = model_load(path))) return(oid);

			}
		}
		if (next != NULL)
			ptr = next;     /* move pointer to the next position */
	}
	errds(LOW, "s3d_import_model_file()", "Could not open %s", fname);
	return(-1); /* nothing in search path ... */
}

static struct material2texture* get_mat2tex(struct material2texture **mat2tex, void *mat_ptr)
{
	while ((*mat2tex) != NULL) {

		if ((*mat2tex)->material_ptr == mat_ptr) return (*mat2tex);

		mat2tex = &(*mat2tex)->next_ptr;

	}

	if ((*mat2tex) == NULL) {

		(*mat2tex) = (struct material2texture *)malloc(sizeof(struct material2texture));

		if ((*mat2tex) == NULL) {
			errs("model_import()", "Sorry - you ran out of memory !\n");
			exit(8);
		}

		(*mat2tex)->next_ptr = NULL;
		(*mat2tex)->material_ptr = mat_ptr;
		(*mat2tex)->texture_ptr = NULL;
		(*mat2tex)->material_id = -1;
		(*mat2tex)->texture_id = -1;

		return (*mat2tex);

	}

	return(NULL);

}



static int model_load(char *file)
{
	G3DContext     *context;
	G3DModel    *model;
	G3DObject     *object;
	G3DFace     *face;
	GSList      *oitem, *oface;
	G3DMatrix rmatrix[16];
	struct material2texture *mat2tex;
	unsigned int i, k;
	int       j, material_count, texture_count, voff, obj_id;
#define      PMAX 100
	uint32_t     polybuf[PMAX * 4], npoly, oldflags;
	float      normalbuf[PMAX * 9], texcoordbuf[PMAX * 6];
	float       swaph;    /* swap helper */
	uint8_t    *s3d_pixeldata = NULL;

	context = g3d_context_new();
	obj_id = -1;
	model = g3d_model_load_full(context, file, 0);

	if (model) {
		g3d_matrix_identity(rmatrix);
		g3d_matrix_rotate_xyz(G_PI * 90.0 / 180, 0.0, 0.0, rmatrix);
		g3d_model_transform(model, rmatrix);

		oitem = model->objects;
		obj_id = s3d_new_object();
		material_count = texture_count = voff = 0;

		while (oitem) {

			object = (G3DObject *)oitem->data;

			/* push vertices */
			for (i = 0; i < object->vertex_count; i++) {
				/* 2. and 3. coord have to change places otherwise the object will be turned */
				object->vertex_data[i * 3 + 0] =  object->vertex_data[i * 3 + 0];
				swaph =         object->vertex_data[i * 3 + 2];
				object->vertex_data[i * 3 + 2] = -object->vertex_data[i * 3 + 1];
				object->vertex_data[i * 3 + 1] = swaph;
			}
			s3d_push_vertices(obj_id, object->vertex_data, object->vertex_count);



			if (NULL == (oface = object->faces)) {
				voff += object->vertex_count; /* increase vertex offset */
				oitem = oitem->next;
				continue;
			}
			npoly = 0;
			oldflags = ((G3DFace *)(oface->data))->flags;

			while (oface) {

				face = (G3DFace *)oface->data;
				mat2tex = get_mat2tex(&mat2tex_root, face->material);

				if (mat2tex->material_id == -1) {   /* create a new texture if nothing found */
					s3d_push_material_a(obj_id,  face->material->r, face->material->g, face->material->b, face->material->a,
					                    face->material->specular[0], face->material->specular[1], face->material->specular[2], face->material->specular[3],
					                    face->material->r, face->material->g, face->material->b, face->material->a);

					mat2tex->material_id = material_count;
					material_count++;

					if (face->tex_image != NULL) {

						/* reorder pixeldata - s3d wants rgba */
						if (s3d_pixeldata != NULL) free(s3d_pixeldata);

						s3d_pixeldata = (uint8_t*)malloc(sizeof(uint8_t) * face->tex_image->width * face->tex_image->height * 32);

						if (s3d_pixeldata == NULL) {
							errs("model_load()", "Sorry - you ran out of memory !\n");
							exit(8);
						}

						for (j = (face->tex_image->height - 1); j >= 0; j--) {
							for (k = 0; k < face->tex_image->width; k++) {
								s3d_pixeldata[(j * face->tex_image->width + k) * 4 + 0 ] = face->tex_image->pixeldata[(j * face->tex_image->width + k) * 4 + 2 ];
								s3d_pixeldata[(j * face->tex_image->width + k) * 4 + 1 ] = face->tex_image->pixeldata[(j * face->tex_image->width + k) * 4 + 1 ];
								s3d_pixeldata[(j * face->tex_image->width + k) * 4 + 2 ] = face->tex_image->pixeldata[(j * face->tex_image->width + k) * 4 + 0 ];
								s3d_pixeldata[(j * face->tex_image->width + k) * 4 + 3 ] = face->tex_image->pixeldata[(j * face->tex_image->width + k) * 4 + 3 ];
							}
						}

						s3d_push_texture(obj_id, face->tex_image->width, face->tex_image->height);
						s3d_pep_material_texture(obj_id, texture_count);
						s3d_load_texture(obj_id, texture_count, 0, 0, face->tex_image->width, face->tex_image->height, s3d_pixeldata);

						mat2tex->texture_id = texture_count;
						texture_count++;

					}
				}
				if (face->flags != oldflags || npoly >= PMAX) {
					/* push things so far */
					s3d_push_polygons(obj_id, polybuf, npoly);
					if (oldflags & G3D_FLAG_FAC_NORMALS)  s3d_pep_polygon_normals(obj_id, normalbuf,   npoly);
					if (oldflags & G3D_FLAG_FAC_TEXMAP)   s3d_pep_polygon_tex_coords(obj_id, texcoordbuf, npoly);
					npoly = 0;
				}
				oldflags = face->flags;

				/* add polygon to the polygon buffer */
				polybuf[npoly*4+0] = face->vertex_indices[0] + voff;
				polybuf[npoly*4+1] = face->vertex_indices[2] + voff;
				polybuf[npoly*4+2] = face->vertex_indices[1] + voff;
				polybuf[npoly*4+3] = mat2tex->material_id;

				if (face->flags & G3D_FLAG_FAC_NORMALS) {
					normalbuf[ npoly*9 + 0] = -face->normals[ 0 ];
					normalbuf[ npoly*9 + 1] = -face->normals[ 2 ];
					normalbuf[ npoly*9 + 2] =  face->normals[ 1 ];
					normalbuf[ npoly*9 + 3] = -face->normals[ 6 ];
					normalbuf[ npoly*9 + 4] = -face->normals[ 8 ];
					normalbuf[ npoly*9 + 5] =  face->normals[ 7 ];
					normalbuf[ npoly*9 + 6] = -face->normals[ 3 ];
					normalbuf[ npoly*9 + 7] = -face->normals[ 5 ];
					normalbuf[ npoly*9 + 8] =  face->normals[ 4 ];
				}
				if (face->flags & G3D_FLAG_FAC_TEXMAP) {
					texcoordbuf[ npoly*6 + 0] = face->tex_vertex_data[ 0 ];
					texcoordbuf[ npoly*6 + 1] = face->tex_vertex_data[ 1 ];
					texcoordbuf[ npoly*6 + 2] = face->tex_vertex_data[ 4 ];
					texcoordbuf[ npoly*6 + 3] = face->tex_vertex_data[ 5 ];
					texcoordbuf[ npoly*6 + 4] = face->tex_vertex_data[ 2 ];
					texcoordbuf[ npoly*6 + 5] = face->tex_vertex_data[ 3 ];
				}
				npoly++;
				oface = oface->next;
			}
			/* push the last packets in buffer */
			if (npoly > 0) {
				s3d_push_polygons(obj_id, polybuf, npoly);
				if (oldflags & G3D_FLAG_FAC_NORMALS)  s3d_pep_polygon_normals(obj_id, normalbuf,   npoly);
				if (oldflags & G3D_FLAG_FAC_TEXMAP)   s3d_pep_polygon_tex_coords(obj_id, texcoordbuf, npoly);
			}

			voff += object->vertex_count; /* increase vertex offset */
			oitem = oitem->next;
		}
		g3d_model_free(model);
	}
	g3d_context_free(context);

	return(obj_id);
}

