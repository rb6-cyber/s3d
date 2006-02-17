#include <sei_triangulate.h>
#include <sys/time.h>
#include <math.h>


static int choose_idx;
static int permute[SEGSIZE];
double mlog2(double x) {
  return log(x)/log(2);
}

/* Generate a random permutation of the segments 1..n */
int generate_random_ordering(n)
     int n;
{
  struct timeval tval;
  struct timezone tzone;
  register int i;
  int m, st[SEGSIZE], *p;
  
  choose_idx = 1;
  gettimeofday(&tval, &tzone);
  srand48(tval.tv_sec);

  for (i = 0; i <= n; i++)
    st[i] = i;

  p = st;
  for (i = 1; i <= n; i++, p++)
    {
      m = lrand48() % (n + 1 - i) + 1;
      permute[i] = p[m];
      if (m != 1)
	p[m] = p[1];
    }
  return 0;
}

  
/* Return the next segment in the generated random ordering of all the */
/* segments in S */
int choose_segment()
{
  int i;

  errds(VLOW,"sei:choose_segment()","%d", permute[choose_idx]);
  return permute[choose_idx++];
}

/* Get log*n for given n */
int math_logstar_n(n)
     int n;
{
  register int i;
  double v;
  
  for (i = 0, v = (double) n; v >= 1; i++)
    v = mlog2(v);
  
  return (i - 1);
}
  

int math_N(n, h)
     int n;
     int h;
{
  register int i;
  double v;

  for (i = 0, v = (int) n; i < h; i++)
    v = mlog2(v);
  
  return (int) ceil((double) 1.0*n/v);
}
