#include <cv.h>
#include <highgui.h>
#include <stdlib.h>
#include <stdio.h>

IplImage* image = 0;
IplImage* dst = 0;
int MAX_CAM_CADR = 10;
int main(int argc, char* argv[]) {
	CvCapture* capture = 0;
	capture = cvCaptureFromCAM(1);
	if (!capture)
		return 1;
	cvNamedWindow("Motion", 1);
	IplImage * image[MAX_CAM_CADR];
	CvScalar color;
	CvPoint cv1;
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.3f, 1.3f, 0, 1, 8);
	cv1.x = 70;
	cv1.y = 120;
	color = CV_RGB(255, 0, 255);
	int i, j;
	for (;;) {
		IplImage* image1;
		if (!cvGrabFrame(capture))
			break;
		image1 = cvRetrieveFrame(capture);
		cvPutText(image1, "Press, when you ready", cv1, &font, color);
		cvShowImage("Motion", image1);
		if (cvWaitKey(10) >= 0)
			break;
	}
//обучаем фон
	for (i = 0; i < MAX_CAM_CADR; i++) {
		if (!cvGrabFrame(capture))
			break;
		image[i] = cvRetrieveFrame(capture);
		cvShowImage("Motion", image[i]);
		if (cvWaitKey(20) >= 0)
			break;
	}
//Теперь надо посчитать m
//для каждых
	float *m = new float[image[0]->height * image[0]->width * 3];
	int i1, j1;
	uchar* ptr;
	for (i = 0; i < image[0]->height; i++)
		for (j = 0; j < image[0]->width; j++)
			for (j1 = 0; j1 < 3; j1++) {
				m[(i * image[0]->width + j) * 3 + j1] = 0;
				for (i1 = 0; i1 < MAX_CAM_CADR; i1++) {
					ptr = (uchar*) (image[i1]->imageData);
					m[(i * image[0]->width + j) * 3 + j1] += ptr[j * 3
							+ i * image[i1]->widthStep + j1];
				}
				m[(i * image[0]->width + j) * 3 + j1] = m[(i * image[0]->width
						+ j) * 3 + j1] / MAX_CAM_CADR;
			}
//Раз запомнили - можно входить в цикл обработки изображений
	float porog = 60.0;
	float k = 0;
	for (;;) {
		IplImage* image1;
		if (!cvGrabFrame(capture))
			break;
		image1 = cvRetrieveFrame(capture);
		ptr = (uchar*) (image1->imageData);
		for (i = 0; i < image[0]->height; i++)
			for (j = 0; j < image[0]->width; j++) {
				k = 0;
				for (j1 = 0; j1 < 3; j1++)
					k += abs(
							m[(i * image1->width + j) * 3 + j1]
									- ptr[j * 3 + i * image1->widthStep + j1]);
				if (k <= porog)
					for (j1 = 0; j1 < 3; j1++) {
						if (j1 == 0)
							ptr[j * 3 + i * image1->widthStep + j1] = 0;
						else
							ptr[j * 3 + i * image1->widthStep + j1] = 100;
					}
			}
		cvShowImage("Motion", image1);
		if (cvWaitKey(10) >= 0)
			break;
	}
//Удаляем все
	cvReleaseCapture(&capture);
	cvDestroyWindow("Motion");
	delete m;

}
