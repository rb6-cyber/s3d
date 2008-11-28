/*
 * graphics.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
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

#include "global.h"
#include <string.h>   /*  memcpy() */

#include <GL/gl.h>   /*  GLint */
#include <GL/glext.h>   /*  GL_RESCALE_NORMAL */
#ifdef G_SDL
#include <SDL.h>  /*  SDL_GL_SwapBuffers */
#endif
#include <math.h>   /*  sin(),cos() */
#ifndef INFINITY
#define INFINITY 1<<30
#endif
/*  this file handles graphics routines */
/*  local prototypes ... */
void render_virtual_object(struct t_obj *o);
/*  ... and types/variables */
static int select_mode = 0;
int winw, winh;

/*  this detects and opens the SDL things */

int graphics_init(void)
{
	GLfloat shin[] = {16.0};
	switch (frame_mode) {
#ifdef G_SDL
	case FRAME_SDL:
		graphics_init_sdl();
		break;
#endif
	default:
		return(-1);
	}
	/* light */
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	/* depth test */
	glEnable(GL_DEPTH_TEST);
	/*     glDepthFunc( GL_LEQUAL ); */

	/* textures */
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DITHER);

	/* lines */
	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	glLineWidth(1.0);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	/* polygon smoothing */
	glDisable(GL_POLYGON_SMOOTH);
	/*    glEnable(GL_POLYGON_SMOOTH);
	    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST); */

	/* normalizing */
	glDisable(GL_AUTO_NORMAL);
	glDisable(GL_NORMALIZE);   /* don't use the expensive GL_NORMALIZE, we use uniform scaling so GL_RESCALE_NORMAL is sufficent */
	glEnable(GL_RESCALE_NORMAL);

	/* blending */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	/*    glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);*/

	/* set shininess */
	/*  glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shin); */
	glMaterialfv(GL_FRONT, GL_SHININESS, shin);
	graphics_reshape(X_RES, Y_RES);

	/* face culling */
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	return(0);
}
/*  this is to be called when the window is resized or created ... */
void graphics_reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	winw = w;
	winh = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w > h)
		glFrustum(-((1.0*w) / h), (1.0*w) / h, -1.0, 1.0, 1.0, 5000);
	else
		glFrustum(-1.0, 1.0, -(1.0*h) / w, (1.0*h) / w, 1.0, 5000);
	glMatrixMode(GL_MODELVIEW);
	if (procs_p != NULL)
		event_cam_changed();
}
void render_virtual_object(struct t_obj *o)
{
	struct t_process *ap;
	struct t_vertex x, y;
	int32_t j, k;
	t_mtrx m;

	glPushMatrix();
	glMultMatrixf(o->m);
	glGetFloatv(GL_MODELVIEW_MATRIX, m);
	cull_get_planes();
	if (NULL == (ap = get_proc_by_pid(o->virtual_pid))) { /*  the clean way */
		errds(HIGH, "render_by_mcp()", "not existing pid (%p) referenced by mcp-object!!", (void*)o);
	} else {
		/*  now go throu the objects of our app  */
		for (j = 0;j < ap->n_obj;j++) {
			if (ap->object[j] != NULL) {
				if (((select_mode == 0) && ap->object[j]->oflags&OF_VISIBLE) || ((select_mode == 1) && (ap->object[j]->oflags&OF_SELECTABLE))) { /* either select mode is off or it's selectable */
					x.x = x.y = x.z = 0.0f;
					mySetMatrix(ap->object[j]->m); /* get into position ... */
					myTransformV(&x);
					y.x = 1.0;
					y.y = y.z = 0.0f;
					myTransformV(&y);
					y.x -= x.x;
					y.y -= x.y;
					y.z -= x.z;

					k = cull_sphere_in_frustum(&x, ap->object[j]->r * sqrt(y.x * y.x + y.y * y.y + y.z * y.z));
					if (k) {
						/*      s3dprintf(HIGH,"object %d is in %s frustum",j,k?"":"not");*/
						if (select_mode == 1)
							glPushName(j);
						obj_render(ap, j);
						if (select_mode == 1)
							glPopName();
					}
				}
			}
		}
	}
	glPopMatrix();
}
/*  this functions renders by going from mcp objects [as it should be],  */
/*  recursively positiniong the objects into the space. */
int render_by_mcp(void)
{
	struct t_process *p = get_proc_by_pid(MCP);
	int32_t i;
	struct t_obj *o;
	struct t_vertex x, y;
	int k;
	for (i = 0 ; i < p->n_obj ; i++) {  /* check all mcp objects ... */
		o = p->object[i];
		if (o != NULL) {
			if ((select_mode == 0 && o->oflags&OF_VISIBLE) || (select_mode == 1 && o->oflags&OF_SELECTABLE)) { /*  it's even visible ;) */
				if (o->oflags&OF_VIRTUAL) { /*  we have an app here. */
					if (o->r != 0.0) {
						cull_get_planes();
						mySetMatrix(o->m);
						x.x = x.y = x.z = 0.0f;
						myTransformV(&x);
						y.x = 1.0;
						y.y = y.z = 0.0f;
						myTransformV(&y);
						y.x -= x.x;
						y.y -= x.y;
						y.z -= x.z;

						k = cull_sphere_in_frustum(&x, o->r * sqrt(y.x * y.x + y.y * y.y + y.z * y.z));
						s3dprintf(VLOW, "mcp-object %d is in %s frustum", i, k ? "" : "not");
						if (k) {
							if (select_mode == 1) {
								s3dprintf(VLOW, "object %d in culling frustrum!", i);
								glLoadName(i);
							}
							render_virtual_object(o);
						} else {
							if (select_mode == 1) {
								s3dprintf(VLOW, "object %d not in culling frustrum!", i);
							}
						}
					}
				} else if ((o->oflags&OF_CLONE) && (p->object[o->clone_ooid]->oflags&OF_VIRTUAL)) { /* it's a clone of an app */
					if (select_mode == 1)
						glLoadName(o->clone_ooid);/*TODO: what to do if a clone is selected?! */
					glPushMatrix();
					render_virtual_object(o);
					render_virtual_object(p->object[o->clone_ooid]);
					glPopMatrix();
				} else { /* it's a "regular" mcp object */
					if (select_mode == 1) {
						s3dprintf(VLOW, "mcp object no. %d", i);
						glLoadName(-1);
						glPushName(i);
					}
					obj_render(p, i);
					if (select_mode == 1)
						glPopName();
				}
			}
		}
	}
	return(0);
}
/* this picks objects from their screen-positions and sends
 * OBK_CLICK-events for the selected object(s).
 * TODO: how big should the select buffer be? */
