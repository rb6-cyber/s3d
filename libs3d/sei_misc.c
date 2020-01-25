// SPDX-License-Identifier: LGPL-2.1-or-later
/* SPDX-FileCopyrightText: 2006-2015  Simon Wunderlich <sw@simonwunderlich.de>
 * SPDX-FileCopyrightText: 1994  A. Narkhede and D .Manocha, who released their code
 * for public domain:
 * <snip>
 *
 * This code is in the public domain. Specifically, we give to the public
 * domain all rights for future licensing of the source code, all resale
 * rights, and all publishing rights.
 *
 * UNC-CH GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE SOFTWARE
 * AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION, WARRANTY
 * OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 *                                 - Atul Narkhede (narkhede@cs.unc.edu)
 * </snip>
 */


#include "sei_triangulate.h"
#include <sys/time.h>
#include <math.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include "s3d.h"
#include "s3dlib.h"


static int choose_idx;
static int permute[SEGSIZE];
static double mlog2(double x)
{
	return log(x) / log(2);
}

/* Generate a random permutation of the segments 1..n */
int generate_random_ordering(int n)
{
	struct timeval tval;
	register int i;
	int m, st[SEGSIZE], *p;

	choose_idx = 1;
	gettimeofday(&tval, NULL);
	srand48(tval.tv_sec);

	for (i = 0; i <= n; i++)
		st[i] = i;

	p = st;
	for (i = 1; i <= n; i++, p++) {
		m = lrand48() % (n + 1 - i) + 1;
		permute[i] = p[m];
		if (m != 1)
			p[m] = p[1];
	}
	return 0;
}


/* Return the next segment in the generated random ordering of all the */
/* segments in S */
int choose_segment(void)
{
	errds(VLOW, "sei:choose_segment()", "%d", permute[choose_idx]);
	return permute[choose_idx++];
}

/* Get log*n for given n */
int math_logstar_n(int n)
{
	register int i;
	double v;

	for (i = 0, v = (double) n; v >= 1; i++)
		v = mlog2(v);

	return i - 1;
}


int math_N(int n, int h)
{
	register int i;
	double v;

	for (i = 0, v = (int) n; i < h; i++)
		v = mlog2(v);

	return (int) ceil((double) 1.0*n / v);
}
