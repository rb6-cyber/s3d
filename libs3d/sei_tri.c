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
#include <string.h> /* memset() */


static int initialise(int n)
{
	register int i;

	for (i = 1; i <= n; i++)
		seg[i].is_inserted = FALSE;

	generate_random_ordering(n);

	return 0;
}

/* Input specified as contours.
 * Outer contour must be anti-clockwise.
 * All inner contours must be clockwise.
 *
 * Every contour is specified by giving all its points in order. No
 * point shoud be repeated. i.e. if the outer contour is a square,
 * only the four distinct endpoints shopudl be specified in order.
 *
 * ncontours: #contours
 * cntr: An array describing the number of points in each
 *  contour. Thus, cntr[i] = #points in the i'th contour.
 * vertices: Input array of vertices. Vertices for each contour
 *           immediately follow those for previous one. Array location
 *           vertices[0] must NOT be used (i.e. i/p starts from
 *           vertices[1] instead. The output triangles are
 *      specified  w.r.t. the indices of these vertices.
 * triangles: Output array to hold triangles.
 *
 * Enough space must be allocated for all the arrays before calling
 * this routine
 */


int sei_triangulate_polygon(int ncontours, int cntr[], double(*vertices)[2], int (*triangles)[3])
{
	register int i;
	int nmonpoly, ccount, npoints;
	int n;

	memset((void *)seg, 0, sizeof(seg));
	ccount = 0;
	i = 1;

	while (ccount < ncontours) {
		int j;
		int first, last;

		npoints = cntr[ccount];
		first = i;
		last = first + npoints - 1;
		for (j = 0; j < npoints; j++, i++) {
			seg[i].v0.x = vertices[i][0];
			seg[i].v0.y = vertices[i][1];

			if (i == last) {
				seg[i].next = first;
				seg[i].prev = i - 1;
				seg[i-1].v1 = seg[i].v0;
			} else if (i == first) {
				seg[i].next = i + 1;
				seg[i].prev = last;
				seg[last].v1 = seg[i].v0;
			} else {
				seg[i].prev = i - 1;
				seg[i].next = i + 1;
				seg[i-1].v1 = seg[i].v0;
			}

			seg[i].is_inserted = FALSE;
		}

		ccount++;
	}

	n = i - 1;

	initialise(n);
	construct_trapezoids(n);
	nmonpoly = monotonate_trapezoids(n);
	return triangulate_monotone_polygons(n, nmonpoly, triangles);
}

