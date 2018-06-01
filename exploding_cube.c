

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <GLUT/glut.h>
#elif __linux__
  #include <GL/gl.h>
  #include <GL/glut.h> 
#endif

#define SQUARE_LENGTH 1
#define LIFE 255
#define VELOCITY_LIMIT 30
#define DEFAULT_CAM 1000.0

/*
* SPACE will explode the cube if rotating
* W to move the camera closer
* S to move the camera farther
*
* Following key do not work if particles are in the shape of a cube:
* Z when particles are moving will revert it back to a cube
* X will stop it from moving, X again to continue time again
* A to move the camera left
* D to move the camera right
*/
typedef struct particle{
    float x;
    float y;
    float z;
    float vi[3];
    int lifespan;
    int mass;
    int r;
    int g;
    int b;
    int time;
    float gravity;
    float damping;
    int active;
} particle;

typedef struct particleSystem{
    //preferably with a even root 3 for even square -27k = 30
    particle particles[9000];
    float x;
    float y;
    float z;
    int active;
} particleSystem;


int   cube_exploded = 0;
float angle = 0.0; // camera rotation angle
particleSystem p;
size_t size = sizeof(p.particles) / sizeof(p.particles[0]);
int stopSpinning = 1;
int reverseTime = 0;
int stopTime = 0;
int camera = DEFAULT_CAM;
int gravityOn = 0;
int dampingOn = 0;

//
// Light sources
GLfloat  light0Amb[4] =  { 1.0, 0.6, 0.2, 1.0 };
GLfloat  light0Dif[4] =  { 1.0, 0.6, 0.2, 1.0 };   
GLfloat  light0Spec[4] = { 0.0, 0.0, 0.0, 1.0 };   
GLfloat  light0Pos[4] =  { 0.0, 0.0, 0.0, 1.0 };
GLfloat  light1Amb[4] =  { 0.0, 0.0, 0.0, 1.0 };
GLfloat  light1Dif[4] =  { 1.0, 1.0, 1.0, 1.0 };   
GLfloat  light1Spec[4] = { 1.0, 1.0, 1.0, 1.0 };   
GLfloat  light1Pos[4] =  { 0.0, 5.0, 5.0, 0.0 };

void display (void);
void keyboard (unsigned char, int, int);
void reshape (int, int);
void idle (void);
void explode_cube (void);
void updateParticleSystem(void);
void initalizeParticles(void);
void randomizeVelocity(void);
void drawParticles(void);
void drawCube(particle);
void reactivate(void);

void display (void)
{
  int i;

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity ();
//
// Place the camera
  glTranslatef (0.0, 0.0, -camera);
  glRotatef (angle, 0.0, 1.0, 0.0);
//
// If no explosion, draw cube
  if (!cube_exploded)
  {
    glEnable (GL_LIGHTING);
    glDisable (GL_LIGHT0);
    glEnable (GL_LIGHT1);
    glEnable (GL_DEPTH_TEST);
    glutSolidCube (cbrtf(size));
  } else {
    if(!stopTime) updateParticleSystem();
    drawParticles();
  }
   
  glutSwapBuffers ();
}


void keyboard (unsigned char key, int x, int y)
{
  switch (key)
  {
    //explodes the cube
    case ' ':
      if(!cube_exploded)
        explode_cube();
      reactivate();
      stopSpinning = 0;
      stopTime = 0;
    break;
    //stop all particles from updating
    case 'x':
        if(!stopTime)
            stopTime = 1;
        else stopTime = 0;
    break;
    //adds gravity and damping
    case 'c':
        if(!cube_exploded){
            if(!gravityOn){
                dampingOn = 1;            
                gravityOn = 1;
            } 
            else {
                dampingOn = 0;            
                gravityOn = 0;
            }
        }         
    break;
    //revert it back to cube
    case 'z':
        if(!reverseTime)
            reverseTime = 1;
        reactivate();
    break;
    //camera closer
    case 'w':
        if(camera - VELOCITY_LIMIT > 0) camera -= VELOCITY_LIMIT;
    break;
    //camera further
    case 's':
        camera += VELOCITY_LIMIT;
    break;
    //camera rotate right
    case 'd':
        if(!stopSpinning) angle -= 0.3;
    break;
    //camera rotate left
    case 'a':
        if(!stopSpinning) angle += 0.3;
    break;
    //press esc
    case 27:
      exit (0);
    break;
  }
}



