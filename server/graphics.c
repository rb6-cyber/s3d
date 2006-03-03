/*
 * graphics.c
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

#include "global.h"
#include <stdlib.h>		 /*  malloc() */
#include <string.h>		 /*  memcpy() */
#include <GL/glut.h> 	 /*  glutWireTorus() -  to be removed later */ 
#include <GL/gl.h>		 /*  GLint */
#ifdef G_SDL
#include <SDL.h>	 /*  SDL_GL_SwapBuffers */
#endif
#include <math.h>		 /*  sin(),cos() */
#ifndef INFINITY
#define INFINITY 1<<30
#endif
/*  this file handles graphics routines */
/*  local prototypes ... */
void render_virtual_object(struct t_obj *o);
/*  ... and types/variables */
int select_mode=0;
int winw,winh;
extern struct t_process *procs_p;
extern int frame_mode;  /*  GLUT, SDL, ... ? */
/*  this detects and opens the SDL things */

int graphics_init ()
{
	GLfloat shin[]={1.0};
	switch (frame_mode)
	{
#ifdef G_SDL
		case G_SDL:graphics_init_sdl();break;
#endif
#ifdef G_GLUT
		case G_GLUT:graphics_init_glut();break;
#endif
		default: return(-1);
	}
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
/*     glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE); */
    glShadeModel (GL_SMOOTH);

/*     glDepthFunc( GL_LEQUAL ); */
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	glDisable(GL_DITHER);
	glEnable(GL_NORMALIZE);
	glDisable(GL_AUTO_NORMAL);
    glLineWidth(2.0);
/* 	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shin); */
	glMaterialfv(GL_FRONT,GL_SHININESS,shin);
/*	cam.translate.x=0;
	cam.translate.y=0;
	cam.translate.z=5;
	cam.rotate.x=0;
	cam.rotate.y=0;
	cam.rotate.z=0;
	cam.scale=1;
	cam.oflags=OF_CAM;
	cam.linkid=-1;
	cam.m_uptodate=0;*/
	graphics_reshape(X_RES,Y_RES);
	return(0);
}
/*  this is to be called when the window is resized or created ... */
void graphics_reshape( int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	winw=w;
	winh=h;
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	if (w>h)
		glFrustum(-((1.0*w)/h),(1.0*w)/h,-1.0,1.0,1.0,5000);
	else
		glFrustum(-1.0,1.0,-(1.0*h)/w,(1.0*h)/w,1.0,5000);
	glMatrixMode(GL_MODELVIEW);
	if (procs_p!=NULL)
	event_cam_changed();
}
void render_virtual_object(struct t_obj *o)
{
	struct t_process *ap;
	struct t_vertex x;
	uint32_t j,k;
	t_mtrx m;

	glPushMatrix();
	glMultMatrixf(o->m);
	glGetFloatv(GL_MODELVIEW_MATRIX,m);
	cull_get_planes();
	if (NULL==(ap=get_proc_by_pid(o->n_mat)))  /*  the clean way */
	{
		errds(HIGH,"render_by_mcp()","not existing pid (%d) referenced by mcp-object!!", o);
	} else {
	 /*  now go throu the objects of our app  */
		for (j=0;j<ap->n_obj;j++)
		{
			if (ap->object[j]!=NULL)
			{
				if (ap->object[j]->oflags&OF_VISIBLE)
				if ((select_mode ==0 ) || (ap->object[j]->oflags&OF_SELECTABLE)) /* either select mode is off or it's selectable */
				{
					x.x=x.y=x.z=0.0f;
					mySetMatrix(ap->object[j]->m); /* get into position ... */
					myTransformV(&x);
					k=cull_sphere_in_frustum(&x,ap->object[j]->r);
					if (k)
					{
/*						dprintf(HIGH,"object %d is in %s frustum",j,k?"":"not");*/
						if (select_mode==1)
							glPushName(j);
						obj_render(ap,j);
						if (select_mode==1)
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
int render_by_mcp()
{
	struct t_process *p=get_proc_by_pid(MCP);
	uint32_t i;
	struct t_obj *o;
	struct t_vertex x;
	int k;
	for (i=0;i<p->n_obj;i++)
	{
		o=p->object[i];
		if (o!=NULL)
		{
			if (o->oflags&OF_VISIBLE)   /*  it's even visible;) */
			{
				if (o->oflags&OF_VIRTUAL)  /*  we have an app here. */
				{
					if (o->r!=0.0)
					{
						cull_get_planes(); 
						mySetMatrix(o->m);
						x.x=x.y=x.z=0.0f;
						myTransformV(&x);
						k=cull_sphere_in_frustum(&x,o->r);
						dprintf(VLOW,"mcp-object %d is in %s frustum",i,k?"":"not");
						if (k)
							{
							if (select_mode==1)
							{
								dprintf(MED,"object %d in culling frustrum!",i);
								glLoadName(i);
							}
							render_virtual_object(o);
						} else {
							if (select_mode==1)
							{
								dprintf(MED,"object %d not in culling frustrum!",i);
							}
						}
					}
				} else if ((o->oflags&OF_CLONE) && (p->object[o->n_vertex]->oflags&OF_VIRTUAL))
				{
					if (select_mode==1)
						glLoadName(o->n_vertex);/*TODO: what to do if a clone is selected?! */
					glPushMatrix();
					render_virtual_object(o);
					render_virtual_object(p->object[o->n_vertex]); 
					glPopMatrix();
				} else {
					if ((select_mode ==0 ) || (p->object[i]->oflags&OF_SELECTABLE)) /* either select mode is off or it's selectable */
					{
						if (select_mode==1)
						{
							dprintf(HIGH,"mcp object no. %d",i);
							glLoadName(-1);
							glPushName(i);
						}
						obj_render(p,i);
						if (select_mode==1)
							glPopName();
					}
				}
			}
		}
	}
	return(0);
}
 /* this picks objects from their screen-positions and sends
  * OBK_CLICK-events for the selected object(s).
  * TODO: how big should the select buffer be? */
#define SBSIZE	65536
int graphics_pick_obj(int x, int y)
{
	int i,j;
	GLint viewport[4];
	GLfloat xpos,ypos;
	float big,z1;
	uint32_t mcp_o,o;
	struct t_process *p=get_proc_by_pid(MCP);
	GLuint select_buf[SBSIZE],*ptr,names,hits;
	t_mtrx m;

	select_mode=1;
	dprintf(LOW,"looking for some objects ...");
	glSelectBuffer(SBSIZE,select_buf);
	glRenderMode(GL_SELECT);
	glMatrixMode(GL_PROJECTION);
	 /*  count the objects .... */
	glPushMatrix();
	glLoadIdentity();
	glGetIntegerv(GL_VIEWPORT, viewport);
	if (winw>winh)
	{
		xpos=((x-winw/2.0)/(winw/2.0))*(((double)winw/winh));
		ypos=(((winh-y)-winh/2.0)/(winh/2.0));
	} else {
		xpos=((x-winw/2.0)/(winw/2.0));
		ypos=(((winh-y)-winh/2.0)/(winh/2.0))*(((double)winh/winw));
	}
#define mnear 0.001  /*  omg this is so dirty ... but works after all */
	glFrustum(xpos-mnear,xpos+mnear,ypos-mnear,ypos+mnear,1,5000);
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
	hits=glRenderMode(GL_RENDER);
	if (hits>0)
	{
		big=INFINITY;
		dprintf(LOW,"had %d hits",hits);
		ptr=select_buf;
		mcp_o=o=names=-1;
		/* check all the hits, only select the nearest ... */
		for (i=0;i<hits;i++)
		{
			names=*ptr;
			dprintf(LOW,"number of names for hit = %d", names); ptr++;
			z1=*ptr/0x7fffffff;
		    dprintf(LOW," z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
		    dprintf(LOW," z2 is %g", (float) *ptr/0x7fffffff); ptr++;
			mcp_o=o=-1;
			for (j=0;j<names;j++)
			{
				if (z1<big)
				{
					switch (j)
					{
						case 0:mcp_o=	*ptr;break;
						case 1:o=		*ptr;break;
					}
				}
				ptr++;
			}
		}
		dprintf(MED,"mcp_o= %d, o= %d",mcp_o,o);
		ptr=select_buf;
		if (mcp_o==-1) /* it's an mcp object */
		{
			dprintf(MED,"clicked on mcp-object no. %d",o);
			event_obj_click(p,o);
		} else 
		if ((names>1) && ((mcp_o>=0)&&(mcp_o<p->n_obj)))
		{ /* it's an usual object */
			dprintf(LOW,"clicked on mcp-object %d, object %d",mcp_o,o);
			if (p->object[mcp_o]!=NULL)  /*  that shouldn't happen anyways ... */
			{
				obj_debug(get_proc_by_pid(p->object[mcp_o]->n_mat),o); 
				event_obj_click(get_proc_by_pid(p->object[mcp_o]->n_mat),o);
			}
 	}
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	select_mode=0;
	return(0);
}


void graphics_main()
{
	struct t_process *p=get_proc_by_pid(MCP);
	t_mtrx m;
	GLfloat pos[]={100.0,20.0,100.0,1.0};
	GLfloat light0_spec[]={0.4,0.4,0.4,0.0};
	GLfloat light0_shininess[] ={50.0};
	GLfloat light0_diff[]={0.5,0.5,0.5,1.0};
	GLfloat light0_amb[]={1.0,1.0,1.0,1.0};
	
	GLfloat wire_amb[]={0.1,0.3,0.1,0.5};
	GLfloat wire_diff[]={0.1,0.3,0.1,0.5};
	GLfloat wire_spec[]={1.0,1.0,1.0,0.5};

	select_mode=0;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  /*  clear screen */
	 /*  set up the cam ... */
	glMatrixMode(GL_MODELVIEW);

	glLightfv(GL_LIGHT0,GL_AMBIENT,light0_amb);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light0_diff);
	glLightfv(GL_LIGHT0,GL_SPECULAR,light0_spec);
	glLightfv(GL_LIGHT0,GL_SHININESS,light0_shininess);
	
	glLoadIdentity();
	
	mySetMatrix(p->object[0]->m);
	myInvert();
	myGetMatrix(m);
	glMultMatrixf(m);
	
	/*glRotatef(-cam.rotate.z, 0.0,0.0,1.0);
	glRotatef(-cam.rotate.x, 1.0,0.0,0.0);
	glRotatef(-cam.rotate.y, 0.0,1.0,0.0);
	glTranslatef(-cam.translate.x,-cam.translate.y,-cam.translate.z);*/

 	glPushMatrix();  /*  save the cam */ 
		glLightfv(GL_LIGHT0,GL_POSITION,pos);
		render_by_mcp();
#ifdef DEBUG
		glPushMatrix();
			glRotatef(90,1.0,0.0,0.0);	
			glMaterialfv(GL_FRONT,GL_AMBIENT,wire_amb);
			glMaterialfv(GL_FRONT,GL_SPECULAR,wire_spec);
			glMaterialfv(GL_FRONT,GL_DIFFUSE,wire_diff);
			glutWireTorus(100,100,40,40);
		glPopMatrix();
#endif
 	glPopMatrix();  /*  restore the cam */ 
	glLoadIdentity();
	glMultMatrixf(m);

	glFlush();
	switch (frame_mode)
	{
#ifdef G_GLUT
		case G_GLUT:glutSwapBuffers();break;
#endif
#ifdef G_SDL
    	case G_SDL:SDL_GL_SwapBuffers();break;
#endif
#ifdef G_GLX
		case G_GLX:...
#endif
	}
}

/*  quit the graphic-interface */
int graphics_quit()
{
	return(0);
}
