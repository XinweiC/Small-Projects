// Distributed two-dimensional Discrete FFT transform
// Xinwei Chen
// ECE8893 Project 1

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <signal.h>
#include <math.h>
#include <mpi.h>
#include "Complex.h"
#include "InputImage.h"

using namespace std;
void T(Complex * h1, Complex * h2, long width, long height);
void Transform1D(Complex* h, long w, Complex* H);
void ReverseTransform1D(Complex* h, long w, Complex* H);
void Transform2D(const char* inputFN);
void Transform1DColumn(Complex* h, long width, long height, Complex* H);
int nCpus;
int myrank;
long width;
long height;

void Transform2D(const char* inputFN) { // Do the 2D transform here.
										// 1) Use the InputImage object to read in the Tower.txt file and
										//    find the width/height of the input image.
	InputImage image(inputFN);
	width = image.GetWidth();
	height = image.GetHeight();
	// 2) Use MPI to find how many CPUs in total, and which one
	//    this process is
	MPI_Comm_size(MPI_COMM_WORLD, &nCpus);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	// 3) Allocate an array of Complex object of sufficient size to
	//    hold the 2d DFT results (size is width * height)
	long size = width * height;
	Complex * array = new Complex[size];
	// 4) Obtain a polonger to the Complex 1d array of input data
	// 5) Do the individual 1D transforms on the rows assigned to your CPU
	Complex * h = image.GetImageData();
	Complex * H = new Complex[width * height];
	Complex * midArray = new Complex[width * height];
	for (long i = myrank * height / nCpus;
			i < myrank * height / nCpus + height / nCpus; i++) {
		Transform1D(&h[width * i], width, &H[width * i]);
	}

	// 6) Send the resultant transformed values to the appropriate
	//    other processors for the next phase.
	// 6a) To send and receive columns, you might need a separate
	//     Complex array of the correct size.
	//T(H,array,width,height);

	Complex * sendArray = new Complex[size];
	for (long i = myrank * height / nCpus;
			i < myrank * height / nCpus + height / nCpus; i++) {
		for (long j = 0; j < width; j++) {

			sendArray[width * (i) + j].real = H[width * i + j].real;
			sendArray[width * (i) + j].imag = H[width * i + j].imag;
			midArray[width * (i) + j].real = H[width * i + j].real;
			midArray[width * (i) + j].imag = H[width * i + j].imag;
		}
	}
	Complex ** receiveArray = new Complex *[nCpus];
	for (long i = 0; i < nCpus; i++)
		receiveArray[i] = new Complex[size];
	MPI_Request* req = new MPI_Request[nCpus];
	for (long i = 0; i < nCpus; i++)
		long rc = MPI_Irecv((char *) (receiveArray[i]), size * sizeof(Complex),
				MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &req[i]);
	for (long desitantion = 0; desitantion < nCpus; desitantion++) {
		MPI_Send((char *) sendArray, size * sizeof(Complex), MPI_CHAR,
				desitantion, 0, MPI_COMM_WORLD);
	}

	for (long count = 0; count < nCpus; count++) {
		MPI_Status status;
		MPI_Wait(&req[count], &status);
		for (long i = status.MPI_SOURCE * height / nCpus;
				i < status.MPI_SOURCE * height / nCpus + height / nCpus; i++) {
			for (long j = 0; j < width; j++) {
				midArray[width * (i) + j].real = receiveArray[count][width * (i)
						+ j].real;
				midArray[width * (i) + j].imag = receiveArray[count][width * (i)
						+ j].imag;
			}
		}

	}

	T(midArray, array, width, height);
	Complex * temp = new Complex[size];

	for (long i = myrank * height / nCpus;
			i < myrank * height / nCpus + height / nCpus; i++) {
		Transform1D(&array[width * i], width, &temp[width * i]);
	}
	T(temp, H, width, height);

	Complex * sendArray1 = new Complex[size];

	for (long i = myrank * width / nCpus;
			i < myrank * width / nCpus + width / nCpus; i++) {
		for (long j = 0; j < height; j++) {
			sendArray1[j * width + i] = H[j * width + i];
			array[j * width + i] = H[j * width + i];
		}
	}

	receiveArray = new Complex *[nCpus];
	for (long i = 0; i < nCpus; i++)
		*(receiveArray + i) = new Complex[size];
	req = new MPI_Request[nCpus];

	if (myrank == 0) {

		for (long i = 0; i < nCpus; i++)
			long rc = MPI_Irecv((char *) (receiveArray[i]),
					size * sizeof(Complex), MPI_CHAR, MPI_ANY_SOURCE, 0,
					MPI_COMM_WORLD, &req[i]);
	}

	long rc = MPI_Send((char *) sendArray1, size * sizeof(Complex), MPI_CHAR, 0,
			0, MPI_COMM_WORLD);

	if (myrank == 0) {

		for (long count = 0; count < nCpus; count++) {
			MPI_Status status;
			MPI_Wait(&req[count], &status);

			for (long i = status.MPI_SOURCE * width / nCpus;
					i < status.MPI_SOURCE * width / nCpus + width / nCpus;
					i++) {
				for (long j = 0; j < height; j++) {
					array[j * width + (i)] = receiveArray[count][j * width + i];
				}
			}
		}

		image.SaveImageData("MyAfter2d.txt", array, width, height);
                ifstream out;
                        string str = "MyAfter2d.txt";
                        out.open(str.c_str(), ios::in);
                        string line;
                        while (!out.eof()) {
                                std::getline(out, line);
                                cout << line << endl;
                        }
                        out.close();
		 for (int desitantion = 1; desitantion < nCpus; desitantion++)
         {
                        MPI_Send((char *) array, size * sizeof(Complex), MPI_CHAR,
                                        desitantion, 0, MPI_COMM_WORLD);
                }
		
	

	}

	else
		{
			MPI_Status status;
			MPI_Recv((char *) (array), size * sizeof(Complex), MPI_CHAR, MPI_ANY_SOURCE, 0,MPI_COMM_WORLD,&status);
		}
		

	MPI_Barrier (MPI_COMM_WORLD);
	 {

		Complex * H = new Complex[width * height];
		Complex * midArray = new Complex[width * height];
		for (long i = myrank * height / nCpus;
				i < myrank * height / nCpus + height / nCpus; i++) {
			//Transform1D(&h[width*i], width, &H[width*i]);
			ReverseTransform1D(&array[width * i], width, &H[width * i]);
		}

		Complex * sendArray = new Complex[size];
		for (long i = myrank * height / nCpus;
				i < myrank * height / nCpus + height / nCpus; i++) {
			for (long j = 0; j < width; j++) {

				sendArray[width * (i) + j].real = H[width * i + j].real;
				sendArray[width * (i) + j].imag = H[width * i + j].imag;
				midArray[width * (i) + j].real = H[width * i + j].real;
				midArray[width * (i) + j].imag = H[width * i + j].imag;
			}
		}
		Complex ** receiveArray = new Complex *[nCpus];
		for (long i = 0; i < nCpus; i++)
			receiveArray[i] = new Complex[size];
		MPI_Request* req = new MPI_Request[nCpus];
		for (long i = 0; i < nCpus; i++)
			long rc = MPI_Irecv((char *) (receiveArray[i]),
					size * sizeof(Complex), MPI_CHAR, MPI_ANY_SOURCE, 0,
					MPI_COMM_WORLD, &req[i]);
		for (long desitantion = 0; desitantion < nCpus; desitantion++) {
			MPI_Send((char *) sendArray, size * sizeof(Complex), MPI_CHAR,
					desitantion, 0, MPI_COMM_WORLD);
		}

		for (long count = 0; count < nCpus; count++) {
			MPI_Status status;
			MPI_Wait(&req[count], &status);
			for (long i = status.MPI_SOURCE * height / nCpus;
					i < status.MPI_SOURCE * height / nCpus + height / nCpus;
					i++) {
				for (long j = 0; j < width; j++) {
					midArray[width * (i) + j].real = receiveArray[count][width
							* (i) + j].real;
					midArray[width * (i) + j].imag = receiveArray[count][width
							* (i) + j].imag;
				}
			}

		}

		T(midArray, array, width, height);
		Complex * temp = new Complex[size];

		for (long i = myrank * height / nCpus;
				i < myrank * height / nCpus + height / nCpus; i++) {
			ReverseTransform1D(&array[width * i], width, &temp[width * i]);
		}
		T(temp, H, width, height);

		Complex * sendArray1 = new Complex[size];

		for (long i = myrank * width / nCpus;
				i < myrank * width / nCpus + width / nCpus; i++) {
			for (long j = 0; j < height; j++) {
				sendArray1[j * width + i] = H[j * width + i];
				array[j * width + i] = H[j * width + i];
			}
		}

		receiveArray = new Complex *[nCpus];
		for (long i = 0; i < nCpus; i++)
			*(receiveArray + i) = new Complex[size];
		req = new MPI_Request[nCpus];

		if (myrank == 0) {

			for (long i = 0; i < nCpus; i++)
				long rc = MPI_Irecv((char *) (receiveArray[i]),
						size * sizeof(Complex), MPI_CHAR, MPI_ANY_SOURCE, 0,
						MPI_COMM_WORLD, &req[i]);
		}

		long rc = MPI_Send((char *) sendArray1, size * sizeof(Complex),
				MPI_CHAR, 0, 0, MPI_COMM_WORLD);

		if (myrank == 0) {

			for (long count = 0; count < nCpus; count++) {
				MPI_Status status;
				MPI_Wait(&req[count], &status);

				for (long i = status.MPI_SOURCE * width / nCpus;
						i < status.MPI_SOURCE * width / nCpus + width / nCpus;
						i++) {
					for (long j = 0; j < height; j++) {
						array[j * width + (i)] = receiveArray[count][j * width
								+ i];
					}
				}
			}
			image.SaveImageData("MyAfterInverse.txt", array, width, height);

			ifstream out;
			string str = "MyAfterInverse.txt";
			out.open(str.c_str(), ios::in);
			string line;
			while (!out.eof()) {
				std::getline(out, line);
				cout << line << endl;
			}
			out.close();

		}
	}

	// Create the helper object for reading the image
	// Step (1) in the comments is the line above.
	// Your code here, steps 2-9
}

