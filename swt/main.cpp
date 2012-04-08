//источник - https://sites.google.com/site/roboticssaurav/strokewidthnokia

#include <fstream>
#include "TextDetection.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

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

	IplImage * byteQueryImage = loadByteImage( argv[1] );

	if ( !byteQueryImage ) {
		printf ( "couldn't load query image\n" );
		return -1;
	}

	// Detect text in the image
	IplImage * output = textDetection ( byteQueryImage, atoi(argv[3]) );
        std::cout << "textDetection completed" << std::endl;

 	cvReleaseImage ( &byteQueryImage );
	cvSaveImage ( argv[2], output );
	cvReleaseImage ( &output );
	return 0;
}
