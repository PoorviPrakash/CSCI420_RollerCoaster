/*
 CSCI 480
 Assignment 2
 POORVI BURLY PRAKASH
 5214974090
 */

#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include "pic.h"
#include <math.h>
#include <vector>


int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

CONTROLSTATE g_ControlState = ROTATE;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

int drag_x_origin;
int drag_y_origin;
int dragging = 0;
int zoom = 0;

float _angle = 30.0f;
float camera_angle_v = 0.0f;
float camera_angle_h = 0.0f;
float zoom_in = 0.0f;
float zoom_out = 0.0f;

float _xposition=0.0f;
float _yposition=0.0f;

float speed=0.0f;
int move_forward=-1;

// GLuint texture;
GLuint myTexture[3];

GLfloat angle = 0.0;
float f;
int _value=0;


// represents one control point along the spline
struct point {
    double x;
    double y;
    double z;
};

// spline struct which contains how many control points, and an array of control points
struct spline {
    int numControlPoints;
    struct point *points;
};

// the spline array
struct spline *g_Splines;

// total number of splines
int g_iNumOfSplines;
point p,t,n,b,v,n1,t1,p1,b1;


void texload(int i, char * filename){
	Pic * img;
	img = jpeg_read(filename, NULL);
	glBindTexture(GL_TEXTURE_2D, myTexture[i]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->nx, img->ny, 0, GL_RGB, GL_UNSIGNED_BYTE, &img->pix[0]);
	pic_free(img);
} 

void init(void)
{
	GLfloat light_position[] = {0.0, 0.0, 1.0, 0.0 };
    GLfloat light_ambient[] = {0.5 , 0.5 , 0.5, 1.0};
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
	glGenTextures(3, myTexture);
    texload(0,"textureGround512.jpg");
    texload(1,"background.jpg");
	texload(2,"wood.jpeg");
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT,light_ambient);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
}

//	Matrix Math functions
point add(point a, point b){
  point res;
  res.x = a.x + b.x;
  res.y = a.y + b.y;
  res.z = a.z + b.z;
  return res;
}

point cross_prod(point a, point b){
  point res;
  res.x = a.y * b.z - a.z * b.y;
  res.y = a.z * b.x - a.x * b.z;
  res.z = a.x * b.y - a.y * b.x;
  return res;
}

point scalar_mult(point a, double s){
  point res;
  res.x = a.x * s;
  res.y = a.y * s;
  res.z = a.z * s;
  return res;
}

point normalize(point a){
  point res;
  double mag = sqrt((a.x*a.x)+(a.y*a.y)+(a.z*a.z));
  res.x = a.x/mag;
  res.y = a.y/mag;
  res.z = a.z/mag;
  return res;
}

/* Write a screenshot to the specified filename */
void saveScreenshot (int val)
{
    
    char buffer[20], buffer2[20];
    strcpy(buffer, ".jpg");
    sprintf(buffer2, "%d", val);
    char *filename = strcat(buffer2, buffer);
    
    int i,j;
    Pic *in = NULL;
    
    if (filename == NULL)
        return;
    
    in = pic_alloc(640, 480, 3, NULL);
    
    for (i=479; i>=0; i--) {
        glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                     &in->pix[i*in->nx*in->bpp]);
    }
    
    if (jpeg_write(filename, in)){
    }
    else
        printf("Error in Saving\n");
    
    pic_free(in);
    if(_value<1000)
        glutTimerFunc(66, saveScreenshot, _value++);
}

