/*
 * 3dsloader.c
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
 *
 * This file is part of s3d, a 3d network display server.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3d is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3d is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3d; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */




#include <s3d.h>
#include <stdio.h>  /* NULL */
#include <time.h>	/* nanosleep() */
#include <g3d/g3d.h>
#include <stdlib.h>


struct material2texture {
	struct material2texture *next_ptr;   /* pointer to next */
	void *material_ptr;
	void *texture_ptr;
	int material_id;
	int texture_id;
};


static struct timespec t={0,100*1000*1000}; /* 100 mili seconds */
struct material2texture *mat2tex_root = NULL;
int i,obj_id;



void mainloop() {
	s3d_rotate(obj_id,0,i,0);
	i++;
	nanosleep(&t,NULL);
}



void object_click(struct s3d_evt *evt) {
	s3d_quit();
}



void *get_mat2tex( struct material2texture **mat2tex, void *mat_ptr ) {

	while ( (*mat2tex) != NULL ) {

		if ( (*mat2tex)->material_ptr == mat_ptr ) return (*mat2tex);

		mat2tex = &(*mat2tex)->next_ptr;

	}

	if ( (*mat2tex) == NULL ) {

		(*mat2tex) = malloc( sizeof( struct material2texture ) );

		if ( (*mat2tex) == NULL ) {
			printf( "Sorry - you ran out of memory !\n" );
			exit(8);
		}

		(*mat2tex)->next_ptr = NULL;
		(*mat2tex)->material_ptr = mat_ptr;
		(*mat2tex)->texture_ptr = NULL;
		(*mat2tex)->material_id = -1;
		(*mat2tex)->texture_id = -1;

		return (*mat2tex);

	}

	return(0);

}



int main (int argc, char **argv) {

	G3DContext *context;
	G3DModel *model;
	G3DObject *object;
	G3DFace *face;
	GSList *oitem, *oface;
	struct material2texture *mat2tex;
	int j, material_count, texture_count;

	if (argc<2) {
		printf("usage: %s [somefile.3ds]\n",argv[0]);
		return(-1);
	}

	context = g3d_context_new();

	if ( !s3d_init( &argc,&argv,"3dsloader_g3d" ) ) {

		s3d_set_callback(S3D_EVENT_OBJ_CLICK,object_click);

		model = g3d_model_load(context, argv[1]);

		if ( model ) {

			oitem = model->objects;
			obj_id = s3d_new_object();
			material_count = texture_count = 0;

			while ( oitem ) {

				object = (G3DObject *)oitem->data;

				/* push vertices */
				for ( j = 0; j < object->vertex_count; j++ ) {
					s3d_push_vertex( obj_id, object->vertex_data[j * 3], object->vertex_data[j * 3 + 1], object->vertex_data[j * 3 + 2] );
				}


				oface = object->faces;

				while ( oface ) {

					face = (G3DFace *)oface->data;

					mat2tex = get_mat2tex( &mat2tex_root, face->material );

					if ( mat2tex->material_id == -1 ) {

						printf( "push material: %i\n", material_count );

						s3d_push_material_a( obj_id, face->material->r, face->material->g, face->material->b, face->material->a, face->material->specular[0], face->material->specular[1], face->material->specular[2], face->material->specular[3], face->material->r, face->material->g, face->material->b, face->material->a );

						mat2tex->material_id = material_count;
						material_count++;

						if ( face->tex_image != NULL ) {

							s3d_push_texture( obj_id, face->tex_image->width, face->tex_image->height );
							s3d_pep_material_texture( obj_id, texture_count );
							s3d_load_texture( obj_id, texture_count, 0, 0, face->tex_image->width, face->tex_image->height, face->tex_image->pixeldata );

							mat2tex->texture_id = texture_count;
							texture_count++;

						}

					}

					printf( "push polygone with material: %i\n", mat2tex->material_id );

					/* push polygones */
					s3d_push_polygon( obj_id, face->vertex_indices[0], face->vertex_indices[1], face->vertex_indices[2], mat2tex->material_id );

					/* has polygone normals */
					if ( face->flags & G3D_FLAG_FAC_NORMALS ) s3d_pep_polygon_normals( obj_id, face->normals, 1 );

					/* face with texture */
					if ( ( mat2tex->texture_id != -1 ) && ( face->flags & G3D_FLAG_FAC_TEXMAP ) ) {

						printf( "neues face: %i\n", face->tex_vertex_count );
/* 						for ( j = 0; j < face->tex_vertex_count; j++ ) {
 							printf( "%i -> %f\n", j, face->tex_vertex_data[j] ); */
							s3d_pep_polygon_tex_coord( obj_id, face->tex_vertex_data[0], face->tex_vertex_data[1], face->tex_vertex_data[2], face->tex_vertex_data[3], face->tex_vertex_data[4], face->tex_vertex_data[5] );
/* 						} */

					}


					oface = oface->next;

				}

				s3d_flags_on( obj_id, S3D_OF_VISIBLE|S3D_OF_SELECTABLE );

				oitem = oitem->next;

			}

			s3d_mainloop(mainloop);

		}

		s3d_quit();

	}

	return(0);

}
