// OpenGL Jet program.
// ECE8893, Georgia Tech,

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

//GLfloat updateRate = 1.0; // Change this later

//Wendy

GLfloat rotate_x = 0;
GLfloat rotate_y = 0;
GLfloat rotate_z = 0;

bool rotx = false;
bool roty = false;
bool rotz = false;

GLfloat scalex = 0;
GLfloat scaley = 0;
GLfloat scalez = 0;


static int lightnum = 0;   //see if works
GLfloat light_position[6][3];
GLenum GL_LIGHT;

class Vertex
{
 public:
       GLfloat x;
       GLfloat y;
       GLfloat z;
// public:
       Vertex ()
       {
         GLfloat x=0;
         GLfloat y=0;
         GLfloat z=0;
       }
       
       Vertex (GLfloat x0, GLfloat y0, GLfloat z0)
       {
          GLfloat a=x;
          GLfloat b=y;
          GLfloat c=z;
       }

};


class Face 
{ 
   private:

   Vertex vertexA;
   Vertex vertexB;
   Vertex vertexC;

   public:
   Face ()
   {
   vertexA.x = 0;
   vertexA.y = 0;
   vertexA.z = 0;
   vertexB.x = 0;
   vertexB.y = 0;
   vertexB.z = 0;
   vertexC.x = 0;
   vertexC.y = 0;
   vertexC.z = 0;
   }


   Face (Vertex a, Vertex b, Vertex c)
   {
    vertexA = a;
    vertexB = b;
    vertexC = c;
   }
};   
   
class Normal : public Vertex
{
   public:
   
   Normal(GLfloat i, GLfloat j, GLfloat k): Vertex (x, y, z){};
   Normal(): Vertex(){};
};


class Material
{ 
  public:
    
  GLfloat Ns;
  GLfloat Tr;
  GLfloat Ka[3];
  GLfloat Kd[3];
  GLfloat Ke[3];
  GLfloat Ks[3];

  Material (GLfloat Ns0, GLfloat Tr0, GLfloat * Ka0, GLfloat * Kd0, GLfloat * Ke0, GLfloat * Ks0 )
{ 
  Ns = Ns0;
  Tr = Tr0;
  for (int i = 0; i < 3; ++i)
    {
       Ka[i] = Ka0[i];
       Kd[i] = Kd0[i];
       Ke[i] = Ke0[i];
       Ks[i] = Ks0[i];
    }
}
   Material ()
  {
   Ns = 0;
   Tr = 0;
   for (int i = 0; i < 3; ++i)
   {
        Ka[i] = 0;
        Kd[i] = 0;
        Ke[i] = 0;
	Ks[i] = 0;
   }
  }
};

class triangle
{
 public:
       int a;
       int b;
       int c;
       int normalIdxA;
       int normalIdxB;
       int normalIdxC;
       int material;
   
	int texA;
	int texB;
	int texC;


       triangle (int a0, int b0, int c0, int normalIdxA0, int normalIdxB0, int normalIdxC0, int material0, int texA0, int texB0, int texC0)
      {
        a = a0;
        b = b0;
        c = c0;
        normalIdxA = normalIdxA0;
        normalIdxB = normalIdxB0;
        normalIdxC = normalIdxC0;
	material = material0;

    	texA = texA0;
	texB = texB0;
	texC = texC0;
       }
     
	triangle ()
      {
        a = 0;
        b = 0;
        c = 0;
        normalIdxA = 0;
        normalIdxB = 0;
        normalIdxC = 0;
	material = 0;

	texA = 0;
	texB = 0;
	texC = 0;

       }


};



//printf("%c\n","H");

//wendy vector
vector<Vertex> verticies;
vector<Normal> normals;
vector<Material> materials;
vector<Face> faces;
vector<triangle> triangles;

