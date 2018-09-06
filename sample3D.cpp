#include<string.h>
#include <stdlib.h> 
#include <math.h> 
#include <stdio.h> 
#include <GL/glut.h>
#include "imageloader.h"
#include "vec3f.h"


using namespace std;
int backlight=0,headlight=1,colour=0,timeee=0;
#define ESC 27
#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define RAD2DEG(deg) (deg *  180.0/PI)

struct fossil{
	float x;
	float y;
	float z;
	int flag;
}fossils[100];

struct tree{
	float x;
	float y;
	float z;
}trees[100];

//-------------------------------------------------------------------------
#define NUM_SPOKES      20 
#define SPOKE_ANGLE      18 
#define RADIUS_WHEEL   1.0f 
#define TUBE_WIDTH      0.2f 
#define RIGHT_ROD      1.6f 
#define RIGHT_ANGLE      48.0f 
#define MIDDLE_ROD      1.7f 
#define MIDDLE_ANGLE   106.0f 
#define BACK_CONNECTOR   0.5f 
#define LEFT_ANGLE      50.0f 
#define WHEEL_OFFSET   0.11f 
#define WHEEL_LEN      1.1f 
#define TOP_LEN         1.5f 
#define CRANK_ROD      0.7f 
#define CRANK_RODS      1.12f 
#define CRANK_ANGLE      8.0f 
#define HANDLE_ROD      1.2f 
#define FRONT_INCLINE   70.0f 
#define HANDLE_LIMIT   70.0f 
#define INC_STEERING   2.0f 
#define INC_SPEED      0.05f 
//----------------------------------------------------------------------
// Global variables
//
// The coordinate system is set up so that the (x,y)-coordinate plane
// is the ground, and the z-axis is directed upwards. The y-axis points
// to the north and the x-axis points to the east.
//
// The values (x,y) are the current camera position. The values (lx, ly)
// point in the direction the camera is looking. The variables angle and
// deltaAngle control the camera's angle. The variable deltaMove
// indicates the amount of incremental motion for the camera with each
// redraw cycle. The variables isDragging and xDragStart are used to
// monitor the mouse when it drags (with the left button down).
//----------------------------------------------------------------------

// Camera position
float x = 0.0, y = -5.0; // initially 5 units south of origin
float xh = 0.0 , yh = -55.0; 
float deltaMove = 0.0; // initially camera doesn't move
float ldeltaMove = 0.0; // initially camera doesn't move
float xval=0,yval=1,zval=1;
// Camera direction
float lx = 0.0, ly = 1.0;// camera points initially along y-axis
float angle = 0.0; // angle of rotation for the camera direction
float deltaAngle = 0.0; // additional angle change when dragging
char camera_view='4';
float t_height=5;
float scale=1;
float bike_speed=0;
float bike_angle=0;
float bike_roll=0;
float bike_yaw=0;
float bike_pitch=0;
int flag_down=0;
int tilt=0;
float back_angle=bike_yaw;
float wheel_angle=0;
// Mouse drag control
int isDragging = 0; // true when dragging
int xDragStart = 0; // records the x-coordinate when dragging starts
int collected=0;
int jump=0;
float  pheight=0;

class Terrain {
	private:
		int w; //Width
		int l; //Length
		float** hs; //Heights
		Vec3f** normals;
		bool computedNormals; //Whether normals is up-to-date
	public:
		Terrain(int w2, int l2) {
			w = w2;
			l = l2;
			
			hs = new float*[l];
			for(int i = 0; i < l; i++) {
				hs[i] = new float[w];
			}
			
			normals = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals[i] = new Vec3f[w];
			}
			
