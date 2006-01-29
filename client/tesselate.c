#include "s3d.h"
#include "s3dlib.h"
static int _s3d_tesssub(struct tessp_t *t, struct t_buf *b,int s);
static int _s3d_addtri(struct tessp_t *t, struct t_buf *b,int p, int i, int n);


static void tessdebug(struct tessp_t *t, struct t_buf *b)
{
	int i,n,p;
	for (i=0;i<b->vn;i++)
	{
		n=t[i].next;
		p=t[i].prev;
		if ((i==n) && (i==p))
			dprintf(HIGH,"[D]ebug %d %d %d [done]",p,i,n);
		else
			dprintf(HIGH,"[D]ebug %d %d %d %3.3f %3.3f",p,i,n,b->vbuf[i*3]*10,b->vbuf[i*3+1]*10);
	}
}

#define POINT_DIR(p,i,n) 	((b->vbuf[i*3]-b->vbuf[p*3]) * (b->vbuf[p*3+1]-b->vbuf[n*3+1]) - (b->vbuf[i*3+1]-b->vbuf[p*3+1]) * (b->vbuf[p*3]-b->vbuf[n*3]))
static int _s3d_addtri(struct tessp_t *t, struct t_buf *b,int p, int i, int n)
{
	int j,jp,nn,jn,jp2;
	float d1, d2, d3;
	dprintf(HIGH,"[P]rocessing polygon %d %d %d",p,i,n);
	/* check if there is a point in the triangle ... */
	for (j=0;j<b->vn;j++)
	{
		if ((j!=i) && (j!=n) && (j!=p))
		{
			d1=POINT_DIR(p,i,j);
			d2=POINT_DIR(i,n,j);
			d3=POINT_DIR(n,p,j);
			if ((d1>0) && (d2>0) && (d3>0))
					{
						dprintf(HIGH,"%d between, break!",j);
						if (j==t[p].prev)
						{
							dprintf(HIGH,"[I]solating %d (%d -> %d)",p,j,i);
							if (_s3d_addtri(t,b,j,p,i))
							{
								t[p].next=p; /* p is out now ... */
								t[p].prev=p; 
							}
						} else if ((t[n].next)==j)
						{
							/* we only need one polygon here */
							dprintf(HIGH,"[I]solating %d (%d -> %d)",n,i,j);
							if (_s3d_addtri(t,b,i,n,j))
							{
								t[n].next=n; /* n is out now ... */
								t[n].prev=n; 
							}

						} else {
							jp=t[j].prev;
							nn=t[n].next;
							/* we need 2 polygons to split properly */
							dprintf(HIGH,"[1]rerouting (%d -> %d)",i,j);
							t[n].next=j;
							t[j].prev=n;
							_s3d_addtri(t,b,i,n,j);
							jn=t[j].next;
							jp2=t[j].prev;
							dprintf(HIGH,"[2]rerouting (%d -> %d)",jp,n);
							/* redo */
							t[j].prev=jp;
							t[n].next=nn;
							
							t[j].next=n;
							t[n].prev=j;
							_s3d_addtri(t,b,jp,j,n);
							t[j].next=jn;
							t[j].prev=jp2;
						}
						return(0);
					}
		}
	}
	/* there wasn't anything  between, we can savely build the polygon .... */
	dprintf(HIGH,"[B]uilding polygon %d %d %d [nr. %d]",p,i,n,b->pn);
	t[p].next=n;
	t[n].prev=p;
	b->pbuf[b->pn*4]=p;
	b->pbuf[b->pn*4+1]=i;
	b->pbuf[b->pn*4+2]=n;
	b->pbuf[b->pn*4+3]=0;
	b->pn+=1;

	return(1);
}
static int _s3d_tesssub(struct tessp_t *t, struct t_buf *b,int s)
{
	int i,n,p;
	int good=0;
	i=s;
	while ((n=t[i].next)!=(p=t[i].prev)) /* while polygon n > 3 */
	{
		dprintf(HIGH,"[L]ooking at %d [[%d]] %d",p,i,n);
		/* find direction */
		if ((POINT_DIR(p,i,n))>0)
		{
			good=1;
			/* good direction */
/*			dprintf(HIGH,"direction correct, polygon possible!!");*/
			if (_s3d_addtri(t,b,p,i,n))
			{
				/* regular out */
				t[i].next=i;
				t[i].prev=i; 
			}
/*			dprintf(HIGH,"[F]inished polygon %d %d %d",p,i,n);*/
		} else {
			i=t[i].next;
			if ((i==s) && (good==0)) 
			{
				dprintf(HIGH,"Everything is bad, returning");
				return(-1);
			}
			dprintf(HIGH,"sorry, bad direction, next one is %d",i);
			/* bad direction */
/*			done=0;*/
		}
		sleep(1);
	}
	/* handle the last line */
	dprintf(HIGH,"[L]ine handling %d %d %d",p,i,n);
	t[i].next=i;
	t[i].prev=i;
	t[n].next=n;
	t[n].prev=n;
	return(0);
}
int _s3d_tesselate(struct tessp_t *t,struct t_buf *b)
{
	int i,g;
	tessdebug(t,b);
	b->pn=0;
	do {
		g=0;
		for (i=0;i<b->vn;i++) 
		{ /* go through the points ... */
			if (!(t[i].next==i)) /* looping to itself, nothing to do*/
				g+=_s3d_tesssub(t,b,i); /* found unfinished polygon, starting here ... */
		}
	} while (g>0);
	dprintf(HIGH,"tesselation done");
	return(0);
}
