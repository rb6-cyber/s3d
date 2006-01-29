#include "global.h"
#include <GL/glut.h> 	 /*  all the glut functions */
/*  local prototypes */
void keyboard(unsigned char key, int x, int y);
void special(int skey, int x, int y);
/* void mouse(int button, int state, int x, int y); */
void mouse_motion(int x, int y);
extern int but;
/*  init user input things for glut */
int user_init_glut()
{
	dprintf(MED,"using GLUT for user input");
	glutKeyboardFunc (keyboard);
	glutSpecialFunc (special);
	glutMouseFunc (user_mouse);
    glutMotionFunc(mouse_motion);
	return(0);
}

void keyboard(unsigned char key, int x, int y)
{
	user_key(key,0);
}
void special(int skey, int x, int y)
{
	user_key(skey,0);
}
void mouse_motion(int x, int y)
{
	user_mouse(but,2,x,y);
}