			computedNormals = false;
		}
		
		~Terrain() {
			for(int i = 0; i < l; i++) {
				delete[] hs[i];
			}
			delete[] hs;
			
			for(int i = 0; i < l; i++) {
				delete[] normals[i];
			}
			delete[] normals;
		}
		
		int width() {
			return w;
		}
		
		int length() {
			return l;
		}
		
		//Sets the height at (x, z) to y
		void setHeight(int x, int z, float y) {
			hs[z][x] = y;
			computedNormals = false;
		}
		
		//Returns the height at (x, z)
		float getHeight(int x, int z) {
			return hs[z][x];
		}
		
		//Computes the normals, if they haven't been computed yet
		void computeNormals() {
			if (computedNormals) {
				return;
			}
			
			//Compute the rough version of the normals
			Vec3f** normals2 = new Vec3f*[l];
			for(int i = 0; i < l; i++) {
				normals2[i] = new Vec3f[w];
			}
			
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum(0.0f, 0.0f, 0.0f);
					
					Vec3f out;
					if (z > 0) {
						out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
					}
					Vec3f in;
					if (z < l - 1) {
						in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
					}
					Vec3f left;
					if (x > 0) {
						left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
					}
					Vec3f right;
					if (x < w - 1) {
						right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
					}
					
					if (x > 0 && z > 0) {
						sum += out.cross(left).normalize();
					}
					if (x > 0 && z < l - 1) {
						sum += left.cross(in).normalize();
					}
					if (x < w - 1 && z < l - 1) {
						sum += in.cross(right).normalize();
					}
					if (x < w - 1 && z > 0) {
						sum += right.cross(out).normalize();
					}
					
					normals2[z][x] = sum;
				}
			}
			
			//Smooth out the normals
			const float FALLOUT_RATIO = 0.5f;
			for(int z = 0; z < l; z++) {
				for(int x = 0; x < w; x++) {
					Vec3f sum = normals2[z][x];
					
					if (x > 0) {
						sum += normals2[z][x - 1] * FALLOUT_RATIO;
					}
					if (x < w - 1) {
						sum += normals2[z][x + 1] * FALLOUT_RATIO;
					}
					if (z > 0) {
						sum += normals2[z - 1][x] * FALLOUT_RATIO;
					}
					if (z < l - 1) {
						sum += normals2[z + 1][x] * FALLOUT_RATIO;
					}
					
					if (sum.magnitude() == 0) {
						sum = Vec3f(0.0f, 1.0f, 0.0f);
					}
					normals[z][x] = sum;
				}
			}
			
			for(int i = 0; i < l; i++) {
				delete[] normals2[i];
			}
			delete[] normals2;
			
			computedNormals = true;
		}
		
		//Returns the normal at (x, z)
		Vec3f getNormal(int x, int z) {
			if (!computedNormals) {
				computeNormals();
			}
			return normals[z][x];
		}
};

Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for(int y = 0; y < image->height; y++) {
		for(int x = 0; x < image->width; x++) {
			unsigned char color =
				(unsigned char)image->pixels[3 * (y * image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			if(h<-5)
				t->setHeight(x, y, -3);
			else			
				t->setHeight(x, y, h);
		}
	}
	
	delete image;
	t->computeNormals();
	return t;
}

GLuint loadTexture(Image* image) {
	GLuint textureId;
	glGenTextures(1, &textureId); //Make room for our texture
	glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
	//Map the image to the texture
	glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
				 0,                            //0 for now
				 GL_RGB,                       //Format OpenGL uses for image
				 image->width, image->height,  //Width and height
				 0,                            //The border of the image
				 GL_RGB, //GL_RGB, because pixels are stored in RGB format
				 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
				                   //as unsigned numbers
				 image->pixels);               //The actual pixel data
	return textureId; //Returns the id of the texture
}

GLuint _textureId; //The id of the texture

float _angle = 60.0f;
Terrain* _terrain;

void cleanup() {
	delete _terrain;
}
void changeSize(int w, int h) 
{
	float ratio =  ((float) w) / ((float) h); // window aspect ratio
	glMatrixMode(GL_PROJECTION); // projection matrix is active
	glLoadIdentity(); // reset the projection
	gluPerspective(45.0, ratio, 0.1, 100.0); // perspective transformation
	glMatrixMode(GL_MODELVIEW); // return to modelview mode
	glViewport(0, 0, w, h); // set viewport (drawing area) to entire window
}
float terrain_point_x(float v1)
{
	return (float(v1)*1.0/scale+(_terrain->width()-1)/2);
}

float terrain_point_y(float v1)
{
	return (float(v1)*1.0/scale+(_terrain->length()-1)/2);
}

void ZCylinder(GLfloat radius,GLfloat length) 
{ 
	GLUquadricObj *cylinder; 
	cylinder=gluNewQuadric(); 
	glPushMatrix(); 
	glTranslatef(0.0f,0.0f,0.0f); 
	gluCylinder(cylinder,radius,radius,length,15,5); 
	glPopMatrix(); 
} 

void drawscore()
{
	glPushMatrix();
	glScalef(1000,1000,1000);
	glPopMatrix();

}

