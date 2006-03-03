/*
 * main.c
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


#include "global.h" 		 /*  contains the prototypes of all modules */
#include <unistd.h> 	 /*  usleep() */
#include <stdlib.h>		 /*  exit() */
#ifdef G_GLUT
#include <GL/glut.h> 	 /*  glutMainLoop() */
#endif
#define		X_RES	800
#define		Y_RES	600
#include <getopt.h>		 /*  getopt() */
#include <string.h>		 /*  strcmp() */
#ifdef SIGS
#include <signal.h>		 /*  signal() */
#endif
#include <errno.h>		 /*  errno() */
int frame_mode=0;
int running;

static void mainloop(void);
#ifdef SIGS
/*  handles the SIGINT command. maybe put signals in a special file? */
void sigint_handler(int sig)
{
	dprintf(HIGH,"oh my gosh there is a sigint/term signal! running away ...");
	quit();
}
#endif
/*  the mainloop, should be handling all signals */
static void mainloop(void) 
{
	while (running)
	{
		one_time();
	}
}

/*  things which should be done each time in main loop go here! this is */
/*  just for the case we use a function for the mainloop like we do for glut... */
void one_time() 
{
	usleep(10000);
	user_main();
	network_main(); 
	graphics_main();
}
/*  this initalizes all components.  */
int init() 
{
	if (!frame_mode)  /*  turn default frame_mode on */
	{
#ifdef G_GLUT
		frame_mode=FRAME_GLUT;
#else
#ifdef G_SDL
		frame_mode=FRAME_SDL;
#endif
#endif
	}
	if (!frame_mode)
	{
		errsf("init()","no framework mode available");
		return(-1);
	}
	graphics_init();
	network_init();
	user_init();
	process_init();
	running=1;
#ifdef SIGS
    if (signal(SIGINT, sigint_handler) == SIG_ERR) 
	        errn("network_init():signal()",errno);
    if (signal(SIGTERM, sigint_handler) == SIG_ERR) 
	        errn("network_init():signal()",errno);
#endif
	return(0);
}

/*  things to be cleaned up  */
int quit()
{
	user_quit();
	network_quit();
	graphics_quit();
	process_quit();
	running=0;
	dprintf(VHIGH,"byebye, s3d quitting ...");
	exit(0);
	return(0);
}
/*  processing arguments from the commandline */
int process_args(int argc, char **argv)
{
	int					 lopt_idx;
	char				 c;
	struct option long_options[] = 
	{
		{"help",0,0,'h'},
		{"use-glut",0,0,'g'},
		{"use-sdl",0,0,'s'},
		{0,0,0,0}
	};
	while (-1!=(c=getopt_long(argc,argv,"?hgs",long_options,&lopt_idx)))
	{
		switch (c)
		{
				case 0:break;
				case 'g':
#ifdef G_GLUT
					frame_mode=FRAME_GLUT;
#else
					errs("process_args()","sorry, GLUT is not available");
#endif
					break;
				case 's':
#ifdef G_SDL
					frame_mode=FRAME_SDL;
#else
					errs("process_args()","sorry, SDL is not available");
#endif					
					break;
				case '?':
				case 'h':
					dprintf(VHIGH,"usage: %s [options]",argv[0]);
					dprintf(VHIGH,"s3d, the 3d server:");
#ifdef G_GLUT
					dprintf(VHIGH," --use-glut, -g:\tuse GLUT as framework-system");
#endif
#ifdef G_SDL
					dprintf(VHIGH," --use-sdl, -s:\tuse SDL as framework-system");
#endif
					dprintf(VHIGH," --help, -?, -h: this helpful text");
					errsf("process_args()","exiting for users sake");
					return(-1);
		}
	}
	return(0);
}
/*  things to be done when program is started */
int main(int argc, char **argv) {
	process_args(argc,argv);
	init();
	switch (frame_mode)
	{
#ifdef G_GLUT
		case FRAME_GLUT:glutMainLoop();break;
#endif
		default:mainloop();
	}
	quit();
	return(0);
}
