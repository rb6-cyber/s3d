/*
 * s3d.h
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>,
 *                         Sven Eckelmann <sven.eckelmann@gmx.de>
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
#ifndef LIBS3D_H
#define LIBS3D_H

#ifdef HAVE_GCCVISIBILITY
#define S3DEXPORT __attribute__ ((visibility("default")))
#else
#define S3DEXPORT
#endif

#include <stdint.h>  /* [u]intXX_t type definitions*/
/* definitions */
struct s3d_evt {
	uint8_t event;
	int length;
	char *buf;
	struct s3d_evt *next;
};

typedef int (*s3d_cb)(struct s3d_evt *);

#define S3D_EVENT_OBJ_CLICK  1
#define S3D_EVENT_KEY   2
#define S3D_EVENT_KEYDOWN  2
#define S3D_EVENT_MBUTTON  3
#define S3D_EVENT_KEYUP   4
#define S3D_EVENT_NEW_OBJECT 16
#define S3D_EVENT_OBJ_INFO  17

#define S3D_EVENT_QUIT   255

/* TODO: don't keep _MCP_ events .. they're ugly */
#define S3D_MCP_OBJECT   32
#define S3D_MCP_DEL_OBJECT  33

#define S3D_PORT    6066

#define S3D_OF_VISIBLE   0x00000001
#define S3D_OF_SELECTABLE  0x00000002
#define S3D_OF_POINTABLE  0x00000004
struct mcp_object {
	uint32_t object;
	float trans_x, trans_y, trans_z;
	float r;
#define MCP_NEW_OBJECT 1
	char name[256];
};
struct s3d_obj_info {
	uint32_t object;
	uint32_t flags;
	float trans_x, trans_y, trans_z;
	float rot_x, rot_y, rot_z;
	float scale;
	float r;
	char name[256];
};
struct s3d_but_info {
	uint8_t button; /* 0 = left, 1 = middle, 2 = right */
	uint8_t state;  /* 0 = down, 1 = up, 2 = moving */
};
struct s3d_key_event {
	uint16_t keysym;  /* the symbol, use this with s3d_keysym.h */
	uint16_t unicode;  /* the unicode or "actually typed" character */
	uint16_t modifier; /* any modifiers involved */
	uint16_t state;  /* 0 = pressed, 1 = released */
};

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

	/* framework functions */
	S3DEXPORT void s3d_usage(void);
	S3DEXPORT int s3d_init(int *argc, char ***argv, const char *name);
	S3DEXPORT int s3d_quit(void);
	S3DEXPORT int s3d_mainloop(void (*f)(void));

	/* object manipulations */
	S3DEXPORT int s3d_push_vertex(int object, float x, float y, float z);
	S3DEXPORT int s3d_push_vertices(int object, const float *vbuf, uint16_t n);
	S3DEXPORT int s3d_push_material(int object,
	                                float amb_r, float amb_g, float amb_b,
	                                float spec_r, float spec_g, float spec_b,
	                                float diff_r, float diff_g, float diff_b);
	S3DEXPORT int s3d_pep_material(int object,
	                               float amb_r, float amb_g, float amb_b,
	                               float spec_r, float spec_g, float spec_b,
	                               float diff_r, float diff_g, float diff_b);
	S3DEXPORT int s3d_push_material_a(int object,
	                                  float amb_r, float amb_g, float amb_b, float amb_a,
	                                  float spec_r, float spec_g, float spec_b, float spec_a,
	                                  float diff_r, float diff_g, float diff_b, float diff_a);
	S3DEXPORT int s3d_push_materials_a(int object, const float *mbuf, uint16_t n);
	S3DEXPORT int s3d_pep_material_a(int object,
	                                 float amb_r, float amb_g, float amb_b, float amb_a,
	                                 float spec_r, float spec_g, float spec_b, float spec_a,
	                                 float diff_r, float diff_g, float diff_b, float diff_a);

	S3DEXPORT int s3d_pep_materials_a(int object, const float *mbuf, uint16_t n);
	S3DEXPORT int s3d_load_materials_a(int object, const float *mbuf, uint32_t start, uint16_t n);
	S3DEXPORT int s3d_push_polygon(int object, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t material);
	S3DEXPORT int s3d_push_polygons(int object, const uint32_t *pbuf, uint16_t n);
	S3DEXPORT int s3d_push_line(int object, uint32_t v1, uint32_t v2, uint32_t material);
	S3DEXPORT int s3d_push_lines(int object, const uint32_t *lbuf, uint16_t n);
	S3DEXPORT int s3d_push_texture(int object, uint16_t w, uint16_t h);
	S3DEXPORT int s3d_push_textures(int object, const uint16_t *tbuf, uint16_t n);
	S3DEXPORT int s3d_pop_vertex(int object, uint32_t n);
	S3DEXPORT int s3d_pop_polygon(int object, uint32_t n);
	S3DEXPORT int s3d_pop_material(int object, uint32_t n);
	S3DEXPORT int s3d_pop_texture(int object, uint32_t n);
	S3DEXPORT int s3d_pop_polygon(int object, uint32_t n);
	S3DEXPORT int s3d_pop_line(int object, uint32_t n);
	S3DEXPORT int s3d_pep_line_normals(int object, const float *nbuf, uint16_t n);
	S3DEXPORT int s3d_pep_polygon_normals(int object, const float *nbuf, uint16_t n);
	S3DEXPORT int s3d_pep_polygon_tex_coord(int object, float x1, float y1, float x2, float y2, float x3, float y3);
	S3DEXPORT int s3d_pep_polygon_tex_coords(int object, const float *tbuf, uint16_t n);
	S3DEXPORT int s3d_pep_material_texture(int object, uint32_t tex);
	S3DEXPORT int s3d_pep_vertex(int object, float x, float y, float z);
	S3DEXPORT int s3d_pep_vertices(int object, const float *vbuf, uint16_t n);
	S3DEXPORT int s3d_pep_line(int object, int v1, int v2, int material);
	S3DEXPORT int s3d_pep_lines(int object, const uint32_t *lbuf, uint16_t n);
	S3DEXPORT int s3d_load_line_normals(int object, const float *nbuf, uint32_t start, uint16_t n);
	S3DEXPORT int s3d_load_polygon_normals(int object, const float *nbuf, uint32_t start, uint16_t n);
	S3DEXPORT int s3d_load_polygon_tex_coords(int object, const float *tbuf, uint32_t start, uint16_t n);
	S3DEXPORT int s3d_load_texture(int object, uint32_t tex, uint16_t xpos, uint16_t ypos, uint16_t w, uint16_t h, const uint8_t *data);

	S3DEXPORT int s3d_new_object(void);
	S3DEXPORT int s3d_del_object(int oid);

	S3DEXPORT int s3d_clone(int oid);
	S3DEXPORT int s3d_clone_target(int oid, int toid);

	S3DEXPORT int s3d_link(int oid_from, int oid_to);
	S3DEXPORT int s3d_unlink(int oid);

	S3DEXPORT int s3d_flags_on(int object, uint32_t flags);
	S3DEXPORT int s3d_flags_off(int object, uint32_t flags);
	S3DEXPORT int s3d_translate(int object, float x, float y, float z);
	S3DEXPORT int s3d_rotate(int object, float x, float y, float z);
	S3DEXPORT int s3d_scale(int object, float s);

	/* high-level object creating */
	S3DEXPORT int s3d_import_model_file(const char *fname);
	S3DEXPORT int s3d_open_file(const char *fname, char **pointer);
	S3DEXPORT int s3d_select_font(const char *mask);
	S3DEXPORT int s3d_draw_string(const char *str, float *xlen);
	S3DEXPORT float s3d_strlen(const char *str);

	/* some vector calculation helpers */

	S3DEXPORT float s3d_vector_length(const float vector[]);
	S3DEXPORT float s3d_vector_dot_product(const float vector1[], const float vector2[]);
	S3DEXPORT void s3d_vector_subtract(const float vector1[], const float vector2[], float result_vector[]);
	S3DEXPORT float s3d_vector_angle(const float vector1[], const float vector2[]);
	S3DEXPORT float s3d_angle_to_cam(const float obj_pos[], const float cam_pos[], float *angle_rad);
	S3DEXPORT void s3d_vector_cross_product(const float vector1[], const float vector2[], float result_vector[]);

	/* event handlers */
	S3DEXPORT void s3d_push_event(struct s3d_evt *newevt);
	S3DEXPORT struct s3d_evt *s3d_pop_event(void);
	S3DEXPORT struct s3d_evt *s3d_find_event(uint8_t event);
	S3DEXPORT int s3d_delete_event(const struct s3d_evt *devt);

	S3DEXPORT void s3d_set_callback(uint8_t event, s3d_cb func);
	S3DEXPORT void s3d_clear_callback(uint8_t event);
	S3DEXPORT void s3d_ignore_callback(uint8_t event);
	S3DEXPORT s3d_cb s3d_get_callback(uint8_t event);
	S3DEXPORT void s3d_process_stack(void);

	/* mcp special */
	S3DEXPORT int s3d_mcp_focus(int object);

	/* for apps which don't employ s3d_mainloop() */
	S3DEXPORT int s3d_net_check(void);

#if defined(__cplusplus) || defined(c_plusplus)
} /* extern "C" */
#endif

#endif
