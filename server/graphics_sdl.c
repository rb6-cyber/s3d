/*
 * graphics_sdl.c
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
int aa_level = 4;

#include <SDL_opengl.h>
#include <SDL.h>

int SDLFlags = 0;      /*  some flags for SDL */
int graphics_init_sdl(void)
{
	SDL_Surface *GLwin = NULL;
	SDL_VideoInfo *VideoInfo;
	int buffers, samples;
	s3dprintf(MED, "Using SDL driver ...");

	SDLFlags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE | SDL_RESIZABLE;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
		errsf("SDL_Init()", SDL_GetError());
	if ((VideoInfo = (SDL_VideoInfo *)SDL_GetVideoInfo()) == NULL)
		errs("SDL_GetVIdeoInfo()", SDL_GetError());
	else {
		if (VideoInfo->hw_available) {
			s3dprintf(LOW, "detected HW_SURFACE");
			SDLFlags |= SDL_HWSURFACE;
		} else {
			s3dprintf(LOW, "detected SW_SURFACE");
			SDLFlags |= SDL_SWSURFACE;
		}
		if (VideoInfo->blit_hw)
			SDLFlags |= SDL_HWACCEL;
		/*     if(SDL_WM_ToggleFullScreen(GLwin) == 0)         SDLerror("SDL_WM_ToggleFullScreen"); */
	}


	/*  set some opengl-attributes */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	do {
		if (aa_level > 0) {
			if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1))   s3dprintf(VHIGH, "error initializing multisampling");
			if (SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, aa_level)) s3dprintf(VHIGH, "no multisampling available");
		}

		/*  more opengl-init-stuff */
		if ((GLwin = SDL_SetVideoMode(X_RES, Y_RES, 16, SDLFlags)) == NULL) {
			if (aa_level > 0) {
				s3dprintf(MED, "retry without multisampling");
				aa_level = 0;
				SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
				SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
			} else
				errsf("SDL_SetVideoMode()", SDL_GetError());
		}
	} while (GLwin == NULL);
	if (aa_level > 0) {
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &buffers);
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &samples);
		s3dprintf(LOW, "Buffers: %d Samples: %d", buffers, samples);
	}
	SDL_WM_SetCaption("S3D", "S3D");

	/*  print some information */
	s3dprintf(VLOW, "Screen BPP: %d", SDL_GetVideoSurface()->format->BitsPerPixel);
	s3dprintf(VLOW, "Vendor     : %s", glGetString(GL_VENDOR));
	s3dprintf(VLOW, "Renderer   : %s", glGetString(GL_RENDERER));
	s3dprintf(VLOW, "Version    : %s", glGetString(GL_VERSION));
	s3dprintf(VLOW, "Extensions : %s", glGetString(GL_EXTENSIONS));

	graphics_reshape(X_RES, Y_RES);
	return(0);
}
int graphics_quit_sdl(void)
{
	SDL_Quit();
	return(0);
}
