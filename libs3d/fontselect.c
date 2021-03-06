// SPDX-License-Identifier: LGPL-2.1-or-later
/* SPDX-FileCopyrightText: 2004-2015  Simon Wunderlich <sw@simonwunderlich.de>
 */


/*  here we select our truetype font. I'd recommend the fontconfig way,  */
/*  as it always gives the best matching font. a xft version and a win32 */
/*  version would be nice too, to be implemented :) */
#include "s3d.h"
#include "s3dlib.h"
#include <dirent.h>   /*  dirent */
#include <X11/Xlib.h>  /*  Display type, XOpenDisplay(), XCloseDIsplay etc. */
#ifdef WITH_FONTCONFIG
#include "ft2build.h"
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>
#endif
#include <stdlib.h>    /*  malloc(), free() */
#include <string.h>    /*  strlen(), strncasecmp(), strncmp() */


#ifdef WITH_FONTCONFIG
char *s3d_findfont(const char *mask)
{
	FcPattern *pattern = NULL, *match = NULL;
	FcChar8 *file = NULL;
	FcResult result;

	pattern = FcNameParse((FcChar8 *)mask);
	FcConfigSubstitute(NULL, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);
	s3dprintf(LOW, "Looking for font %s", mask);

	if (!(match = FcFontMatch(NULL, pattern, &result)))
		return NULL;
	if (FcPatternGetString(match, FC_FILE, 0, &file) != FcResultMatch)
		return NULL;
	return (char *)file;
}
#else
/*  this uses the xserver to get a font-path and scan it for ttf-fonts. */
/*  if it matches, give it out ... it's not nice, right, and might not */
/*  work on your place. */
char *s3d_findfont(const char *mask)
{
	char **flist = NULL;
	int fnum = 0;
	char *disp = NULL;
	int n;
	char *fname;
	char *good = NULL;
	struct dirent **namelist;
	Display *dpy;

	dpy = XOpenDisplay(disp);  /*  Open display and check for success */
	if (dpy == NULL)
		errds(VHIGH, "s3d_findfont()", "unable to open display %s", XDisplayName(disp));
	else {
		if (!(flist = XGetFontPath(dpy, &fnum))) {
			errds(VHIGH, "s3d_findfont():XGetFontPath()", "unable to get font path.");
		} else
			while (fnum--) {
				/*  now scan the directories  */
				n =  scandir(flist[fnum], &namelist, 0, alphasort);
				while (n-- > 0) {
					fname = namelist[n]->d_name;
					if (strlen(fname) > (strlen(mask) + 3)) { /*  there should be enough space for the .ttf ending */
						/*  check for the first n characters */
						if (0 == strncasecmp(fname, mask, strlen(mask))) {
							/*  name matches! now check for the end... */
							if (0 == strncasecmp(fname + (strlen(fname) - 3), "ttf", 3)) { /*  check if it has a ttf-ending */
								if (good == NULL)
									good = malloc(256);
								strncpy(good, flist[fnum], 255);
								good[256] = 0;        /* just in case */
								strncat(good, fname, 255 - strlen(good));
								if ((strlen(mask) + 4) == strlen(fname)) {
									return good;
								}
							}
						}
					}
				}
			}
		XCloseDisplay(dpy);
	}
	return good;
}
#endif
