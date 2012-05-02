#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <ctype.h>

#include "cv.h"
#include "highgui.h"

#include "TextDetection.h"

IplImage *image = 0, *grey = 0, *prev_grey = 0, *pyramid = 0, *prev_pyramid = 0,
		*swap_temp;
int win_size = 10;
const int MAX_COUNT = 500;
const int SCALE = 10;
CvPoint2D32f* points[2] = { 0, 0 }, *swap_points;
char* status = 0;
int count = 0;
int need_to_init = 0;
int night_mode = 0;
int flags = 0;

CvPoint pt1 = cvPoint(0, 0);
CvPoint pt2 = cvPoint(0, 0);
CvRect rec;

void setRect()
{
	int w = std::abs(pt1.x - pt2.x);
	int h = std::abs(pt1.y - pt2.y);
	int x = std::min(pt1.x, pt2.x);
	int y = std::min(pt1.y, pt2.y);
	rec = cvRect(x, y, w, h);
}

//обработка событий с мыши
void on_mouse(int event, int x, int y, int flags, void* param) {
	if (!image)
		return;
	if (image -> origin)
		y = image -> height - y;
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
int main(int argc, char** argv) {
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
//Печатается приветствие и краткий хелп по программе
	printf("Welcome to lkdemo, using OpenCV version %s (%d.%d.%d)\n",
			CV_VERSION, CV_MAJOR_VERSION, CV_MINOR_VERSION,
			CV_SUBMINOR_VERSION);
	printf("Hot keys: \n"
			"\tESC - quit the program\n"
			"\tr - auto-initialize tracking\n"
			"\tc - delete all the points\n"
			"\tn - switch the \"night\" mode on/off\n"
			"To add/remove a feature point click it\n");
	cvNamedWindow("LkDemo", 0);
	//Устанавливаем функцию обработчик мыши, сама функция находится выше
	cvSetMouseCallback("LkDemo", on_mouse, 0);
	//Главный рабочий цикл программы - работает бесконечно, пока вы
	/*принудительно
	 его
	 не
	 закроете, например
	 нажав
	 ESCAPE*/
	for (;;) {
		float minX = 100500.0f;
		float minY = 100500.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;
		
		IplImage* frame = 0;
		int i, k, c;
		//Получаем фрейм из видеопотока
		frame = cvQueryFrame(capture);
		if (!frame)
			break;
		if (!image) {
			/*Если изображение image не создано, то надо распределить все буферы.
			 Изображение не создано первоначально,
			 поэтому это всего навсего инициализация.
			 */
			image = cvCreateImage(cvGetSize(frame), 8, 3);
			image -> origin = frame -> origin;
			grey = cvCreateImage(cvGetSize(frame), 8, 1);
			prev_grey = cvCreateImage(cvGetSize(frame), 8, 1);
			pyramid = cvCreateImage(cvGetSize(frame), 8, 1);
			prev_pyramid = cvCreateImage(cvGetSize(frame), 8, 1);
			//Выделяем массивы под точки, судя по MAX_COUNT максимально у нас
			//можетбыть500точек
			points[0] = (CvPoint2D32f*) cvAlloc(
					MAX_COUNT * sizeof(points[0][0]));
			points[1] = (CvPoint2D32f*) cvAlloc(
					MAX_COUNT * sizeof(points[0][0]));
			status = (char*) cvAlloc(MAX_COUNT);
			flags = 0;
		}
		//копирование и перевод в чёрно-белое изображение
		cvCopy(frame, image, 0);
		cvCvtColor(image, grey, CV_BGR2GRAY);
		//Это для эффекта, когда вы нажимаете n изображение становится тёмным и
		/*вы
		 видите
		 только
		 перемещающиеся*/
		//назначенные вами точки
		if (night_mode)
			cvZero(image);
		if (need_to_init) {
			//Нажимаем r и запускаем детектор
			IplImage* eig = cvCreateImage(cvGetSize(grey), 32, 1);
			IplImage* temp = cvCreateImage(cvGetSize(grey), 32, 1);
			double quality = 0.01;
			double min_distance = 10;
			count = MAX_COUNT;
			
			//Находим текст детектором
			CvRect* rects;
			int len = getComp(image, &rects);
			CvRect r = getRegion(rects, len);
			
			//Находим точки за которыми будем следить
			cvSetImageROI(grey, r);
			cvGoodFeaturesToTrack(grey, eig, temp, points[1], &count, quality,
					min_distance, 0, 3, 0, 0.04);
			//уточняет местоположение углов
			cvFindCornerSubPix(grey, points[1], count,
					cvSize(win_size, win_size), cvSize(-1, -1),
					cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20,
							0.03));
			for(int i = 0; i < count; i++)
			{
				points[1][i].x += r.x;
				points[1][i].y += r.y;
			}
			//удаляет ненужные изображения
			cvResetImageROI(grey);
			cvReleaseImage(&eig);
			cvReleaseImage(&temp);
		} else if (count > 4) {
			//Если есть хотя бы одна точка на экране, то за ней надо следить
			//Вычисляем оптический поток на факт поиска особенностей
			//points[0] - предыдущие точки points[1] - текущие точки
			cvCalcOpticalFlowPyrLK(prev_grey, grey, prev_pyramid, pyramid,
					points[0], points[1], count, cvSize(win_size, win_size), 3,
					status, 0,
					cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20,
							0.03), flags);
			flags |= CV_LKFLOW_PYR_A_READY;
			//перебираем все точки
			for (i = k = 0; i < count; i++) {
				if (!status[i])
					continue;
				points[1][k++] = points[1][i];
				if(points[1][i].x > maxX)
				{
					maxX = points[1][i].x;
				}
				if(points[1][i].x < minX)
				{
					minX = points[1][i].x;
				}
				if(points[1][i].y > maxY)
				{
					maxY = points[1][i].y;
				}
				if(points[1][i].y < minY)
				{
					minY = points[1][i].y;
				}				
				//Отображаем точку на экране
				cvCircle(image, cvPointFrom32f(points[1][i]), 3,
						CV_RGB(0,255,0), -1, 8, 0);
			}
			count = k;
			
			float num = count;
			float diffX = (maxX - minX) / SCALE; 
			float diffY = (maxY - minY) / SCALE;
			float curX1 = minX;
			float curX2 = maxX;
			float curY1 = minY;
			float curY2 = maxY;
			while(1)
			{
				num = 0;
				for(i = 0; i < count; i++)
				{
					float p_x = points[1][i].x;
					float p_y = points[1][i].y;
					if(p_x > curX1 && p_x < curX2 && p_y > curY1 && p_y < curY2)
					{
						num++;
					} 
				}
				if (num < count * 0.7f)
				{
					break;
				}
				curX1 += diffX;
				curX2 -= diffX;
				curY1 += diffY;
				curY2 -= diffY;
			}
			cvRectangle(image, cvPoint(static_cast<int>(std::floor(curX1)), static_cast<int>(std::floor(curY1))),
					   cvPoint(static_cast<int>(std::floor(curX2)), static_cast<int>(std::floor(curY2))), CV_RGB(125,200,100), 1, 8, 0);
		}

		/*
		if (add_remove_pt && count < MAX_COUNT) {
			//Если точка прошла процедуру проверки (см. выше), то добавляем её в	буфер
			points[1][count++] = cvPointTo32f(pt);
			//уточняет местоположение углов для всех точек
			cvFindCornerSubPix(grey, points[1] + count - 1, 1,
					cvSize(win_size, win_size), cvSize(-1, -1),
					cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20,
							0.03));
			add_remove_pt = 0;
		}
		*/
		CV_SWAP( prev_grey, grey, swap_temp);
		CV_SWAP( prev_pyramid, pyramid, swap_temp);
		//Меняем местами точки - новые становятся старыми на следующем шаге цикла
		CV_SWAP( points[0], points[1], swap_points);
		need_to_init = 0;
		cvShowImage("LkDemo", image);
		c = cvWaitKey(100);
		if ((char) c == 27)
			break;
		switch ((char) c) {
		case 'r':
			need_to_init = 1;
			break;
		case 'c':
			count = 0;
			break;
		case 'n':
			night_mode ^= 1;
			break;
		default:
			;
		}
	}
	cvReleaseCapture(&capture);
	cvDestroyWindow("LkDemo");
	return 0;
}

