//источник - https://sites.google.com/site/roboticssaurav/strokewidthnokia
#include <fstream>
#include "TextDetection.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <sys/time.h>

#include <cstdio>
#include <iostream>

using namespace cv;
using namespace std;

IplImage* loadByteImage ( const char* name )
{
  namedWindow("left", 1);
  IplImage* image = cvLoadImage(name);

  if ( !image ) return 0;

  cvCvtColor (image, image, CV_BGR2RGB );
  return image;
}

int main ( int argc, char** argv ) { 

	if ( ( argc != 4 ) ){
			printf ( "usage: %s imagefile resultImage darkText(bool)\n", argv[0] );
			return -1;
	}
	struct timeval startpoint;
	gettimeofday(&startpoint, NULL);
	struct timeval start, end;
	long mtime, seconds, useconds;
	gettimeofday(&start, NULL);
	seconds  = start.tv_sec  - startpoint.tv_sec;
	useconds = start.tv_usec - startpoint.tv_usec;

	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

	printf("TextDetection start at %ld\n", mtime);

	IplImage * byteQueryImage = loadByteImage( argv[1] );

	if ( !byteQueryImage ) {
		printf ( "couldn't load query image\n" );
		return -1;
	}

	// Detect text in the image
	IplImage * output = textDetection ( byteQueryImage, atoi(argv[3]) );
        std::cout << "textDetection completed" << std::endl;

	gettimeofday(&end, NULL);
	seconds  = end.tv_sec  - startpoint.tv_sec;
	useconds = end.tv_usec - startpoint.tv_usec;
	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
	printf("TextDetection finish at %ld\n", mtime);
	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;

	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;

	printf("textDetection time : %ld ms\n", mtime);

 	cvReleaseImage ( &byteQueryImage );
	cvSaveImage ( argv[2], output );
	cvReleaseImage ( &output );


	IplImage * byteQueryImage1 = loadByteImage( argv[1] );

	if ( !byteQueryImage1 ) {
		printf ( "couldn't load query image\n" );
		return -1;
	}


	CvRect** regions;
	getComp(byteQueryImage1, regions);
	return 0;
}