//wendy copy of loadmodel
void loadModel()
{
  //open file
  ifstream inputFile ("jet.obj");
  string line;
  int currMaterial = 0;

  if(inputFile.is_open())
    { 
      while(inputFile.good())
        {
          
          getline(inputFile, line);

          if(line[0] == 'u')
            { 
              sscanf(line.c_str(), "usemtl %d", &currMaterial);
            }

          //process vertex
          else if(line[0] == 'v' && line[1] == ' ')
            {
              
              Vertex vert;
              sscanf(line.c_str(), "%*s %f %f %f", &vert.x, &vert.y, &vert.z);
              verticies.push_back(vert);
              //cout << "Vertex! ";
            }
          //process normal
          else if(line[0] == 'v' && line[1] == 'n')
            { 
              Normal normal;
              sscanf(line.c_str(), "%*s %f %f %f", &normal.x, &normal.y, &normal.z);
              normals.push_back(normal);
              //cout<<normal.x<<endl;
            }
          //process texture
          //else if(line[0] == 'v' && line[1] == 't')
            //{
              //texture_t tex;
              //sscanf(line.c_str(), "%*s %f %f", &tex.u, &tex.v);
              //texCords.push_back(tex);
            //}
          //process face
          else if(line[0] == 'f')
            {
              size_t first;
              size_t second;

              triangle tri;

              tri.material = currMaterial;

              first = line.find("/");
              second = line.find("/", first+1);


              if(second - first == 1) //then there isn't texture data
                {
                  sscanf(line.c_str(), "%*s %d//%d %d//%d %d//%d" , &tri.a, &tri.normalIdxA, &tri.b, &tri.normalIdxB, &tri.c, &tri.normalIdxC);
		
                }
              else //there is texture data
                {
                  sscanf(line.c_str(), "%*s %d/%d/%d %d/%d/%d %d/%d/%d" , &tri.a, &tri.texA, &tri.normalIdxA, &tri.b, &tri.texB, &tri.normalIdxB, &tri.c, &tri.texC, &tri.normalIdxC);
		//cout<<tri.a<<endl;
                }
              cout << "Triangle: " << tri.a << ", " << tri.b << ", " << tri.c << endl;
              //cout << line << endl;

              triangles.push_back(tri);
		//cout<<"triangle size"<<triangles.size()<<endl;
			cout<<"popa"<<triangles.back().a<<endl;
            }
        }
    }
}

// wendy copy end of load model



//wendy commit
// code to read the obj file
/*
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
*/


// Code to read material file
void ReadMtl()  //wendy
{  //cout<<"!!!"<<endl;   does not come in here!!!!
  ifstream ifs("jet.mtl");
  if (!ifs) return;
  cout.precision(4);

  Material material;  //wendy
	
  while(ifs)
    {
      string header;
      ifs >> header;
      if (header == string("newmtl"))
        {
          int n;
          ifs >> n;
          //cout << header << " " << n << endl;
        }
      else if (header == string("Ns"))
        {
          GLfloat Ns;
          ifs >> Ns;
          material.Ns = Ns;

         // cout << header << " " << Ns << endl;
        }
      else if (header == string("Tr"))
        {
          GLfloat Tr;
          ifs >> Tr;
          material.Tr = Tr;
          
          // more here
        }
       else if (header == string ("Ka"))
	{
	 GLfloat Ka[3];
         ifs >> Ka[0] >> Ka[1] >> Ka[2];
	 for (int i = 0; i < 3; ++i)
		material.Ka[i] = Ka [i];
	}

	else if (header == string ("Kd"))
	{
	 GLfloat Kd[3];
	 ifs >> Kd[0] >> Kd[1] >> Kd[2];
	 for (int i = 0; i < 3; ++i)
	 material.Kd[i] = Kd[i];
	}

	else if (header == string ("Ke"))
	{
	 GLfloat Ke[3];
	 ifs >> Ke[0] >> Ke[1] >> Ke[2];
         for (int i = 0; i < 3; ++i)
		material.Ke[i] = Ke[i];
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
  
  specular[0] = materials[materialId].Ks[0];
  specular[1] = materials[materialId].Ks[1];
  specular[2] = materials[materialId].Ks[2];
  specular[3] = 1 - materials[materialId].Tr;
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  
  ambient[0] = materials[materialId].Ka[0];
  ambient[1] = materials[materialId].Ka[1];
  ambient[2] = materials[materialId].Ka[2];
  ambient[3] = 1 - materials[materialId].Tr;
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
  
  diffuse[0] = materials[materialId].Kd[0];
  diffuse[1] = materials[materialId].Kd[1];
  diffuse[2] = materials[materialId].Kd[2];
  diffuse[3] = 1 - materials[materialId].Tr;
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
  
  emmisive[0] = materials[materialId].Ke[0];
  emmisive[1] = materials[materialId].Ke[1];
  emmisive[2] = materials[materialId].Ke[2];
  emmisive[3] = 1 - materials[materialId].Tr;
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emmisive);
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &materials[materialId].Ns);
}

