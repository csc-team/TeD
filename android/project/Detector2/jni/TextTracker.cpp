#include <cmath>
#include <cstdlib>
#include <ctype.h>

#include "cv.h"
#include "highgui.h"

#include "TextDetection.h"
#include "TextTracker.h"

#define sqr(x) ((x)*(x))

const bool HARRIS = true;

const int MAX_COUNT = 500;
const int WIN_SIZE = 10;
const int SCALE = 50;
const int LIMIT = 1000;
const int MIN_POINTS_TO_TRACK = 4;
const int BLOCK_SIZE = 10;
const int BORDER = 3;

const float AMOUNT = 0.9f;
const float LOST = 0.6f;
const float INF = 100500.0f;
const float ZERO = 0.0f;
const float MAX_DIST = 30.0f;

const double HARRIS_K = 0.025;
const double QUALITY = 0.15;
const double FEATURE_DISTANCE = 5;

IplImage* image = 0; 
IplImage* grey = 0; 
IplImage* prev_grey = 0; 
IplImage* pyramid = 0;
IplImage* prev_pyramid = 0; 
IplImage* swap_image = 0;

CvPoint2D32f* points = 0;
CvPoint2D32f* prev_points = 0;
CvPoint2D32f* swap_points = 0;
CvPoint2D32f* shifts = 0;
CvPoint2D32f origin = cvPoint2D32f(ZERO, ZERO);
CvPoint2D32f state = cvPoint2D32f(ZERO, ZERO);

char* status = 0;
int flags = 0;
int count = 0;

int init_count = 0;
int init_width = 0;
int init_height = 0;
/*
 ***
 	Эвклидово расстояние между двумя точками типа CvPoint2D32f
 ***
*/

inline float dist(const CvPoint2D32f a, const CvPoint2D32f b = cvPoint2D32f(ZERO, ZERO))
{
	return sqrt( sqr((a.x) - (b.x)) + sqr((a.y) - (b.y)) );
}


/*
 ***
 	Выделение необходимых ресурсов. Нужно вызвать перед первым запуском
 ***
 */
void initResources(IplImage* frame)
{
	image = cvCreateImage(cvGetSize(frame), 8, 3);
	image -> origin = frame -> origin;
	grey = cvCreateImage(cvGetSize(frame), 8, 1);
	prev_grey = cvCreateImage(cvGetSize(frame), 8, 1);
	pyramid = cvCreateImage(cvGetSize(frame), 8, 1);
	prev_pyramid = cvCreateImage(cvGetSize(frame), 8, 1);
	swap_image = cvCreateImage(cvGetSize(frame), 8, 1);

	points = (CvPoint2D32f*) cvAlloc(
			MAX_COUNT * sizeof(prev_points));
	prev_points = (CvPoint2D32f*) cvAlloc(
			MAX_COUNT * sizeof(prev_points));
	swap_points = (CvPoint2D32f*) cvAlloc(
			MAX_COUNT * sizeof(prev_points));
	shifts = (CvPoint2D32f*) cvAlloc(
			MAX_COUNT * sizeof(prev_points));
	status = (char*) cvAlloc(MAX_COUNT);
};
/*
 ***
 	Освобождение выделенных ресурсов
 ***
 */
