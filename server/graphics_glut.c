/*  this is the file for operation with glut ... */
/*  maybe we are able to choose if we want to use glut or sdl or any other lib later ... */

#include "global.h"
#include <stdio.h>		/* NULL */
#include <GL/glut.h> 	 /*  all the glut functions */
#include <GL/gl.h>		 /*  of course, the gl header */
/*  glut version of graphics init ... */
int graphics_init_glut()
{
	/* XXX: Faking argc and argv is probably not a good idea. */
	int argc=1;
	char *argv[]={"s3d", NULL};
	dprintf(MED,"Using GLUT for GL/windowing ...");
 	glutInit(&argc, argv); 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutInitWindowSize (X_RES, Y_RES);
	glutCreateWindow("grmbl");
    glutIdleFunc(one_time);
	glutDisplayFunc(graphics_main);
	glutReshapeFunc(graphics_reshape);
	return(0);
}
/*  nothing to be done ... */
int graphics_quit_glut()
{
	return(0);	
}