void wheel()
{
	glScalef(0.5,0.5,0.5);
	glRotatef(90,0,1,0);
	glScalef(1,1,2);
	glRotatef(-2*wheel_angle,0.0f,0.0f,1.0f);
	glutSolidTorus(0.06f,0.92f,4,30); 
	//   Draw The Central Cylinder 
	//   Length of cylinder  0.12f 
	//glColor3f(1.0f,1.0f,0.5f); 
	glColor3f(1.0f,0.5f,0.0f); 
	glPushMatrix(); 
//	glTranslatef(0.0f,0.0f,-0.06f); 
//	ZCylinder(0.02f,0.12f); 
	glPopMatrix(); 
	glScalef(1,1,0.5);
	glutSolidTorus(0.2f,0.2f,10,20); 
	glScalef(1,1,2);
	//   Draw The Spokes 
	glColor3f(1.0f,0.5f,0.0f); 
	for(int i=0;i<NUM_SPOKES/2;++i)
	{ 
		glPushMatrix(); 
		glRotatef(i*SPOKE_ANGLE*2,0.0f,0.0f,1.0f); 
		glScalef(1,1,1);
		glLineWidth(50);
		glBegin(GL_LINES);
		if(i==1)
			glColor3f(1,0,0);
		else
			glColor3f(1.0f,0.5f,0.0f); 
		glVertex3f(0.0f,0.02f,0.0f); 
		glVertex3f(0.0f,0.86f,0.0f); 
		glEnd(); 
		glLineWidth(1);
		glPopMatrix(); 
	} 
	//   Draw The Tyre 
	glColor3f(0.0f,0.0f,0.0f); 
	glutSolidTorus(TUBE_WIDTH,RADIUS_WHEEL,10,30); 
	glColor3f(1.0f,0.0f,0.0f);
}

void update(void) 
{
	for(int i=0;i<20;i++)
	{
		if(fossils[i].flag!=0 && sqrt(pow(fossils[i].x-xval,2)+pow(fossils[i].y-yval-1,2))<=1)
		{
			fossils[i].flag=0;
			collected+=1;
		}
		//printf("%f %f -- %f %f");
	}
	if(flag_down==0)
	{
		if(bike_speed>-0.002 && bike_speed<0.002)
			bike_speed=0;
		else if(bike_speed-0.004>0)
			bike_speed-=0.004;
		else if(bike_speed+0.002<0)
			bike_speed+=0.002;

		if(bike_roll>0)
			bike_roll-=0.5;
		else if(bike_roll<0)
			bike_roll+=0.5;
	}	
	if(tilt!=0)
	{
		bike_yaw+=2.5*tilt;
		tilt=0;
		if(bike_yaw-bike_angle>60)
			bike_yaw-=2.5;
		else if(bike_yaw-bike_angle<-60)
			bike_yaw+=2.5;
	}
	if(flag_down==1)
	{
		//angle+=bike_yaw;
		if(deltaMove!=0)
		{
			bike_speed+=0.004*deltaMove;
			if(deltaMove<0 and bike_speed>0)
				bike_speed-=0.001;
			if(deltaMove>0 and bike_speed<0)
				bike_speed+=0.001;
		}
		if(ldeltaMove!=0)
		{
			if(bike_roll+ldeltaMove<=45 && bike_roll+ldeltaMove>=-45)
			{
				bike_roll+=1*ldeltaMove;
			}
		}
	}
	//printf("%f %f\n",bike_angle,bike_yaw);
	if(bike_angle<bike_yaw)
	{
		if(bike_speed>0)
		{
			bike_angle+=bike_speed*30;
		}
		else if(bike_speed<0)
		{
			bike_angle-=bike_speed*30;
		}
	}
	else if(bike_angle>bike_yaw)
	{
		if(bike_speed>0)
		{
			bike_angle-=bike_speed*30;
		}
		else if(bike_speed<0)
		{
			bike_angle+=bike_speed*30;
		}
	}

	/*if(bike_angle*ff<bike_yaw*ff && bike_speed!=0)
	{
		bike_angle-=bike_speed*30*ff*fg;
	}*/
	wheel_angle+=bike_speed*30;
	if(1)
	{
		x+=lx*bike_speed;
		y+=ly*bike_speed;
		xval+=lx*bike_speed;
		yval+=ly*bike_speed;
		if(x>49)
			x=49;
		if(x<-49)
			x=-49;
		if(y>49)
			y=49;
		if(y<-49)
			y=-49;
		if(xval>49)
			xval=49;
		if(xval<-49)
			xval=-49;
		if(yval<-49)
			yval=-49;
		if(yval>49)
			yval=49;
	}
	if(1)
	{
		lx = -sin(angle + bike_yaw*PI/180);
		ly = cos(angle + bike_yaw*PI/180);
	}
	glutPostRedisplay(); // redisplay everything
}

void drawbike()
{
	Vec3f normal = _terrain->getNormal(terrain_point_x(xval),terrain_point_y(yval+1));
	bike_pitch=-atan(normal[2]/sqrt(normal[0]*normal[0]+normal[1]*normal[1]))*180/PI;
	glRotatef(-atan(normal[2]/sqrt(normal[0]*normal[0]+normal[1]*normal[1]))*180/PI,1,0,0);
	wheel();

}

void drawTree()
{
	glColor3f(0.55,0.27,0.1);
	GLUquadricObj *quadratic;
	quadratic = gluNewQuadric();
	glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
	gluCylinder(quadratic,0.25f,0.25f,2.2f,32,32);
	glPushMatrix();
	glColor3f(0.0,0.5,0.0);
	glTranslatef(0.0,0.0,2.0);
	glScalef(1.0,1.2,1.5);
	glutSolidSphere(0.75, 20, 20);
	glPopMatrix();
}

