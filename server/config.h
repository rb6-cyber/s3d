/*  to be filled with configure options ...  */
/*   */
/*  on which port do we listen? */
#define S3D_PORT	6066
/*  resolution on startup */
#define X_RES	800
#define Y_RES	600
/* how many frames to wait until test the connection if it's still here */
#define MAX_IDLE	50
/*  this is to be set dynamicly later on */
#define VLOW	1
#define	LOW		2
#define MED		3
#define HIGH	4
#define	VHIGH	5
/*  which is the minimum level of debugmessage we want to see? */
#define DEBUG	LOW
#define NO_DEBUG	1 /* we want no debugging !! */
/*  which subsystem do we use for rendering and ? */

/* GLUT is the GL utility library which you obtain at 
 * http: //  freeglut.sourceforge.net/  
 */
#define G_GLUT	1
/* SDL is a framework for simple media access which contains
 * opengl support besides music, cdrom etc.
 */
/*  #define G_SDL	2 */
/*  do we want signals? usually, yes. it makes network things  */
/*  with polling a lot faster and  */
/*  we can go down properly on a terminate signal... */
/*  windows does not support that, so ... */
#ifndef WIN32
#define SIGS	1
#define SHM		1
#endif
#define TCP		1
