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
  IplImage* image = cvLoadImage(name);

  if ( !image ) return 0;
  return image;
}

int main ( int argc, char** argv ) { 

	if ( ( argc != 3 ) ){
			printf ( "usage: %s imagefile resultImage \n", argv[0] );
			return -1;
	}

	IplImage * image = loadByteImage( argv[1] );

	if ( !image ) {
		printf ( "couldn't load query image\n" );
		return -1;
	}

     CvRect r = getRegion (image);
     CvScalar c = cvScalar(0, 255, 0);
     cvRectangle(image ,cvPoint(r.x, r.y), cvPoint(r.x + r.width, r.y + r.height ), c, 2);

     cvSaveImage(argv[2], image );

     return 0;
}
