/*
 * main.c
 *
 * Copyright (C) 2004-2008 Simon Wunderlich <dotslash@packetmixer.de>
 * Copyright (C) 2002 Alexander Graf <helly@gmx.net>
 *
 * This file is part of s3dvt, a 3d terminal emulator for s3d.
 * See http://s3d.berlios.de/ for more updates.
 *
 * s3dvt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * s3dvt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with s3dvt; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "s3dvt.h"
#include <stdio.h>    /*  FILE,NULL */
#include <unistd.h>   /*  read(),write(), sleep(),close() ... */
#include <errno.h>   /*  errno() */
#include <fcntl.h>    /*  open() */
#include <signal.h>   /*  signal() */
#include <stdlib.h>   /*  exit(),getenv(),setenv() */
#include <sys/ioctl.h>   /*  ioctl() */
#include <pthread.h>  /*  pthread_create() */
#include <s3d.h>   /*  s3d_* */
#include <s3d_keysym.h>  /*  key symbols */
#include <time.h> /* nanosleep() */
static struct timespec t = {
	0, 10*1000*1000
}; /* 10 mili seconds */


static int pid;
static int term_mode = 0;

static int cursor;  /* the object id of the cursor */

static pthread_t term_thread;

#ifdef M_PIPE
static int mpipe_in[2];
static int mpipe_out[2];
#endif
#ifdef M_PTY
static int curtty, curpty;
#endif

#ifdef M_CHAR
static unsigned int charbuf[256];
static int screenbuf[MAX_LINES*MAX_CHARS];
static char   last_c[MAX_LINES*MAX_CHARS];
#endif

#ifdef M_LINE
static int    lines[MAX_LINES];
#endif

static void* thread_terminal(void *S3DVTUNUSED(a))
{
	int iscon = 1, ret;
	char buffer[1024];
	while (iscon) {
		switch (term_mode) {
		case M_PIPE:
			ret = read(mpipe_out[0], &buffer, 1000);
			break;
		case M_PTY:
			ret = read(curpty, &buffer, 1000);
			break;
		default:
			ret = -1;
		}
		if (ret > 0) {
			buffer[ret] = '\0';
			printf("<<<new data(%d bytes)\n", ret);
			AddChar(buffer);
		} else {
			if (ret == -1) {
				perror("read()");
				printf("Connection to pty lost (ret=%d)\n", ret);
				close(mpipe_in[0]);
				close(mpipe_out[1]);
				/*   close(curpty);  */
				/*   close(curtty);  */
				printf("********CONNECTION TO PTY LOST!*******\n");
				sleep(1);
				s3d_quit();
				exit(0);
			}
		}
	}
	return(NULL); /* huh?! */
}
static int pty_init_terminal(void)
{
	int i;
	char buf[256];
	char tmpstr[1024];
	int curtty;
	int uid = 0, gid = 0;
	char exe[] = "/bin/bash";
	char curchar;

	uid = getuid();
	gid = getgid();
	term_mode = M_PTY;
	for (curchar = 'p'; curchar < 'z';curchar++) {
		for (i = 0;i < 16;i++) {
			sprintf(buf, "/dev/pty%c%x", curchar, i);
			curpty = open(buf, O_RDWR);
			if (curpty >= 0)
				goto endloop;
		}
	}
endloop:
	if (curpty < 0) {
		printf("Error opening pty\n");
		return 0;
	}
	/*  fnctl(F_SETFL,O_NONBLOCK); */
	signal(SIGCHLD, SIG_IGN);
	pid = fork();
	if (!pid) {
		buf[5] = 't';
		curtty = open(buf, O_RDWR);
		if (curtty < 0) {
			printf("Error opening tty\n");
			return 0;
		}
		setuid(uid);
		setgid(gid);
		if (setsid() < 0)
			printf("ERROR (setsid)\n");
		/*     tcflush(curpty, TCIOFLUSH); */
#ifdef TIOCSCTTY
		if (ioctl(curtty, TIOCSCTTY, NULL))
			printf("ERROR! (ttyflush)\n");;
#endif /* TIOCSCTTY */
		dup2(curtty, 0);
		dup2(curtty, 1);
		dup2(curtty, 2);
		sprintf(tmpstr, "%d", MAX_LINES - 1);
		setenv("LINES", tmpstr, 1);
		sprintf(tmpstr, "%d", MAX_CHARS - 1);
		setenv("COLUMNS", tmpstr, 1);
		setenv("TERM", "rxvt", 1);
		execl(exe, exe, NULL);
		sleep(1);
		printf("that's it, exiting");
		close(curtty);
		exit(0);
	} else if (pid < 0) {
		printf("Cant fork()\n");
		exit(0);
	} else {
		pthread_create(&term_thread, NULL, thread_terminal, NULL);
	}
	return 1;
}
/*  terminal.c */
void term_addchar(char toprint)
{
	/*    printf("sending: %.3d\n", toprint);*/
	switch (term_mode) {
	case M_PIPE:
		write(mpipe_in[1], &toprint, 1);
		write(mpipe_out[1], &toprint, 1);
		break;
	case M_PTY:
		write(curpty, &toprint, 1);
		break;
	}
}


