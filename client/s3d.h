/*
 * s3d.h
 *
 * Copyright (C) 2004-2006 Simon Wunderlich <dotslash@packetmixer.de>
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


struct s3d_evt {
	unsigned char event;
	int length;
	char *buf;
	struct s3d_evt *next;
};

typedef void (*s3d_cb)(struct s3d_evt *);
void s3d_usage();
int s3d_init(int *argc, char ***argv, char *name);
int s3d_quit();
int s3d_mainloop(void (*f)());

int s3d_push_vertex(int object, float x, float y, float z);
int s3d_push_vertices(int object, float *vbuf, unsigned short n);
int s3d_push_material( int object, 
						float amb_r, float amb_g, float amb_b,
						float spec_r, float spec_g, float spec_b,
						float diff_r, float diff_g, float diff_b);
int s3d_pep_material( int object, 
						float amb_r, float amb_g, float amb_b,
						float spec_r, float spec_g, float spec_b,
						float diff_r, float diff_g, float diff_b);
int s3d_push_material_a( int object, 
						float amb_r, float amb_g, float amb_b, float amb_a,
						float spec_r, float spec_g, float spec_b, float spec_a,
						float diff_r, float diff_g, float diff_b, float diff_a);
int s3d_push_materials_a(int object, float *mbuf, unsigned short n);
int s3d_pep_material_a( int object, 
						float amb_r, float amb_g, float amb_b, float amb_a,
						float spec_r, float spec_g, float spec_b, float spec_a,
						float diff_r, float diff_g, float diff_b, float diff_a);

int s3d_pep_materials_a(int object, float *mbuf, unsigned short n);
int s3d_load_materials_a(int object, float *mbuf, unsigned long start, unsigned short n);
int s3d_push_polygon(int object, int v1, int v2, int v3, int material);
int s3d_push_polygons(int object, unsigned long *pbuf, unsigned short n);
int s3d_push_texture(int object, unsigned short w, unsigned short h);
int s3d_pop_vertex(int object, unsigned long n);
int s3d_pop_polygon(int object, unsigned long n);
int s3d_pop_material(int object, unsigned long n);
int s3d_pop_texture(int object, unsigned long n);
int s3d_pep_polygon_normals(int object, float *nbuf,unsigned short n);
int s3d_pep_polygon_tex_coord(int object, float x1, float y1, float x2, float y2, float x3, float y3);
int s3d_pep_polygon_tex_coords(int object, float *tbuf,unsigned short n);
int s3d_pep_material_texture(int object, unsigned long mat, unsigned long tex);
int s3d_load_polygon_normals(int object, float *nbuf,unsigned long start, unsigned short n);
int s3d_load_polygon_tex_coords(int object, float *tbuf, unsigned long start, unsigned short n);
int s3d_load_texture(int object, unsigned long tex, unsigned short xpos, unsigned short ypos, unsigned short w, unsigned short h, char *data);
int s3d_new_object();
int s3d_del_object(unsigned long oid);
int s3d_clone(unsigned long oid);
int s3d_clone_target(unsigned long oid, unsigned long toid);
int s3d_link(unsigned long oid_from, unsigned long oid_to);
int s3d_unlink(unsigned long oid);
int s3d_import_3ds_file(char *fname);
int s3d_import_3ds(char *buf);
int s3d_open_file(char *fname, char **pointer);
int s3d_select_font(char *mask);
int s3d_draw_string( char *str, float *xlen);
int s3d_flags_on(int object, unsigned long flags);
int s3d_flags_off(int object, unsigned long flags);
int s3d_translate(int object, float x, float y, float z);
int s3d_rotate(int object, float x, float y, float z);
int s3d_scale(int object, float s);

void s3d_push_event(struct s3d_evt *newevt);
struct s3d_evt *s3d_pop_event();
struct s3d_evt *s3d_find_event(unsigned char event);
int s3d_delete_event(struct s3d_evt *devt);


void s3d_set_callback(unsigned char event, s3d_cb func);
void s3d_clear_callback(unsigned char event);
void s3d_ignore_callback(unsigned char event);
s3d_cb s3d_get_callback(unsigned char event);
void s3d_process_stack();

int s3d_mcp_focus(int object);
#define S3D_EVENT_OBJ_CLICK		1
#define S3D_EVENT_KEY			2
#define S3D_EVENT_NEW_OBJECT	16
#define S3D_EVENT_OBJ_INFO		17

#define S3D_EVENT_QUIT			255

/* TODO: don't keep _MCP_ events .. they're ugly */
#define S3D_MCP_OBJECT			32
#define S3D_MCP_DEL_OBJECT		33

#define S3D_PORT				6066

#define	S3D_OF_VISIBLE			0x00000001
#define	S3D_OF_SELECTABLE		0x00000002
#define S3D_OF_POINTABLE		0x00000004
struct mcp_object 
{
	unsigned long object;
	float trans_x,trans_y,trans_z;
	float r;
#define MCP_NEW_OBJECT	1
	char name[256]; 
};
struct s3d_obj_info 
{
	unsigned long object;
	unsigned long flags;
	float trans_x,trans_y,trans_z;
	float rot_x,rot_y,rot_z;
	float scale;
	float r;
	char name[256]; 
};

