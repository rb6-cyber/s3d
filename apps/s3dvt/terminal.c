/*
 * terminal.c
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
#include <stdio.h>    /*  NULL,perror() */
#include <string.h>   /*  memcpy() */
#include <stdlib.h>   /*  atoi() */
#define MOVE_RIGHT  1
#define MOVE_LEFT  2
#define MOVE_UP   3
#define MOVE_DOWN  4


int cx = 0, cy = 0;

int gotnewdata = 1;
t_line line[MAX_LINES+1];
static int isansi = 0;
static int isansi2 = 0;

static int bottom = MAX_LINES - 1;
static int top = 0;

static int curfgcolor = DEFAULT_FGCOLOR;
static int curbgcolor = DEFAULT_BGCOLOR;

static void move_all_lines_up(void)
{
	t_line *pfirstline = (t_line*) & line;
	t_line *psecondline = (t_line*) & line + 1;
	t_line tmpline[MAX_LINES];
	memcpy(&tmpline, psecondline, (MAX_LINES - 1)*sizeof(struct line_struct));
	memcpy(pfirstline, &tmpline, (MAX_LINES - 1)*sizeof(struct line_struct));
}

static void clear_char(int lineid, int charid)
{
	line[lineid].chars[charid].character = 0;
	line[lineid].chars[charid].fgcolor = DEFAULT_FGCOLOR;
	line[lineid].chars[charid].bgcolor = DEFAULT_BGCOLOR;
}

static void clear_line(int lineid)
{
	int i;
	for (i = 0;i < MAX_CHARS;i++)
		clear_char(lineid, i);
}

static void clear_line_after_lastchar(void)
{
	int i;
	for (i = cx;i < MAX_CHARS;i++)
		clear_char(cy, i);
}

static t_line* line_is_full(void)
{
	t_line *pcurline;
	cy++;
	if (cy >= MAX_LINES) {     /*  damn it ... our display is filled ... let's move everything upwards */
		cy = MAX_LINES - 1;
		pcurline = (t_line*) & line + cy;
		move_all_lines_up();
		clear_line(cy);
		gotnewdata = 1;
	} else {
		pcurline = (t_line*) & line + cy;
	}
	cx = 0;
	return pcurline;
}

static void add_char_append(char toappend)
{
	int shouldinc = 1;
	t_line *pcurline = (t_line*) & line + cy;
	if (cx == MAX_CHARS - 1) {
		pcurline = line_is_full();
		shouldinc = 0;
	}  /*  our line is full */
	pcurline->chars[cx].character = toappend;
	pcurline->chars[cx].fgcolor = curfgcolor;
	pcurline->chars[cx].bgcolor = curbgcolor;
	if (shouldinc)
		cx++;
}

static void backspace(void)
{
	if (cx > 0)
		cx--;
	else
		cx = 0;
}

static void endansi(void)
{
	printf(" [/ANSI(%d)]\n", isansi2);
	isansi = 0;
	isansi2 = 0;
}

/*
              Parameter                              Parameter Meaning

       0                                      Attributes off
       1                                      Bold or increased intensity
       4                                      Underscore
       5                                      Blink
       7                                      Negative (reverse) image
*/

static void ansi_change_graphic(char **args)
{
	int curcol;
	int i;

	if (args[0][0] == '\0') {
		args[0][0] = '0';
		args[0][1] = '\0';
	}

	for (i = 0;i < 5;i++) {
		if (args[i][0]) {
			curcol = atoi(args[i]);

			switch (curcol) {
			case 0:
				curbgcolor = DEFAULT_BGCOLOR;
				curfgcolor = DEFAULT_FGCOLOR;
				break;
			case 1:  /*  Bold or increased intensity */
			case 4:  /*  Underscore */
			case 5:  /*  Blink */
			case 7:  /*  Negative (reverse) image */
			case 10:  /*  primary font */
			case 11:  /*  alternate font */
				break;
			case 30:
			case 31:
			case 32:
			case 33:
			case 34:
			case 35:
			case 36:
			case 37:
				curfgcolor = curcol - 30;
				break;
			case 39:
				curbgcolor = DEFAULT_FGCOLOR;
				break;
			case 40:
			case 41:
			case 42:
			case 43:
			case 44:
			case 45:
			case 46:
			case 47:
				curbgcolor = curcol - 40;
				break;

			case 49:
				curbgcolor = DEFAULT_BGCOLOR;
				break;

			default:
				printf("*** don't know color-code %d\n", curcol);
				break;
			}
		}
	}
}
static void move_up_x_lines(char *arg)
{
	t_line *pfirstline;
	t_line *psecondline;
	t_line tmpline[MAX_LINES];
	int amount;
	int i;

	if (arg[0]) amount = atoi(arg);
	else amount = 0;

	printf("moving up %d lines", amount);

	for (i = 0;i < amount;i++) {

		pfirstline = (t_line*) & line + cy;
		psecondline = pfirstline + 1;

		memcpy(&tmpline, psecondline, (bottom - top)*sizeof(struct line_struct));
		memcpy(pfirstline, &tmpline, (bottom - top)*sizeof(struct line_struct));

	}
}

