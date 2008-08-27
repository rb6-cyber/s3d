/*
 * matrix.c
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

/*  this file gives some simple matrix functionality for things I was unable */
/*  to do with OpenGL */
#include "global.h"
#include <string.h>  /*  memcpy() */

#define DEG2RAD (M_PI/180.0)
static t_mtrx MAT;
t_mtrx Identity = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};
void myLoadIdentity(void)
{
	memcpy(MAT, Identity, sizeof(t_mtrx));
}
#define I(x, y)  x*4+y
#define M(x, y)  MAT[I(x, y)]
static void mat_debug(t_mtrx S)
{
	s3dprintf(MED, "MAT_0: %.2f %.2f %.2f %.2f", S[I(0,0)], S[I(1,0)], S[I(2,0)], S[I(3,0)]);
	s3dprintf(MED, "MAT_1: %.2f %.2f %.2f %.2f", S[I(0,1)], S[I(1,1)], S[I(2,1)], S[I(3,1)]);
	s3dprintf(MED, "MAT_2: %.2f %.2f %.2f %.2f", S[I(0,2)], S[I(1,2)], S[I(2,2)], S[I(3,2)]);
	s3dprintf(MED, "MAT_3: %.2f %.2f %.2f %.2f", S[I(0,3)], S[I(1,3)], S[I(2,3)], S[I(3,3)]);
}
void myMultMatrix(t_mtrx mat2)
{
	int i, j, k;
	t_mtrx mat_d;  /*  destination matrix */
	for (i = 0;i < 4;i++)
		for (j = 0;j < 4;j++) {
			mat_d[I(i,j)] = 0.0F;
			for (k = 0;k < 4;k++)
				mat_d[I(i,j)] += M(k, j) * mat2[I(i,k)];
		}
	memcpy(MAT, mat_d, sizeof(t_mtrx));
}
void myGetMatrix(t_mtrx mat)
{
	memcpy(mat, MAT, sizeof(t_mtrx));
}
void mySetMatrix(t_mtrx mat)
{
	memcpy(MAT, mat, sizeof(t_mtrx));
}
void myTransform4f(float *v)
{
	float w[4];
	w[0] = v[0] * M(0, 0) + v[1] * M(1, 0) + v[2] * M(2, 0) + v[3] * M(3, 0);
	w[1] = v[0] * M(0, 1) + v[1] * M(1, 1) + v[2] * M(2, 1) + v[3] * M(3, 1);
	w[2] = v[0] * M(0, 2) + v[1] * M(1, 2) + v[2] * M(2, 2) + v[3] * M(3, 2);
	w[3] = v[0] * M(0, 3) + v[1] * M(1, 3) + v[2] * M(2, 3) + v[3] * M(3, 3);
	memcpy(v, w, sizeof(w));
}
void myTransform3f(float *v)
{
	float w[3];
	w[0] = v[0] * M(0, 0) + v[1] * M(1, 0) + v[2] * M(2, 0) + 1.0F * M(3, 0);
	w[1] = v[0] * M(0, 1) + v[1] * M(1, 1) + v[2] * M(2, 1) + 1.0F * M(3, 1);
	w[2] = v[0] * M(0, 2) + v[1] * M(1, 2) + v[2] * M(2, 2) + 1.0F * M(3, 2);
	memcpy(v, w, sizeof(w));
}
void myTransformV(struct t_vertex *v)
{
	struct t_vertex w;
	w.x = v->x * M(0, 0) + v->y * M(1, 0) + v->z * M(2, 0) + 1.0F * M(3, 0);
	w.y = v->x * M(0, 1) + v->y * M(1, 1) + v->z * M(2, 1) + 1.0F * M(3, 1);
	w.z = v->x * M(0, 2) + v->y * M(1, 2) + v->z * M(2, 2) + 1.0F * M(3, 2);
	memcpy(v, &w, sizeof(struct t_vertex));
}

#undef M
#define M(x, y)  Mm[I(x, y)]
#define P(x, y)  Pm[I(x, y)]
/* this inverts the matrix M into P in the gauss way */
int myInvert(void)
{
	t_mtrx Mm, Pm;
	int l, lh; /* line*/
	float f; /* factor */
	int i; /* number */
	memcpy(Mm, MAT, sizeof(t_mtrx)); /* backup matrix */
	memcpy(Pm, Identity, sizeof(t_mtrx));  /* target */

	/* s3dprintf(MED,"start:");
	 mat_debug(MAT);*/

	/* s3dprintf(LOW,"inverting matrix, we shall begin now ...");*/

	/* step 1 */
	for (l = 0;l < 4;l++) {
check:
		if (M(l, l)*M(l, l) > 0.00000001F) { /* it won't work with real zero */

			/*   s3dprintf(MED,"normalizing line %d",l);*/
			/* normalize */
			f = 1 / M(l, l);
			M(l, l) = 1.0;
			for (i = l + 1;i < 4;i++)
				M(i, l) *= f; /* the left side ... */
			for (i = 0;i < 4;i++)
				P(i, l) *= f; /* ... and the right */
			/*   mat_debug(Mm);
			   s3dprintf(MED,"-");
			   mat_debug(Pm);*/
			/* mult/fac */
			for (lh = l + 1;lh < 4;lh++) {
				/* s3dprintf(MED,"adding line %d for %d",lh,l);*/
				if (M(l, lh) != 0) { /* "first" element of the line */
					f = -M(l, lh);
					M(l, lh) = 0.0; /* yes, this WILL be zero! ... */
					for (i = l + 1;i < 4;i++) /* left side */
						M(i, lh) += f * M(i, l);
					for (i = 0;i < 4;i++) /* ... and the right one! */
						P(i, lh) += f * P(i, l);
				} /*else s3dprintf(MED,"element already zero!");*/
			}
		} else {
			M(l, l) = 0.0F;
			/*   s3dprintf(MED,"already zero now check and try to swap lines ...");*/
			for (lh = l + 1;lh < 4;lh++)
				if (M(l, lh) != 0.0) {
					/*     s3dprintf(MED,"swapping lines %d and %d",l,lh);*/
					for (i = 0;i < 4;i++) {
						f = M(i, l);
						M(i, l) = M(i, lh);
						M(i, lh) = f;
						f = P(i, l);
						P(i, l) = P(i, lh);
						P(i, lh) = f;

					}
					goto check;
				}
			s3dprintf(MED, "nothing to swap, can't reverse this matrix! returning ... ");
			mat_debug(Mm);
			return(-1); /* the dead end!! */
		}
	}
	/* matrix should look like this now: */
	/* (1???|????)
	 * (01??|????)
	 * (001?|????)
	 * (0001|????)
	 *
	 *  (M = left side, P = right side)
	 * */

	/* step 2 */
	/* s3dprintf(MED,"S.T.E.P. 2!!");*/
	for (l = 3;l > 0;l--) {
		/* mult/fac */
		for (lh = l - 1;lh >= 0;lh--) {
			/*   s3dprintf(MED,"adding line %d for %d",lh,l);*/
			if (M(l, lh) != 0) { /* "first" element of the line */
				f = -M(l, lh);
				M(l, lh) = 0;
				for (i = 0;i < 4;i++) { /* ... and the right one! */
					P(i, lh) += f * P(i, l);
				}

			}
			/*   mat_debug(Mm);
			   mat_debug(Pm);*/
		}
	}
	/* now, Mm,is Identity and Pm is result!*/
	/* s3dprintf(MED,"result:");
	 mat_debug(Pm);*/
	memcpy(MAT, Pm, sizeof(t_mtrx)); /* copy result */
	return(0);
}