void T(Complex * h1, Complex * h2, long width, long height) {
	for (long i = 0; i < width; i++)
		for (long j = 0; j < height; j++) {
			h2[width * j + i] = h1[width * i + j];
		}
}

void Transform1D(Complex* h, long w, Complex* H) {
	// Implement a simple 1-d DFT using the double summation equation
	// given in the assignment handout.  h is the time-domain input
	// data, w is the width (N), and H is the output array.
	for (long n = 0; n < w; ++n) {
		for (long k = 0; k < w; ++k) {

			Complex temp;
			temp.real = cos(2 * M_PI * k * n / w);
			temp.imag = -sin(2 * M_PI * k * n / w);
			temp = temp * h[k];
			H[n] = H[n] + temp;

		}
	}
}
void ReverseTransform1D(Complex* h, long w, Complex* H) {
	// Implement a simple 1-d DFT using the double summation equation
	// given in the assignment handout.  h is the time-domain input
	// data, w is the width (N), and H is the output array.
	for (long n = 0; n < w; ++n) {
		for (long k = 0; k < w; ++k) {

			Complex temp;
			temp.real = cos(2 * M_PI * k * n / w);
			temp.imag = sin(2 * M_PI * k * n / w);
			temp = temp * h[k];
			H[n] = H[n] + temp;

		}
		H[n] = H[n] * ((double) 1 / w);

	}
}

int main(int argc, char** argv) {
	string fn("Tower.txt"); // default file name
	if (argc > 1)
		fn = string(argv[1]);  // if name specified on cmd line
	// MPI initialization here
	MPI_Init(&argc, &argv);
	Transform2D(fn.c_str()); // Perform the transform.
	MPI_Finalize();
}