static void move_down_x_lines(char *arg)
{
	t_line *pfirstline;
	t_line *psecondline;
	t_line tmpline[MAX_LINES];
	int amount;
	int i;

	if (arg[0]) amount = atoi(arg);
	else amount = 0;

	printf("moving down %d lines", amount);

	for (i = 0;i < amount;i++) {
		pfirstline = (t_line*) & line + cy;
		psecondline = pfirstline + 1;

		memcpy(&tmpline, pfirstline, (bottom - top)*sizeof(struct line_struct));
		memcpy(psecondline, &tmpline, (bottom - top)*sizeof(struct line_struct));
	}
}
static void delete_x_letters(char *arg1)
{
	int tmpint;
	int i;
	if (arg1[0]) tmpint = atoi(arg1);
	else     tmpint = 1;

	if (tmpint + cx > MAX_CHARS)
		tmpint = MAX_CHARS - cx;
	for (i = cx;i < cx + tmpint;i++)
		clear_char(cy, i);
}
static void move_x_letters(int mode, char *arg1)
{
	int tmpint;
	int i;
	if (arg1[0])
		tmpint = atoi(arg1);
	else
		tmpint = 1;

	switch (mode) {
	case MOVE_RIGHT:
		for (i = 0;i < tmpint;i++) {
			cx++;
			if (cx == MAX_CHARS)
				line_is_full();  /*  our line is full */
		}
		break;
	case MOVE_LEFT:
		for (i = 0;i < tmpint;i++) {
			cx--;
			if (cx == -1) {
				cy--;
				cx = MAX_CHARS - 1;
			}  /*  need to go up one line */
		}
		break;
	case MOVE_UP:
		cy--;
		break;
	case MOVE_DOWN:
		line_is_full();
		break;
	default:
		break;
	}
}
static void remove_beginning_from_curpos(void)
{
	int i, j = cx;
	for (i = cy;i < MAX_LINES;i++) {
		for (;j < MAX_CHARS;j++) {
			clear_char(i, j);
		}
		j = 0;
	}
}