static int pipe_init_terminal(void)
{
	int uid = 0, gid = 0;
	const char *exe = "/bin/bash";
	const char *args = "-i";

	term_mode = M_PIPE;
	if ((pipe(mpipe_in) == -1) || (pipe(mpipe_out) == -1)) {
		printf("pipe failed\n");
		return(-1);
	}
	uid = getuid();
	gid = getgid();
	pid = fork();
	if (pid == 0) { /*  the child */
		char tmpstr[1024];
		setuid(uid);
		setgid(gid);
		if (setsid() < 0)
			printf("ERROR (setsid)\n");
		/*     tcflush(curpty, TCIOFLUSH); */
		/*     if(ioctl(curtty, TIOCSCTTY, NULL)) printf("ERROR! (ttyflush)\n");; */
		setvbuf(stdout, (char*)NULL, _IONBF, 0);
		dup2(mpipe_in[0], fileno(stdin));
		dup2(mpipe_out[1], fileno(stdout));
		dup2(mpipe_out[1], fileno(stderr));
		/*  close unneded things: */
		close(mpipe_out[0]);
		close(mpipe_in[1]);
		sprintf(tmpstr, "%d", MAX_LINES - 1);
		setenv("LINES", tmpstr, 1);
		sprintf(tmpstr, "%d", MAX_CHARS - 1);
		setenv("COLUMNS", tmpstr, 1);
		setenv("TERM", "rxvt", 1);
		execl(exe, exe, args, NULL);
		printf("that's it, exiting");
		close(curtty);
		exit(0);
	} else if (pid < 0) {
		printf("Cant fork()\n");
		exit(0);
	} else {
		/*  close unneded things... */
		close(mpipe_in[0]);
		pthread_create(&term_thread, NULL, thread_terminal, NULL);
	}
	return 1;
}
static int init_terminal(void)
{
	int i;
	for (i = 0;i < 5;i++)
		if (pty_init_terminal())  /*  find an open pty. */
			return(0);
	return(pipe_init_terminal());  /*  if not, fallback to pipe mode */
}
static void term_unload(void)
{
	printf("unloading tty!!\n");
	switch (term_mode) {
	case M_PTY:
		write(curpty, "\0", 1);  /*  send an EOF, just in case */
		close(curpty);
		close(curtty);
		/* kill(pid); */
		break;
	case M_PIPE:
		/*  bash should fade with "broken pipe" */
		close(mpipe_in[1]);
		close(mpipe_out[0]);
		break;
	}
}
#ifdef M_CHAR
void paintit(void)
{
	int cline;
	int c;
	unsigned char ch;
	unsigned int ci;
	int i, line_end;

	s3d_translate(cursor, cx*X_RATIO*CS - CS*X_RATIO*MAX_CHARS / 2, -cy*CS + CS*MAX_LINES / 2, 0);
	s3d_scale(cursor, CS);
	for (cline = 0;cline < MAX_LINES;cline++) {
		line_end = 0;
		for (c = 0;c < MAX_CHARS;c++) {
			i = cline * MAX_CHARS + c;    /*  calculate position */
			if (((ch = line[cline].chars[c].character) != last_c[i])) {
				if (screenbuf[i] == -1) {
					screenbuf[i] = s3d_new_object();
					s3d_translate(screenbuf[i], c*X_RATIO*CS - CS*X_RATIO*MAX_CHARS / 2, -cline*CS + CS*MAX_LINES / 2, 0);
					s3d_scale(screenbuf[i], CS);
					s3d_flags_on(screenbuf[i], S3D_OF_VISIBLE);
				}
				if ((ch == 0) || (line_end)) { /*  the new character is zero! delete! */
					line[cline].chars[c].character = 0;
					ci = charbuf[' '];
					s3d_clone_target(screenbuf[i], ci);
					line_end = 1;
					/*      printf("-%03d",ch); */
				} else {
					ci = charbuf[ch];
					/*      printf("!%03d",ch); */
					s3d_clone_target(screenbuf[i], ci);
				}
			} else {
				/*     printf("=%03d",ch); */
			}
			last_c[i] = ch;
		}
		/*   printf("\n"); */
	}
	gotnewdata = 0;
}
#endif
#ifdef M_LINE
void paintit(void)
{
	int cline;
	int oid, c;
	int len;
	int changed;
	char cl[MAX_CHARS];
	for (cline = 0;cline < MAX_LINES;cline++) {
		len = MAX_CHARS;
		changed = 0;
		for (c = MAX_CHARS;c >= 0;c--) {
			cl[c] = line[cline].chars[c].character;
			if (line[cline].chars[c].character != line[cline].chars[c].last_c) {
				changed = c + 1;
				/*    printf("changed [%d/%d] from %d to %d\n",cline,c,
				        line[cline].chars[c].last_c,
				        line[cline].chars[c].character
				        );*/
				line[cline].chars[c].last_c = line[cline].chars[c].character;
			}
			if (cl[c] == 0) len = c;
		}
		if (changed) {
			if (len > 0) {
				printf("[p: line %d [len: %d/changed: %d]\n", cline, len, changed);
				oid = s3d_draw_string(cl, NULL);
				s3d_translate(oid, 0, -cline, 0);
				if (lines[cline] != -1) {
					s3d_del_object(lines[cline]);
					lines[cline] = -1;
				}
				s3d_flags_on(oid, S3D_OF_VISIBLE);
				lines[cline] = oid;
			} else {
				if (lines[cline] != -1) {
					s3d_del_object(lines[cline]);
					lines[cline] = -1;
				}
			}
		}

	}
	gotnewdata = 0;
}
#endif
/*
void paint_chars()
{
 char c;
 int oid;
 int cline,cchar;
 for(cline=0;cline<MAX_LINES;cline++)
 {
     for(cchar=0;cchar<MAX_CHARS;cchar++)
  {
   c=line[cline].chars[cchar].character;
/ *    printf("%02x|",c); * /
         if(!islastchar(cline,cchar))
          if(line[cline].chars[cchar].character && line[cline].chars[cchar].character != ' ')
    {
/ *     if (screenbuf[MAX_LINES*MAX_CHARS]!=-1) s3d_del_object(screenbuf[MAX_LINES*MAX_CHARS]); * /
    oid=s3d_clone(cobjs[c]);
    s3d_translate(oid,10*cchar,10*cline,0);
    s3d_flags_on(oid,S3D_OF_VISIBLE);
    screenbuf[MAX_LINES*MAX_CHARS]=oid;
    }
     }
/ *   printf("\n"); * /
 }
}*/
static int keypress(struct s3d_evt *event)
{
	struct s3d_key_event *keys = (struct s3d_key_event *)event->buf;
	int key;
	/* printf("received key: %d\n",key);*/
	switch (key = keys->keysym) {
	case S3DK_F1:
	case S3DK_F2:
	case S3DK_F3:
	case S3DK_F4:
	case S3DK_F5:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('1');
		term_addchar(key - S3DK_F1 + '1');
		break;
	case S3DK_F6:
	case S3DK_F7:
	case S3DK_F8:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('1');
		term_addchar((key - S3DK_F6) + '7');
		break;
	case S3DK_F9:
	case S3DK_F10:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('2');
		term_addchar((key - S3DK_F9) + '0');
		break;
	case S3DK_F11:
	case S3DK_F12:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('2');
		term_addchar((key - S3DK_F11) + '3');
		break;
	case S3DK_UP:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('A');
		break;
	case S3DK_DOWN:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('B');
		break;
	case S3DK_RIGHT:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('C');
		break;
	case S3DK_LEFT:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('D');
		break;
	case S3DK_PAGEUP:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('5');
		term_addchar('~');
		break;
	case S3DK_PAGEDOWN:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('6');
		term_addchar('~');
		break;
	case S3DK_HOME:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('7');
		term_addchar('~');
		break;
	case S3DK_END:
		term_addchar(0x1b);
		term_addchar('[');
		term_addchar('8');
		term_addchar('~');
		break;
	case 13:
		term_addchar(10);
		break;
	default:
		if ((char)keys->unicode)  /*  \0 is no good idea .. */
			term_addchar((char)keys->unicode);
	}
	return(0);

}
static int i = 0;
static void mainloop(void)
{
	usleep(10000);
	nanosleep(&t, NULL);

	if ((i += 2) > 100) {
		if (i % 2) {
			s3d_flags_on(cursor, S3D_OF_VISIBLE);
			i = 0;
		} else {
			s3d_flags_off(cursor, S3D_OF_VISIBLE);
			i = 1;
		}
	}
	if (gotnewdata) {
		/*   printf("got new data,displaying\n"); */
		paintit();
	}
}
static int stop(struct s3d_evt *S3DVTUNUSED(event))
{
	s3d_quit();
	return(0);
}
static unsigned int draw_background(void)
{
	unsigned int b;
	b = s3d_new_object();
	s3d_push_vertex(b, -MAX_CHARS / 2*X_RATIO*CS , CS + CS*MAX_LINES / 2  , -0.01);
	s3d_push_vertex(b, MAX_CHARS / 2*X_RATIO*CS , CS + CS*MAX_LINES / 2  , -0.01);
	s3d_push_vertex(b, MAX_CHARS / 2*X_RATIO*CS , -CS*MAX_LINES / 2  , -0.01);
	s3d_push_vertex(b, -MAX_CHARS / 2*X_RATIO*CS , -CS*MAX_LINES / 2  , -0.01);
	s3d_push_material_a(b, 0.5, 0.5, 0.5, 0.7,
	                    1, 1, 1, 0.7,
	                    0, 0, 0, 0.7);
	s3d_push_polygon(b, 1, 2, 0, 0);
	s3d_push_polygon(b, 2, 3, 0, 0);
	s3d_flags_on(b, S3D_OF_VISIBLE);
	return(b);
}
static void chars_s3d_init(void)
{
#ifdef M_CHAR
	char c[2];
	c[1] = '\0';
	for (i = 0;i < 128;i++) {
		c[0] = i;
		charbuf[i] = s3d_draw_string(c, NULL);
	}
	for (i = 128;i < 256;i++) {
		charbuf[i] = s3d_new_object();
	}
	cursor = s3d_new_object();
	s3d_clone_target(cursor, charbuf['_']);
#endif
}
static void chars_init(void)
{
#ifdef M_CHAR
	int x, y;
	for (y = 0;y < (MAX_LINES);y++)
		for (x = 0;x < (MAX_CHARS);x++) {
			line[y].chars[x].character = line[y].chars[x].character = 0;
			i = y * MAX_CHARS + x;
			screenbuf[i] = -1;
			last_c[i] = 0;
		}
#endif
#ifdef M_LINE
	int i;
	for (i = 0;i < MAX_LINES;i++)
		lines[i] = -1;
#endif
}
int main(int argc, char **argv)
{
	chars_init();
	init_terminal();
	s3d_set_callback(S3D_EVENT_QUIT, stop);
	s3d_set_callback(S3D_EVENT_OBJ_CLICK, stop);
	s3d_set_callback(S3D_EVENT_KEY, keypress);

	if (!s3d_init(&argc, &argv, "s3dvt")) {
		if (s3d_select_font("vera")) {
			printf("font not found\n");
			exit(-1);
		}
		draw_background();
		chars_s3d_init();
		s3d_mainloop(mainloop);
		s3d_quit();
	}
	term_unload();
	return(0);
}
