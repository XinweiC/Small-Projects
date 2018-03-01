// OpenGL Jet program.
// ECE8893, Georgia Tech, Fall 2012

#include <iostream>

#ifdef LINUX
//Linux Headers
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#ifdef OSX
// MAC Headers
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif

#ifdef WINDOWS
//Windows Headers
#include <Windows.h>
#include <gl/GL.h>
#include <gl/glut.h>
#endif

#include <fstream>
#include <vector>

using namespace std;

GLfloat updateRate = 1.0; // Change this later

// code to read the obj file
void Read()
{
  ifstream ifs("jet.obj");
  if (!ifs) return;
  while(ifs)
    {
      string header;
      ifs >> header;
      if (header == string("mtllib"))
        {
          string mtllib;
          ifs >> mtllib;
          cout << header << " " << mtllib << endl;
        }
      else if (header == string("v"))
        {
          float v[3];
          ifs >> v[0] >> v[1] >> v[2];
          cout << header << " " << v[0] << " " << v[1] << " " << v[2] << endl;
        }
      else if (header == string("vn"))
        {
          // more here
        }
    }
}

// Code to read material file
void ReadMtl()
{
  ifstream ifs("jet.mtl");
  if (!ifs) return;
  cout.precision(4);
  while(ifs)
    {
      string header;
      ifs >> header;
      if (header == string("newmtl"))
        {
          int n;
          ifs >> n;
          cout << header << " " << n << endl;
        }
      else if (header == string("Ns"))
        {
          float Ns;
          ifs >> Ns;
          cout << header << " " << Ns << endl;
        }
      else if (header == string("Tr"))
        {
          
          // more here
        }
    }
  
}

void setMaterial(int materialId)
{
  float specular[4];
  float ambient[4];
  float diffuse[4];
  float emmisive[4];
  float shiny;
  
  // Ugly but works
  
  specular[0] = materials[materialId].sr;
  specular[1] = materials[materialId].sg;
  specular[2] = materials[materialId].sb;
  specular[3] = 1 - materials[materialId].transparency;
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  
  ambient[0] = materials[materialId].ar;
  ambient[1] = materials[materialId].ag;
  ambient[2] = materials[materialId].ab;
  ambient[3] = 1 - materials[materialId].transparency;
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
  
  diffuse[0] = materials[materialId].dr;
  diffuse[1] = materials[materialId].dg;
  diffuse[2] = materials[materialId].db;
  diffuse[3] = 1 - materials[materialId].transparency;
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
  
  emmisive[0] = materials[materialId].er;
  emmisive[1] = materials[materialId].eg;
  emmisive[2] = materials[materialId].eb;
  emmisive[3] = 1 - materials[materialId].transparency;
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emmisive);
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &materials[materialId].shiny);
}

void drawModel()
{
  // Your code to draw the model here
}


void init(void)
{ // Called from main
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
}


double updateRate = 20;

void timer(int)
{
  // Adjust rotation angles as needed here
  // Then tell glut to redisplay
  glutPostRedisplay();
  // And reset tht timer
  glutTimerFunc(1000.0 / updateRate, timer, 0);
}

void display(void)
{
  //glClearColor(0.0, 0.0, 0.0, 1.0); // black background
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 15.0, -25.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    // rotations and scaling here
    glScalef(1.2, 1.2, 1.2);

    // Draw th emodel
    drawModel();
    // Swap the double buffers
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    height = h;
    width = w;
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (GLfloat)w / (GLfloat)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}



void keyboard (unsigned char key, int x, int y) 
{
  // Keystroke processing here
}


int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Jet Display");
    //loadModel(); // Uncomment when the model reader is coded.
    //loadMaterials();  // Uncomment when the materials reader is coded
    init();
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutKeyboardFunc (keyboard);
    glutReshapeFunc(reshape);
    // Compute the update rate here...
    glutTimerFunc(1000.0 / updateRate, timer, 0);
    glutMainLoop();
    return 0;
}