#define SBSIZE 65536
int graphics_pick_obj(int x, int y)
{
	int i, j;
	GLint viewport[4];
	GLfloat xpos, ypos;
	float big, z1, z2;
	int32_t mcp_o, o;
	struct t_process *p = get_proc_by_pid(MCP);
	GLuint select_buf[SBSIZE], *ptr;
	int hits, names;
	t_mtrx m;

	select_mode = 1;
	glSelectBuffer(SBSIZE, select_buf);
	glRenderMode(GL_SELECT);
	glMatrixMode(GL_PROJECTION);
	/*  count the objects .... */
	glPushMatrix();
	glLoadIdentity();
	glGetIntegerv(GL_VIEWPORT, viewport);
	if (winw > winh) {
		xpos = ((x - winw / 2.0) / (winw / 2.0)) * (((double)winw / winh));
		ypos = (((winh - y) - winh / 2.0) / (winh / 2.0));
	} else {
		xpos = ((x - winw / 2.0) / (winw / 2.0));
		ypos = (((winh - y) - winh / 2.0) / (winh / 2.0)) * (((double)winh / winw));
	}
#define mnear 0.001  /*  omg this is so dirty ... but works after all */
	glFrustum(xpos - mnear, xpos + mnear, ypos - mnear, ypos + mnear, 1, 5000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();  /*  get into position ... */
	mySetMatrix(p->object[0]->m);
	myInvert();
	myGetMatrix(m);
	glMultMatrixf(m);
	/*
	 glRotatef(-cam.rotate.x, 1.0,0.0,0.0);
	 glRotatef(-cam.rotate.y, 0.0,1.0,0.0);
	 glTranslatef(-cam.translate.x,-cam.translate.y,-cam.translate.z);*/

	glInitNames();
	glPushName(0);
	render_by_mcp();
	glFlush();
	hits = glRenderMode(GL_RENDER);
	if (hits > 0) {
		big = INFINITY;
		s3dprintf(LOW, "had %d hits", hits);
		ptr = select_buf;
		mcp_o = o = names = -1;
		/* check all the hits, only select the nearest ... */
		for (i = 0 ; i < hits ; i++) {
			names = *ptr;
			ptr++;
			z1 = (float) * ptr / 0x7fffffff;
			ptr++;
			z2 = (float) * ptr / 0x7fffffff;
			ptr++;
			if (z1 < big) {
				mcp_o = o = -1;
				for (j = 0;j < names;j++) {
					switch (j) {
					case 0:
						mcp_o = *ptr;
						break;
					case 1:
						o =  *ptr;
						break;
					}
					ptr++;
				}
				big = z1;
			} else
				for (j = 0;j < names;j++)
					ptr++;
			s3dprintf(VLOW, "[HIT %d] names %d [z1:%f|z2:%f] mcp_o=%d, o=%d ", i, names, z1, z2, mcp_o, o);
		}
		s3dprintf(VLOW, "mcp_o= %d, o= %d", mcp_o, o);
		if (mcp_o == -1) { /* it's an mcp object */
			s3dprintf(LOW, "clicked on mcp-object no. %d", o);
			event_obj_click(p, o);
		} else
			if ((names > 1) && ((mcp_o >= 0) && (mcp_o < p->n_obj))) { /* it's an usual object */
				s3dprintf(LOW, "clicked on mcp-object %d, object %d", mcp_o, o);
				if (p->object[mcp_o] != NULL) { /*  that shouldn't happen anyways ... */
					obj_debug(get_proc_by_pid(p->object[mcp_o]->virtual_pid), o);
					event_obj_click(get_proc_by_pid(p->object[mcp_o]->virtual_pid), o);
				}
			}
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	select_mode = 0;
	return(0);
}


void graphics_main(void)
{
	struct t_process *p = get_proc_by_pid(MCP);
	t_mtrx m;
	GLfloat pos[] = {0, 50, 50, 1.0};
	GLfloat light0_spec[] = {0.7, 0.7, 0.7, 0.0};
	GLfloat light0_shininess[] = {1.0};
	GLfloat light0_diff[] = {0.5, 0.5, 0.5, 1.0};
	GLfloat light0_amb[] = {1.0, 1.0, 1.0, 1.0};

	select_mode = 0;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  /*  clear screen */
	/*  set up the cam ... */
	glMatrixMode(GL_MODELVIEW);

	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_amb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diff);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_spec);
	glLightfv(GL_LIGHT0, GL_SHININESS, light0_shininess);

	glLoadIdentity();

	mySetMatrix(p->object[0]->m);
	myInvert();
	myGetMatrix(m);
	glMultMatrixf(m);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	/*glRotatef(-cam.rotate.z, 0.0,0.0,1.0);
	glRotatef(-cam.rotate.x, 1.0,0.0,0.0);
	glRotatef(-cam.rotate.y, 0.0,1.0,0.0);
	glTranslatef(-cam.translate.x,-cam.translate.y,-cam.translate.z);*/

	glPushMatrix();  /*  save the cam */
	render_by_mcp();
	glPopMatrix();  /*  restore the cam */
	glLoadIdentity();
	glMultMatrixf(m);

	switch (frame_mode) {
#ifdef G_SDL
	case FRAME_SDL:
		/* SDL will glFlush itself */
		SDL_GL_SwapBuffers();
		break;
#endif
#ifdef G_GLX
	case FRAME_GLX:
		...
#endif
	}
}

/*  quit the graphic-interface */
int graphics_quit(void)
{
	return(0);
}
