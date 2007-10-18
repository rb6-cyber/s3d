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


#include "global.h"    /*  contains the prototypes of all modules */
#include <time.h>   /* nanosleep() */
#include <stdlib.h>   /* exit() */
#include <unistd.h>   /* sleep(), fork() */
#ifdef G_GLUT
#include <GL/glut.h>   /*  glutMainLoop() */
#endif
#define  X_RES 800
#define  Y_RES 600
#include <getopt.h>   /*  getopt() */
#include <string.h>   /*  strcmp() */
#ifdef SIGS
#include <signal.h>   /*  signal() */
#endif
#include <errno.h>   /*  errno() */
int frame_mode = 0;
static int kidpid = 0;
static int norc = 0;
int running;
static char *rc = NULL;
static char *homerc = "~/.s3drc";
static char *etcrc = "/etc/s3drc";
/*static int father_done=0;*/
extern int aa_level;
static char **s3drc[] = {&rc, &homerc, &etcrc};

static void mainloop(void);
#ifdef SIGS
/*  handles the SIGINT command. maybe put signals in a special file? */
static void sigint_handler(int S3DUNUSED(sig))
{
	s3dprintf(HIGH, "oh my gosh there is a sigint/term signal! running away ...");
	quit();
}
static void sigchld_handler(int S3DUNUSED(sig))
{
	if (kidpid != 0) {
		kidpid = 0;
		s3dprintf(HIGH, "how cruel, my kid died!!");
		quit();
	}
}
#endif
static void sigusr_handler(int S3DUNUSED(sig))
{
	s3dprintf(HIGH, "father told use he's done, so lets start to think about the rc file ...");
	running = 1;
}

int rc_init(void)
{
#ifdef SIGS
	int ret, i;
	struct timespec t = {
		0, 10*1000*1000
	}; /* 10 mili seconds */
	kidpid = fork();
	if (kidpid == -1) {
		errsf("rc_init()", "*sobsob*, can't fork");
		exit(1);
	}
	if (kidpid == 0) {
		if (signal(SIGUSR1, sigusr_handler) == SIG_ERR)
			errn("init():signal()", errno);

		/* giving the father lots of time to set his signal handler
		 * and all his sockets up */
		while (!running)
			nanosleep(&t, NULL);
		for (i = 0 ; i < ((int)(sizeof(s3drc) / sizeof(char **))) ; i++) {
			if ((*s3drc[i]) != NULL) {
				s3dprintf(LOW, "[RC] launching %s", *s3drc[i]);
				ret = system(*s3drc[i]);
				s3dprintf(VLOW, "[RC] system() said %d", ret);
				if (ret < 128) {
					s3dprintf(LOW, "V[RC] system() did well, I guess. let's die clean now.");
					exit(0);
				}
			}
		}
		errs("rc_init()", "no usuable rc script found.");
		if (rc == NULL) {
			errs("rc_init()", "You don't have an rc-script? Think about creating one (~/.s3drc), its handy :)");
			errs("rc_init()", "Starting anyway ...");
			while (1) sleep(1);
		} else {
			errs("rc_init()", "no usuable rc script found.");
			errs("rc_init()", "Check your rc-script!");
		}
		exit(1);
	} else {
		if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
		}
		/* father just returns */
	}
#endif
	return(0);
}
/*  the mainloop, should be handling all signals */
static void mainloop(void)
{
	while (running) {
		one_time();
	}
}

/*  things which should be done each time in main loop go here! this is */
/*  just for the case we use a function for the mainloop like we do for glut... */