void releaseResources()
{
	cvReleaseImage(&image);
	cvReleaseImage(&grey);
	cvReleaseImage(&prev_grey);	
	cvReleaseImage(&pyramid);
	cvReleaseImage(&prev_pyramid);
	cvReleaseImage(&swap_image);
	cvFree(&points);
	cvFree(&prev_points);
	cvFree(&swap_points);
	cvFree(&status);
};
/*
***
	Отслеживает текст на текущем изображении.
	Возвращает прямоугольник содержащий текстовую область
***
*/
CvRect TextTracking(IplImage* frame, int mode)  // если mode != 0 нужно вызвать детектор
{
	CvRect r = cvRect(0, 0, 0, 0);

	float minShift = INF;
	float minX = INF;
	float minY = INF;
	float maxX = ZERO;
	float maxY = ZERO;
	

	int i = 0, k = 0, c = 0, min_index = 0;
	if (!frame)
	{
		return r;
	}
	//копирование и перевод в чёрно-белое изображение
	cvCopy(frame, image, 0);
	cvCvtColor(image, grey, CV_BGR2GRAY);
	if (mode) {
		count = 0;
		// mode = 1 => запускаем детектом
		//Находим текст детектором
		r = getRegion(image); 
		r = cvRect(r.x + BORDER, r.y + BORDER, r.width + BORDER, r.height + BORDER);
		if(r.width * r.height > LIMIT)
		{
			findKeyPoints(r);
			init_count = count;
			init_width = r.width;
			init_height = r.height;
		}
		else
		{
			r = cvRect(0, 0, 0, 0);
		}
		origin.x = r.x;
		origin.y = r.y;
	} 
	else if (count > MIN_POINTS_TO_TRACK) 
	{
		cvCalcOpticalFlowPyrLK( prev_grey, grey, prev_pyramid, pyramid,
					prev_points, points, count, cvSize(WIN_SIZE, WIN_SIZE), 3,status, 0,
					cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20,0.03), flags);
		flags |= CV_LKFLOW_PYR_A_READY;
		//перебираем все точки
		for (i = k = 0; i < count; i++) 
		{
			if (!status[i])
			{
				continue;
			}

			float distance = dist(points[i], prev_points[i]);
			
			if (distance > MAX_DIST)
			{
				continue;
			}
			
			shifts[k].x = shifts[i].x + points[i].x - prev_points[i].x;
			shifts[k].y = shifts[i].y + points[i].y - prev_points[i].y;
			
			float shift = dist(shifts[k]);
			if(shift < minShift)
			{
				minShift = shift;
				min_index = k;
			}

			points[k++] = points[i];
			if(points[i].x > maxX)
			{
				maxX = points[i].x;
			}
			if(points[i].x < minX)
			{
				minX = points[i].x;
			}
			if(points[i].y > maxY)
			{
				maxY = points[i].y;
			}
			if(points[i].y < minY)
			{
				minY = points[i].y;
			}				
		}
		count = k;
		state.x = origin.x + shifts[min_index].x;
		state.y = origin.y + shifts[min_index].y;

		float num = static_cast<float>(count);
		float diffX = (maxX - minX) / SCALE; 
		float diffY = (maxY - minY) / SCALE;
		float curX1 = minX;
		float curX2 = maxX;
		float curY1 = minY;
		float curY2 = maxY;
		if(count < static_cast<int>(std::floor(init_count * LOST)))
		{
			r = cvRect(static_cast<int>(std::floor(state.x)), 
			   	   static_cast<int>(std::floor(state.y)),
			   	   init_width, 
			   	   init_height);
			findKeyPoints(r);
		}

		else
		{
			r = cvRect(static_cast<int>(std::floor(curX1)), 
			 	   static_cast<int>(std::floor(curY1)),
			   	   static_cast<int>(std::floor(curX2 - curX1)), 
			   	   static_cast<int>(std::floor(curY2 - curY1)));
		}
	} 
	IplImage* swap_temp;
	CvPoint2D32f* swap_points;
	CV_SWAP(prev_grey, grey, swap_temp);
	CV_SWAP(prev_pyramid, pyramid, swap_temp);
	//Меняем местами точки - новые становятся старыми на следующем шаге цикла
	CV_SWAP(prev_points, points, swap_points);
	return r;
};

/*
***
	Находит особые точки на заданном участке изображения
***
*/
void findKeyPoints(CvRect r)
{
	IplImage* eig = cvCreateImage(cvGetSize(grey), 32, 1);
	IplImage* temp = cvCreateImage(cvGetSize(grey), 32, 1);
	count = MAX_COUNT;
	flags = 0;
	//Находим точки за которыми будем следить
	cvSetImageROI(grey, r);
	cvGoodFeaturesToTrack(grey, eig, temp, points, &count, QUALITY,
	FEATURE_DISTANCE, 0, BLOCK_SIZE, HARRIS, HARRIS_K);
	//уточняет местоположение углов
	cvFindCornerSubPix(	grey, points, count,
				cvSize(WIN_SIZE, WIN_SIZE), cvSize(-1, -1),
				cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20,0.03));
	
	for(int i = 0; i < count; i++)
	{
		points[i].x += r.x;
		points[i].y += r.y;
	}
	//удаляет ненужные изображения
	cvResetImageROI(grey);
	cvReleaseImage(&eig);
	cvReleaseImage(&temp);
}