void drawcube()
{
	//glScalef(100,100,100);
	glBegin(GL_QUADS);      // Draw The Cube Using quads
	glColor3f(1.0f,0.0f,0.0f);    // Color Red    
	glVertex3f( 1.0f, 1.0f,-1.0f);    // Top Right Of The Quad (Top)
	glVertex3f(-1.0f, 1.0f,-1.0f);    // Top Left Of The Quad (Top)
	glVertex3f(-1.0f, 1.0f, 1.0f);    // Bottom Left Of The Quad (Top)
	glVertex3f( 1.0f, 1.0f, 1.0f);    // Bottom Right Of The Quad (Top)
	glColor3f(0.8f,0.8f,0.0f);    // Color Red    
	glVertex3f( 1.0f,-1.0f, 1.0f);    // Top Right Of The Quad (Bottom)
	glVertex3f(-1.0f,-1.0f, 1.0f);    // Top Left Of The Quad (Bottom)
	glVertex3f(-1.0f,-1.0f,-1.0f);    // Bottom Left Of The Quad (Bottom)
	glVertex3f( 1.0f,-1.0f,-1.0f);    // Bottom Right Of The Quad (Bottom)	
	glColor3f(1.0f,0.0f,0.0f);    // Color Red    
	glVertex3f( 1.0f, 1.0f, 1.0f);    // Top Right Of The Quad (Front)
	glVertex3f(-1.0f, 1.0f, 1.0f);    // Top Left Of The Quad (Front)
	glVertex3f(-1.0f,-1.0f, 1.0f);    // Bottom Left Of The Quad (Front)
	glVertex3f( 1.0f,-1.0f, 1.0f);    // Bottom Right Of The Quad (Front)
	glColor3f(0.7f,0.0f,0.2f);    // Color Red    
	glVertex3f( 1.0f,-1.0f,-1.0f);    // Top Right Of The Quad (Back)
	glVertex3f(-1.0f,-1.0f,-1.0f);    // Top Left Of The Quad (Back)
	glVertex3f(-1.0f, 1.0f,-1.0f);    // Bottom Left Of The Quad (Back)
	glVertex3f( 1.0f, 1.0f,-1.0f);    // Bottom Right Of The Quad (Back)
	glColor3f(0,0.7f,0.3f);    // Color Red    
	glVertex3f(-1.0f, 1.0f, 1.0f);    // Top Right Of The Quad (Left)
	glVertex3f(-1.0f, 1.0f,-1.0f);    // Top Left Of The Quad (Left)
	glVertex3f(-1.0f,-1.0f,-1.0f);    // Bottom Left Of The Quad (Left)
	glVertex3f(-1.0f,-1.0f, 1.0f);    // Bottom Right Of The Quad (Left)
	glVertex3f( 1.0f, 1.0f,-1.0f);    // Top Right Of The Quad (Right)
	glVertex3f( 1.0f, 1.0f, 1.0f);    // Top Left Of The Quad (Right)
	glVertex3f( 1.0f,-1.0f, 1.0f);    // Bottom Left Of The Quad (Right)
	glVertex3f( 1.0f,-1.0f,-1.0f);    // Bottom Right Of The Quad (Right)
	glEnd();
}