static struct timespec t = {
	0, 10*1000*1000
}; /* 10 mili seconds */
void one_time(void)
{
	nanosleep(&t, NULL);
	user_main();
	network_main();
	graphics_main();
}
/*  this initalizes all components.  */
int init(void)
{
#ifdef __APPLE__
	NSApplicationLoad();
#endif
#ifdef SIGS
	if (!norc)
		rc_init();
#else
	s3dprintf(VHIGH, "rc-files won't work without signals :(");
#endif
	if (!frame_mode) { /*  turn default frame_mode on */
#ifdef G_SDL
		frame_mode = FRAME_SDL;
#else
#ifdef G_GLUT
		frame_mode = FRAME_GLUT;
#endif
#endif
	}
	if (!frame_mode) {
		errsf("init()", "no framework mode available");
		return(-1);
	}
	graphics_init();
	network_init();
	user_init();
	process_init();
	running = 1;
#ifdef SIGS
	if (signal(SIGINT, sigint_handler) == SIG_ERR)
		errn("init():signal()", errno);
	if (signal(SIGTERM, sigint_handler) == SIG_ERR)
		errn("init():signal()", errno);
	if (kidpid != 0)
		kill(kidpid, SIGUSR1);
#endif
	return(0);
}

/*  things to be cleaned up  */
void quit(void)
{
	if (running != 0) {
		user_quit();
		network_quit();
		graphics_quit();
		process_quit();
		if (kidpid != 0) { /* our kid is most probably still alive. kill it!! */
			s3dprintf(HIGH, "kill all the kids!!");
			kill(kidpid, SIGTERM);
			kidpid = 0;
		}
	}
	running = 0;
	s3dprintf(VHIGH, "byebye, s3d quitting ...");
	exit(0);
}
/*  processing arguments from the commandline */
static int process_args(int argc, char **argv)
{
	int      lopt_idx;
	char     c;
	struct option long_options[] = {
		{"multisample",  1, NULL, 'm'
		}, {"rc",    1, NULL, 'r'}, {"help",   0, NULL, 'h'}, {"use-glut",  0, NULL, 'g'}, {"use-sdl",   0, NULL, 's'}, {"no-rc",   0, NULL, 'n'}, {NULL, 0, NULL, 0}
	};
	while (-1 != (c = getopt_long(argc, argv, "?hgsnr:m:", long_options, &lopt_idx))) {
		switch (c) {
		case 0:
			break;
		case 'g':
#ifdef G_GLUT
			frame_mode = FRAME_GLUT;
#else
			errsf("process_args()", "sorry, GLUT is not available");
#endif
			break;
		case 's':
#ifdef G_SDL
			frame_mode = FRAME_SDL;
#else
			errsf("process_args()", "sorry, SDL is not available");
#endif
			break;
		case 'r':
			s3dprintf(VHIGH, "using rc file: %s", optarg);
			rc = optarg;
			break;
		case 'm':
			aa_level = atoi(optarg);
			if (aa_level >= 0 || aa_level <= 16)
				s3dprintf(VHIGH, "aa_level: %d", aa_level);
			else
				errsf("process_args()", "bad multisampling level");
			break;
		case 'n':
			s3dprintf(VHIGH, "Using no rc file!");
			norc = 1;
			break;

		case '?':
		case 'h':
			s3dprintf(VHIGH, "usage: %s [options]", argv[0]);
			s3dprintf(VHIGH, "s3d, the 3d server:");
			s3dprintf(VHIGH, " --multisample, -m:\tSpecify Multisampling level (antialiasing) if available.\n\t\t(value 1-16, default 4, 0 = off),");
			s3dprintf(VHIGH, " --rc, -r:\tspecify a rc (startup script)");
			s3dprintf(VHIGH, " --no-rc, -n:\tdon't use a rc file (useful for debugging mcp's)");
#ifdef G_GLUT
			s3dprintf(VHIGH, " --use-glut, -g:\tuse GLUT as framework-system");
#endif
#ifdef G_SDL
			s3dprintf(VHIGH, " --use-sdl, -s:\tuse SDL as framework-system");
#endif
			s3dprintf(VHIGH, " --help, -?, -h: this helpful text");
			errsf("process_args()", "exiting for users sake");
			return(-1);
		}
	}
	return(0);
}
/*  things to be done when program is started */
int main(int argc, char **argv)
{
	process_args(argc, argv);
	init();
	switch (frame_mode) {
#ifdef G_GLUT
	case FRAME_GLUT:
		glutMainLoop();
		break;
#endif
	default:
		mainloop();
	}
	quit();
	return(0);
}