void reshape (int w, int h)
{
  glViewport (0.0, 0.0, (GLfloat) w, (GLfloat) h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (45.0, (GLfloat) w / (GLfloat) h, 0.1, camera);
  glMatrixMode (GL_MODELVIEW);
}

void idle (void)
{
  if(stopSpinning) angle += 0.3;  /* Always continue to rotate the camera */
  glutPostRedisplay ();
}


//reset any values changed
void explode_cube(void)
{
  //disable background light
  glDisable (GL_LIGHT0);
  glDisable (GL_LIGHTING);
  glDisable (GL_LIGHT1);
  reverseTime = 0;
  cube_exploded = 1;
  if(!p.active) initalizeParticles();
  else randomizeVelocity();
  //printf ("BOOM!\n");
}


//preset all the cubes position
void initalizeParticles(void)
{
    int i = 0;
    float cap = cbrtf(size);
    float baseX = 0;
    float baseY = 0;
    float baseZ = 0;
    while(i < size){
        //set color
        p.particles[i].r = 255;
        p.particles[i].g = 255;
        p.particles[i].b = 255;
                
        //set position
        if(baseX >= cap){
            baseX = 0;
            baseZ += 1;
        }
        if(baseZ >= cap){
            baseZ = 0;
            baseY += 1;
        }        
        p.particles[i].x = baseX;
        p.particles[i].y = baseY;
        p.particles[i].z = baseZ;
        baseX += 1;
      
        //set to active
        p.particles[i].active = 1;
        p.particles[i].lifespan = LIFE;
        p.particles[i].time = 0;
        
        //set mass
        p.particles[i].mass = 1;
        
        //set damping/gravity
        p.particles[i].damping = 0.88;
        p.particles[i].gravity = -1.3;
        
        //increment
        i++;
    }
    //set velocity
    randomizeVelocity();
       
    p.active = 1;
}

//randomize all particle velocities
void randomizeVelocity(void){
    int flip = 1;
    int i = 0;
    while(i < size){
        //Even Clustering - Direction based on quadrants
        // x+, y+, z+        
        p.particles[i].vi[0] = (rand() % VELOCITY_LIMIT);
        p.particles[i].vi[1] = (rand() % VELOCITY_LIMIT);
        p.particles[i].vi[2] = (rand() % VELOCITY_LIMIT);
        // x-, y+, z+        
        if(i % 8 == 1){
        p.particles[i].vi[0] *= -1;

        }
        // x+, y-, z+        
        if(i % 8 == 2){
        p.particles[i].vi[1] *= -1;

        }
        // x+, y+, z-        
        if(i % 8 == 3){
        p.particles[i].vi[2] *= -1;

        }
        // x-, y-, z+        
        if(i % 8 == 4){
        p.particles[i].vi[0] *= -1;
        p.particles[i].vi[1] *= -1;

        }
        // x-, y+, z-        
        if(i % 8 == 5){
        p.particles[i].vi[0] *= -1;
        p.particles[i].vi[2] *= -1;
        }
        // x+, y-, z-        
        if(i % 8 == 6){
        p.particles[i].vi[1] *= -1;
        p.particles[i].vi[2] *= -1;

        }
        // x-, y-, z-        
        if(i % 8 == 7){
        p.particles[i].vi[0] *= -1;
        p.particles[i].vi[1] *= -1;
        p.particles[i].vi[2] *= -1;

        }

        /* Current Algorithm, Odd Clustering
        p.particles[i].vi[0] = (rand() % VELOCITY_LIMIT) * flip;
        if((int) p.particles[i].vi[0] % 2 == 1) p.particles[i].vi[0] *= -1;
        p.particles[i].vi[1] = (rand() % VELOCITY_LIMIT) * flip;
        if((int) p.particles[i].vi[1] % 3 == 1) p.particles[i].vi[1] *= -1;
        p.particles[i].vi[2] = (rand() % VELOCITY_LIMIT) * flip;
        if((int) p.particles[i].vi[2] % 3 == 1) p.particles[i].vi[2] *= -1;
        flip *= -1;
        */
        i++;
    }
}

//individually update all particles position
void updateParticleSystem(void)
{
    int i = 0;
    while(i < size){
        if(p.particles[i].active){
            if(!reverseTime){
                //change by velocity 
                /*
                p.particles[i].x += p.particles[i].vi[0];
                p.particles[i].y += p.particles[i].vi[1];
                p.particles[i].z += p.particles[i].vi[2];
                */
                //change velocity by damping
                /*                
                if(dampingOn){
                p.particles[i].vi[0] *= p.particles[i].damping;
                p.particles[i].vi[1] *= p.particles[i].damping;
                p.particles[i].vi[2] *= p.particles[i].damping;
               
                } */              
                //change color
                if(p.particles[i].r > 0)p.particles[i].r -= abs(p.particles[i].vi[0]) + 1;
                if(p.particles[i].r < 0)p.particles[i].r = 0;
                if(p.particles[i].g > 0)p.particles[i].g -= abs(p.particles[i].vi[1]) + 1;
                if(p.particles[i].g < 0)p.particles[i].g = 0;
                if(p.particles[i].b > 0)p.particles[i].b -= abs(p.particles[i].vi[2]) + 1;
                if(p.particles[i].b < 0)p.particles[i].b = 0;
           
                //increase time
                p.particles[i].time++;
                if(p.particles[i].lifespan - p.particles[i].time < 0) p.particles[i].active = 0;
             }
             else {
                if(p.particles[i].time > 0){               
                    //change by velocity
                    /*
                    p.particles[i].x -= p.particles[i].vi[0];
                    p.particles[i].y -= p.particles[i].vi[1];
                    p.particles[i].z -= p.particles[i].vi[2];
                    */
                    /*
                    if(dampingOn){
                        //change velocity by damping
                        p.particles[i].vi[0] /= p.particles[i].damping;
                        p.particles[i].vi[1] /= p.particles[i].damping;
                        p.particles[i].vi[2] /= p.particles[i].damping;
                    }
                    */
                    //change color
                    if(p.particles[i].r < 255)p.particles[i].r += abs(p.particles[i].vi[0]) + 1;
                    if(p.particles[i].r > 255)p.particles[i].r = 255;
                    if(p.particles[i].g < 255)p.particles[i].g += abs(p.particles[i].vi[1]) + 1;
                    if(p.particles[i].g > 255)p.particles[i].g = 255;
                    if(p.particles[i].b < 255)p.particles[i].b += abs(p.particles[i].vi[2]) + 1;
                    if(p.particles[i].b > 255)p.particles[i].b = 255;
                
                    p.particles[i].time--;                    
                    if(p.particles[i].time <= 0){
                        stopSpinning = 1;
                        cube_exploded = 0;
                    }                 
                } 
             }        
        }
        i++;
    }
}

//iterate through all particles
void drawParticles(void){
    int i = 0;
    while(i < size){
        if(p.particles[i].active){    
            //settings
            glPushMatrix();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glColor3f((GLfloat) (p.particles[i].r/255.0), (GLfloat) (p.particles[i].g/255.0), (GLfloat) (p.particles[i].b/255.0));
            //draw cube
            drawCube(p.particles[i]);
            glPopMatrix();
        }
        i++;
    }
    glutSwapBuffers();
    glutPostRedisplay();
}

//draw every particle as a 1 pixel cube
void drawCube(particle particle){
    //with damping
    if(dampingOn){
        glTranslatef(p.x + particle.vi[0] * (particle.time * particle.damping), 
        p.y + particle.vi[1] * (particle.time * particle.damping), 
        p.z + particle.vi[2] * (particle.time * particle.damping));
    
    } else {
    //displacement
    glTranslatef(p.x + particle.vi[0] * particle.time, p.y + particle.vi[1] * particle.time, p.z + particle.vi[2] * particle.time);
    }   
    glBegin(GL_TRIANGLE_FAN);
        //upper surface
        glVertex3f(particle.x, particle.y, particle.z);
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y, particle.z);
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y + SQUARE_LENGTH, particle.z);
        glVertex3f(particle.x, particle.y + SQUARE_LENGTH, particle.z);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
        //bottom surface
        glVertex3f(particle.x, particle.y, particle.z + SQUARE_LENGTH);
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y, particle.z + SQUARE_LENGTH);
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y + SQUARE_LENGTH, particle.z + SQUARE_LENGTH);        
        glVertex3f(particle.x, particle.y + SQUARE_LENGTH, particle.z + SQUARE_LENGTH);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
        //north surface
        glVertex3f(particle.x, particle.y + SQUARE_LENGTH, particle.z);
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y + SQUARE_LENGTH, particle.z);
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y + SQUARE_LENGTH, particle.z + SQUARE_LENGTH);
        glVertex3f(particle.x, particle.y + SQUARE_LENGTH, particle.z + SQUARE_LENGTH);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
        //south surface
        glVertex3f(particle.x, particle.y, particle.z);
        glVertex3f(particle.x, particle.y, particle.z + SQUARE_LENGTH);
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y, particle.z + SQUARE_LENGTH);
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y, particle.z);
    glEnd();    

    glBegin(GL_TRIANGLE_FAN);
        //west surface
        glVertex3f(particle.x, particle.y, particle.z);
        glVertex3f(particle.x, particle.y, particle.z + SQUARE_LENGTH);
        glVertex3f(particle.x, particle.y + SQUARE_LENGTH, particle.z + SQUARE_LENGTH);
        glVertex3f(particle.x, particle.y + SQUARE_LENGTH, particle.z);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
        //east surface -
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y, particle.z);
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y + SQUARE_LENGTH, particle.z);
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y + SQUARE_LENGTH, particle.z + SQUARE_LENGTH);
        glVertex3f(particle.x + SQUARE_LENGTH, particle.y, particle.z + SQUARE_LENGTH);
    glEnd();
}