void mousedrag(int x, int y)
{
    int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
    
    
    switch (g_ControlState)
    {
        case TRANSLATE:
            if (g_iLeftMouseButton)
            {
                g_vLandTranslate[0] += vMouseDelta[0]*0.001;
                g_vLandTranslate[1] -= vMouseDelta[1]*0.001;
                
                if (abs(x - (int)drag_x_origin) > 20) {
                    _yposition = x-drag_x_origin-0.1;
                }
                
                else if (abs(y - (int)drag_y_origin) > 20){
                    _xposition = y-drag_y_origin-0.1;
                }
            }
            if (g_iMiddleMouseButton)
            {
                g_vLandTranslate[2] += vMouseDelta[1]*0.01;
            }
            break;
        case ROTATE:
            if (g_iLeftMouseButton)
            {
                g_vLandRotate[0] += vMouseDelta[1];
                g_vLandRotate[1] += vMouseDelta[0];
                
                camera_angle_v += (y - drag_y_origin)*0.3;
                camera_angle_h += (x - drag_x_origin)*0.3;
                drag_x_origin = x;
                drag_y_origin = y;
                
            }
            if (g_iMiddleMouseButton)
            {
                g_vLandRotate[2] += vMouseDelta[1];
                
                camera_angle_v -= (y - drag_y_origin)*0.3;
                camera_angle_h -= (x - drag_x_origin)*0.3;
                drag_x_origin = x;
                drag_y_origin = y;
                
            }
            break;
        case SCALE:
            if (g_iLeftMouseButton)
            {
                g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
                g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
                zoom_in +=0.1;
            }
            if (g_iMiddleMouseButton)
            {
                g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
                zoom_in -=0.1;
                
                
                
            }
            break;
    }
    g_vMousePos[0] = x;
    g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
    g_vMousePos[0] = x;
    g_vMousePos[1] = y;
    
}

void mousebutton(int button, int state, int x, int y)
{
    
    switch (button)
    {
        case GLUT_LEFT_BUTTON:
            g_iLeftMouseButton = (state==GLUT_DOWN);
            dragging=1;
            drag_x_origin = x;
            drag_y_origin = y;
            break;
        case GLUT_MIDDLE_BUTTON:
            g_iMiddleMouseButton = (state==GLUT_DOWN);
            break;
        case GLUT_RIGHT_BUTTON:
            g_iRightMouseButton = (state==GLUT_DOWN);
            break;
            
    }
    
    switch(glutGetModifiers())
    {
        case GLUT_ACTIVE_CTRL:
            g_ControlState = TRANSLATE;
            break;
        case GLUT_ACTIVE_SHIFT:
            g_ControlState = SCALE;
            break;
        default:
            g_ControlState = ROTATE;
            break;
    }
    
    g_vMousePos[0] = x;
    g_vMousePos[1] = y;
}

void doIdle()
{
    glutPostRedisplay();
}


int loadSplines(char *argv) {
    char *cName = (char *)malloc(128 * sizeof(char));
    FILE *fileList;
    FILE *fileSpline;
    int iType, i = 0, j, iLength;
    
    
    // load the track file
    fileList = fopen(argv, "r");
    if (fileList == NULL) {
        printf ("can't open file\n");
        exit(1);
    }
    
    // stores the number of splines in a global variable
    fscanf(fileList, "%d", &g_iNumOfSplines);
    
    g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));
    
    // reads through the spline files
    for (j = 0; j < g_iNumOfSplines; j++) {
        i = 0;
        fscanf(fileList, "%s", cName);
        fileSpline = fopen(cName, "r");
        
        if (fileSpline == NULL) {
            printf ("can't open file\n");
            exit(1);
        }
        
        // gets length for spline file
        fscanf(fileSpline, "%d %d", &iLength, &iType);
        
        // allocate memory for all the points
        g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
        g_Splines[j].numControlPoints = iLength;
        
        // saves the data to the struct
        while (fscanf(fileSpline, "%lf %lf %lf",
                      &g_Splines[j].points[i].x,
                      &g_Splines[j].points[i].y,
                      &g_Splines[j].points[i].z) != EOF) {
            i++;
        }
    }
    
    free(cName);
    
    return 0;
}

