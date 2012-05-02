/*
 * DisplayImage.cpp
 *
 *  Created on: 01.05.2012
 *      Author: stas
 */
#include <cstdlib>
#include <cstdio> 
 
#include <cv.h>
#include <highgui.h>

#include "TextDetection.h"

IplImage* image = 0;
IplImage* src = 0;



int main(int argc, char* argv[])
{
        // имя картинки задаётся первым параметром
        const char* filename = argc == 2 ? argv[1] : "Image0.jpg";
        // получаем картинку
        image = cvLoadImage(filename,1);
        // клонируем картинку 
        src = cvCloneImage(image);

        printf("[i] image: %s\n", filename);
        assert( src != 0 );

        // окно для отображения картинки
        cvNamedWindow("original",CV_WINDOW_AUTOSIZE);
	
	CvRect* rects;
	int len = getComp(image, &rects);
	CvRect r = getRegion(rects, len);
	
	cvRectangle(image, cvPoint(r.x, r.y), cvPoint(r.x + r.width, r.y + r.height), CV_RGB(125,200,100), 1, 8, 0);
	
        // показываем картинку
        cvShowImage("original",image);

        // выводим в консоль информацию о картинке
        printf( "[i] channels:  %d\n",        image->nChannels );
        printf( "[i] pixel depth: %d bits\n",   image->depth );
        printf( "[i] width:       %d pixels\n", image->width );
        printf( "[i] height:      %d pixels\n", image->height );
        printf( "[i] image size:  %d bytes\n",  image->imageSize );
        printf( "[i] width step:  %d bytes\n",  image->widthStep );	
	
	
        // ждём нажатия клавиши
        cvWaitKey(0);

        // освобождаем ресурсы
        cvReleaseImage(& image);
        cvReleaseImage(&src);
        // удаляем окно
        cvDestroyWindow("original");
        return 0;
}
