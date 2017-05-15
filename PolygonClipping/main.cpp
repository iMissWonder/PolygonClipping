#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MAX_POLYGON_VERTICES 30
#define RADIUS 4

int rect_corner_from_y = 0;
int rect_corner_from_x = 0;
int rect_corner_to_y = 0;
int rect_corner_to_x = 0;

// State indicator
enum STATE
{
	drawing_polygon,
	clipping,
	clipped
} display_state;


// Point structure
typedef struct
{
	int x;
	int y;
} pointT;

// Polygon structure
typedef struct
{
	pointT p[MAX_POLYGON_VERTICES];
	int vertices;
} polygonT;

polygonT pol;
polygonT clipped_pol_left;
polygonT clipped_pol_right;
polygonT clipped_pol_bottom;
polygonT clipped_pol_top;

void clear_screen()
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();
}

void draw_point(int x, int y)
{
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
	glutSwapBuffers();
}

void init()
{
	glEnable(GL_BLEND);		// blend on
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	display_state = drawing_polygon;	//initial state: drawing
	glPointSize(4);
	glClearColor(0, 0, 0, 0);
	glColor3f(1, 0, 0);
	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
}

void draw_polygon(polygonT pol)
{
	glBegin(GL_POLYGON);
	glColor3f(1, 0, 0);
	for (int i = 0; i < pol.vertices; i++)
	{
		glVertex2i(pol.p[i].x, pol.p[i].y);
	}
	glEnd();
	glutSwapBuffers();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (display_state == clipping)
	{
		glColor3f(1, 0, 0);
		draw_polygon(pol);
		glColor4f(0, 1, 0, 0.5);
		glRecti(rect_corner_from_x, rect_corner_from_y, rect_corner_to_x, rect_corner_to_y);
		glColor3f(1, 0, 0);
		glutSwapBuffers();
	}
	else if (display_state == clipped)
	{
		glColor3f(1, 0, 0);
		draw_polygon(clipped_pol_top);
		glutSwapBuffers();
	}
	else if (display_state == drawing_polygon)
	{
		glColor3f(1, 0, 0);
		draw_polygon(pol);
		glutSwapBuffers();
	}
}

void mouseFunc(int btn, int state, int mouse_x, int mouse_y)
{
	static int x, y;
	static int vertices = 0;

	x = mouse_x;
	y = WINDOW_HEIGHT - mouse_y;

	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		// we use WINDOW_HEIGHT  - y
		// because mouse position starts counting from above. 
		// y = 0 means upper pixel of the window

		if (display_state == drawing_polygon)
		{
			draw_point(x, y);
			pol.p[vertices].x = x;
			pol.p[vertices].y = y;
			pol.vertices = ++vertices;

			// if the point is in a radius of RADIUS pixels around the 1st point
			if (pow(pol.p[0].x - x, 2) + pow(pol.p[0].y - y, 2) < pow(RADIUS, 2) && vertices > 2)
			{
				pol.vertices = vertices - 1;
				glutPostRedisplay();
				vertices = 0;
			}

		}
		else if (display_state == clipping)
		{
			rect_corner_from_x = x;
			rect_corner_from_y = y;
		}
	}
	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		pol.vertices = 0;
		clipped_pol_left.vertices = 0;
		clipped_pol_right.vertices = 0;
		clipped_pol_bottom.vertices = 0;
		clipped_pol_top.vertices = 0;
		memset(pol.p, 0, sizeof(pol.p));
		memset(clipped_pol_left.p, 0, sizeof(clipped_pol_left.p));
		memset(clipped_pol_right.p, 0, sizeof(clipped_pol_right.p));
		memset(clipped_pol_bottom.p, 0, sizeof(clipped_pol_bottom.p));
		clear_screen();
		display_state = drawing_polygon;
		vertices = 0;
		printf("Screen Cleared\n");
	}
}

void specialFunc(int key, int x, int y)
{

	if (key == GLUT_KEY_F1)
	{
		if (display_state == drawing_polygon)
		{
			printf("Switching to Clipping state\n");
			display_state = clipping;
			glColor3f(0, 1, 0);
		}
		else if (display_state == clipping)
		{
			printf("Switching to Drawing polygon state\n");
			display_state = drawing_polygon;
			glColor3f(1, 0, 0);
			clear_screen();
		}
		else if (display_state == clipped)
		{
			printf("Switching to clipped state\n");
			display_state = drawing_polygon;
			glColor3f(1, 0, 0);
			clear_screen();
		}
	}
}