void reactivate(void){
    int i = 0;
    while(i < size){
        if(!p.particles[i].active) p.particles[i].active = 1;
        i++;
    }
}

int main (int argc, char *argv[])
{
  //initialize particleSystem - center the cube
  p.x = -cbrtf(size)/2;
  p.y = -cbrtf(size)/2;
  p.z = -cbrtf(size)/2;
  p.active = 0;

  //Screen Setup
  srand(0);
  glutInit (&argc, argv);
  glutInitWindowPosition (0, 0);
  glutInitWindowSize (1280, 1024);
  glutInitDisplayMode (GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow ("Particle explosion");
  glutKeyboardFunc (keyboard);
  glutIdleFunc (idle);
  glutDisplayFunc (display);
  glutReshapeFunc (reshape);
  glEnable (GL_LIGHT0);
  glEnable (GL_LIGHT1);
  glLightfv (GL_LIGHT0, GL_AMBIENT, light0Amb);
  glLightfv (GL_LIGHT0, GL_DIFFUSE, light0Dif); 
  glLightfv (GL_LIGHT0, GL_SPECULAR, light0Spec);
  glLightfv (GL_LIGHT0, GL_POSITION, light0Pos);
  glLightfv (GL_LIGHT1, GL_AMBIENT, light1Amb);
  glLightfv (GL_LIGHT1, GL_DIFFUSE, light1Dif);
  glLightfv (GL_LIGHT1, GL_SPECULAR, light1Spec);
  glLightfv (GL_LIGHT1, GL_POSITION, light1Pos);
  glLightModelf (GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glEnable (GL_NORMALIZE);
  glutMainLoop ();
  return 0;
}