void drawboundary()
{
	glBegin(GL_QUADS);      // Draw The Cube Using quads
	glColor3f(1.0f,0.0f,0.0f);    // Color Red    


	glVertex3f(50,-50,50);
	glVertex3f(50,-50,-50);
	glVertex3f(50,50,-50);
	glVertex3f(50,50,50);

	glVertex3f(-50,50,50);
	glVertex3f(-50,50,-50);
	glVertex3f(50,50,-50);
	glVertex3f(50,50,50);
	glEnd();
}
void renderScene(void) 
{
	if(timeee!=1)
		timeee+=10;
	if(timeee==120000)
		timeee=1;
	if(timeee!=1)
	{
		
		// Clear color and depth buffers
		glClearColor(0.0, 0.7, 1.0, 1.0); // sky color is light blue
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Reset transformations
		glLoadIdentity();
		// Set the camera centered at (x,y,1) and looking along directional
		// vector (lx, ly, 0), with the z-axis pointing up
		drawboundary();
		GLfloat ambientColor11[] = {0.3f, 0.3f, 0.3f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor11);

		GLfloat lightColor011[] = {0.6f, 0.6f, 0.6f, 1.0f};
		GLfloat lightPos011[] = {-0.5f, 0.8f, 0.1f, 0.0f};
		glLightfv(GL_LIGHT4, GL_DIFFUSE, lightColor011);
		glLightfv(GL_LIGHT4, GL_POSITION, lightPos011);
		glEnable(GL_LIGHT4);


		if(camera_view=='1')
			gluLookAt(	
					xval,   yval+1,  t_height +0.3* abs(t_height),
					xval + 10*lx, yval + 10*ly , t_height + 0.3*abs(t_height),
					sin(bike_roll*PI/180),0,  cos(bike_roll*PI/180));
		if(camera_view=='2')
		{
			gluLookAt(	xval - lx,   yval -ly+1,   _terrain->getHeight(terrain_point_x(xval-lx),terrain_point_y(yval+1-ly)) +0* abs(t_height),
					xval+lx, yval+ly+1, _terrain->getHeight(terrain_point_x(xval-lx),terrain_point_y(yval+1-ly)) + 0*abs(t_height),
					sin(bike_roll*PI/180),0,  cos(bike_roll*PI/180));

		}
		if(camera_view=='3')
		{
			gluLookAt(	xval,   yval+1,   45,
					xval+lx, yval+ly+1, t_height + 0.2*abs(t_height),
					0.0,    0.0,    1.0);

		}
		if(camera_view=='4')
		{
			gluLookAt(	xval - 20*lx,   yval -20*ly,  10,
					xval+lx, yval+ly+1, t_height +0.2*abs(t_height),
					0.0,    0.0,    1.0);
		}
		if(camera_view=='5')
		{
			gluLookAt(	xval + 10*lx,   yval + 10*ly,   t_height +0.2* abs(t_height),
					xval+lx, yval+ly+1, t_height + 0.2*abs(t_height),
					0.0,    0.0,    1.0);
		}
		if(camera_view=='6')
		{
			gluLookAt(	xval + 7*ly,   yval + 7*lx,   t_height +0.2* abs(t_height),
					xval-ly, yval-lx+1, t_height + 0.2*abs(t_height),
					0.0,    0.0,    1.0);
		}
		if(camera_view=='7')
		{
			gluLookAt(	xval + 10*lx,   yval + 10*ly,   t_height +0.2* abs(t_height),
					xval+lx, yval+ly+1, t_height + 0.2*abs(t_height),
					0.0,    0.0,    1.0);
		}
		//drawscore();
		for(int i=0;i<20;i++)
		{
			glPushMatrix();
			glTranslatef(fossils[i].x,fossils[i].y,fossils[i].z+0.1*abs(fossils[i].z));
			glScalef(0.6,0.6,0.6);
			if(fossils[i].flag==1)
				drawcube();
			glPopMatrix();
		}
		for(int i=0;i<10;i++)
		{
			if(trees[i].z<2)
			{
				glPushMatrix();
				glTranslatef(trees[i].x,trees[i].y,trees[i].z+0.1*abs(trees[i].z));
				glScalef(2.5,2.5,2.5);
				drawTree();
				glPopMatrix();
			}
		}
		glEnable(GL_LIGHT3);
		glPushMatrix();
		GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);
		GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
		GLfloat lightPos0[] = {-25, -25, 0, 0.0f};
		glLightfv(GL_LIGHT3, GL_DIFFUSE, lightColor0);
		glLightfv(GL_LIGHT3, GL_POSITION, lightPos0);
		glPopMatrix();
		if(headlight==1)
			glEnable(GL_LIGHT0);
		else
			glDisable(GL_LIGHT0);
		if(backlight==1)
			glEnable(GL_LIGHT1);
		else
			glDisable(GL_LIGHT1);
		glPushMatrix();
		glTranslatef(xval,yval+1, t_height +  0.2*abs(t_height));
		glRotatef(bike_yaw,0.0f,0.0f,1.0f); 
		GLfloat light0_ambient[] = { 1, 1, 1, 1.0 };
		GLfloat light0_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat light0_specular[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat light0_position[] = { 0.0, 0.0, 0.0, 1.0 }; 
		GLfloat spot_direction0[] = { 0.0, 1.0, 0.0 }; 
		glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular); 
		glLightfv(GL_LIGHT0, GL_POSITION, light0_position); 
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 35.0); 
		glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spot_direction0);
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 2.0);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(xval,yval-1.5, t_height +  0.2*abs(t_height));
		glRotatef(bike_yaw,0.0f,0.0f,1.0f); 
		GLfloat light0_ambient1[] = { 0.5, 0.5, 0.5, 1.0 };
		GLfloat light0_diffuse1[] = { 1.0, 0.0, 0.0, 1.0 };
		GLfloat light0_specular1[] = { 1.0, 0.0, 0.0, 1.0 };
		GLfloat light0_position1[] = { 0.0, 0.0, 0.0, 1.0 }; 
		GLfloat spot_direction01[] = { 0.0, -1.0, 0.0 }; 
		glLightfv(GL_LIGHT1, GL_AMBIENT, light0_ambient1);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, light0_diffuse1);
		glLightfv(GL_LIGHT1, GL_SPECULAR, light0_specular1); 
		glLightfv(GL_LIGHT1, GL_POSITION, light0_position1); 
		glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 35.0); 
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_direction01);
		glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0);
		glPopMatrix();
		/**************************************************/

		glPushMatrix();

		glScalef(scale, scale, scale);
		glTranslatef(-(float)(_terrain->width() - 1) / 2,
				-(float)(_terrain->length() - 1) / 2,0);
		//pheight=t_height;
		t_height=_terrain->getHeight(terrain_point_x(xval),terrain_point_y(yval+1));
		t_height*=scale;
		//printf("%d\n",collected);
		if(jump)
		{
			pheight-=0.15;
		}
		if( pheight - t_height > 0.3)
		{
			jump=1;
			//	printf("%d --  %f -- %f\n ",jump,pheight,t_height);
		}
		else
		{
			jump=0;
			pheight=t_height;
		}
		glColor3f(0.3f, 0.8f, 0.0f);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, _textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		for(int y1 = 0; y1 < _terrain->length() - 1; y1++) {
			//Makes OpenGL draw a triangle at every three consecutive vertices
			glBegin(GL_TRIANGLE_STRIP);
			for(int x = 0; x < _terrain->width(); x++) {
				Vec3f normal = _terrain->getNormal(x, y1);
				glNormal3f(normal[0], normal[1], normal[2]);
				//glVertex3f(x, y1,_terrain->getHeight(x, y1));
				glColor3f(0,1,0);
				if(_terrain->getHeight(x,y1)*scale<-5.1)
					glColor3f(0, 0, 0.8);
				else if(_terrain->getHeight(x,y1)*scale>-2)
					glColor3f(139.0/255,69.0/255,15.0/255);   
					
					//glColor3f(1, 1, 1);
				else
					glColor3f(0.3f, 0.8f, 0.0f);
				glTexCoord2f(x,y1);
				glVertex3f(x, y1,_terrain->getHeight(x, y1 ));
				normal = _terrain->getNormal(x, y1 + 1);
				glNormal3f(normal[0], normal[1], normal[2]);
				glTexCoord2f(x,y1+1);
				glVertex3f(x, y1+1,_terrain->getHeight(x, y1 + 1));
			}
			glEnd();
		}
		glPopMatrix();
		glPushMatrix();
		if(jump)
			glTranslatef(xval,yval+1,pheight+0.9);
		else
			glTranslatef(xval,yval+1,t_height+0.9);
		glRotatef(+bike_yaw,0,0,1);
		glRotatef(bike_roll,0,1,0);
		glColor3f(0.0f,0.0f,0.0f);

		glPushMatrix();
		glColor3f(1,0,0);
		glTranslatef(0,0.5,0.8);
		glRotatef(90,1,0,0);
		GLUquadricObj *quadratic;
		quadratic = gluNewQuadric();
		glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
		gluCylinder(quadratic,0.3f,0.1f,0.5f,32,32);
		glPopMatrix();

		drawbike();
		glPopMatrix();

		//	printf("%lf %lf\n",xval-lx*2,yval+1-ly*2);
		glPushMatrix();
		if(jump)
			glTranslatef(xval-lx*2,yval+1-ly*2,pheight+0.9);
		else
			glTranslatef(xval-lx*2,yval+1-ly*2,_terrain->getHeight(terrain_point_x(xval-lx*2),terrain_point_y(yval+1-ly*2))*scale+0.9);
		glRotatef(bike_angle,0,0,1);
		glRotatef(bike_roll,0,1,0);
		drawbike();
		glPopMatrix();

		glPushMatrix();
		if(jump)
			glTranslatef(xval,yval+1,pheight+0.9);
		else
			glTranslatef(xval,yval+1,t_height+0.9);
		glRotatef(bike_yaw,0,0,1);
		glTranslatef(0.22,0,0);
		glScalef(0.5,1,1);
		char a[2][100];
		int j;
		glLineWidth(2);
		strcpy(a[0],"Score: ");
		if(timeee==10000-10)
		{
			glTranslatef(-1,0,2);
			strcpy(a[0],"TIME UP!! Score: ");
		}

		sprintf(a[1], "%d", collected);
		strcat(a[0],a[1]);
		glPushMatrix();
		glScalef(0.02,0.02,0.02);
		glColor3f(0,0,0);
		glRotatef(90,1,0,0);
		//	glTranslatef(xval,yval+1, t_height+0.9);
		for(j=0;j<(int)strlen(a[0]);j++)
		{
			glutStrokeCharacter(GLUT_STROKE_ROMAN,a[0][j]);
		}
		glPopMatrix();
		glLineWidth(1);
		glRotatef(bike_pitch,1,0,0);
		glRotatef(bike_roll,0,1,0);
		ZCylinder(0.07,1.2);
		glPopMatrix();

		glPushMatrix();
		if(jump)
			glTranslatef(xval,yval+1,pheight+0.9);
		else
			glTranslatef(xval,yval+1,t_height+0.9);
		glRotatef(bike_yaw,0,0,1);
		glRotatef(bike_pitch,1,0,0);
		glRotatef(bike_roll,0,1,0);
		glTranslatef(-0.22,0,0);
		glScalef(0.5,1,1);
		ZCylinder(0.07,1.2);
		glPopMatrix();

		glPushMatrix();
		if(jump)
			glTranslatef(xval,yval+1,pheight+0.9);
		else
			glTranslatef(xval,yval+1,t_height+0.9);
		glRotatef(bike_yaw,0,0,1);
		glRotatef(bike_pitch,1,0,0);
		glRotatef(bike_roll,0,1,0);
		//	glTranslatef(-0.22,0,0);
		glTranslatef(0,0,1.2);
		glRotatef(90,0,1,0);
		ZCylinder(0.05,0.5);
		glColor3f(0,0,0);
		glTranslatef(0,0,0.4);
		ZCylinder(0.06,0.25);
		glTranslatef(0,0,-0.4);
		glColor3f(1,0,0);
		glRotatef(-180,0,1,0);
		ZCylinder(0.05,0.4);
		glColor3f(0,0,0);
		glTranslatef(0,0,0.4);
		ZCylinder(0.06,0.25);
		glPopMatrix();

		glPushMatrix();
		if(jump)
			glTranslatef(xval-lx*2,yval+1-ly*2,pheight+0.9);
		else
			glTranslatef(xval-lx*2,yval+1-ly*2,_terrain->getHeight(terrain_point_x(xval-lx*2),terrain_point_y(yval+1-ly*2))*scale+0.9);
		glRotatef(bike_yaw,0,0,1);
		float ang=atan((t_height-scale*_terrain->getHeight(terrain_point_x(xval-lx*2),terrain_point_y(yval+1-ly*2)))*1.0/sqrt(pow(ly*2,2)+pow(lx*    2,2)))*180.0/PI;
		if(jump==1)
			ang=0;
		glRotatef(ang,1,0,0);
		glRotatef(bike_roll,0,1,0);
		glScalef(0.5,0.5,0.5);
		glBegin(GL_QUADS);

		glColor3f(0,0,1);
		glVertex3f(0.55,RADIUS_WHEEL,0);
		glVertex3f(0.55,RADIUS_WHEEL*1.1+2,0);
		glVertex3f(0.55,RADIUS_WHEEL*1.6+2,1.1*1.5);
		glVertex3f(0.55,RADIUS_WHEEL*0.4,1.1*1.5);

		glVertex3f(-0.55,RADIUS_WHEEL,0);
		glVertex3f(-0.55,RADIUS_WHEEL*1.1+2,0);
		glVertex3f(-0.55,RADIUS_WHEEL*1.6+2,1.1*1.5);
		glVertex3f(-0.55,RADIUS_WHEEL*0.4,1.1*1.5);

		glVertex3f(-0.55,RADIUS_WHEEL,0);
		glVertex3f(0.55,RADIUS_WHEEL,0);
		glVertex3f(0.55,RADIUS_WHEEL*0.4,1.1*1.5);
		glVertex3f(-0.55,RADIUS_WHEEL*0.4,1.1*1.5);

		glVertex3f(-0.55,RADIUS_WHEEL+2,0);
		glVertex3f(0.55,RADIUS_WHEEL+2,0);
		glVertex3f(0.55,RADIUS_WHEEL*1.4+2,1.1*1.5);
		glVertex3f(-0.55,RADIUS_WHEEL*1.4+2,1.1*1.5);

		glColor3f(0,0,1);
		glVertex3f(-0.55,RADIUS_WHEEL*0.4,1.1*1.5);
		glVertex3f(0.55,RADIUS_WHEEL*0.4,1.1*1.5);
		glVertex3f(0.55,RADIUS_WHEEL*1.4+2,1.1*1.5);
		glVertex3f(-0.55,RADIUS_WHEEL*1.4+2,1.1*1.5);
		glEnd();
		glPopMatrix();
	}
	glutSwapBuffers(); // Make it all visible
} 