void getcrossingpoints(polygonT pol, int xmin, int xmax, int ymin,int ymax,
	int x_int[MAX_POLYGON_VERTICES][2], int y_int[MAX_POLYGON_VERTICES][2])
{
	float grad[MAX_POLYGON_VERTICES];
	for (int i = 1; i <= pol.vertices; i++)
	{
		if (pol.p[i % pol.vertices].x == pol.p[i - 1].x)
		{
			x_int[i - 1][0] = pol.p[i - 1].x;
			y_int[i - 1][0] = ymin;

			x_int[i - 1][1] = pol.p[i - 1].x;
			y_int[i - 1][1] = ymax;
		}
		else if (pol.p[i % pol.vertices].y == pol.p[i - 1].y)
		{
			x_int[i - 1][0] = xmin;
			y_int[i - 1][0] = pol.p[i - 1].y;

			x_int[i - 1][1] = xmax;
			y_int[i - 1][1] = pol.p[i - 1].y;
		}
		else
		{
			grad[i - 1] = (pol.p[i % pol.vertices].y - pol.p[i - 1].y)
				/ ((float)pol.p[i % pol.vertices].x - pol.p[i - 1].x);
			x_int[i - 1][0] = (ymin - pol.p[i - 1].y) / grad[i - 1] + pol.p[i - 1].x;
			y_int[i - 1][0] = pol.p[i - 1].y + grad[i - 1] * ((float)xmin - pol.p[i - 1].x);

			x_int[i - 1][1] = (ymax - pol.p[i - 1].y) / grad[i - 1] + pol.p[i - 1].x;
			y_int[i - 1][1] = pol.p[i - 1].y + grad[i - 1] * ((float)xmax - pol.p[i - 1].x);
		}
	}
}

void sutherland_hodgeman_clipping(polygonT pol, int xmin, int ymin, int xmax, int ymax)
{
	int i;
	int y_int[MAX_POLYGON_VERTICES][2];
	int x_int[MAX_POLYGON_VERTICES][2];
	int cv; 

	// Left
	cv = 0;
	getcrossingpoints(pol, xmin, xmax, ymin, ymax, x_int, y_int);
	printf("--LEFT--\n");
	printf("start vertices : %d\n", pol.vertices);
	for (i = 1; i <= pol.vertices; i++)
	{
		if (xmin <= pol.p[i - 1].x && xmin <= pol.p[i % pol.vertices].x)
		{
			printf("A:in B:in\n");
			clipped_pol_left.p[cv].x = pol.p[i - 1].x;
			clipped_pol_left.p[cv].y = pol.p[i - 1].y;
			cv++;
		}
		else if (xmin <= pol.p[i - 1].x && xmin > pol.p[i % pol.vertices].x)
		{
			printf("A:in B:out\n");
			clipped_pol_left.p[cv].x = pol.p[i - 1].x;
			clipped_pol_left.p[cv].y = pol.p[i - 1].y;
			cv++;
			clipped_pol_left.p[cv].x = xmin;
			clipped_pol_left.p[cv].y = y_int[i - 1][0];
			cv++;
		}
		else if (xmin > pol.p[i - 1].x && xmin <= pol.p[i % pol.vertices].x)
		{
			printf("A:out B:in\n");
			clipped_pol_left.p[cv].x = xmin;
			clipped_pol_left.p[cv].y = y_int[i - 1][0];
			cv++;
		}
		else
		{
			printf("A:out B:out\n");
		}
	}
	clipped_pol_left.vertices = cv;
	printf("left vert : %d\n", clipped_pol_left.vertices);

	pol.vertices = cv;
	for (i = 0; i < cv; i++)
	{
		pol.p[i].x = clipped_pol_left.p[i].x;
		pol.p[i].y = clipped_pol_left.p[i].y;
	}
	
	// Right
	cv = 0;
	getcrossingpoints(pol, xmin, xmax, ymin, ymax, x_int, y_int);
	for (i = 1; i <= pol.vertices; i++)
	{
		if (xmax >= pol.p[i - 1].x && xmax >= pol.p[i % pol.vertices].x)
		{
			printf("A:in B:in\n");
			clipped_pol_right.p[cv].x = pol.p[i - 1].x;
			clipped_pol_right.p[cv].y = pol.p[i - 1].y;
			cv++;
		}
		else if (xmax >= pol.p[i - 1].x && xmax < pol.p[i % pol.vertices].x)
		{
			printf("A:in B:out\n");
			clipped_pol_right.p[cv].x = pol.p[i - 1].x;
			clipped_pol_right.p[cv].y = pol.p[i - 1].y;
			cv++;
			clipped_pol_right.p[cv].x = xmax;
			clipped_pol_right.p[cv].y = y_int[i - 1][1];
			cv++;
		}
		else if (xmax < pol.p[i - 1].x && xmax >= pol.p[i % pol.vertices].x)
		{
			printf("A:out B:in\n");
			clipped_pol_right.p[cv].x = xmax;
			clipped_pol_right.p[cv].y = y_int[i - 1][1];
			cv++;
		}
		else
		{
			printf("A:out B:out\n");
		}
	}
	clipped_pol_right.vertices = cv;
	printf("right vert : %d\n", clipped_pol_right.vertices);

	pol.vertices = cv;
	for (i = 0; i < cv; i++)
	{
		pol.p[i].x = clipped_pol_right.p[i].x;
		pol.p[i].y = clipped_pol_right.p[i].y;
	}
	
	// Bottom
	cv = 0;
	getcrossingpoints(pol, xmin, xmax, ymin, ymax, x_int, y_int);
	for (i = 1; i <= pol.vertices; i++)
	{
		if (ymin <= pol.p[i - 1].y && ymin <= pol.p[i % pol.vertices].y)
		{
			printf("A:in B:in\n");
			clipped_pol_bottom.p[cv].x = pol.p[i - 1].x;
			clipped_pol_bottom.p[cv].y = pol.p[i - 1].y;
			cv++;
		}
		else if (ymin <= pol.p[i - 1].y && ymin > pol.p[i % pol.vertices].y)
		{
			printf("A:in B:out\n");
			clipped_pol_bottom.p[cv].x = pol.p[i - 1].x;
			clipped_pol_bottom.p[cv].y = pol.p[i - 1].y;
			cv++;
			clipped_pol_bottom.p[cv].x = x_int[i - 1][0];
			clipped_pol_bottom.p[cv].y = ymin;
			cv++;
		}
		else if (ymin > pol.p[i - 1].y && ymin <= pol.p[i % pol.vertices].y)
		{
			printf("A:out B:in\n");
			clipped_pol_bottom.p[cv].x = x_int[i - 1][0];
			clipped_pol_bottom.p[cv].y = ymin;
			cv++;
		}
		else
		{
			printf("A:out B:out\n");
		}
	}
	clipped_pol_bottom.vertices = cv;
	printf("bottom vert : %d\n", clipped_pol_bottom.vertices);

	pol.vertices = cv;
	for (i = 0; i < cv; i++)
	{
		pol.p[i].x = clipped_pol_bottom.p[i].x;
		pol.p[i].y = clipped_pol_bottom.p[i].y;
	}
	
	// Top
	cv = 0;
	getcrossingpoints(pol, xmin, xmax, ymin, ymax, x_int, y_int);
	for (i = 1; i <= pol.vertices; i++)
	{
		if (ymax >= pol.p[i - 1].y && ymax >= pol.p[i % pol.vertices].y)
		{
			printf("A:in B:in\n");
			clipped_pol_top.p[cv].x = pol.p[i - 1].x;
			clipped_pol_top.p[cv].y = pol.p[i - 1].y;
			cv++;
		}
		else if (ymax >= pol.p[i - 1].y && ymax < pol.p[i % pol.vertices].y)
		{
			printf("A:in B:out\n");
			clipped_pol_top.p[cv].x = pol.p[i - 1].x;
			clipped_pol_top.p[cv].y = pol.p[i - 1].y;
			cv++;
			clipped_pol_top.p[cv].x = x_int[i - 1][1];
			clipped_pol_top.p[cv].y = ymax;
			cv++;
		}
		else if (ymax < pol.p[i - 1].y && ymax >= pol.p[i % pol.vertices].y)
		{
			printf("A:out B:in\n");
			clipped_pol_top.p[cv].x = x_int[i - 1][1];
			clipped_pol_top.p[cv].y = ymax;
			cv++;
		}
		else
		{
			printf("A:out B:out\n");
		}
	}
	clipped_pol_top.vertices = cv;
	printf("top vert : %d\n", clipped_pol_top.vertices);

	pol.vertices = cv;
	for (i = 0; i < cv; i++)
	{
		pol.p[i].x = clipped_pol_top.p[i].x;
		pol.p[i].y = clipped_pol_top.p[i].y;
	}
}