point getSplinePointCoords (point p0, point p1, point p2, point p3, double u){
  point coords;
  coords.x = 0.5 *((2 * p1.x) + (- p0.x + p2.x) * u + (2*p0.x - 5*p1.x+ 4*p2.x - p3.x) * pow(u,2) + (-p0.x + 3*p1.x- 3*p2.x + p3.x) * pow(u,3));
  coords.y = 0.5 *((2 * p1.y) + (- p0.y + p2.y) * u + (2*p0.y - 5*p1.y+ 4*p2.y - p3.y) * pow(u,2) + (-p0.y + 3*p1.y- 3*p2.y + p3.y) * pow(u,3));
  coords.z = 0.5 *((2 * p1.z) + (- p0.z + p2.z) * u + (2*p0.z - 5*p1.z+ 4*p2.z - p3.z) * pow(u,2) + (-p0.z + 3*p1.z- 3*p2.z + p3.z) * pow(u,3));
  return coords;
} 

point getTangents (point p0, point p1, point p2, point p3, double u){
  point coords;
  coords.x = 0.5 *((-p0.x + p2.x) + (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * (2 * u) + (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * (3 * pow(u,2)));
  coords.y = 0.5 *((-p0.y + p2.y) + (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * (2 * u) + (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * (3 * pow(u,2)));
  coords.z = 0.5 *((-p0.z + p2.z) + (2 * p0.z - 5 * p1.z + 4 * p2.z - p3.z) * (2 * u) + (-p0.z + 3 * p1.z - 3 * p2.z + p3.z) * (3 * pow(u,2)));
  return coords;
} 

void drawSkyBox()
{

    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, myTexture[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_QUADS);

    glTexCoord2d(0.0,0.0); glVertex3d(-100,-100.0,-100);
    glTexCoord2d(1.0,0.0); glVertex3d(100.0,-100.0,-100);
    glTexCoord2d(1.0,1.0); glVertex3d(100.0,-100.0,100);
    glTexCoord2d(0.0,1.0); glVertex3d(-100.0,-100.0,100);

    glTexCoord2d(0.0,0.0); glVertex3d(-100,-100,-100);
    glTexCoord2d(1.0,0.0); glVertex3d(-100,100.0,-100);
    glTexCoord2d(1.0,1.0); glVertex3d(-100,100.0,100);
    glTexCoord2d(0.0,1.0); glVertex3d(-100,-100.0,100);

    glTexCoord2d(0.0,0.0); glVertex3d(-100,100.0,-100);
    glTexCoord2d(1.0,0.0); glVertex3d(100.0,100.0,-100);
    glTexCoord2d(1.0,1.0); glVertex3d(100.0,100.0,100);
    glTexCoord2d(0.0,1.0); glVertex3d(-100.0,100.0,100);

    glTexCoord2d(0.0,0.0); glVertex3d(100,-100,-100);
    glTexCoord2d(1.0,0.0); glVertex3d(100,100.0,-100);
    glTexCoord2d(1.0,1.0); glVertex3d(100,100.0,100);
    glTexCoord2d(0.0,1.0); glVertex3d(100,-100.0,100);

    glTexCoord2d(0.0,0.0); glVertex3d(-100,-100.0,100);
    glTexCoord2d(1.0,0.0); glVertex3d(100.0,-100.0,100);
    glTexCoord2d(1.0,1.0); glVertex3d(100.0,100.0,100);
    glTexCoord2d(0.0,1.0); glVertex3d(-100.0,100.0,100);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void drawGroundTexture(){
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, myTexture[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBegin(GL_QUADS);
    glTexCoord2d(0.0,0.0); glVertex3d(-100,-100.0,-2);
    glTexCoord2d(200.0,0.0); glVertex3d(100.0,-100.0,-2);
    glTexCoord2d(200.0,200.0); glVertex3d(100.0,100.0,-2);
    glTexCoord2d(0.0,200.0); glVertex3d(-100.0,100.0,-2);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0, 1.0, 1.0);
}

void drawSplines()
{
    float alpha=0.003,h=0.04;
    for(int r=0;r<2;r++){
        glBegin(GL_QUADS);
		glLineWidth(3);
        for (int j = 0; j < g_iNumOfSplines; j++) {
            for(int i=-2;i<g_Splines[j].numControlPoints-1;i++)
            { 
                for(float u=0.0;u<1.0;u+=0.02){
                    //Point
                    p = getSplinePointCoords(g_Splines[j].points[i], g_Splines[j].points[i+1], g_Splines[j].points[i+2], g_Splines[j].points[i+3],u);
                    //Tangent
                    t= getTangents(g_Splines[j].points[i], g_Splines[j].points[i+1], g_Splines[j].points[i+2], g_Splines[j].points[i+3],u);
                    t = normalize(t);
                    //Normal
                    n = cross_prod(t, v);
                    n = normalize(n);
                    //Binormal
                    b = cross_prod(t, n);
                    b = normalize(b);
                    
					p1 = getSplinePointCoords(g_Splines[j].points[i], g_Splines[j].points[i+1], g_Splines[j].points[i+2], g_Splines[j].points[i+3],u+0.02);
					t1 = getTangents(g_Splines[j].points[i], g_Splines[j].points[i+1], g_Splines[j].points[i+2], g_Splines[j].points[i+3],u+0.02);
                    t1 = normalize(t1);
                    
                    n1 = cross_prod(t1, v);
                    n1 = normalize(n1);
                    b1 = cross_prod(t1, n1);
                    b1 = normalize(b1);
                    
                    if(r==1)
                    {
						p = add(p,scalar_mult(n,f));
                        p1 = add(p1,scalar_mult(n1,f));
                    }
                    
                    glVertex3f(p.x, p.y, p.z);
				
					glColor3f(1.0, 0.0, 0.0);
					glVertex3f(p.x+alpha*(n.x-b.x),p.y+alpha*(n.y-b.y),p.z+alpha*(n.z-b.z));
					glVertex3f(p.x+alpha*(n.x+b.x),p.y+alpha*(n.y+b.y),p.z+alpha*(n.z+b.z));
					glVertex3f(p.x+alpha*(-n.x+b.x),p.y+alpha*(-n.y+b.y),p.z+alpha*(-n.z+b.z));
					glVertex3f(p.x+alpha*(-n.x-b.x),p.y+alpha*(-n.y-b.y),p.z+alpha*(-n.z-b.z));
					glVertex3f(p1.x+alpha*(n1.x-b1.x),p1.y+alpha*(n1.y-b1.y),p1.z+alpha*(n1.z-b1.z));
					glVertex3f(p1.x+alpha*(n1.x+b1.x),p1.y+alpha*(n1.y+b1.y),p1.z+alpha*(n1.z+b1.z));
					glVertex3f(p1.x+alpha*(-n1.x+b1.x),p1.y+alpha*(-n1.y+b1.y),p1.z+alpha*(-n1.z+b1.z));
					glVertex3f(p1.x+alpha*(-n1.x-b1.x),p1.y+alpha*(-n1.y-b1.y),p1.z+alpha*(-n1.z-b1.z));
					glVertex3f(p.x+alpha*(n.x-b.x),p.y+alpha*(n.y-b.y),p.z+alpha*(n.z-b.z));
					glVertex3f(p.x+alpha*(-n.x-b.x),p.y+alpha*(-n.y-b.y),p.z+alpha*(-n.z-b.z));
					glVertex3f(p1.x+alpha*(-n1.x-b1.x),p1.y+alpha*(-n1.y-b1.y),p1.z+alpha*(-n1.z-b1.z));
					glVertex3f(p1.x+alpha*(n1.x-b1.x),p1.y+alpha*(n1.y-b1.y),p1.z+alpha*(n1.z-b1.z));
					glVertex3f(p.x+alpha*(-n.x+b.x),p.y+alpha*(-n.y+b.y),p.z+alpha*(-n.z+b.z));
					glVertex3f(p.x+alpha*(-n.x-b.x),p.y+alpha*(-n.y-b.y),p.z+alpha*(-n.z-b.z));
					glVertex3f(p1.x+alpha*(-n1.x-b1.x),p1.y+alpha*(-n1.y-b1.y),p1.z+alpha*(-n1.z-b1.z));
					glVertex3f(p1.x+alpha*(-n1.x+b1.x),p1.y+alpha*(-n1.y+b1.y),p1.z+alpha*(-n1.z+b1.z));
					glVertex3f(p.x+alpha*(n.x+b.x),p.y+alpha*(n.y+b.y),p.z+alpha*(n.z+b.z));
					glVertex3f(p.x+alpha*(-n.x+b.x),p.y+alpha*(-n.y+b.y),p.z+alpha*(-n.z+b.z));
					glVertex3f(p1.x+alpha*(-n1.x+b1.x),p1.y+alpha*(-n1.y+b1.y),p1.z+alpha*(-n1.z+b1.z));
					glVertex3f(p1.x+alpha*(n1.x+b1.x),p1.y+alpha*(n1.y+b1.y),p1.z+alpha*(n1.z+b1.z));
					glVertex3f(p.x+alpha*(n.x-b.x),p.y+alpha*(n.y-b.y),p.z+alpha*(n.z-b.z));
					glVertex3f(p.x+alpha*(n.x+b.x),p.y+alpha*(n.y+b.y),p.z+alpha*(n.z+b.z));
					glVertex3f(p1.x+alpha*(n1.x+b1.x),p1.y+alpha*(n1.y+b1.y),p1.z+alpha*(n1.z+b1.z));
					glVertex3f(p1.x+alpha*(n1.x-b1.x),p1.y+alpha*(n1.y-b1.y),p1.z+alpha*(n1.z-b1.z));
                }
            }
        }
        glEnd();
    }
    glColor3f(1.0, 1.0, 1.0);
}

void drawRailSeg()
{
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, myTexture[2]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBegin(GL_QUADS);
    for (int j = 0; j < g_iNumOfSplines; j++) {
        for(int i=g_Splines[j].numControlPoints-2;i>=-2;i--)
        {
            for(float u=0.0;u<1.0;u+=0.05){
                
                //Point
                p =getSplinePointCoords(g_Splines[j].points[i], g_Splines[j].points[i+1], g_Splines[j].points[i+2], g_Splines[j].points[i+3],u);
                //Tangent
                t = getTangents(g_Splines[j].points[i], g_Splines[j].points[i+1], g_Splines[j].points[i+2], g_Splines[j].points[i+3],u);
                t = normalize(t);
                //Normal
                n = cross_prod(t, v);
                n = normalize(n);
                
                p1 = getSplinePointCoords(g_Splines[j].points[i], g_Splines[j].points[i+1], g_Splines[j].points[i+2], g_Splines[j].points[i+3],u+0.01);
                t1 = getTangents(g_Splines[j].points[i], g_Splines[j].points[i+1], g_Splines[j].points[i+2], g_Splines[j].points[i+3],u+0.01);
                t1 = normalize(t1);
                
                n1 = cross_prod(t1, v);
                n1 = normalize(n1);
                
                glColor3f(0.1f, 0.1f, 0.1f);
                glTexCoord2d(0.0,0.0);glVertex3f(p.x,p.y,p.z);
                glTexCoord2d(3,0.0);glVertex3f(p.x+f*(n.x),p.y+f*(n.y),p.z+f*(n.z));
                glTexCoord2d(3,1);glVertex3f(p1.x+f*(n1.x),p1.y+f*(n1.y),p1.z+f*(n1.z));
                glTexCoord2d(0.0,1);glVertex3f(p1.x,p1.y,p1.z);
                
                glColor3f(0.1f, 0.1f, 0.1f);
                glTexCoord2d(0.0,0.0);glVertex3f(p.x,p.y,p.z-0.006);
                glTexCoord2d(3.0,0.0);glVertex3f(p.x+f*(n.x),p.y+f*(n.y),p.z+f*(n.z)-0.006);
                glTexCoord2d(3.0,1.0);glVertex3f(p1.x+f*(n1.x),p1.y+f*(n1.y),p1.z+f*(n1.z)-0.006);
                glTexCoord2d(0.0,1.0);glVertex3f(p1.x,p1.y,p1.z-0.006);
                
                glColor3f(0.1f, 0.1f, 0.1f);
                glTexCoord2d(0.0,0.0);glVertex3f(p.x,p.y,p.z);
                glTexCoord2d(3.0,0.0);glVertex3f(p.x+f*(n.x),p.y+f*(n.y),p.z+f*(n.z));
                glTexCoord2d(3.0,1.0);glVertex3f(p.x+f*(n.x),p.y+f*(n.y),p.z+f*(n.z)-0.006);
                glTexCoord2d(0.0,1.0);glVertex3f(p.x,p.y,p.z-0.006);
                
                glColor3f(0.1f, 0.1f, 0.1f);
                glTexCoord2d(0.0,0.0);glVertex3f(p1.x,p1.y,p1.z);
                glTexCoord2d(3.0,0.0);glVertex3f(p1.x+f*(n1.x),p1.y+f*(n1.y),p1.z+f*(n1.z));
                glTexCoord2d(3.0,1.0);glVertex3f(p1.x+f*(n1.x),p1.y+f*(n1.y),p1.z+f*(n1.z)-0.006);
                glTexCoord2d(0.0,1.0);glVertex3f(p1.x,p1.y,p1.z-0.006);  
            }
        }
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0, 1.0, 1.0);
}

//MOVE THE CAMERA ON THE ROLLER COASTER
void cameraSetUp()
{
    int j=0;
    int i=move_forward;
    
    p = getSplinePointCoords(g_Splines[0].points[i], g_Splines[0].points[i+1], g_Splines[0].points[i+2], g_Splines[0].points[i+3], speed);
	t = getTangents(g_Splines[0].points[i], g_Splines[0].points[i+1], g_Splines[0].points[i+2], g_Splines[0].points[i+3], speed);
    t = normalize(t);
    
    n = cross_prod(t, v);
    n = normalize(n);
    
    b = cross_prod(t, n);
    b = normalize(b);
    
    point eye,center;
	eye = add(p,add(scalar_mult(n,(f/2.0)),scalar_mult(b,0.1)));
    center = add(add(scalar_mult(n,(f/2.0)), t),p);	

	gluLookAt(
              // 5.0,5.0,14.0,
              eye.x,eye.y,eye.z,
              center.x,center.y,center.z,
              b.x,b.y,b.z);
}

void display(void)
{
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    
    glPushMatrix();
    glTranslatef(g_vLandTranslate[0],g_vLandTranslate[1],g_vLandTranslate[2]);
	glRotatef(g_vLandRotate[0],1,0,0);
	glRotatef(g_vLandRotate[1],0,1,0);
	glRotatef(g_vLandRotate[2],0,1,0);
	glScalef(-g_vLandScale[0],g_vLandScale[1],g_vLandScale[2]);
    
    f=0.08;
    v.x=0.0;v.y=0.0;v.z=-1.0;
    
    cameraSetUp();
    drawSkyBox();
	drawGroundTexture();
    drawRailSeg();
    drawSplines();

    glPopMatrix();
    glutSwapBuffers();
}


void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    
    glLoadIdentity();
    gluPerspective(20.0, (GLfloat)w/(GLfloat)h, 0.01, 1000.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void handleKeypress(unsigned char key, int x, int y)
{
    switch(key) {
        case 'e': exit(0);
            
        case 'a': speed+=0.02;
            if(speed>=1.0){
                move_forward++;
                speed=0.0;
                if(move_forward>=g_Splines[0].numControlPoints-3)
                    move_forward=0;
            }
    }
    
}

int main (int argc, char ** argv)
{
    if (argc<2)
    {
        printf ("usage: %s <trackfile>\n", argv[0]);
        exit(0);
    }
    
    loadSplines(argv[1]);
    
    glutInit(&argc, argv);
    
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (640, 480);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("CSCI420 Assignment 2");
    init ();
    glClearDepth(1.0);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    
    
    glutKeyboardFunc(handleKeypress);
    glutIdleFunc(doIdle);
    glutMotionFunc(mousedrag);
    glutPassiveMotionFunc(mouseidle);
    glutMouseFunc(mousebutton);
    //saveScreenshot(0);
    glutMainLoop();
    return 0;
    
}