void drawModel()
{
  // Your code to draw the model here
   glBegin(GL_TRIANGLES); 

	vector<triangle>::iterator it;
    

	for (it = triangles.begin(); it < triangles.end(); it++)
	{
		//if (useMaterials)
                       // cout<<"!!!"<<endl;
			//cout<<"size of triangles"<<triangles.size()<<endl;
			
		cout<<"it .a"<<it->a<<endl;
		cout<<"triangleback"<<triangle.back().material<<endl; 
          		setMaterial((*it).material);
		
	glVertex3f(verticies[(*it).a-1].x,verticies[(*it).a-1].y,verticies[(*it).a-1].z);
	glNormal3f(normals[(*it).normalIdxA-1].x,normals[(*it).normalIdxA-1].y,normals[(*it).normalIdxA-1].z);
	glVertex3f(verticies[(*it).b-1].x,verticies[(*it).b-1].y,verticies[(*it).b-1].z);
	glNormal3f(normals[(*it).normalIdxB-1].x,normals[(*it).normalIdxB-1].y,normals[(*it).normalIdxB-1].z);
	glVertex3f(verticies[(*it).c-1].x,verticies[(*it).c-1].y,verticies[(*it).c-1].z);
	glNormal3f(normals[(*it).normalIdxC-1].x,normals[(*it).normalIdxC-1].y,normals[(*it).normalIdxC-1].z);	
	}

	glEnd();

}


void init(void)
{ // Called from main
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
}


GLfloat updateRate = 20;

void timer(int)
{
  // Adjust rotation angles as needed here

//wendy

	if (rotx) rotate_x += 3.6;
	if (roty) rotate_y += 3.6;
	if (rotz) rotate_z += 3.6;   
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
//wendy
    //glScalef(1.2, 1.2, 1.2);
	glScalef(scalex, scaley, scalez); 

	glRotatef(rotx, 1.0, 0.0, 0.0);

	glRotatef(roty, 0.0, 1.0, 0.0);

	glRotatef(rotz, 0.0, 0.0, 1.0);


    // Draw th emodel
    drawModel();
    // Swap the double buffers
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    // height = h;
    // width = w;
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (GLfloat)w / (GLfloat)h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}



void keyboard (unsigned char key, int x, int y) 
{
  // Keystroke processing here
	switch (key)
	{
	case 'x':  
		rotx = (!rotx);
		break;
	case 'y':  
		roty= (!roty);
		break;
	case 'z':  
		rotz= (!rotz);
		break;
	case 'S':
		scalex+= 0.01;
		scaley+= 0.01;
		scalez+= 0.01;
		break;
	case 's':
		scalex-= 0.01;
		scaley-= 0.01;
		scalez-= 0.01;
		break;
	case 'L':
	       {
		
		if (lightnum == 6) 
			cout<< "Light sourses already reach 6. No more light source."<<endl;
		else {
			lightnum++;
			cout << "Please choose the light location in x/y/z. x="<<endl;
			cin >> light_position[lightnum][0];
			cout <<"y=" << endl;
			cin >> light_position[lightnum][1];
			cout <<"z=" << endl;
			cin >>light_position[lightnum][2];
			glLightfv (GL_LIGHT, GL_POSITION, light_position[lightnum]);
			glEnable (GL_LIGHT);
		       }
		break;
		}

	case 'l':
		{
		if (lightnum == 0)
			cout << "No light source!" << endl;
		else
		{
			glLightfv (GL_LIGHT, GL_POSITION,  light_position[lightnum]);
			glDisable (GL_LIGHT);
			lightnum--;
			cout << "Light source:" <<light_position[lightnum][0]<<','<<light_position[lightnum][1]<<','<<light_position[lightnum][2]<<"removed."<<endl;
		}
		break;
		}
	case 'q':
		exit(0);
	default:
		return;
	}
	glutPostRedisplay();
		 	
}


int main(int argc, char* argv[])
{
 
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Jet Display");
    loadModel(); // Uncomment when the model reader is coded.
    ReadMtl();  //wendy
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