void keyboardFunc(unsigned char key, int x, int y)
{
	int xmin;
	int ymin;
	int xmax;
	int ymax;

	if (display_state == clipping)
	{
		if (key == ' ')
		{
			printf("### Clipping polygon ###\n");
			xmin = (rect_corner_from_x < rect_corner_to_x) ? rect_corner_from_x : rect_corner_to_x;
			xmax = (rect_corner_from_x > rect_corner_to_x) ? rect_corner_from_x : rect_corner_to_x;
			ymin = (rect_corner_from_y < rect_corner_to_y) ? rect_corner_from_y : rect_corner_to_y;
			ymax = (rect_corner_from_y > rect_corner_to_y) ? rect_corner_from_y : rect_corner_to_y;
			printf("xmin : %d\n", xmin);
			printf("xmax : %d\n", xmax);
			printf("ymin : %d\n", ymin);
			printf("ymax : %d\n", ymax);
			printf("rec_to_x : %d\n", rect_corner_to_x);
			printf("rec_to_y : %d\n", rect_corner_to_y);
			printf("rec_from_x : %d\n", rect_corner_from_x);
			printf("rec_from_y : %d\n", rect_corner_from_y);

			printf("####### Result #######\n");
			sutherland_hodgeman_clipping(pol, xmin, ymin, xmax, ymax);
			display_state = clipped;
			glutPostRedisplay();
		}
	}
}

void motionFunc(int x, int y)
{
	if (display_state == clipping)
	{
		rect_corner_to_x = x;
		rect_corner_to_y = WINDOW_HEIGHT - y;
		glutPostRedisplay();
	}
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Polygon Clipping");
	glutDisplayFunc(display);
	init();
	glutMotionFunc(motionFunc);
	glutMouseFunc(mouseFunc);
	glutSpecialFunc(specialFunc);
	glutKeyboardFunc(keyboardFunc);
	clear_screen();
	glutMainLoop();
	return 0;
}