void processNormalKeys(unsigned char key, int xx, int yy)
{
	if (key == ESC || key == 'q' || key == 'Q') { cleanup();exit(0);}
	else if(key=='7')
		camera_view='7';
	else if(key=='6')
		camera_view='6';
	else if(key=='5')
		camera_view='5';
	else if (key=='4')
		camera_view='4';
	else if (key=='1')
		camera_view='1';
	else if (key=='2')
		camera_view='2';
	else if (key=='3')
		camera_view='3';
	else if(key=='a')
		tilt=1;
	else if(key=='d')
		tilt=-1;
	else if(key=='h')
		headlight=(headlight+1)%2;
}

void pressSpecialKey(int key, int xx, int yy)
{
	flag_down=1;
	switch (key) {
		case GLUT_KEY_UP : deltaMove = 1.0; break;
		case GLUT_KEY_DOWN :{backlight=1 ;deltaMove = -1.0; break;}
		case GLUT_KEY_LEFT : ldeltaMove = -1.0; break;
		case GLUT_KEY_RIGHT : ldeltaMove = 1.0; break;
	}
//	printf("------> lx=%f ly=%f x=%f y=%f angle=%lf\n",lx,ly,x,y,angle*180.0/3.14);
} 

void releaseSpecialKey(int key, int x, int y) 
{
	flag_down=0;
	switch (key) {
		case GLUT_KEY_UP : deltaMove = 0.0; break;
		case GLUT_KEY_DOWN :{backlight=0; deltaMove = 0.0; break;}
		case GLUT_KEY_LEFT : ldeltaMove = 0; break;
		case GLUT_KEY_RIGHT : ldeltaMove = 0; break;
	}
} 

