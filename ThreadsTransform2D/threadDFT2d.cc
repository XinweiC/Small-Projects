// Threaded two-dimensional Discrete FFT transform
// Xinwei Chen
// ECE8893 Project 2


#include <iostream>
#include <string>
#include <math.h>
#include <pthread.h>
#include "Complex.h"
#include "InputImage.h"

// You will likely need global variables indicating how
// many threads there are, and a Complex* that points to the
// 2d image being transformed.

using namespace std;

int nThreads=16;
long width;
long height;
string s="Tower.txt";
InputImage image(s.c_str()); 
pthread_mutex_t mutex;
pthread_mutex_t mutex1d;
pthread_cond_t cond;
int count;
Complex * h;
// Function to reverse bits in an unsigned integer
// This assumes there is a global variable N that is the
// number of points in the 1D transform.
//

unsigned ReverseBits(unsigned v)
{ //  Provided to students
  unsigned n = width; // Size of array (which is even 2 power k value)
  unsigned r = 0; // Return value
   
  for (--n; n > 0; n >>= 1)
    {
      r <<= 1;        // Shift return value
      r |= (v & 0x1); // Merge in next bit
      v >>= 1;        // Shift reversal value
    }
  return r;
}

// GRAD Students implement the following 2 functions.
// Undergrads can use the built-in barriers in pthreads.

// Call MyBarrier_Init once in main
void MyBarrier_Init()// you will likely need some parameters)
{
    count=nThreads;
   pthread_mutex_init(&mutex,0);
   pthread_mutex_init(&mutex1d,0);
   pthread_cond_init(&cond, 0);

}

// Each thread calls MyBarrier after completing the row-wise DFT
void MyBarrier() // Again likely need parameters
{
    pthread_mutex_lock(&mutex);
    count = count - 1;
    if(count == 0){
        pthread_cond_broadcast(&cond);
        count=nThreads;
    }
    else
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
}
void RTransform1D(Complex* h, int N)
{

pthread_mutex_lock(&mutex1d);
	for(unsigned i=0;i<N;i++)
	{
		if(i<ReverseBits(i))
		{
		Complex temp = h[i];
		h[i]=h[ReverseBits(i)];
		h[ReverseBits(i)]=temp;
		}
	}

	for(int np=2;np<=N;np=np*2)
		for(int w=0;w<N/np;w++)
			for(int k=0;k<np/2;k++)
			{
				Complex wn;
                        	wn.real = cos(2 * M_PI * k  / np);
                        	wn.imag = sin(2 * M_PI * k  / np);
				int index = np*w+k;
			 	int pair = np*w+k+np/2;
				Complex temp = h[index];
				h[index]=h[index]+h[index+np/2]*wn;
				h[pair]=temp-h[index+np/2]*wn;
			}
     for(int i=0;i<N;i++)
	h[i]=h[i]*((double) 1/ N);
pthread_mutex_unlock(&mutex1d);


}
    
void Transform1D(Complex* h, int N)
{

pthread_mutex_lock(&mutex1d);
	for(unsigned i=0;i<N;i++)
	{
		if(i<ReverseBits(i))
		{
		Complex temp = h[i];
		h[i]=h[ReverseBits(i)];
		h[ReverseBits(i)]=temp;
		}
	}

	for(int np=2;np<=N;np=np*2)
		for(int w=0;w<N/np;w++)
			for(int k=0;k<np/2;k++)
			{
				Complex wn;
                        	wn.real = cos(2 * M_PI * k  / np);
                        	wn.imag = -sin(2 * M_PI * k  / np);
				int index = np*w+k;
			 	int pair = np*w+k+np/2;
				Complex temp = h[index];
				h[index]=h[index]+h[index+np/2]*wn;
				h[pair]=temp-h[index+np/2]*wn;
			}
pthread_mutex_unlock(&mutex1d);

}
void T(Complex * h1, Complex * h2, long width, long height) {
	for (long i = 0; i < width; i++)
		for (long j = 0; j < height; j++) {
			h2[width * j + i] = h1[width * i + j];
		}
}
void* Transform2DTHread(void* v)
{ // This is the thread startign point.  "v" is the thread number
  // Calculate 1d DFT for assigned rows
  // wait for all to complete
  // Calculate 1d DFT for assigned columns
  // Decrement active count and signal main if all complete
   long myrank=(long)v;
    for (long i = myrank * height / nThreads;
         i < myrank * height / nThreads + height / nThreads; i++) {	
             Transform1D(&h[width * i], width);
	}
    MyBarrier();
    if(myrank==0)
   {
    Complex* array = new Complex[width*height];
    T(h, array, width, height); 
   
   h = array;
   }
   MyBarrier();
	for (long i = myrank * height / nThreads;
         i < myrank * height / nThreads + height / nThreads; i++) {
		Transform1D(&h[width * i], width);
	}

MyBarrier();
   if(myrank==0)
   {
    Complex* array = new Complex[width*height];
    T(h, array, width, height);
   
   h = array;
   image.SaveImageData("Tower-DFT2D.txt", h, width, height);
   }
	
MyBarrier();

    for (long i = myrank * height / nThreads;
         i < myrank * height / nThreads + height / nThreads; i++) {	
            RTransform1D(&h[width * i], width);
	}
    MyBarrier();
    if(myrank==0)
   {
    Complex* array = new Complex[width*height];
    T(h, array, width, height); 
   
   h = array;
   }
   MyBarrier();
//	Complex * temp = new Complex[width*height];
    
	for (long i = myrank * height / nThreads;
         i < myrank * height / nThreads + height / nThreads; i++) {
		RTransform1D(&h[width * i], width);
	}
 
MyBarrier();
   if(myrank==0)
   {
    Complex* array = new Complex[width*height];
    T(h, array, width, height);
   
   h = array;
   }


      if(myrank==0)
{

     image.SaveImageData("MyAfterInverse.txt", h, width, height);
/*
			ifstream out;
			string str = "Tower-DFT2DInverse.txt";
			out.open(str.c_str(), ios::in);
			string line;
			while (!out.eof()) {
				std::getline(out, line);
				cout << line << endl;
			}
			out.close();
*/
}
  return 0;
}
void Transform2D(const char* inputFN) 
{ // Do the 2D transform here
 MyBarrier_Init();
 // Create the helper object for reading the image
  // Create the global pointer to the image array data

 image =*( new InputImage(inputFN));
width = image.GetWidth();
	height = image.GetHeight();
	// 2) Use MPI to find how many CPUs in total, and which one
	// 	//    this process is
	// 		MPI_Comm_size(MPI_COMM_WORLD, &nCpus);
	// 			MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	
    h=image.GetImageData();
    pthread_t * pt=new pthread_t[nThreads];
    for (long i = 0; i < nThreads; ++i)
    {
        // pThread variable (output param from create)
        pthread_create((pt)+i, 0, Transform2DTHread, (void*)i);
    }
   for (long i = 0; i < nThreads; ++i)
    {   
    //    pthread_t pt; // pThread variable (output param from create)
        pthread_join(*(pt+i),NULL);
    } 
  // pthread_join(thread1_id, NULL);
  // Create 16 threads
  // Wait for all threads complete
  // Write the transformed data
}

int main(int argc, char** argv)
{
 string fn("Tower.txt"); 

  // InputImage image(inputFN); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
  // MPI initialization here
  Transform2D(fn.c_str()); // Perform the transform.
}  
  

  