static int parseansi(char curchar)
{
	static char arg1[16] = "";
	static char arg2[16] = "";
	static char arg3[16] = "";
	static char arg4[16] = "";
	static char arg5[16] = "";
	static char curindex = 0;
	static char curarg = 0;
	static char *args[] = {arg1, arg2, arg3, arg4, arg5};
	/*  static char *args[]={&arg1,&arg2,&arg3,&arg4,&arg5}; */
	printf("%c", curchar);
	switch (isansi2) {
	case 1:
		switch (curchar) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':  /*  we got an argument */
			args[(int)curarg][(int)curindex] = curchar;
			curindex++;
			args[(int)curarg][(int)curindex] = '\0';
			break;
		case ';':  /*  some arg is finished */
			curarg++;
			curindex = 0;
			args[(int)curarg][0] = '\0';
			break;
		case 'J':  /*  remove beginning from current cursor to end of screen */
			remove_beginning_from_curpos();
			gotnewdata = 1;
			break;
		case 'K':  /*  remove everything in line beginning from lastchar */
			clear_line_after_lastchar();
			break;
		case 'H':  /*  move to position x=arg1 y=arg2 */
			if (arg1[0]) cy = atoi(arg1) - 1;
			else  cy = 0;
			if (arg2[0]) cx = atoi(arg2) - 1;
			else  cx = 0;
			break;
		case 'G':  /*  move to position x=arg1 y=MAX */
			if (arg1[0]) cx = atoi(arg1) - 1;
			else cx = -1;
			cy = bottom - 1;
			break;
		case 'd':  /*  move to position x=MAX y=arg1 */
			if (arg1[0]) cy = atoi(arg1) - 1;
			else cy = 0;
			cy = top + cy;
			cx = MAX_CHARS - 1;
			break;
		case 'm':  /*  change graphic */
			ansi_change_graphic(args);
			break;
		case 'M':  /*  Move memory in range ('r') one uo */
			move_up_x_lines(arg1);
			break;
		case 'L':  /*  Move memory in range ('r') one down */
			move_down_x_lines(arg1);
			break;
		case 'l':
			/*  RM -- Reset Mode

			  ESC [ Ps ; Ps ; . . . ; Ps l                                                                                                          default value: none

			  Resets one or more VT100 modes as specified by each selective parameter in the parameter string. Each mode to be reset is specified by a separate
			  parameter. [See Set Mode (SM) control sequence]. (See Modes following this section).*/

			/*  mc only resets the '4' !?! => IRM (Insert/Replacement-Mode) */
			/*  perhaps Set Cursor to Block mode ? */
			break;
		case 'r':  /*  define scroll-range  */
			if (arg1[0]) top = atoi(arg1);
			else top = 0;
			if (arg2[0]) bottom = atoi(arg2);
			else bottom = 0;
			cy = 0;
			cx = 0;
			break;
		case 'a':
		case 'A':  /*  move x letters up */
			move_x_letters(MOVE_UP, arg1);
			break;
		case 'B':  /*  move x letters down */
			move_x_letters(MOVE_DOWN, arg1);
			break;
		case 'C':  /*  move x letters right */
			move_x_letters(MOVE_RIGHT, arg1);
			break;
		case 'D':  /*  move x letters left */
			move_x_letters(MOVE_LEFT, arg1);
			break;
		case 'P':  /*  delete x letters */
			delete_x_letters(arg1);
			break;
		case '?':
			isansi2 = 4;
			return 0;
		default:
			printf("***unknown***");
		}
		if (!((curchar >= '0' && curchar <= '9') || curchar == ';')) { /*  clean our args */
			args[0][0] = '\0';
			args[1][0] = '\0';
			args[2][0] = '\0';
			args[3][0] = '\0';
			args[4][0] = '\0';
			curindex = 0;
			curarg = 0;
			endansi();
		}
		break;
	case 0:
		switch (curchar) {
		case '[':
			isansi2 = 1;
			break;
		case '(':
			isansi2 = 2;
			break;
		case ')':
			isansi2 = 3;
			break;
		case ']':
			isansi2 = 5;
			break;
		default:
			endansi();
			break;
		}
		break;
	case 4:
		if (curchar > '9' || curchar < '0')
			endansi();
		break;
	case 5:
		if (curchar == 7) endansi(); /* FIXME: Window Title*/
		break;
	default:
		endansi();
		break;
	}
	return(0);
}

void AddChar(char *_toadd)
{
	char *toadd;
	char curchar;

	for (toadd = _toadd;toadd[0];toadd++) {
		curchar = toadd[0];
		/* printf("%.3d (", curchar);*/
		if (isansi)
			parseansi(curchar);
		else {
			switch (curchar) {
			case 7:
				printf("<BEEP>\n");
				break;
			case 8:
				printf("<BS>\n");
				backspace();
				break;
			case 27:
				/*  ANSI */
				printf("<ESC>[ANSI] ");
				isansi = 1;
				break;
			case 10:
				printf("<LF>\n");/* get onto the next line */
				line_is_full();
				break;
			case 13:
				printf("<CR>\n");/* carriage return, get back */
				if (cx == MAX_CHARS - 1)
					line_is_full();
				cx = 0;
				break;
			default:
				if ((curchar >= 32) && (curchar < 127)) {
					/*     printf("%c", curchar);*/
					add_char_append(curchar);
				}
				break;
			}
			/* printf(")\n");*/
		}
	}
	gotnewdata = 1;
}