//----------------------------------------------------------------------
// Process mouse drag events
// 
// This is called when dragging motion occurs. The variable
// angle stores the camera angle at the instance when dragging
// started, and deltaAngle is a additional angle based on the
// mouse movement since dragging started.
//----------------------------------------------------------------------
void mouseMove(int x, int y) 
{ 	
	if (isDragging) { // only when dragging
		// update the change in angle
		deltaAngle = (x - xDragStart) * 0.005;

		// camera's direction is set to angle + deltaAngle
		lx = -sin(angle + deltaAngle);
		ly = cos(angle + deltaAngle);
	}
}

void mouseButton(int button, int state, int x, int y) 
{
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) { // left mouse button pressed
			isDragging = 1; // start dragging
			xDragStart = x; // save x where button first pressed
		}
		else  { /* (state = GLUT_UP) */
			angle += deltaAngle; // update camera turning angle
			isDragging = 0; // no longer dragging
		}
	}
}

//----------------------------------------------------------------------
// Main program  - standard GLUT initializations and callbacks
//----------------------------------------------------------------------
int main(int argc, char **argv) 
{
	printf("\n\
-----------------------------------------------------------------------\n\
  OpenGL Sample Program:\n\
  - Drag mouse left-right to rotate camera\n\
  - Hold up-arrow/down-arrow to move camera forward/backward\n\
  - q or ESC to quit\n\
-----------------------------------------------------------------------\n");

	// general initializations
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 400);
	glutCreateWindow("OpenGL/GLUT Sample Program");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT2);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
	Image* image = loadBMP("vtr.bmp");
	_textureId = loadTexture(image);
	delete image;
	_terrain = loadTerrain("heightmap.bmp", 10);
	scale = 100.0f / max(_terrain->width() - 1, _terrain->length() - 1);
	for(int i=0;i<10;i++)
	{
		trees[i].y=(float)(rand()%((int)(_terrain->length()*scale)))-scale*(float)((_terrain->length()-1)/2);
		trees[i].x=(float)(rand()%((int)(_terrain->width()*scale)))-scale*(float)((_terrain->width()-1)/2);
		trees[i].z=_terrain->getHeight(terrain_point_x(trees[i].x),terrain_point_y(trees[i].y));
		trees[i].z*=scale;
	}
	for(int i=0;i<20;i++)
	{
		fossils[i].x=(float)(rand()%((int)(_terrain->width()*scale)))-scale*(float)((_terrain->width()-1)/2);
		fossils[i].y=(float)(rand()%((int)(_terrain->length()*scale)))-scale*(float)((_terrain->length()-1)/2);
		fossils[i].flag=1;
		fossils[i].z=_terrain->getHeight(terrain_point_x(fossils[i].x),terrain_point_y(fossils[i].y));
		fossils[i].z*=scale;
	}
	glutReshapeFunc(changeSize); // window reshape callback
	glutDisplayFunc(renderScene); // (re)display callback
	glutIdleFunc(update); // incremental update 
	glutMouseFunc(mouseButton); // process mouse button push/release
	glutMotionFunc(mouseMove); // process mouse dragging motion
	glutKeyboardFunc(processNormalKeys); // process standard key clicks
	glutSpecialFunc(pressSpecialKey); // process special key pressed
	glutSpecialUpFunc(releaseSpecialKey); // process special key release
	glEnable(GL_DEPTH_TEST);
	glutMainLoop();
	return 0; // this is just to keep the compiler happy
}

