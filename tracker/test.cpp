#include <cstdio>

#include "cv.h"
#include "highgui.h"

#include "TextTracker.h"
int init = 0;
int need_track = 0;
CvRect rec;
CvRect rec2;

CvPoint pt1 = cvPoint(0, 0);
CvPoint pt2 = cvPoint(0, 0);

void setRect()
{
	int w = std::abs(pt1.x - pt2.x);
	int h = std::abs(pt1.y - pt2.y);
	int x = std::min(pt1.x, pt2.x);
	int y = std::min(pt1.y, pt2.y);
	rec2 = cvRect(x, y, w, h);
}


//обработка событий с мыши
void on_mouse(int event, int x, int y, int flags, void* param) {
	if (event == CV_EVENT_LBUTTONDOWN) 
	{
		pt1 = cvPoint(x, y);
	}
	if (event == CV_EVENT_RBUTTONDOWN)
	{
		pt2 = cvPoint(x, y);
	}
	setRect();
}

int main(int argc, char** argv)
{
	CvCapture* capture = 0;
	//Получаем видеопоток с камеры или видеофайла, в зависимости от входныхпараметров
	if (argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
		capture = cvCaptureFromCAM(argc == 2 ? argv[1][0] - '0' : 0);
	else if (argc == 2)
		capture = cvCaptureFromAVI(argv[1]);
	if (!capture) {
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}
	cvNamedWindow("Text Detector demo", 0);
	//Устанавливаем функцию обработчик мыши, сама функция находится выше
	cvSetMouseCallback("Text Detector demo", on_mouse, 0);
	for(;;)
	{
		IplImage* frame = 0;
		//Получаем фрейм из видеопотока
		frame = cvQueryFrame(capture);
		if (!frame)
		{
			break;
		}
		if(!init)
		{
			initResources(frame);
			init++;
		}
		if (need_track)
		{
			rec = TextTracking(frame, 1);
		}
		else
		{
			rec = TextTracking(frame);
		}
		need_track = 0;
		cvRectangle(frame, cvPoint(rec.x, rec.y), cvPoint(rec.x + rec.width, rec.y + rec.height), CV_RGB(255,0,0), 3, 8, 0);
		cvShowImage("Text Detector demo", frame);
		int c = cvWaitKey(50);
		if ((char) c == 27)
			break;
		switch ((char) c) {
		case 'r':
			need_track = 1;
			break;
		case 'c':
			break;
		}		
	}
	cvReleaseCapture(&capture);
	cvDestroyWindow("Text Detector demo");
	return 0;
}
